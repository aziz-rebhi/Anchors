#include "taskentry.h"

QJsonObject TaskEntry::toJson() const
{
    QJsonObject obj;
    obj["id"] = m_id;
    obj["title"] = m_title;
    obj["done"] = m_done;
    obj["dueAt"] = m_dueAt;
    obj["createdAt"] = m_createdAt;
    obj["updatedAt"] = m_updatedAt;
    obj["projectId"] = m_projectId;
    return obj;
}

TaskEntry TaskEntry::fromJson(const QJsonObject &obj)
{
    TaskEntry e;
    e.m_id = obj.value(QStringLiteral("id")).toString();
    e.m_title = obj.value(QStringLiteral("title")).toString();
    e.m_done = obj.value(QStringLiteral("done")).toBool();
    e.m_dueAt = static_cast<qint64>(obj.value(QStringLiteral("dueAt")).toDouble());
    e.m_createdAt = static_cast<qint64>(obj.value(QStringLiteral("createdAt")).toDouble());
    e.m_updatedAt = static_cast<qint64>(obj.value(QStringLiteral("updatedAt")).toDouble());
    e.m_projectId = obj.value(QStringLiteral("projectId")).toString();
    return e;
}