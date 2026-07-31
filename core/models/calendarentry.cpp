#include "Calendarentry.h"

QJsonObject CalendarEntry::toJson() const
{
    QJsonObject obj;
    obj["id"] = m_id;
    obj["title"] = m_title;
    obj["description"] = m_description;
    obj["start"] = m_start.toString(Qt::ISODate);
    obj["end"] = m_end.toString(Qt::ISODate);
    obj["allDay"] = m_allDay;
    obj["color"] = m_color.name();
    return obj;
}

CalendarEntry CalendarEntry::fromJson(const QJsonObject &obj)
{
    CalendarEntry e;
    e.m_id = obj.value("id").toString();
    e.m_title = obj.value("title").toString();
    e.m_description = obj.value("description").toString();
    e.m_start = QDateTime::fromString(obj.value("start").toString(), Qt::ISODate);
    e.m_end = QDateTime::fromString(obj.value("end").toString(), Qt::ISODate);
    e.m_allDay = obj.value("allDay").toBool();
    e.m_color = QColor(obj.value("color").toString());
    return e;
}