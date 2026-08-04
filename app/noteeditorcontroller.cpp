#include "noteeditorcontroller.h"
#include "../core/models/document.h"
#include "../core/models/blockmodel.h"
#include "../core/models/blockcommands.h"
#include "../core/storage/notesdatabase.h"
#include <QDebug>
#include <QJsonDocument>
#include <QJsonArray>

NoteEditorController::NoteEditorController(QObject* parent)
    : QObject(parent)
    , m_db(NotesDatabase::instance())
{
}

NoteEditorController::~NoteEditorController()
{
    delete m_document;
}

QAbstractItemModel* NoteEditorController::model() const
{
    if (!m_document) return nullptr;
    return m_document->model();
}

QString NoteEditorController::noteTitle() const
{
    if (!m_document) return QString();
    return m_document->title();
}

void NoteEditorController::setNoteTitle(const QString& title)
{
    if (!m_document) return;
    if (m_document->title() == title) return;
    m_document->setTitle(title);
    emit noteTitleChanged(title);
}

QUuid NoteEditorController::documentId() const
{
    if (!m_document) return QUuid();
    return m_document->id();
}

bool NoteEditorController::canUndo() const
{
    return m_document && m_document->undoStack() && m_document->undoStack()->canUndo();
}

bool NoteEditorController::canRedo() const
{
    return m_document && m_document->undoStack() && m_document->undoStack()->canRedo();
}

QString NoteEditorController::pendingFocusId() const { return m_pendingFocusId; }
QString NoteEditorController::focusedBlockId() const { return m_focusedBlockId; }

void NoteEditorController::createNewNote(const QString& title)
{
    if (m_document) {
        saveNote();
        delete m_document;
        m_document = nullptr;
    }


    QUuid docId = QUuid::createUuid();
    m_document = new Document(docId, title.isEmpty() ? "Untitled" : title, this);
    connect(m_document, &Document::documentModified,
            this, &NoteEditorController::documentModified);

    BlockData data = ParagraphData{""};
    m_document->insertBlock(QUuid(), 0, data);

    if (!m_db->saveDocument(*m_document)) {
        emit errorOccurred("Failed to save new note");
    }

    emit documentChanged();
    emit noteTitleChanged(m_document->title());
    emit modelChanged();
}

void NoteEditorController::loadNote(const QString& id)
{
    QUuid docId = QUuid::fromString(id);
    if (docId.isNull()) return;

    if (m_document) {
        saveNote();
        delete m_document;
        m_document = nullptr;
    }

    m_document = m_db->loadDocument(docId);
    if (!m_document) {
        emit errorOccurred("Failed to load note");
        return;
    }
    connect(m_document, &Document::documentModified,
            this, &NoteEditorController::documentModified);
    emit documentChanged();
    emit noteTitleChanged(m_document->title());
    emit modelChanged();
}

void NoteEditorController::saveNote()
{
    if (!m_document) return;
    if (!m_db->saveDocument(*m_document)) {
        emit errorOccurred("Failed to save note");
    }
}

void NoteEditorController::deleteNote()
{
    if (!m_document) return;
    if (!m_db->deleteDocument(m_document->id())) {
        emit errorOccurred("Failed to delete note");
        return;
    }
    delete m_document;
    m_document = nullptr;
    emit documentChanged();
    emit noteTitleChanged(QString());
}

void NoteEditorController::undo()
{
    if (m_document && m_document->undoStack()) {
        m_document->undoStack()->undo();
    }
}

void NoteEditorController::redo()
{
    if (m_document && m_document->undoStack()) {
        m_document->undoStack()->redo();
    }
}

// Helper to set pending focus and emit
void NoteEditorController::setFocusedBlock(const QString& blockId)
{
    if (m_focusedBlockId != blockId) {
        m_focusedBlockId = blockId;
        emit focusedBlockIdChanged(m_focusedBlockId);
    }
}

