#pragma once
#include <QUuid>
#include <QDateTime>
#include "blockdata.h"

class Block {
public:
    Block();
    Block(QUuid id, QUuid parentId, int orderIndex, BlockData data);

    QUuid id() const;
    QUuid parentId() const;
    int orderIndex() const;
    const BlockData& data() const;
    QDateTime created() const;
    QDateTime updated() const;

    void setParentId(QUuid parentId);
    void setOrderIndex(int order);
    void setData(const BlockData& data);
    void updateTimestamp();

    // Serialization
    QJsonObject toJson() const;
    static Block fromJson(const QJsonObject& obj);

private:
    QUuid m_id;
    QUuid m_parentId;
    int m_orderIndex = 0;
    BlockData m_data = ParagraphData{""};
    QDateTime m_created;
    QDateTime m_updated;
};