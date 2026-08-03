#include "blockmodel.h"
#include "document.h"
#include <QDebug>

BlockModel::BlockModel(Document* doc, QObject* parent)
    : QAbstractItemModel(parent)
    , m_document(doc)
{
    Q_ASSERT(doc);
    rebuildMaps();

    connect(doc, &Document::blockDataChanged,
            this, [this](QUuid blockId) {
                QModelIndex idx = indexForId(blockId);
                if (idx.isValid())
                    emit dataChanged(idx, idx);
            });
}

BlockModel::~BlockModel() = default;

void BlockModel::rebuildMaps()
{
    m_blockMap.clear();
    m_childrenMap.clear();
    m_rootIds.clear();

    if (!m_document) return;

    const QList<Block>& topBlocks = m_document->blocks();
    for (const Block& block : topBlocks) {
        m_rootIds.append(block.id());
        m_blockMap[block.id()] = const_cast<Block*>(&block);
    }

    emit layoutChanged();
}

QModelIndex BlockModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    QUuid parentId;
    if (parent.isValid()) {
        Block* parentBlock = static_cast<Block*>(parent.internalPointer());
        if (!parentBlock) return QModelIndex();
        parentId = parentBlock->id();
    } else {
        parentId = QUuid();
    }

    QList<QUuid> childIds = (parentId.isNull() ? m_rootIds : m_childrenMap.value(parentId));
    if (row < 0 || row >= childIds.size())
        return QModelIndex();

    QUuid childId = childIds.at(row);
    Block* childBlock = m_blockMap.value(childId);
    if (!childBlock) return QModelIndex();

    return createIndex(row, column, childBlock);
}

QModelIndex BlockModel::parent(const QModelIndex& child) const
{
    if (!child.isValid())
        return QModelIndex();

    Block* childBlock = static_cast<Block*>(child.internalPointer());
    if (!childBlock) return QModelIndex();

    QUuid parentId = childBlock->parentId();
    if (parentId.isNull())
        return QModelIndex();

    Block* parentBlock = m_blockMap.value(parentId);
    if (!parentBlock) return QModelIndex();

    QUuid grandParentId = parentBlock->parentId();
    QList<QUuid> siblingIds = (grandParentId.isNull() ? m_rootIds : m_childrenMap.value(grandParentId));
    int row = siblingIds.indexOf(parentId);
    if (row < 0) return QModelIndex();

    return createIndex(row, 0, parentBlock);
}

int BlockModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid() && parent.column() > 0)
        return 0;

    QUuid parentId;
    if (parent.isValid()) {
        Block* parentBlock = static_cast<Block*>(parent.internalPointer());
        if (!parentBlock) return 0;
        parentId = parentBlock->id();
    } else {
        parentId = QUuid();
    }

    if (parentId.isNull())
        return m_rootIds.size();
    else
        return m_childrenMap.value(parentId).size();
}

int BlockModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return 1;
}

QVariant BlockModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();

    Block* block = static_cast<Block*>(index.internalPointer());
    if (!block) return QVariant();

    switch (role) {
    case Qt::DisplayRole:
        return QString("Block %1").arg(block->id().toString());
    case IdRole:
        return block->id().toString(QUuid::WithoutBraces);
    case TypeRole:
        return QString::number(block->data().index());
    case DataRole: {
        QVariantMap map;
        std::visit([&](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, ParagraphData>) {
                map["text"] = arg.text;
            } else if constexpr (std::is_same_v<T, HeadingData>) {
                map["text"] = arg.text;
                map["level"] = arg.level;
            }
            // ... other types
        }, block->data());
        return map;
    }
    default:
        return QVariant();
    }
}

Qt::ItemFlags BlockModel::flags(const QModelIndex& index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
}

Block* BlockModel::blockFromIndex(const QModelIndex& index) const
{
    if (!index.isValid())
        return nullptr;
    return static_cast<Block*>(index.internalPointer());
}

QModelIndex BlockModel::indexForId(QUuid id, const QModelIndex& parent) const
{
    if (id.isNull()) return QModelIndex();
    Block* block = m_blockMap.value(id);
    if (!block) return QModelIndex();
    QUuid parentId = block->parentId();
    QList<QUuid> siblings = parentId.isNull() ? m_rootIds : m_childrenMap.value(parentId);
    int row = siblings.indexOf(id);
    if (row < 0) return QModelIndex();
    QModelIndex parentIndex = indexForId(parentId);
    return createIndex(row, 0, block);
}

QHash<int, QByteArray> BlockModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[IdRole] = "id";
    roles[TypeRole] = "type";
    roles[DataRole] = "data";
    return roles;
}