// type: 0=Paragraph, 1=H1, 2=H2, 3=H3, 4=Todo, 5=Code, 6=Image, 7=Table, 8=Divider, 9=Quote
void NoteEditorController::insertBlock(const QString& parentId, int row, int type, const QString& content)
{
    if (!m_document) {
        emit errorOccurred("No note open");
        return;
    }

    QUuid parentUuid = parentId.isEmpty() ? QUuid() : QUuid::fromString(parentId);
    BlockData data;

    switch (type) {
    case 0: data = ParagraphData{content}; break;
    case 1: data = HeadingData{1, content}; break;
    case 2: data = HeadingData{2, content}; break;
    case 3: data = HeadingData{3, content}; break;
    case 4: data = TodoData{content, false}; break;
    case 5: data = CodeData{"", content}; break;
    case 6: data = ImageData{content, "", 0, 0}; break;
    case 7: {
        QVector<QVector<QString>> initCells(3, QVector<QString>(1, ""));
        data = TableData{1, 1, initCells};
        break;
    }
    case 8: data = DividerData{}; break;
    case 9: data = QuoteData{content}; break;
    default:
        emit errorOccurred("Unsupported block type");
        return;
    }

    QUuid newId = QUuid::createUuid();
    auto* cmd = new InsertBlockCommand(m_document, parentUuid, row, data, newId);
    m_document->undoStack()->push(cmd);

    // Notify QML to focus the new block
    m_pendingFocusId = newId.toString(QUuid::WithoutBraces);
    emit pendingFocusIdChanged(m_pendingFocusId);
    emit documentModified();
}

void NoteEditorController::insertBlockAfter(const QString& blockId, int type, const QString& content)
{
    if (!m_document) return;
    QUuid id = QUuid::fromString(blockId);
    int idx = m_document->findBlockIndex(id);
    if (idx < 0) return;
    insertBlock("", idx + 1, type, content);
}

void NoteEditorController::deleteBlock(const QString& blockId)
{
    if (!m_document) return;
    QUuid id = QUuid::fromString(blockId);
    if (id.isNull()) return;

    int idx = m_document->findBlockIndex(id);
    if (idx < 0) return;

    // Decide which block should receive focus after deletion
    QString focusId;
    int total = m_document->blocks().size();
    if (total > 1) {
        // Prefer previous block; if deleting the first, focus the next one
        int focusIdx = (idx > 0) ? idx - 1 : 1;
        Block* focusBlock = m_document->blockAt(focusIdx);
        if (focusBlock)
            focusId = focusBlock->id().toString(QUuid::WithoutBraces);
    }

    auto* cmd = new DeleteBlockCommand(m_document, id);
    m_document->undoStack()->push(cmd);

    // If document became empty, create a fresh empty paragraph
    if (m_document->blocks().isEmpty()) {
        insertBlock("", 0, 0, "");
        return; // insertBlock already sets pendingFocusId
    }

    if (!focusId.isEmpty()) {
        m_pendingFocusId = focusId;
        emit pendingFocusIdChanged(m_pendingFocusId);
    }
    emit documentModified();
}

void NoteEditorController::updateBlockContent(const QString& blockId, const QString& content)
{
    if (!m_document) return;
    QUuid id = QUuid::fromString(blockId);
    if (id.isNull()) return;

    auto* cmd = new EditTextCommand(m_document, id, content);
    m_document->undoStack()->push(cmd);
    emit documentModified();
}

void NoteEditorController::updateBlockCodeLanguage(const QString& blockId, const QString& language)
{
    if (!m_document) return;
    QUuid id = QUuid::fromString(blockId);
    if (id.isNull()) return;

    Block* block = m_document->findBlock(id);
    if (!block) return;

    // Only works on CodeData blocks
    std::visit([&](auto&& arg) {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, CodeData>) {
            CodeData updated = arg;
            updated.language = language;
            m_document->updateBlockData(id, updated);
        }
    }, block->data());
    emit documentModified();
}

void NoteEditorController::toggleBlockChecked(const QString& blockId)
{
    if (!m_document) return;
    QUuid id = QUuid::fromString(blockId);
    Block* block = m_document->findBlock(id);
    if (!block) return;

    BlockData newData = std::visit([](auto&& arg) -> BlockData {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, TodoData>) {
            return TodoData{arg.text, !arg.checked};
        }
        return arg;
    }, block->data());

    m_document->updateBlockData(id, newData);
    emit documentModified();
}

