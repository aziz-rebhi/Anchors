#include "noteeditorcontroller.h"
#include "../core/models/document.h"
#include "../core/models/blockmodel.h"
#include "../core/models/blockcommands.h"
#include "../core/storage/notesdatabase.h"

#include <QDebug>
#include <QJsonDocument>
#include <QJsonArray>
#include <QUndoStack>

NoteEditorController::NoteEditorController(QObject* parent)
    : QObject(parent)
    , m_db(NotesDatabase::instance())
{
}

NoteEditorController::~NoteEditorController()
{
    delete m_document;
}

void NoteEditorController::bindDocumentSignals()
{
    if (!m_document) return;

    connect(m_document, &Document::documentModified,
            this, &NoteEditorController::documentModified);

    if (m_document->undoStack()) {
        connect(m_document->undoStack(), &QUndoStack::canUndoChanged,
                this, &NoteEditorController::canUndoChanged);
        connect(m_document->undoStack(), &QUndoStack::canRedoChanged,
                this, &NoteEditorController::canRedoChanged);
    }
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
    emit documentModified();
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
    m_document = new Document(docId, title.isEmpty() ? QStringLiteral("Untitled") : title, this);
    bindDocumentSignals();

    BlockData data = ParagraphData{""};
    m_document->insertBlock(QUuid(), 0, data);

    if (m_db && !m_db->saveDocument(*m_document))
        emit errorOccurred(QStringLiteral("Failed to save new note"));

    emit documentChanged();
    emit noteTitleChanged(m_document->title());
    emit modelChanged();
    emit canUndoChanged();
    emit canRedoChanged();
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

    m_document = m_db ? m_db->loadDocument(docId) : nullptr;
    if (!m_document) {
        emit errorOccurred(QStringLiteral("Failed to load note"));
        return;
    }
    m_document->setParent(this);
    bindDocumentSignals();

    emit documentChanged();
    emit noteTitleChanged(m_document->title());
    emit modelChanged();
    emit canUndoChanged();
    emit canRedoChanged();
}

void NoteEditorController::saveNote()
{
    if (!m_document || !m_db) return;
    if (!m_db->saveDocument(*m_document))
        emit errorOccurred(QStringLiteral("Failed to save note"));
}

void NoteEditorController::deleteNote()
{
    if (!m_document || !m_db) return;
    if (!m_db->deleteDocument(m_document->id())) {
        emit errorOccurred(QStringLiteral("Failed to delete note"));
        return;
    }
    delete m_document;
    m_document = nullptr;
    emit documentChanged();
    emit noteTitleChanged(QString());
    emit modelChanged();
}

void NoteEditorController::undo()
{
    if (m_document && m_document->undoStack()) {
        m_document->undoStack()->undo();
        emit canUndoChanged();
        emit canRedoChanged();
        emit documentModified();
    }
}

void NoteEditorController::redo()
{
    if (m_document && m_document->undoStack()) {
        m_document->undoStack()->redo();
        emit canUndoChanged();
        emit canRedoChanged();
        emit documentModified();
    }
}

void NoteEditorController::setFocusedBlock(const QString& blockId)
{
    if (m_focusedBlockId != blockId) {
        m_focusedBlockId = blockId;
        emit focusedBlockIdChanged(m_focusedBlockId);
    }
}

void NoteEditorController::insertBlock(const QString& parentId, int row, int type, const QString& content)
{
    if (!m_document) {
        emit errorOccurred(QStringLiteral("No note open"));
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
        QVector<QVector<QString>> initCells(1, QVector<QString>(1, ""));
        data = TableData{1, 1, initCells};
        break;
    }
    case 8: data = DividerData{}; break;
    case 9: data = QuoteData{content}; break;
    case 10: data = HeadingData{4, content}; break;
    case 11: data = BulletData{content}; break;
    case 12: data = CalloutData{content, QStringLiteral("💡")}; break;
    case 13: data = NumberedData{content}; break;
    case 14: data = EquationData{content.isEmpty() ? QStringLiteral("E = mc^2") : content, true}; break;
    case 15: data = ToggleData{content, false}; break;
    default:
        emit errorOccurred(QStringLiteral("Unsupported block type"));
        return;
    }

    QUuid newId = QUuid::createUuid();
    auto* cmd = new InsertBlockCommand(m_document, parentUuid, row, data, newId);
    m_document->undoStack()->push(cmd);

    m_pendingFocusId = newId.toString(QUuid::WithoutBraces);
    emit pendingFocusIdChanged(m_pendingFocusId);
    emit documentModified();
    emit canUndoChanged();
    emit canRedoChanged();
}

