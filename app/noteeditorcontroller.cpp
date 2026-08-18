#include "noteeditorcontroller.h"
#include "../core/models/document.h"
#include "../core/models/blockmodel.h"
#include "../core/models/blockcommands.h"
#include "../core/storage/notesdatabase.h"

#include <QDebug>
#include <QJsonDocument>
#include <QJsonArray>
#include <QUndoStack>
#include <QGuiApplication>
#include <QClipboard>
#include <QImage>
#include <QMimeData>
#include <QStandardPaths>
#include <QDir>
#include <QDateTime>

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
    case 6: data = ImageData{QString(), QString(), 0, 0}; break;
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
    case 16: data = ColumnData{2}; break;
    default:
        emit errorOccurred(QStringLiteral("Unsupported block type"));
        return;
    }

    QUuid newId = QUuid::createUuid();
    auto* cmd = new InsertBlockCommand(m_document, parentUuid, row, data, newId);
    m_document->undoStack()->push(cmd);

    if (type == 16) {
        const QString colsId = newId.toString(QUuid::WithoutBraces);
        insertInColumn(colsId, 0, 0, "");
        insertInColumn(colsId, 1, 0, "");
    }

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
        else if constexpr (std::is_same_v<T, BulletData>) currentText = arg.text;
        else if constexpr (std::is_same_v<T, CalloutData>) currentText = arg.text;
        else if constexpr (std::is_same_v<T, NumberedData>) currentText = arg.text;
        else if constexpr (std::is_same_v<T, ToggleData>) currentText = arg.text;
        else if constexpr (std::is_same_v<T, EquationData>) currentText = arg.latex;
    }, block->data());

    // Slash / toolbar → Columns
    if (newType == 16) {
        m_document->updateBlockData(id, ColumnData{2});
        insertInColumn(blockId, 0, 0, currentText);
        insertInColumn(blockId, 1, 0, QString());
        if (m_document->model())
            m_document->model()->rebuildMaps();
        m_pendingFocusId = blockId;
        emit pendingFocusIdChanged(m_pendingFocusId);
        emit documentModified();
        return;
    }

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
    emit canUndoChanged();
    emit canRedoChanged();
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

    int myIndent = 0;
    std::visit([&](auto&& arg) {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, NumberedData>) myIndent = arg.indent;
    }, self->data());

    int n = 1;
    for (int i = idx - 1; i >= 0; --i) {
        Block* b = m_document->blockAt(i);
        if (!b) break;
        if (b->parentId() != parentId) break;

        bool stop = true;
        std::visit([&](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, NumberedData>) {
                if (arg.indent == myIndent) { ++n; stop = false; }
                else if (arg.indent > myIndent) { stop = false; } // nested deeper: skip
                else stop = true; // shallower: end of run
            }
        }, b->data());
        if (stop) break;
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

void NoteEditorController::indentBlock(const QString& blockId)
{
    if (!m_document) return;
    QUuid id = QUuid::fromString(blockId);
    Block* block = m_document->findBlock(id);
    if (!block) return;

    std::visit([&](auto&& arg) {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, BulletData>) {
            BulletData u = arg;
            if (u.indent < 8) ++u.indent;
            m_document->updateBlockData(id, u);
        } else if constexpr (std::is_same_v<T, NumberedData>) {
            NumberedData u = arg;
            if (u.indent < 8) ++u.indent;
            m_document->updateBlockData(id, u);
        }
    }, block->data());

    emit documentModified();
}

void NoteEditorController::outdentBlock(const QString& blockId)
{
    if (!m_document) return;
    QUuid id = QUuid::fromString(blockId);
    Block* block = m_document->findBlock(id);
    if (!block) return;

    std::visit([&](auto&& arg) {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, BulletData>) {
            BulletData u = arg;
            if (u.indent > 0) --u.indent;
            m_document->updateBlockData(id, u);
        } else if constexpr (std::is_same_v<T, NumberedData>) {
            NumberedData u = arg;
            if (u.indent > 0) --u.indent;
            m_document->updateBlockData(id, u);
        }
    }, block->data());

    emit documentModified();
}

static int typeCodeOf(const BlockData& data)
{
    return std::visit([](auto&& arg) -> int {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, ParagraphData>) return 0;
        else if constexpr (std::is_same_v<T, HeadingData>) return arg.level >= 4 ? 10 : arg.level;
        else if constexpr (std::is_same_v<T, TodoData>) return 4;
        else if constexpr (std::is_same_v<T, CodeData>) return 5;
        else if constexpr (std::is_same_v<T, ImageData>) return 6;
        else if constexpr (std::is_same_v<T, TableData>) return 7;
        else if constexpr (std::is_same_v<T, DividerData>) return 8;
        else if constexpr (std::is_same_v<T, QuoteData>) return 9;
        else if constexpr (std::is_same_v<T, BulletData>) return 11;
        else if constexpr (std::is_same_v<T, CalloutData>) return 12;
        else if constexpr (std::is_same_v<T, NumberedData>) return 13;
        else if constexpr (std::is_same_v<T, EquationData>) return 14;
        else if constexpr (std::is_same_v<T, ToggleData>) return 15;
        else if constexpr (std::is_same_v<T, ColumnData>) return 16;
        return 0;
    }, data);
}