void NoteEditorController::mergeWithPrevious(const QString& blockId)
{
    if (!m_document) return;
    QUuid id = QUuid::fromString(blockId);
    int idx = m_document->findBlockIndex(id);
    if (idx <= 0) {
        Block* block = m_document->findBlock(id);
        if (block) {
            bool isEmpty = std::visit([](auto&& arg) -> bool {
                using T = std::decay_t<decltype(arg)>;
                if constexpr (std::is_same_v<T, ParagraphData>) return arg.text.isEmpty();
                else if constexpr (std::is_same_v<T, HeadingData>) return arg.text.isEmpty();
                else if constexpr (std::is_same_v<T, TodoData>) return arg.text.isEmpty();
                else if constexpr (std::is_same_v<T, QuoteData>) return arg.text.isEmpty();
                else if constexpr (std::is_same_v<T, CodeData>) return arg.code.isEmpty();
                else return true;
            }, block->data());
            if (isEmpty) deleteBlock(blockId);
        }
        return;
    }

    Block* current = m_document->findBlock(id);
    Block* prev = m_document->blockAt(idx - 1);
    if (!current || !prev) return;

    auto extractText = [](const BlockData& d) -> QString {
        return std::visit([](auto&& arg) -> QString {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, ParagraphData>) return arg.text;
            else if constexpr (std::is_same_v<T, HeadingData>) return arg.text;
            else if constexpr (std::is_same_v<T, TodoData>) return arg.text;
            else if constexpr (std::is_same_v<T, QuoteData>) return arg.text;
            else if constexpr (std::is_same_v<T, CodeData>) return arg.code;
            else return QString();
        }, d);
    };

    QString currentText = extractText(current->data());
    QString prevText = extractText(prev->data());

    m_document->undoStack()->beginMacro("Merge block");

    auto* editCmd = new EditTextCommand(m_document, prev->id(), prevText + currentText);
    m_document->undoStack()->push(editCmd);

    auto* delCmd = new DeleteBlockCommand(m_document, id);
    m_document->undoStack()->push(delCmd);

    m_document->undoStack()->endMacro();
    emit documentModified();
}

void NoteEditorController::changeBlockType(const QString& blockId, int newType)
{
    if (!m_document) return;
    QUuid id = QUuid::fromString(blockId);
    Block* block = m_document->findBlock(id);
    if (!block) return;

    QString currentText;
    bool isChecked = false;
    std::visit([&](auto&& arg) {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, ParagraphData>) currentText = arg.text;
        else if constexpr (std::is_same_v<T, HeadingData>) currentText = arg.text;
        else if constexpr (std::is_same_v<T, TodoData>) { currentText = arg.text; isChecked = arg.checked; }
        else if constexpr (std::is_same_v<T, QuoteData>) currentText = arg.text;
        else if constexpr (std::is_same_v<T, CodeData>) currentText = arg.code;
    }, block->data());

    BlockData newData;
    switch (newType) {
    case 0: newData = ParagraphData{currentText}; break;
    case 1: newData = HeadingData{1, currentText}; break;
    case 2: newData = HeadingData{2, currentText}; break;
    case 3: newData = HeadingData{3, currentText}; break;
    case 4: newData = TodoData{currentText, isChecked}; break;
    case 5: newData = CodeData{"", currentText}; break;
    case 6: newData = ImageData{"", "", 0, 0}; break;          // Image
    case 7: {                                                     // Table 1×1
        QVector<QVector<QString>> initCells(1, QVector<QString>(1, ""));
        newData = TableData{1, 1, initCells};
        break;
    }
    case 8: newData = DividerData{}; break;
    case 9: newData = QuoteData{currentText}; break;
    default: return;
    }

    m_document->updateBlockData(id, newData);
    m_pendingFocusId = blockId;
    emit pendingFocusIdChanged(m_pendingFocusId);
    emit documentModified();
}

QVariantList NoteEditorController::getDocuments() const
{
    QVariantList list;
    if (!m_db) return list;
    QList<QUuid> ids = m_db->allDocumentIds();
    for (const QUuid& id : ids) {
        Document* doc = m_db->loadDocument(id);
        if (doc) {
            QVariantMap map;
            map["id"] = doc->id().toString(QUuid::WithoutBraces);
            map["title"] = doc->title();
            list.append(map);
            delete doc;
        }
    }
    return list;
}

