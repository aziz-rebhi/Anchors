#ifndef NOTEEDITORCONTROLLER_H
#define NOTEEDITORCONTROLLER_H

#include <QObject>
#include <QUuid>
#include <QAbstractItemModel>
#include <QVariantList>

class Document;
class NotesDatabase;

class NoteEditorController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QAbstractItemModel* model READ model NOTIFY documentChanged)
    Q_PROPERTY(QString noteTitle READ noteTitle WRITE setNoteTitle NOTIFY noteTitleChanged)
    Q_PROPERTY(QUuid documentId READ documentId NOTIFY documentChanged)

public:
    explicit NoteEditorController(QObject* parent = nullptr);
    ~NoteEditorController();

    // Model access
    QAbstractItemModel* model() const;

    // Document properties
    QString noteTitle() const;
    void setNoteTitle(const QString& title);
    QUuid documentId() const;

    // Document management
    Q_INVOKABLE void createNewNote(const QString& title);
    Q_INVOKABLE void loadNote(const QString& id);
    Q_INVOKABLE void saveNote();
    Q_INVOKABLE void deleteNote();

    // Block operations
    Q_INVOKABLE void insertBlock(const QString& parentId, int row, int type, const QString& content);
    Q_INVOKABLE void deleteBlock(const QString& blockId);
    Q_INVOKABLE void updateBlockContent(const QString& blockId, const QString& content);
    Q_INVOKABLE void loadFromContent(const QString& title, const QStringList& paragraphs);

    // List all documents
    Q_INVOKABLE QVariantList getDocuments() const;

    // Undo/redo
    Q_INVOKABLE void undo();
    Q_INVOKABLE void redo();
    Q_INVOKABLE bool canUndo() const;
    Q_INVOKABLE bool canRedo() const;

signals:
    void documentChanged();
    void noteTitleChanged(const QString& title);
    void errorOccurred(const QString& error);
    void modelChanged();

private:
    Document* m_document = nullptr;
    NotesDatabase* m_db = nullptr;
};

#endif // NOTEEDITORCONTROLLER_H