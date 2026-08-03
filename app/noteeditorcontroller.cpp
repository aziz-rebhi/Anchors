#include "noteeditorcontroller.h"
#include "../core/models/document.h"
#include "../core/models/blockmodel.h"
#include "../core/storage/notesdatabase.h"
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

    // Insert a default paragraph block so the user can start typing
    BlockData data = ParagraphData{""};
    m_document->insertBlock(QUuid(), 0, data);

    // Save to database
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
    default:
        emit errorOccurred("Unsupported block type");
        return;
    }

    m_document->insertBlock(parentUuid, row, data);
}

void NoteEditorController::deleteBlock(const QString& blockId)
{
    if (!m_document) return;
    QUuid id = QUuid::fromString(blockId);
    if (id.isNull()) return;
    m_document->deleteBlock(id);
}

void NoteEditorController::updateBlockContent(const QString& blockId, const QString& content)
{
    if (!m_document) return;
    QUuid id = QUuid::fromString(blockId);
    if (id.isNull()) return;

    Block* block = m_document->findBlock(id);
    if (!block) return;

    // Preserve the existing block type — only update the text
    BlockData newData = std::visit([&content](auto&& arg) -> BlockData {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, ParagraphData>) {
            return ParagraphData{content};
        } else if constexpr (std::is_same_v<T, HeadingData>) {
            return HeadingData{arg.level, content};
        } else {
            return ParagraphData{content};
        }
    }, block->data());

    m_document->updateBlockData(id, newData);
}

QVariantList NoteEditorController::getDocuments() const
{
    QVariantList list;
    if (!m_db) return list;
    QList<QUuid> ids = m_db->allDocumentIds();
    for (const QUuid& id : ids) {
        // Load only title for now
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

void NoteEditorController::loadFromContent(const QString& title, const QStringList& paragraphs){
    if (m_document){
        delete m_document;
        m_document = nullptr;
    }

    QUuid docId = QUuid::createUuid();
    m_document =  new Document(docId, title, this);

    int row = 0;
    if (paragraphs.isEmpty()){
        BlockData Data = ParagraphData{""};
        m_document->insertBlock(QUuid(), row++, Data);
    } else {
        for  (const QString& para : paragraphs){
            BlockData Data = ParagraphData{para};
            m_document->insertBlock(QUuid(), row++, Data);
        }
    }

    emit documentChanged();
    emit noteTitleChanged(title);
    emit modelChanged();
}