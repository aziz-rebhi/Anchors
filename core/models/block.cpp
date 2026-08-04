#include "block.h"
#include <QJsonObject>
#include <QJsonArray>
#include <QDateTime>

// ----------------------------------------------------------------
// BlockData serialization helpers
// ----------------------------------------------------------------

static QJsonObject blockDataToJson(const BlockData& data)
{
    QJsonObject obj;
    std::visit([&](auto&& arg) {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, ParagraphData>) {
            obj["type"] = "paragraph";
            obj["text"] = arg.text;
        } else if constexpr (std::is_same_v<T, HeadingData>) {
            obj["type"] = "heading";
            obj["level"] = arg.level;
            obj["text"] = arg.text;
        } else if constexpr (std::is_same_v<T, CodeData>) {
            obj["type"] = "code";
            obj["language"] = arg.language;
            obj["code"] = arg.code;
        } else if constexpr (std::is_same_v<T, ImageData>) {
            obj["type"] = "image";
            obj["filePath"] = arg.filePath;
            obj["caption"] = arg.caption;
            obj["width"] = arg.width;
            obj["height"] = arg.height;
        } else if constexpr (std::is_same_v<T, TableData>) {
            obj["type"] = "table";
            obj["rows"] = arg.rows;
            obj["cols"] = arg.cols;
            QJsonArray cellsArray;
            for (const auto& row : arg.cells) {
                QJsonArray rowArray;
                for (const auto& cell : row) {
                    rowArray.append(cell);
                }
                cellsArray.append(rowArray);
            }
            obj["cells"] = cellsArray;
        } else if constexpr (std::is_same_v<T, TodoData>) {
            obj["type"] = "todo";
            obj["text"] = arg.text;
            obj["checked"] = arg.checked;
        } else if constexpr (std::is_same_v<T, QuoteData>){
            obj["type"] = "quote";
            obj["text"] = arg.text;
        } else if constexpr (std::is_same_v<T, DividerData>){
            obj["type"] = "divider";
        }
    }, data);
    return obj;
}

static BlockData blockDataFromJson(const QJsonObject& obj)
{
    QString type = obj["type"].toString();

    if (type == "heading") {
        return HeadingData{ obj["level"].toInt(1), obj["text"].toString() };
    } else if (type == "code") {
        return CodeData{ obj["language"].toString(), obj["code"].toString() };
    } else if (type == "image") {
        ImageData img;
        img.filePath = obj["filePath"].toString();
        img.caption  = obj["caption"].toString();
        img.width    = obj["width"].toInt(0);
        img.height   = obj["height"].toInt(0);
        return img;
    } else if (type == "table") {
        TableData table;
        table.rows = obj["rows"].toInt(0);
        table.cols = obj["cols"].toInt(0);
        QJsonArray cellsArray = obj["cells"].toArray();
        for (const QJsonValue& rowVal : cellsArray) {
            QList<QString> row;
            QJsonArray rowArr = rowVal.toArray();
            for (const QJsonValue& cellVal : rowArr) {
                row.append(cellVal.toString());
            }
            table.cells.append(row);
        }
        return table;
    } else if (type == "todo") {
        return TodoData{ obj["text"].toString(), obj["checked"].toBool(false) };
    } else if (type == "quote") {
    return QuoteData{ obj["text"].toString() };
    } else if (type == "divider") {
    return DividerData{};
    }

    // Default fallback — paragraph
    return ParagraphData{ obj["text"].toString() };
}

// ----------------------------------------------------------------
// Block constructors
// ----------------------------------------------------------------

Block::Block()
    : m_id(QUuid::createUuid())
    , m_parentId(QUuid())
    , m_orderIndex(0)
    , m_created(QDateTime::currentDateTime())
    , m_updated(QDateTime::currentDateTime())
{
}

Block::Block(QUuid id, QUuid parentId, int orderIndex, BlockData data)
    : m_id(id)
    , m_parentId(parentId)
    , m_orderIndex(orderIndex)
    , m_data(data)
    , m_created(QDateTime::currentDateTime())
    , m_updated(QDateTime::currentDateTime())
{
}

// ----------------------------------------------------------------
// Getters
// ----------------------------------------------------------------

QUuid Block::id() const { return m_id; }
QUuid Block::parentId() const { return m_parentId; }
int Block::orderIndex() const { return m_orderIndex; }
const BlockData& Block::data() const { return m_data; }
QDateTime Block::created() const { return m_created; }
QDateTime Block::updated() const { return m_updated; }

// ----------------------------------------------------------------
// Setters
// ----------------------------------------------------------------

void Block::setParentId(QUuid parentId) { m_parentId = parentId; }
void Block::setOrderIndex(int order) { m_orderIndex = order; }
void Block::setData(const BlockData& data) { m_data = data; updateTimestamp(); }
void Block::updateTimestamp() { m_updated = QDateTime::currentDateTime(); }

// ----------------------------------------------------------------
// Serialization
// ----------------------------------------------------------------

QJsonObject Block::toJson() const
{
    QJsonObject obj;
    obj["id"]         = m_id.toString(QUuid::WithoutBraces);
    obj["parentId"]   = m_parentId.toString(QUuid::WithoutBraces);
    obj["orderIndex"] = m_orderIndex;
    obj["created"]    = m_created.toString(Qt::ISODate);
    obj["updated"]    = m_updated.toString(Qt::ISODate);

    QJsonObject dataObj = blockDataToJson(m_data);
    obj["blockType"] = dataObj["type"];   // top-level type tag for easy querying
    obj["data"]      = dataObj;           // full data object (includes "type" too)

    return obj;
}

Block Block::fromJson(const QJsonObject& obj)
{
    QUuid id       = QUuid::fromString(obj["id"].toString());
    QUuid parentId = QUuid::fromString(obj["parentId"].toString());
    int orderIndex = obj["orderIndex"].toInt(0);

    BlockData data = blockDataFromJson(obj["data"].toObject());

    Block block(id, parentId, orderIndex, data);

    if (obj.contains("created"))
        block.m_created = QDateTime::fromString(obj["created"].toString(), Qt::ISODate);
    if (obj.contains("updated"))
        block.m_updated = QDateTime::fromString(obj["updated"].toString(), Qt::ISODate);

    return block;
}