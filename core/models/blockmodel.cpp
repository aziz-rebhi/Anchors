#include "blockmodel.h"
#include "document.h"
#include "blockdata.h"
#include <QDebug>

BlockModel::BlockModel(Document* doc, QObject* parent)
    : QAbstractItemModel(parent)
    , m_document(doc)
{
    Q_ASSERT(doc);
    rebuildMaps();

    // Never call rebuildMaps() on ordinary text edits — that resets the model
    // and steals focus. Collapse changes are handled explicitly in
    // NoteEditorController::toggleCollapsed().
    connect(doc, &Document::blockDataChanged, this, [this](QUuid blockId) {
        QModelIndex idx = indexForId(blockId);
        if (idx.isValid())
            emit dataChanged(idx, idx);
    });
}

BlockModel::~BlockModel() = default;

void BlockModel::rebuildMaps()
{
    beginResetModel();
    m_visibleIds.clear();

    if (!m_document) {
        endResetModel();
        return;
    }

    const QList<Block>& all = m_document->blocks();

    auto isExpandedToggle = [](const Block& b) -> bool {
        return std::visit([](auto&& arg) -> bool {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, ToggleData>)
                return !arg.collapsed;
            return false;
        }, b.data());
    };

    for (int i = 0; i < all.size(); ++i) {
        const Block& b = all[i];
        if (!b.parentId().isNull())
            continue;

        m_visibleIds.append(b.id());

        if (isExpandedToggle(b)) {
            for (int j = 0; j < all.size(); ++j) {
                if (all[j].parentId() == b.id())
                    m_visibleIds.append(all[j].id());
            }
        }
    }

    endResetModel();
}

void BlockModel::notifyInserted(int /*row*/, const QUuid& /*blockId*/)
{
    rebuildMaps();
}

void BlockModel::notifyRemove(int /*row*/)
{
    rebuildMaps();
}

Block* BlockModel::blockAt(int index) const
{
    if (!m_document || index < 0 || index >= m_visibleIds.size())
        return nullptr;
    return m_document->findBlock(m_visibleIds[index]);
}

QModelIndex BlockModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();
    return createIndex(row, column, nullptr);
}

QModelIndex BlockModel::parent(const QModelIndex&) const
{
    return QModelIndex();
}

int BlockModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
        return 0;
    return m_visibleIds.size();
}

int BlockModel::columnCount(const QModelIndex&) const
{
    return 1;
}

static int blockTypeCode(const BlockData& data)
{
    return std::visit([](auto&& arg) -> int {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, ParagraphData>) return 0;
        else if constexpr (std::is_same_v<T, HeadingData>) {
            if (arg.level >= 4) return 10;
            return arg.level;
        }
        else if constexpr (std::is_same_v<T, TodoData>) return 4;
        else if constexpr (std::is_same_v<T, CodeData>) return 5;
        else if constexpr (std::is_same_v<T, ImageData>) return 6;
        else if constexpr (std::is_same_v<T, TableData>) return 7;
        else if constexpr (std::is_same_v<T, DividerData>) return 8;
        else if constexpr (std::is_same_v<T, QuoteData>) return 9;
        else if constexpr (std::is_same_v<T, BulletData>) return 11;
        else if constexpr (std::is_same_v<T, CalloutData>) return 12;
        else if constexpr (std::is_same_v<T, NumberedData>) return 13;
        else if constexpr (std::is_same_v<T, EquationData>) return 14;
        else if constexpr (std::is_same_v<T, ToggleData>) return 15;
        return 0;
    }, data);
}

QVariant BlockModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || !m_document)
        return QVariant();

    Block* block = blockAt(index.row());
    if (!block) return QVariant();

    switch (role) {
    case IdRole:
        return block->id().toString(QUuid::WithoutBraces);
    case TypeRole:
        return blockTypeCode(block->data());
    case ParentIdRole:
        return block->parentId().isNull()
                   ? QString()
                   : block->parentId().toString(QUuid::WithoutBraces);
    case DepthRole:
        return block->parentId().isNull() ? 0 : 1;
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
                map["indent"] = arg.indent;
            } else if constexpr (std::is_same_v<T, CalloutData>) {
                map["text"] = arg.text;
                map["emoji"] = arg.emoji;
            } else if constexpr (std::is_same_v<T, NumberedData>) {
                map["text"] = arg.text;
                map["indent"] = arg.indent;
            } else if constexpr (std::is_same_v<T, EquationData>) {
                map["latex"] = arg.latex;
                map["displayMode"] = arg.displayMode;
            } else if constexpr (std::is_same_v<T, ToggleData>) {
                map["text"] = arg.text;
                map["collapsed"] = arg.collapsed;
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
    return blockAt(index.row());
}

QModelIndex BlockModel::indexForId(QUuid id, const QModelIndex&) const
{
    if (id.isNull()) return QModelIndex();
    for (int i = 0; i < m_visibleIds.size(); ++i) {
        if (m_visibleIds[i] == id)
            return createIndex(i, 0, nullptr);
    }
    return QModelIndex();
}

QHash<int, QByteArray> BlockModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[IdRole] = "id";
    roles[TypeRole] = "type";
    roles[DataRole] = "blockData";
    roles[ParentIdRole] = "parentId";
    roles[DepthRole] = "depth";
    return roles;
}