void NoteEditorController::insertBlockAfter(const QString& blockId, int type, const QString& content)
{
    if (!m_document) return;
    QUuid id = QUuid::fromString(blockId);
    Block* block = m_document->findBlock(id);
    if (!block) return;

    const QUuid parentId = block->parentId();

    if (!parentId.isNull()) {
        // Sibling under same parent: child index = current child order + 1
        int childIndex = 0;
        int seen = 0;
        for (const Block& b : m_document->blocks()) {
            if (b.parentId() != parentId) continue;
            if (b.id() == id) {
                childIndex = seen + 1;
                break;
            }
            ++seen;
        }
        insertBlock(parentId.toString(QUuid::WithoutBraces), childIndex, type, content);
        return;
    }

    int idx = m_document->findBlockIndex(id);
    if (idx < 0) return;
    insertBlock(QString(), idx + 1, type, content);
}

void NoteEditorController::deleteBlock(const QString& blockId)
{
    if (!m_document) return;
    QUuid id = QUuid::fromString(blockId);
    if (id.isNull()) return;

    int idx = m_document->findBlockIndex(id);
    if (idx < 0) return;

    QString focusId;
    const int total = m_document->blocks().size();
    if (total > 1) {
        const int focusIdx = (idx > 0) ? idx - 1 : 1;
        if (Block* focusBlock = m_document->blockAt(focusIdx))
            focusId = focusBlock->id().toString(QUuid::WithoutBraces);
    }

    auto* cmd = new DeleteBlockCommand(m_document, id);
    m_document->undoStack()->push(cmd);

    if (m_document->blocks().isEmpty()) {
        insertBlock(QString(), 0, 0, QString());
        return;
    }

    if (!focusId.isEmpty()) {
        m_pendingFocusId = focusId;
        emit pendingFocusIdChanged(m_pendingFocusId);
    }
    emit documentModified();
    emit canUndoChanged();
    emit canRedoChanged();
}

void NoteEditorController::updateBlockContent(const QString& blockId, const QString& content)
{
    if (!m_document) return;
    QUuid id = QUuid::fromString(blockId);
    if (id.isNull()) return;

    auto* cmd = new EditTextCommand(m_document, id, content);
    m_document->undoStack()->push(cmd);
    emit documentModified();
    emit canUndoChanged();
    emit canRedoChanged();
}

