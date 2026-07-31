#include "taskentry.h"

QJsonObject TaskEntry::toJson() const {
    QJsonObject obj;
    obj["id"] = m_id;
    obj["title"] = m_title;
    obj["done"] = m_done;
    obj["dueAt"] = m_dueAt;
    obj["createdAt"] = m_createdAt;
    obj["updatedAt"] = m_updatedAt;
    return obj;
}

TaskEntry TaskEntry::fromJson(const QJsonObject &obj) {
    TaskEntry e;
    e.m_id = obj.value("id").toString();
    e.m_title = obj.value("title").toString();
    e.m_done = obj.value("done").toBool();
    e.m_dueAt = static_cast<qint64>(obj.value("dueAt").toDouble());
    e.m_createdAt = static_cast<qint64>(obj.value("createdAt").toDouble());
    e.m_updatedAt = static_cast<qint64>(obj.value("updatedAt").toDouble());
    return e;
}
