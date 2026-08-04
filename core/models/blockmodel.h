#ifndef BLOCKMODEL_H
#define BLOCKMODEL_H

#include <QAbstractItemModel>
#include <QUuid>
#include <QHash>
#include <QList>



class Block;
class Document;

class BlockModel : public QAbstractItemModel {
    Q_OBJECT

public:
    enum Roles {
        IdRole = Qt::UserRole + 1,
        TypeRole = Qt::UserRole + 2,
        DataRole = Qt::UserRole + 3
    };

    explicit BlockModel(Document* doc, QObject* parent = nullptr);
    ~BlockModel() override;

    // QAbstractItemModel interface
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& child) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    QHash<int, QByteArray> roleNames() const override;

    // Helpers
    Block* blockFromIndex(const QModelIndex& index) const;
    QModelIndex indexForId(QUuid id, const QModelIndex& parent = QModelIndex()) const;
    void rebuildMaps();
    void notifyInserted(int row, const QUuid& blockId);
    void notifyRemove(int row);

    // Use index-based access instead of raw pointers
    Block* blockAt(int index) const;

private:
    Document* m_document = nullptr;
    QList<QUuid> m_rootIds;
    QHash<QUuid, QList<QUuid>> m_childrenMap;
};

#endif // BLOCKMODEL_H
