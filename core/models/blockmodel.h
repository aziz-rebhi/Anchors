// blockmodel.h
#pragma once
#include <QAbstractItemModel>
#include <QList>
#include <QUuid>
#include "block.h"

class Document;

class BlockModel : public QAbstractItemModel {
    Q_OBJECT
public:
    enum Roles {
        IdRole = Qt::UserRole + 1,
        TypeRole,
        DataRole,
        // etc.
    };

    explicit BlockModel(Document* doc, QObject* parent = nullptr);
    ~BlockModel();

    // QAbstractItemModel overrides
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& child) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    QHash<int, QByteArray> roleNames() const override;

    // Add/remove/move (called by Document or commands)
    void insertBlock(QUuid parentId, int row, const Block& block);
    void removeBlock(QUuid blockId);
    void moveBlock(QUuid blockId, QUuid newParentId, int newRow);
    void updateBlockData(QUuid blockId);
    void notifyInserted(QUuid parentId, int row);
    void notifyRemove(QUuid parentId, int row);

    // Get block by index
    Block* blockFromIndex(const QModelIndex& index) const;
    void rebuildMaps();

private:
    Document* m_document;
    QHash<QUuid, QList<QUuid>> m_childrenMap; // parentId -> list of child ids
    QHash<QUuid, Block*> m_blockMap; // id -> Block*
    QList<QUuid> m_rootIds; // top-level ids

    // Helper: find index of a block id
    QModelIndex indexForId(QUuid id, const QModelIndex& parent = QModelIndex()) const;

};