#ifndef CALENDARREPOSITORY_H
#define CALENDARREPOSITORY_H

#pragma once

#include "core/models/Calendarentry.h"
#include <QByteArray>
#include <QVector>

class CalendarRepository
{
public:
    explicit CalendarRepository(const QByteArray &sessionKey);

    QVector<CalendarEntry> loadAll(bool *ok = nullptr) const;
    bool saveAll(const QVector<CalendarEntry> &entries) const;

    bool addEntry(CalendarEntry entry) const;
    bool updateEntry(const CalendarEntry &entry) const;
    bool deleteEntry(const QString &id) const;

private:
    QByteArray m_sessionKey;
};

#endif // CALENDARREPOSITORY_H