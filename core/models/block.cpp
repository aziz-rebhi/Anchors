#include "block.h"
#include <QJsonObject>
#include <QJsonDocument>

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
    // TODO: serialize block data
    return obj;
}

Block Block::fromJson(const QJsonObject& obj)
{
    // Stub
    return Block();
}