#include "block.h"
#include <QJsonObject>
#include <QJsonArray>

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
            obj["source"] = arg.source;
            obj["caption"] = arg.caption;
            obj["width"] = arg.width;
            obj["height"] = arg.height;
        } else if constexpr (std::is_same_v<T, TableData>) {
            obj["type"] = "table";
            obj["rows"] = arg.rows;
            obj["cols"] = arg.cols;
            QJsonArray cellsArr;
            for (int r = 0; r < arg.cells.size(); ++r) {
                QJsonArray rowArr;
                for (int c = 0; c < arg.cells[r].size(); ++c) {
                    rowArr.append(arg.cells[r][c]);
                }
                cellsArr.append(rowArr);
            }
            obj["cells"] = cellsArr;
        } else if constexpr (std::is_same_v<T, TodoData>) {
            obj["type"] = "todo";
            obj["text"] = arg.text;
            obj["checked"] = arg.checked;
        } else if constexpr (std::is_same_v<T, DividerData>) {
            obj["type"] = "divider";
            obj["orientation"] = arg.orientation;
        } else if constexpr (std::is_same_v<T, QuoteData>) {
            obj["type"] = "quote";
            obj["text"] = arg.text;
        } else if constexpr (std::is_same_v<T, BulletData>) {
            obj["type"] = "bullet";
            obj["text"] = arg.text;
            obj["indent"] = arg.indent;
        } else if constexpr (std::is_same_v<T, CalloutData>) {
            obj["type"] = "callout";
            obj["text"] = arg.text;
            obj["emoji"] = arg.emoji;
        } else if constexpr (std::is_same_v<T, NumberedData>) {
            obj["type"] = "numbered";
            obj["text"] = arg.text;
            obj["indent"] = arg.indent;
        } else if constexpr (std::is_same_v<T, EquationData>) {
            obj["type"] = "equation";
            obj["latex"] = arg.latex;
            obj["displayMode"] = arg.displayMode;
        } else if constexpr (std::is_same_v<T, ToggleData>) {
            obj["type"] = "toggle";
            obj["text"] = arg.text;
            obj["collapsed"] = arg.collapsed;
        }

    }, data);
    return obj;
}

static BlockData blockDataFromJson(const QJsonObject& obj)
{
    QString type = obj["type"].toString();
    if (type == "paragraph") {
        return ParagraphData{obj["text"].toString()};
    } else if (type == "heading") {
        return HeadingData{obj["level"].toInt(1), obj["text"].toString()};
    } else if (type == "code") {
        return CodeData{obj["language"].toString(), obj["code"].toString()};
    } else if (type == "image") {
        return ImageData{obj["source"].toString(), obj["caption"].toString(),
                         obj["width"].toInt(), obj["height"].toInt()};
    } else if (type == "table") {
        TableData td;
        td.rows = obj["rows"].toInt();
        td.cols = obj["cols"].toInt();
        QJsonArray cellsArr = obj["cells"].toArray();
        td.cells.resize(td.rows);
        for (int r = 0; r < cellsArr.size() && r < td.rows; ++r) {
            QJsonArray rowArr = cellsArr[r].toArray();
            td.cells[r].resize(td.cols);
            for (int c = 0; c < rowArr.size() && c < td.cols; ++c) {
                td.cells[r][c] = rowArr[c].toString();
            }
        }
        return td;
    } else if (type == "todo") {
        return TodoData{obj["text"].toString(), obj["checked"].toBool()};
    } else if (type == "divider") {
        return DividerData{obj["orientation"].toInt(0)};
    } else if (type == "quote") {
        return QuoteData{obj["text"].toString()};
    } else if (type == "bullet") {
        return BulletData{obj["text"].toString(), obj["indent"].toInt(0)};
    } else if (type == "callout") {
        return CalloutData{
            obj["text"].toString(),
            obj.contains("emoji") ? obj["emoji"].toString() : QStringLiteral("💡")
        };
    } else if (type == "numbered") {
        return NumberedData{obj["text"].toString(), obj["indent"].toInt(0)};
    } else if (type == "equation") {
        return EquationData{
            obj["latex"].toString(),
            obj["displayMode"].toBool(true)
        };
    } else if (type == "toggle") {
        return ToggleData{
            obj["text"].toString(),
            obj["collapsed"].toBool(false)
        };
    }
    return ParagraphData{""};
}

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

QUuid Block::id() const { return m_id; }
QUuid Block::parentId() const { return m_parentId; }
int Block::orderIndex() const { return m_orderIndex; }
const BlockData& Block::data() const { return m_data; }
BlockData& Block::data() { return m_data; }
QDateTime Block::created() const { return m_created; }
QDateTime Block::updated() const { return m_updated; }

void Block::setParentId(QUuid parentId) { m_parentId = parentId; }
void Block::setOrderIndex(int order) { m_orderIndex = order; }
void Block::setData(const BlockData& data) { m_data = data; updateTimestamp(); }
void Block::updateTimestamp() { m_updated = QDateTime::currentDateTime(); }

QJsonObject Block::toJson() const
{
    QJsonObject obj;
    obj["id"] = m_id.toString(QUuid::WithoutBraces);
    obj["parentId"] = m_parentId.toString(QUuid::WithoutBraces);
    obj["orderIndex"] = m_orderIndex;
    obj["created"] = m_created.toMSecsSinceEpoch();
    obj["updated"] = m_updated.toMSecsSinceEpoch();
    obj["blockType"] = blockDataToJson(m_data)["type"].toString();
    obj["data"] = blockDataToJson(m_data);
    return obj;
}

Block Block::fromJson(const QJsonObject& obj)
{
    QUuid id = QUuid::fromString(obj["id"].toString());
    QUuid parentId = QUuid::fromString(obj["parentId"].toString());
    int orderIndex = obj["orderIndex"].toInt();
    BlockData data = blockDataFromJson(obj["data"].toObject());

    Block block(id, parentId, orderIndex, data);
    return block;
}