void NoteEditorController::loadFromContent(const QString& title, const QStringList& paragraphs)
{
    if (m_document) {
        delete m_document;
        m_document = nullptr;
    }


    QUuid docId = QUuid::createUuid();
    m_document = new Document(docId, title, this);
    connect(m_document, &Document::documentModified,
            this, &NoteEditorController::documentModified);

    int row = 0;
    if (paragraphs.isEmpty()) {
        BlockData data = ParagraphData{""};
        m_document->insertBlock(QUuid(), row++, data);
    } else {
        for (const QString& para : paragraphs) {
            BlockData data = ParagraphData{para};
            m_document->insertBlock(QUuid(), row++, data);
        }
    }

    emit documentChanged();
    emit noteTitleChanged(title);
    emit modelChanged();
}

void NoteEditorController::updateBlockImageSource(const QString& blockId, const QString& source)
{
    if (!m_document) return;
    QUuid id = QUuid::fromString(blockId);
    if (id.isNull()) return;

    Block* block = m_document->findBlock(id);
    if (!block) return;

    std::visit([&](auto&& arg) {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, ImageData>) {
            ImageData updated = arg;
            updated.source = source;
            m_document->updateBlockData(id, updated);
        }
    }, block->data());
    emit documentModified();
}

void NoteEditorController::updateBlockTableData(const QString& blockId, const QVariantList& cells)
{
    if (!m_document) return;
    QUuid id = QUuid::fromString(blockId);
    if (id.isNull()) return;

    Block* block = m_document->findBlock(id);
    if (!block) return;

    std::visit([&](auto&& arg) {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, TableData>) {
            TableData updated = arg;
            int rows = cells.size();
            int cols = 0;
            updated.cells.clear();
            updated.cells.resize(rows);
            for (int r = 0; r < rows; ++r) {
                QVariantList rowList = cells[r].toList();
                cols = std::max(cols, (int)rowList.size());
                updated.cells[r].resize(rowList.size());
                for (int c = 0; c < rowList.size(); ++c) {
                    updated.cells[r][c] = rowList[c].toString();
                }
            }
            updated.rows = rows;
            updated.cols = cols;
            m_document->updateBlockData(id, updated);
        }
    }, block->data());
    emit documentModified();
}

void NoteEditorController::updateBlockDividerOrientation(const QString& blockId, int orientation)
{
    if (!m_document) return;
    QUuid id = QUuid::fromString(blockId);
    if (id.isNull()) return;

    Block* block = m_document->findBlock(id);
    if (!block) return;

    std::visit([&](auto&& arg) {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, DividerData>) {
            DividerData updated = arg;
            updated.orientation = orientation;
            m_document->updateBlockData(id, updated);
        }
    }, block->data());
    emit documentModified();
}

QString NoteEditorController::documentToJson() const
{
    if (!m_document) return "{}";
    QJsonObject obj = m_document->toJson();
    return QString::fromUtf8(QJsonDocument(obj).toJson(QJsonDocument::Compact));
}

void NoteEditorController::loadFromJson(const QString& title, const QString& jsonContent)
{
    if (m_document) {
        delete m_document;
        m_document = nullptr;
    }


    // Try to parse as our JSON block format
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(jsonContent.toUtf8(), &err);

    if (err.error == QJsonParseError::NoError && doc.isObject()) {
        QJsonObject obj = doc.object();
        // Check if it has a "blocks" array (our Document format)
        if (obj.contains("blocks") && obj["blocks"].isArray()) {
            m_document = Document::fromJson(obj, nullptr);
            if (m_document) {
                m_document->setParent(this);
                connect(m_document, &Document::documentModified,
                        this, &NoteEditorController::documentModified);
                emit documentChanged();
                emit noteTitleChanged(m_document->title());
                emit modelChanged();
                return;
            }
        }
    }

    // Fallback: plain text paragraphs
    QStringList paragraphs = jsonContent.split('\n');
    loadFromContent(title, paragraphs);
}
