#ifndef BLOCK_H
#define BLOCK_H

#include "blockdata.h"
#include <QUuid>
#include <QDateTime>
#include <QJsonObject>

class Block {
public:
    Block();
    Block(QUuid id, QUuid parentId, int orderIndex, BlockData data);

    QUuid id() const;
    QUuid parentId() const;
    int orderIndex() const;
    const BlockData& data() const;
    BlockData& data();
    QDateTime created() const;
    QDateTime updated() const;

    void setParentId(QUuid parentId);
    void setOrderIndex(int order);
    void setData(const BlockData& data);
    void updateTimestamp();

    QJsonObject toJson() const;
    static Block fromJson(const QJsonObject& obj);

    int columnIndex() const;
    void setColumnIndex(int index) ;

private:
    QUuid m_id;
    QUuid m_parentId;
    int m_orderIndex;
    BlockData m_data;
    QDateTime m_created;
    QDateTime m_updated;
    int m_columnIndex = 0;
};

#endif // BLOCK_H