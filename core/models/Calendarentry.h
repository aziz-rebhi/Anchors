#ifndef CALENDARENTRY_H
#define CALENDARENTRY_H

#pragma once

#include <QString>
#include <QDateTime>
#include <QJsonObject>
#include <QColor>

class CalendarEntry
{
    QString m_id;
    QString m_title;
    QString m_description;
    QDateTime m_start;
    QDateTime m_end;
    bool m_allDay = false;
    QColor m_color = Qt::blue;

    friend class CalendarRepository;
    friend class CalendarPage;
    friend class CalendarController;

public:
    CalendarEntry() = default;

    QJsonObject toJson() const;
    static CalendarEntry fromJson(const QJsonObject &obj);

    // Getters
    QString id() const { return m_id; }
    QString title() const { return m_title; }
    QString description() const { return m_description; }
    QDateTime start() const { return m_start; }
    QDateTime end() const { return m_end; }
    bool allDay() const { return m_allDay; }
    QColor color() const { return m_color; }
};

#endif // CALENDARENTRY_H