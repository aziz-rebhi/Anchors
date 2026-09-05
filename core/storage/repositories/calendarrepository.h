#ifndef CALENDARREPOSITORY_H
#define CALENDARREPOSITORY_H

#pragma once

#include "core/models/Calendarentry.h"
#include "core/crypto/SecureBuffer.h"
#include <QVector>

class CalendarRepository
{
public:
    explicit CalendarRepository(const SecureBuffer &sessionKey);

    QVector<CalendarEntry> loadAll(bool *ok = nullptr) const;
    bool saveAll(const QVector<CalendarEntry> &entries) const;

    bool addEntry(CalendarEntry entry) const;
    bool updateEntry(const CalendarEntry &entry) const;
    bool deleteEntry(const QString &id) const;

private:
    const SecureBuffer &m_sessionKey;
};

#endif // CALENDARREPOSITORY_H