void NoteEditorController::updateBlockCodeLanguage(const QString& blockId, const QString& language)
{
    if (!m_document) return;
    QUuid id = QUuid::fromString(blockId);
    if (id.isNull()) return;
    Block* block = m_document->findBlock(id);
    if (!block) return;

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
        if constexpr (std::is_same_v<T, TodoData>)
            return TodoData{arg.text, !arg.checked};
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
            const bool isEmpty = std::visit([](auto&& arg) -> bool {
                using T = std::decay_t<decltype(arg)>;
                if constexpr (std::is_same_v<T, ParagraphData>) return arg.text.isEmpty();
                else if constexpr (std::is_same_v<T, HeadingData>) return arg.text.isEmpty();
                else if constexpr (std::is_same_v<T, TodoData>) return arg.text.isEmpty();
                else if constexpr (std::is_same_v<T, QuoteData>) return arg.text.isEmpty();
                else if constexpr (std::is_same_v<T, CodeData>) return arg.code.isEmpty();
                else if constexpr (std::is_same_v<T, BulletData>) return arg.text.isEmpty();
                else if constexpr (std::is_same_v<T, CalloutData>) return arg.text.isEmpty();
                else if constexpr (std::is_same_v<T, NumberedData>) return arg.text.isEmpty();
                else if constexpr (std::is_same_v<T, ToggleData>) return arg.text.isEmpty();
                else if constexpr (std::is_same_v<T, EquationData>) return arg.latex.isEmpty();
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
            else if constexpr (std::is_same_v<T, BulletData>) return arg.text;
            else if constexpr (std::is_same_v<T, CalloutData>) return arg.text;
            else if constexpr (std::is_same_v<T, NumberedData>) return arg.text;
            else if constexpr (std::is_same_v<T, ToggleData>) return arg.text;
            else if constexpr (std::is_same_v<T, EquationData>) return arg.latex;
            else return QString();
        }, d);
    };

    const QString currentText = extractText(current->data());
    const QString prevText = extractText(prev->data());

    m_document->undoStack()->beginMacro(QStringLiteral("Merge block"));
    m_document->undoStack()->push(new EditTextCommand(m_document, prev->id(), prevText + currentText));
    m_document->undoStack()->push(new DeleteBlockCommand(m_document, id));
    m_document->undoStack()->endMacro();

    emit documentModified();
    emit canUndoChanged();
    emit canRedoChanged();
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
    case 6: newData = ImageData{"", "", 0, 0}; break;
    case 7: {
        QVector<QVector<QString>> initCells(1, QVector<QString>(1, ""));
        newData = TableData{1, 1, initCells};
        break;
    }
    case 8: newData = DividerData{}; break;
    case 9: newData = QuoteData{currentText}; break;
    case 10: newData = HeadingData{4, currentText}; break;
    case 11: newData = BulletData{currentText}; break;
    case 12: newData = CalloutData{currentText, QStringLiteral("💡")}; break;
    case 13: newData = NumberedData{currentText}; break;
    case 14: newData = EquationData{currentText.isEmpty() ? QStringLiteral("E = mc^2") : currentText, true}; break;
    case 15: newData = ToggleData{currentText, false}; break;
    default: return;
    }

    m_document->updateBlockData(id, newData);
    m_pendingFocusId = blockId;
    emit pendingFocusIdChanged(m_pendingFocusId);
    emit documentModified();
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