QVariantList NoteEditorController::columnChildren(const QString& columnsId) const
{
    QVariantList out;
    if (!m_document) return out;

    const QUuid pid = QUuid::fromString(columnsId);
    if (pid.isNull()) return out;

    for (const Block& b : m_document->blocks()) {
        if (b.parentId() != pid) continue;

        QString text;
        bool checked = false;
        std::visit([&](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, ParagraphData>) text = arg.text;
            else if constexpr (std::is_same_v<T, HeadingData>) text = arg.text;
            else if constexpr (std::is_same_v<T, TodoData>) {
                text = arg.text;
                checked = arg.checked;
            }
            else if constexpr (std::is_same_v<T, QuoteData>) text = arg.text;
            else if constexpr (std::is_same_v<T, CodeData>) text = arg.code;
            else if constexpr (std::is_same_v<T, BulletData>) text = arg.text;
            else if constexpr (std::is_same_v<T, CalloutData>) text = arg.text;
            else if constexpr (std::is_same_v<T, NumberedData>) text = arg.text;
            else if constexpr (std::is_same_v<T, ToggleData>) text = arg.text;
            else if constexpr (std::is_same_v<T, EquationData>) text = arg.latex;
        }, b.data());

        QVariantMap m;
        m[QStringLiteral("id")] = b.id().toString(QUuid::WithoutBraces);
        m[QStringLiteral("columnIndex")] = b.columnIndex();
        m[QStringLiteral("type")] = typeCodeOf(b.data());
        m[QStringLiteral("text")] = text;
        m[QStringLiteral("checked")] = checked;
        out.append(m);
    }
    return out;
}

void NoteEditorController::insertInColumn(const QString& columnsId, int columnIndex,
                                          int type, const QString& content)
{
    if (!m_document) return;

    const QUuid pid = QUuid::fromString(columnsId);
    if (pid.isNull() || !m_document->findBlock(pid)) return;

    int childOrdinal = 0;
    int insertRow = 0;
    bool foundInCol = false;
    for (const Block& b : m_document->blocks()) {
        if (b.parentId() != pid) continue;
        if (b.columnIndex() == columnIndex) {
            foundInCol = true;
            insertRow = childOrdinal + 1;
        } else if (!foundInCol) {
            insertRow = childOrdinal + 1;
        }
        ++childOrdinal;
    }
    if (!foundInCol)
        insertRow = childOrdinal;

    BlockData data;
    switch (type) {
    case 0:  data = ParagraphData{content}; break;
    case 1:  data = HeadingData{1, content}; break;
    case 2:  data = HeadingData{2, content}; break;
    case 3:  data = HeadingData{3, content}; break;
    case 4:  data = TodoData{content, false}; break;
    case 5:  data = CodeData{"", content}; break;
    case 9:  data = QuoteData{content}; break;
    case 10: data = HeadingData{4, content}; break;
    case 11: data = BulletData{content, 0}; break;
    case 12: data = CalloutData{content, QStringLiteral("💡")}; break;
    case 13: data = NumberedData{content, 0}; break;
    case 14: data = EquationData{content.isEmpty() ? QStringLiteral("E = mc^2") : content, true}; break;
    case 15: data = ToggleData{content, false}; break;
    default: data = ParagraphData{content}; break;
    }

    const QUuid newId = QUuid::createUuid();
    auto* cmd = new InsertBlockCommand(m_document, pid, insertRow, data, newId);
    m_document->undoStack()->push(cmd);

    if (Block* b = m_document->findBlock(newId))
        b->setColumnIndex(columnIndex);

    m_pendingFocusId = newId.toString(QUuid::WithoutBraces);
    emit pendingFocusIdChanged(m_pendingFocusId);
    emit documentModified();
    emit canUndoChanged();
    emit canRedoChanged();
}

void NoteEditorController::addColumn(const QString& columnsId)
{
    if (!m_document) return;
    const QUuid id = QUuid::fromString(columnsId);
    Block* block = m_document->findBlock(id);
    if (!block) return;

    std::visit([&](auto&& arg) {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, ColumnData>) {
            ColumnData u = arg;
            if (u.count < 4) {
                const int newIndex = u.count;
                u.count += 1;
                m_document->updateBlockData(id, u);
                insertInColumn(columnsId, newIndex, 0, "");
            }
        }
    }, block->data());

    emit documentModified();
}

