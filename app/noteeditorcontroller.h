#ifndef NOTEEDITORCONTROLLER_H
#define NOTEEDITORCONTROLLER_H

#include <QObject>
#include <QUuid>
#include <QVariantList>
#include <QAbstractItemModel>

class Document;
class NotesDatabase;

class NoteEditorController : public QObject {
    Q_OBJECT
    Q_PROPERTY(QAbstractItemModel* model READ model NOTIFY modelChanged)
    Q_PROPERTY(QString noteTitle READ noteTitle WRITE setNoteTitle NOTIFY noteTitleChanged)
    Q_PROPERTY(bool canUndo READ canUndo NOTIFY canUndoChanged)
    Q_PROPERTY(bool canRedo READ canRedo NOTIFY canRedoChanged)
    Q_PROPERTY(QString pendingFocusId READ pendingFocusId NOTIFY pendingFocusIdChanged)
    Q_PROPERTY(QString focusedBlockId READ focusedBlockId NOTIFY focusedBlockIdChanged)

public:
    explicit NoteEditorController(QObject* parent = nullptr);
    ~NoteEditorController();

    QAbstractItemModel* model() const;
    QString noteTitle() const;
    void setNoteTitle(const QString& title);
    QUuid documentId() const;
    bool canUndo() const;
    bool canRedo() const;
    QString pendingFocusId() const;
    QString focusedBlockId() const;

    Q_INVOKABLE void createNewNote(const QString& title = "");
    Q_INVOKABLE void loadNote(const QString& id);
    Q_INVOKABLE void saveNote();
    Q_INVOKABLE void deleteNote();
    Q_INVOKABLE void undo();
    Q_INVOKABLE void redo();

    // type: 0=Paragraph, 1=H1, 2=H2, 3=H3, 4=Todo, 5=Code, 6=Image, 7=Table, 8=Divider, 9=Quote
    Q_INVOKABLE void insertBlock(const QString& parentId, int row, int type, const QString& content = "");
    Q_INVOKABLE void insertBlockAfter(const QString& blockId, int type, const QString& content = "");
    Q_INVOKABLE void deleteBlock(const QString& blockId);
    Q_INVOKABLE void updateBlockContent(const QString& blockId, const QString& content);
    Q_INVOKABLE void updateBlockCodeLanguage(const QString& blockId, const QString& language);
    Q_INVOKABLE void updateBlockImageSource(const QString& blockId, const QString& source);
    Q_INVOKABLE void updateBlockImageCaption(const QString& blockId, const QString& caption);
    Q_INVOKABLE void updateBlockTableData(const QString& blockId, const QVariantList& cells);
    Q_INVOKABLE void updateBlockDividerOrientation(const QString& blockId, int orientation);
    Q_INVOKABLE void toggleBlockChecked(const QString& blockId);
    Q_INVOKABLE void mergeWithPrevious(const QString& blockId);
    Q_INVOKABLE void changeBlockType(const QString& blockId, int newType);
    Q_INVOKABLE void setFocusedBlock(const QString& blockId);
    Q_INVOKABLE void moveBlock(const QString& blockId, int newRow);

    Q_INVOKABLE QVariantList getDocuments() const;
    Q_INVOKABLE void loadFromContent(const QString& title, const QStringList& paragraphs);
    Q_INVOKABLE QString documentToJson() const;
    Q_INVOKABLE void loadFromJson(const QString& title, const QString& jsonContent);
    Q_INVOKABLE void toggleCollapsed(const QString& blockId);
    Q_INVOKABLE void insertInside(const QString& parentBlockId, int type, const QString& content = "");
    Q_INVOKABLE int numberedIndex(const QString& blockId) const;
    Q_INVOKABLE void exitContainer(const QString& blockId, int type = 0, const QString& content = "");
    Q_INVOKABLE void focusAdjacent(const QString& blockId, bool next);
    Q_INVOKABLE void indentBlock(const QString& blockId);
    Q_INVOKABLE void outdentBlock(const QString& blockId);

    Q_INVOKABLE QVariantList columnChildren(const QString& columnsId) const;
    Q_INVOKABLE void insertInColumn(const QString& columnsId, int columnIndex, int type = 0, const QString& content = "");
    Q_INVOKABLE void addColumn(const QString& columnsId);
    Q_INVOKABLE void removeColumn(const QString& columnsId, int columnIndex);

    Q_INVOKABLE void duplicateBlock(const QString& blockId);
    Q_INVOKABLE bool pasteImageFromClipboard();



signals:
    void modelChanged();
    void noteTitleChanged(const QString& title);
    void documentChanged();
    void canUndoChanged();
    void canRedoChanged();
    void pendingFocusIdChanged(const QString& id);
    void focusedBlockIdChanged(const QString& id);
    void errorOccurred(const QString& message);
    void documentModified();

private:
    void bindDocumentSignals();

    Document* m_document = nullptr;
    NotesDatabase* m_db = nullptr;
    QString m_pendingFocusId;
    QString m_focusedBlockId;
};

#endif // NOTEEDITORCONTROLLER_H