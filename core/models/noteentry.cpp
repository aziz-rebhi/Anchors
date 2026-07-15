#include "noteentry.h"
#include <QUuid>
#include <QDateTime>

QJsonObject NoteEntry::toJson() const {
    QJsonObject obj;
    obj["id"] = m_id;
    obj["title"] = m_title;
    obj["content"] = m_content;
    obj["createdAt"] = m_createdAt;
    obj["UpdatedAt"] = m_updatedAt;
    return obj;
}

NoteEntry NoteEntry::fromJson(const QJsonObject &obj){
    NoteEntry e;
    e.m_id = obj.value("id").toString();
    e.m_title = obj.value("title").toString();
    e.m_content = obj.value("content").toString();
    e.m_createdAt = static_cast<qint64>(obj.value("createdAt").toDouble());
    e.m_updatedAt = static_cast<qint64>(obj.value("updatedAt").toDouble());
    return e;
}