#pragma once
#include <QUuid>
#include <QList>
#include <QPointer>
#include <QUndoStack>
#include "block.h"

class BlockModel;
class NotesDatabase; // forward declare repository
class BlockFactory;

class Document : public QObject {
    Q_OBJECT

    friend class InsertBlockCommand;
    friend class DeleteBlockCommand;
    friend class EditTextCommand;


public:
    explicit Document(QUuid id, const QString& title, QObject* parent = nullptr);
    ~Document();

    QUuid id() const;
    QString title() const;
    void setTitle(const QString& title);

    QList<Block> blocks() const; // top-level blocks
    Block* findBlock(QUuid id);
    QList<Block*> childrenOf(QUuid parentId);

    // Mutations
    void insertBlock(QUuid parentId, int row, const BlockData& data);
    void deleteBlock(QUuid blockId);
    void moveBlock(QUuid blockId, QUuid newParentId, int newRow);
    void updateBlockData(QUuid blockId, const BlockData& newData);

    // Undo/redo
    QUndoStack* undoStack() const;

    // Model
    BlockModel* model() const;

    // Persistence
    void loadFromDatabase(NotesDatabase* db);
    void saveToDatabase(NotesDatabase* db);

    // Serialization (JSON)
    QJsonObject toJson() const;
    static Document* fromJson(const QJsonObject& obj, NotesDatabase* db = nullptr);

    int findBlockIndex(QUuid id) const;
    Block* blockAt(int index);

signals:
    void titleChanged(const QString& title);
    void blockInserted(QUuid blockId, QUuid parentId, int row);
    void blockDeleted(QUuid blockId);
    void blockMoved(QUuid blockId, QUuid oldParent, int oldRow, QUuid newParent, int newRow);
    void blockDataChanged(QUuid blockId);
    void documentModified();

private:
    QUuid m_id;
    QString m_title;
    QList<Block> m_blocks; // top-level, sorted by orderIndex

    QUndoStack* m_undoStack;
    BlockModel* m_blockModel;

    // Helper: rebuild order indices
    void reorderSiblings(QUuid parentId);
    void insertBlockInternal(QUuid parentId, int row, const Block& block);
    void removeBlockInternal(QUuid blockId);
};