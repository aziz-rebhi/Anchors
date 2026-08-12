#ifndef DOCUMENT_H
#define DOCUMENT_H

#include <QObject>
#include <QUuid>
#include <QString>
#include <QList>
#include <QJsonObject>
#include "block.h"

class BlockModel;
class QUndoStack;
class NotesDatabase;

class Document : public QObject {
    Q_OBJECT

public:
    Document(QUuid id, const QString& title, QObject* parent = nullptr);
    ~Document() override;

    QUuid id() const;
    QString title() const;
    void setTitle(const QString& title);

    QList<Block> blocks() const;
    Block* findBlock(QUuid id);
    int findBlockIndex(QUuid id) const;
    Block* blockAt(int index);

    void insertBlock(QUuid parentId, int row, const BlockData& data);
    void deleteBlock(QUuid blockId);
    void updateBlockData(QUuid blockId, const BlockData& newData);
    void moveBlock(QUuid blockId, QUuid newParentId, int newRow);

    QUndoStack* undoStack() const;
    BlockModel* model() const;

    QJsonObject toJson() const;
    static Document* fromJson(const QJsonObject& obj, NotesDatabase* db = nullptr);

signals:
    void titleChanged(const QString& title);
    void blockInserted(QUuid blockId, QUuid parentId, int row);
    void blockDeleted(QUuid blockId);
    void blockDataChanged(QUuid blockId);
    void documentModified();

private:
    void insertBlockInternal(QUuid parentId, int row, const Block& block);
    void removeBlockInternal(QUuid blockId);

    QUuid m_id;
    QString m_title;
    QList<Block> m_blocks;
    BlockModel* m_blockModel = nullptr;
    QUndoStack* m_undoStack = nullptr;


    // used by MoveBlockCommand
    void moveBlockInternal(QUuid blockId, int fromRow, int toRow);

    // friends:
    friend class MoveBlockCommand;

    // Friends for undo/redo commands
    friend class InsertBlockCommand;
    friend class DeleteBlockCommand;
    friend class EditTextCommand;
};

#endif // DOCUMENT_H
