#ifndef CALENDARCONTROLLER_H
#define CALENDARCONTROLLER_H
#pragma once

#include <QObject>
#include <QString>
#include <QVariantList>

// QML-facing bridge to CalendarRepository. Dates cross the QML boundary
// as ISO 8601 strings (Qt::ISODate) since QML's Date <-> QDateTime
// marshalling can be lossy around timezones; the QML side is expected to
// format/parse via Date / toISOString().
class CalendarController : public QObject
{
    Q_OBJECT

public:
    explicit CalendarController(QObject *parent = nullptr);

    // Returns all events as a list of maps: id, title, description,
    // start (ISO string), end (ISO string), allDay, color (name string).
    Q_INVOKABLE QVariantList entries() const;

    Q_INVOKABLE bool addEntry(const QString &title, const QString &description,
                              const QString &startIso, const QString &endIso,
                              bool allDay, const QString &colorName);

    Q_INVOKABLE bool updateEntry(const QString &id, const QString &title,
                                 const QString &description, const QString &startIso,
                                 const QString &endIso, bool allDay, const QString &colorName);

    Q_INVOKABLE bool deleteEntry(const QString &id);

signals:
    void entriesChanged();
    void operationFailed(QString reason);
};

#endif // CALENDARCONTROLLER_H