void NoteEditorController::removeColumn(const QString& columnsId, int columnIndex)
{
    if (!m_document) return;

    const QUuid pid = QUuid::fromString(columnsId);
    if (pid.isNull() || !m_document->findBlock(pid))
        return;

    int count = 2;
    if (Block* p0 = m_document->findBlock(pid)) {
        std::visit([&](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, ColumnData>)
                count = arg.count;
        }, p0->data());
    }

    if (count <= 1 || columnIndex < 0 || columnIndex >= count)
        return;

    QList<QUuid> toDelete;
    for (const Block& b : m_document->blocks()) {
        if (b.parentId() == pid && b.columnIndex() == columnIndex)
            toDelete.append(b.id());
    }

    m_document->undoStack()->beginMacro(QStringLiteral("Remove column"));

    for (const QUuid& bid : toDelete)
        m_document->undoStack()->push(new DeleteBlockCommand(m_document, bid));

    // Shift higher indices (use ids collected first — list may have moved)
    QList<QUuid> toShift;
    for (const Block& b : m_document->blocks()) {
        if (b.parentId() == pid && b.columnIndex() > columnIndex)
            toShift.append(b.id());
    }
    for (const QUuid& bid : toShift) {
        if (Block* pb = m_document->findBlock(bid))
            pb->setColumnIndex(pb->columnIndex() - 1);
    }

    // CRITICAL: parent pointer from before deletes is invalid
    if (Block* parent = m_document->findBlock(pid)) {
        std::visit([&](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, ColumnData>) {
                ColumnData u = arg;
                u.count = std::max(1, u.count - 1);
                m_document->updateBlockData(pid, u);
            }
        }, parent->data());
    }

    m_document->undoStack()->endMacro();

    if (m_document->model())
        m_document->model()->rebuildMaps();

    emit documentModified();
    emit canUndoChanged();
    emit canRedoChanged();
}

void NoteEditorController::duplicateBlock(const QString& blockId)
{
    if (!m_document) return;
    QUuid id = QUuid::fromString(blockId);
    Block* block = m_document->findBlock(id);
    if (!block) return;

    const int type = 0;
    QString text;
    std::visit([&](auto&& arg) {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, ParagraphData>) text = arg.text;
        else if constexpr (std::is_same_v<T, HeadingData>) text = arg.text;
        else if constexpr (std::is_same_v<T, TodoData>) text = arg.text;
        else if constexpr (std::is_same_v<T, QuoteData>) text = arg.text;
        else if constexpr (std::is_same_v<T, CodeData>) text = arg.code;
        else if constexpr (std::is_same_v<T, BulletData>) text = arg.text;
        else if constexpr (std::is_same_v<T, CalloutData>) text = arg.text;
        else if constexpr (std::is_same_v<T, NumberedData>) text = arg.text;
        else if constexpr (std::is_same_v<T, ToggleData>) text = arg.text;
        else if constexpr (std::is_same_v<T, EquationData>) text = arg.latex;
    }, block->data());

    // typeCodeOf already exists in your .cpp — reuse it
    insertBlockAfter(blockId, typeCodeOf(block->data()), text);
}


bool NoteEditorController::pasteImageFromClipboard()
{
    if (!m_document)
        return false;

    const QClipboard* clip = QGuiApplication::clipboard();
    if (!clip)
        return false;

    QImage image = clip->image();
    if (image.isNull()) {
        // Some apps put image data only in the MIME payload
        const QMimeData* mime = clip->mimeData();
        if (mime && mime->hasImage())
            image = qvariant_cast<QImage>(mime->imageData());
    }
    if (image.isNull())
        return false;

    const QString dirPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
                            + QStringLiteral("/Anchors/images");
    QDir().mkpath(dirPath);

    const QString fileName = QStringLiteral("img_%1.png")
                                 .arg(QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd_hhmmss_zzz")));
    const QString fullPath = dirPath + QLatin1Char('/') + fileName;

    if (!image.save(fullPath, "PNG")) {
        emit errorOccurred(QStringLiteral("Failed to save clipboard image"));
        return false;
    }

    // Prefer file:// so QML Image loads it reliably
    const QString source = QUrl::fromLocalFile(fullPath).toString();

    // If focus is an empty image block → fill it; else insert after focused / at end
    QString focusId = m_focusedBlockId;
    if (!focusId.isEmpty()) {
        QUuid id = QUuid::fromString(focusId);
        if (Block* b = m_document->findBlock(id)) {
            bool isEmptyImage = false;
            std::visit([&](auto&& arg) {
                using T = std::decay_t<decltype(arg)>;
                if constexpr (std::is_same_v<T, ImageData>)
                    isEmptyImage = arg.source.isEmpty();
            }, b->data());

            if (isEmptyImage) {
                updateBlockImageSource(focusId, source);
                return true;
            }
        }
        insertBlockAfter(focusId, 6, source); // type 6 = image; content ignored for ImageData in insert
        // insertBlockAfter with type 6 may not put source in content — set it on the new block:
        if (!m_pendingFocusId.isEmpty())
            updateBlockImageSource(m_pendingFocusId, source);
        return true;
    }

    insertBlock(QString(), m_document->blocks().size(), 6, QString());
    if (!m_pendingFocusId.isEmpty())
        updateBlockImageSource(m_pendingFocusId, source);
    return true;
}