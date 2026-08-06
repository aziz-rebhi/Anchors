#include "blockmodel.h"
#include "document.h"
#include <QDebug>

BlockModel::BlockModel(Document* doc, QObject* parent)
    : QAbstractItemModel(parent)
    , m_document(doc)
{
    Q_ASSERT(doc);
    rebuildMaps();

    // When a block's content changes in-place, emit dataChanged() for
    // that single index instead of nuking the entire model with layoutChanged().
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
    m_rootIds.clear();
    m_childrenMap.clear();

    if (!m_document) return;

    const QList<Block>& topBlocks = m_document->blocks();
    for (const Block& block : topBlocks) {
        m_rootIds.append(block.id());
    }

    emit layoutChanged();
}

void BlockModel::notifyInserted(int row, const QUuid& blockId)
{
    beginInsertRows(QModelIndex(), row, row);
    m_rootIds.insert(row,blockId);
    endInsertRows();
}

void BlockModel::notifyRemove(int row)
{
    beginRemoveRows(QModelIndex(), row, row);
    m_rootIds.removeAt(row);
    endRemoveRows();
}

Block* BlockModel::blockAt(int index) const
{
    if (!m_document) return nullptr;
    return m_document->blockAt(index);
}

QModelIndex BlockModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    // For root items, use row index directly into the document's block list
    // Store the row as the internal pointer (safe from reallocation)
    return createIndex(row, column, nullptr);
}

QModelIndex BlockModel::parent(const QModelIndex& child) const
{
    // Single-level model: all blocks are root children
    Q_UNUSED(child);
    return QModelIndex();
}

int BlockModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid() && parent.column() > 0)
        return 0;

    if (!parent.isValid())
        return m_rootIds.size();

    return 0;
}

int BlockModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return 1;
}

// Helper to map BlockData variant to a semantic type code for QML
static int blockTypeCode(const BlockData& data)
{
    return std::visit([](auto&& arg) -> int {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, ParagraphData>) {
            return 0;
        } else if constexpr (std::is_same_v<T, HeadingData>) {
            if (arg.level >= 4) return 10;
            return arg.level; // 1, 2, or 3
        } else if constexpr (std::is_same_v<T, TodoData>) {
            return 4;
        } else if constexpr (std::is_same_v<T, CodeData>) {
            return 5;
        } else if constexpr (std::is_same_v<T, ImageData>) {
            return 6;
        } else if constexpr (std::is_same_v<T, TableData>) {
            return 7;
        } else if constexpr (std::is_same_v<T, DividerData>) {
            return 8;
        } else if constexpr (std::is_same_v<T, QuoteData>) {
            return 9;
        } else if constexpr (std::is_same_v<T, BulletData>){
            return 11;
        } else if constexpr (std::is_same_v<T, CalloutData>){
            return 12;
        }
        return 0;
    }, data);
}

QVariant BlockModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || !m_document)
        return QVariant();

    // Use row-based access instead of stored pointers (safe from QList reallocation)
    Block* block = m_document->blockAt(index.row());
    if (!block) return QVariant();

    switch (role) {
    case Qt::DisplayRole:
        return QString("Block %1").arg(block->id().toString());
    case IdRole:
        return block->id().toString(QUuid::WithoutBraces);
    case TypeRole:
        return blockTypeCode(block->data());
    case DataRole: {
        QVariantMap map;
        std::visit([&](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, ParagraphData>) {
                map["text"] = arg.text;
            } else if constexpr (std::is_same_v<T, HeadingData>) {
                map["text"] = arg.text;
                map["level"] = arg.level;
            } else if constexpr (std::is_same_v<T, TodoData>) {
                map["text"] = arg.text;
                map["checked"] = arg.checked;
            } else if constexpr (std::is_same_v<T, CodeData>) {
                map["language"] = arg.language;
                map["code"] = arg.code;
            } else if constexpr (std::is_same_v<T, ImageData>) {
                map["source"] = arg.source;
                map["caption"] = arg.caption;
                map["width"] = arg.width;
                map["height"] = arg.height;
            } else if constexpr (std::is_same_v<T, TableData>) {
                map["rows"] = arg.rows;
                map["cols"] = arg.cols;
                QVariantList rows;
                for (const auto& row : arg.cells) {
                    QVariantList r;
                    for (const auto& cell : row) r.append(cell);
                    rows.append(QVariant(r));
                }
                map["cells"] = rows;
            } else if constexpr (std::is_same_v<T, DividerData>) {
                map["orientation"] = arg.orientation;
            } else if constexpr (std::is_same_v<T, QuoteData>) {
                map["text"] = arg.text;
            } else if constexpr (std::is_same_v<T, BulletData>) {
                map["text"] = arg.text;
            } else if constexpr (std::is_same_v<T, CalloutData>) {
                map["text"] = arg.text;
                map["emoji"] = arg.emoji;
            }
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
    if (!index.isValid() || !m_document)
        return nullptr;
    return m_document->blockAt(index.row());
}

QModelIndex BlockModel::indexForId(QUuid id, const QModelIndex&) const
{
    if (id.isNull() || !m_document) return QModelIndex();
    int row = m_document->findBlockIndex(id);
    if (row < 0) return QModelIndex();
    return createIndex(row, 0, nullptr);
}

QHash<int, QByteArray> BlockModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[IdRole] = "id";
    roles[TypeRole] = "type";
    roles[DataRole] = "blockData"; // CRITICAL: NOT "data" (shadows QML's model.data())
    return roles;
}