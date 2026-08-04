#include "noteeditorcontroller.h"
#include "../core/models/document.h"
#include "../core/models/blockmodel.h"
#include "../core/models/blockdata.h"
#include "../core/storage/notesdatabase.h"
#include "../core/models/blockcommands.h"
#include <QDebug>

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

void NoteEditorController::createNewNote(const QString& title)
{
    if (m_document) {
        saveNote();
        delete m_document;
        m_document = nullptr;
    }

    QUuid docId = QUuid::createUuid();
    m_document = new Document(docId, title.isEmpty() ? "Untitled" : title, this);

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

// Block type constants (must match BlockModel::TypeRole values)
// 0 = Paragraph, 1 = H1, 2 = H2, 3 = H3, 4 = Todo

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
    case 6: data = ImageData{"", "", 0, 0}; break;
    case 7: data = TableData{}; break;
    case 8: data = QuoteData{content}; break;
    case 9: data = DividerData{}; break;
    default:
        emit errorOccurred("Unsupported block type");
        return;
    }

    m_document->undoStack()->push(new InsertBlockCommand(m_document, parentUuid, row, data));
}

void NoteEditorController::deleteBlock(const QString& blockId)
{
    if (!m_document) return;
    QUuid id = QUuid::fromString(blockId);
    if (id.isNull()) return;
    m_document->undoStack()->push(new DeleteBlockCommand(m_document, id));
}

void NoteEditorController::updateBlockContent(const QString& blockId, const QString& content)
{
    if (!m_document) return;
    QUuid id = QUuid::fromString(blockId);
    if (id.isNull()) return;

    Block* block = m_document->findBlock(id);
    if (!block) return;

    BlockData oldData = block->data();

    BlockData newData = std::visit([&content](auto&& arg) -> BlockData {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, ParagraphData>) {
            return ParagraphData{content};
        } else if constexpr (std::is_same_v<T, HeadingData>) {
            return HeadingData{arg.level, content};
        } else if constexpr (std::is_same_v<T, TodoData>) {
            return TodoData{content, arg.checked};
        } else if constexpr (std::is_same_v<T, QuoteData>) {
            return QuoteData{content};
        } else if constexpr (std::is_same_v<T, CodeData>) {
            return CodeData{arg.language, content};
        } else {
            return ParagraphData{content};
        }
    }, oldData);

    m_document->undoStack()->push(new EditTextCommand(m_document, id, oldData, newData));
}

void NoteEditorController::toggleBlockChecked(const QString& blockId, bool checked)
{
    if (!m_document) return;
    QUuid id = QUuid::fromString(blockId);
    if (id.isNull()) return;

    Block* block = m_document->findBlock(id);
    if (!block) return;
    if (!std::holds_alternative<TodoData>(block->data())) return;

    BlockData oldData = block->data();
    auto todo = std::get<TodoData>(oldData);
    todo.checked = checked;

    m_document->undoStack()->push(new EditTextCommand(m_document, id, oldData, todo));
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

bool NoteEditorController::canUndo() const
{
    return m_document && m_document->undoStack() && m_document->undoStack()->canUndo();
}

bool NoteEditorController::canRedo() const
{
    return m_document && m_document->undoStack() && m_document->undoStack()->canRedo();
}

void NoteEditorController::loadFromContent(const QString& title, const QStringList& paragraphs)
{
    if (m_document){
        delete m_document;
        m_document = nullptr;
    }

    QUuid docId = QUuid::createUuid();
    m_document =  new Document(docId, title, this);

    int row = 0;
    if (paragraphs.isEmpty()){
        BlockData data = ParagraphData{""};
        m_document->insertBlock(QUuid(), row++, data);
    } else {
        for  (const QString& para : paragraphs){
            BlockData data = ParagraphData{para};
            m_document->insertBlock(QUuid(), row++, data);
        }
    }

    emit documentChanged();
    emit noteTitleChanged(title);
    emit modelChanged();
}


void NoteEditorController::insertBlockAfter(const QString& blockId, int type, const QString& content)
{
    if (!m_document) return;
    QUuid id = QUuid::fromString(blockId);
    if (id.isNull()) return;

    int row = m_document->findBlockIndex(id);
    if (row < 0) return;

    insertBlock("", row + 1, type, content);
}

void NoteEditorController::mergeWithPrevious(const QString& blockId)
{
    if (!m_document) return;
    QUuid id = QUuid::fromString(blockId);
    if (id.isNull()) return;

    int row = m_document->findBlockIndex(id);
    if (row <= 0) return;

    Block* current = m_document->findBlock(id);
    Block* previous = m_document->blockAt(row - 1);
    if (!current || !previous) return;

    // Get text from current block
    QString curText = std::visit([](auto&& arg) -> QString {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, ParagraphData>)   return arg.text;
        else if constexpr (std::is_same_v<T, HeadingData>) return arg.text;
        else if constexpr (std::is_same_v<T, TodoData>)    return arg.text;
        else return "";
    }, current->data());

    if (curText.isEmpty()) {
        // Empty block — just delete it
        deleteBlock(blockId);
        return;
    }

    // Get previous block's text length (for cursor positioning later)
    QString prevText = std::visit([](auto&& arg) -> QString {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, ParagraphData>)   return arg.text;
        else if constexpr (std::is_same_v<T, HeadingData>) return arg.text;
        else if constexpr (std::is_same_v<T, TodoData>)    return arg.text;
        else return "";
    }, previous->data());

    // Wrap both ops in one undo step
    m_document->undoStack()->beginMacro("Merge block");

    // Append current text to previous block (preserves previous block's type)
    updateBlockContent(previous->id().toString(QUuid::WithoutBraces), prevText + curText);

    // Delete current block
    deleteBlock(blockId);

    m_document->undoStack()->endMacro();
}