void NoteEditorController::updateBlockImageCaption(const QString& blockId, const QString& caption)
{
    if (!m_document) return;
    QUuid id = QUuid::fromString(blockId);
    Block* block = m_document->findBlock(id);
    if (!block) return;

    std::visit([&](auto&& arg) {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, ImageData>) {
            ImageData updated = arg;
            updated.caption = caption;
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
            const int rows = cells.size();
            int cols = 0;
            updated.cells.clear();
            updated.cells.resize(rows);
            for (int r = 0; r < rows; ++r) {
                const QVariantList rowList = cells[r].toList();
                cols = std::max(cols, static_cast<int>(rowList.size()));
                updated.cells[r].resize(rowList.size());
                for (int c = 0; c < rowList.size(); ++c)
                    updated.cells[r][c] = rowList[c].toString();
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

void NoteEditorController::moveBlock(const QString& blockId, int newRow)
{
    if (!m_document) return;
    QUuid id = QUuid::fromString(blockId);
    if (id.isNull() || newRow < 0) return;
    m_document->moveBlock(id, QUuid(), newRow);
    emit documentModified();
}

QVariantList NoteEditorController::getDocuments() const
{
    QVariantList list;
    if (!m_db) return list;
    const QList<QUuid> ids = m_db->allDocumentIds();
    for (const QUuid& id : ids) {
        Document* doc = m_db->loadDocument(id);
        if (doc) {
            QVariantMap map;
            map[QStringLiteral("id")] = doc->id().toString(QUuid::WithoutBraces);
            map[QStringLiteral("title")] = doc->title();
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

    m_document = new Document(QUuid::createUuid(), title, this);
    bindDocumentSignals();

    int row = 0;
    if (paragraphs.isEmpty()) {
        m_document->insertBlock(QUuid(), row++, ParagraphData{""});
    } else {
        for (const QString& para : paragraphs)
            m_document->insertBlock(QUuid(), row++, ParagraphData{para});
    }

    emit documentChanged();
    emit noteTitleChanged(title);
    emit modelChanged();
    emit canUndoChanged();
    emit canRedoChanged();
}

QString NoteEditorController::documentToJson() const
{
    if (!m_document) return QStringLiteral("{}");
    return QString::fromUtf8(QJsonDocument(m_document->toJson()).toJson(QJsonDocument::Compact));
}

void NoteEditorController::loadFromJson(const QString& title, const QString& jsonContent)
{
    if (m_document) {
        delete m_document;
        m_document = nullptr;
    }

    if (jsonContent.trimmed().isEmpty() || jsonContent.trimmed() == QStringLiteral("{}")){
        emit documentChanged();
        emit noteTitleChanged(QString());
        emit modelChanged();
        return;
    }

    QJsonParseError err;
    const QJsonDocument doc = QJsonDocument::fromJson(jsonContent.toUtf8(), &err);
    if (err.error == QJsonParseError::NoError && doc.isObject()) {
        const QJsonObject obj = doc.object();
        if (obj.contains(QLatin1String("blocks")) && obj.value(QLatin1String("blocks")).isArray()) {
            m_document = Document::fromJson(obj, nullptr);
            if (m_document) {
                m_document->setParent(this);
                bindDocumentSignals();
                emit documentChanged();
                emit noteTitleChanged(m_document->title().isEmpty() ? title : m_document->title());
                emit modelChanged();
                emit canUndoChanged();
                emit canRedoChanged();
                return;
            }
        }
    }

    loadFromContent(title, jsonContent.split(QLatin1Char('\n')));
}

void NoteEditorController::insertInside(const QString& parentBlockId, int type, const QString& content)
{
    if (!m_document) return;

    QUuid parentId = QUuid::fromString(parentBlockId);
    if (parentId.isNull()) return;

    Block* parent = m_document->findBlock(parentId);
    if (!parent) return;

    // Expand toggle if collapsed
    std::visit([&](auto&& arg) {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, ToggleData>) {
            if (arg.collapsed) {
                ToggleData u = arg;
                u.collapsed = false;
                m_document->updateBlockData(parentId, u);
            }
        }
    }, parent->data());

    // Count existing children to append at end
    int childCount = 0;
    const auto blocks = m_document->blocks();
    for (const Block& b : blocks) {
        if (b.parentId() == parentId)
            ++childCount;
    }

    insertBlock(parentBlockId, childCount, type, content);
}

void NoteEditorController::toggleCollapsed(const QString& blockId)
{
    if (!m_document) return;
    QUuid id = QUuid::fromString(blockId);
    Block* block = m_document->findBlock(id);
    if (!block) return;

    std::visit([&](auto&& arg) {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, ToggleData>) {
            ToggleData u = arg;
            u.collapsed = !u.collapsed;
            m_document->updateBlockData(id, u);
            // Refresh visible rows (children show/hide)
            if (m_document->model())
                m_document->model()->rebuildMaps();
        }
    }, block->data());

    emit documentModified();
}

int NoteEditorController::numberedIndex(const QString& blockId) const
{
    if (!m_document) return 1;
    QUuid id = QUuid::fromString(blockId);
    int idx = m_document->findBlockIndex(id);
    if (idx < 0) return 1;

    Block* self = m_document->blockAt(idx);
    if (!self) return 1;
    const QUuid parentId = self->parentId();

    int n = 1;
    for (int i = idx - 1; i >= 0; --i) {
        Block* b = m_document->blockAt(i);
        if (!b) break;
        if (b->parentId() != parentId) break;

        const bool isNum = std::visit([](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;
            return std::is_same_v<T, NumberedData>;
        }, b->data());
        if (!isNum) break;
        ++n;
    }
    return n;
}

void NoteEditorController::exitContainer(const QString& blockId, int type, const QString& content)
{
    if (!m_document) return;

    QUuid id = QUuid::fromString(blockId);
    Block* block = m_document -> findBlock(id);
    if (!block) return;

    const QUuid parentId = block -> parentId();
    if (parentId.isNull()){
        insertBlockAfter(blockId, type, content);
        return;
    }

    insertBlockAfter(parentId.toString(QUuid::WithoutBraces), type, content);
}

void NoteEditorController::focusAdjacent(const QString& blockId, bool next)
{
    if (!m_document || !m_document->model())
        return;

    BlockModel* model = m_document->model();
    const QUuid id = QUuid::fromString(blockId);
    const QModelIndex idx = model->indexForId(id);
    if (!idx.isValid())
        return;

    const int row = idx.row() + (next ? 1 : -1);
    if (row < 0 || row >= model->rowCount())
        return;

    Block* b = model->blockAt(row);
    if (!b)
        return;

    m_pendingFocusId = b->id().toString(QUuid::WithoutBraces);
    emit pendingFocusIdChanged(m_pendingFocusId);
}