#include "projectentry.h"

QJsonObject ProjectEntry::toJson() const
{
    QJsonObject obj;
    obj["id"] = m_id;
    obj["name"] = m_name;
    obj["emoji"] = m_emoji;
    obj["color"] = m_color;
    obj["order"] = m_order;
    obj["createdAt"] = m_createdAt;
    obj["updatedAt"] = m_updatedAt;
    return obj;
}

ProjectEntry ProjectEntry::fromJson(const QJsonObject &obj)
{
    ProjectEntry e;
    e.m_id = obj.value(QStringLiteral("id")).toString();
    e.m_name = obj.value(QStringLiteral("name")).toString();
    e.m_emoji = obj.value(QStringLiteral("emoji")).toString();
    e.m_color = obj.value(QStringLiteral("color")).toString(QStringLiteral("#89b4fa"));
    e.m_order = obj.value(QStringLiteral("order")).toInt();
    e.m_createdAt = static_cast<qint64>(obj.value(QStringLiteral("createdAt")).toDouble());
    e.m_updatedAt = static_cast<qint64>(obj.value(QStringLiteral("updatedAt")).toDouble());
    return e;
}