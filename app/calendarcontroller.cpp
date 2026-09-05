#include "calendarcontroller.h"
#include "core/models/Calendarentry.h"
#include "core/storage/repositories/calendarrepository.h"
#include "session.h"


#include <QDateTime>
#include <QVariantMap>

CalendarController::CalendarController(QObject *parent) : QObject(parent)
{
}

QVariantList CalendarController::entries() const
{
    QVariantList list;

    if (!Session::instance()->isUnlocked()) {
        return list;
    }

    CalendarRepository repo(Session::instance()->secureKey());
    bool ok = false;
    const QVector<CalendarEntry> all = repo.loadAll(&ok);
    if (!ok) {
        return list;
    }

    list.reserve(all.size());
    for (const CalendarEntry &e : all) {
        QVariantMap m;
        m["id"] = e.id();
        m["title"] = e.title();
        m["description"] = e.description();
        m["start"] = e.start().toString(Qt::ISODate);
        m["end"] = e.end().toString(Qt::ISODate);
        m["allDay"] = e.allDay();
        m["color"] = e.color().name();
        list.append(m);
    }
    return list;
}

bool CalendarController::addEntry(const QString &title, const QString &description,
                                  const QString &startIso, const QString &endIso,
                                  bool allDay, const QString &colorName)
{
    if (!Session::instance()->isUnlocked()) {
        emit operationFailed(QStringLiteral("Calendar is locked."));
        return false;
    }

    CalendarRepository repo(Session::instance()->secureKey());
    CalendarEntry e;
    e.m_title = title;
    e.m_description = description;
    e.m_start = QDateTime::fromString(startIso, Qt::ISODate);
    e.m_end = QDateTime::fromString(endIso, Qt::ISODate);
    e.m_allDay = allDay;
    e.m_color = QColor(colorName);

    const bool ok = repo.addEntry(e);
    if (ok) {
        emit entriesChanged();
    } else {
        emit operationFailed(QStringLiteral("Could not save the event."));
    }
    return ok;
}

bool CalendarController::updateEntry(const QString &id, const QString &title,
                                     const QString &description, const QString &startIso,
                                     const QString &endIso, bool allDay, const QString &colorName)
{
    if (!Session::instance()->isUnlocked()) {
        emit operationFailed(QStringLiteral("Calendar is locked."));
        return false;
    }

    CalendarRepository repo(Session::instance()->secureKey());
    CalendarEntry e;
    e.m_id = id;
    e.m_title = title;
    e.m_description = description;
    e.m_start = QDateTime::fromString(startIso, Qt::ISODate);
    e.m_end = QDateTime::fromString(endIso, Qt::ISODate);
    e.m_allDay = allDay;
    e.m_color = QColor(colorName);

    const bool ok = repo.updateEntry(e);
    if (ok) {
        emit entriesChanged();
    } else {
        emit operationFailed(QStringLiteral("Could not update the event."));
    }
    return ok;
}

bool CalendarController::deleteEntry(const QString &id)
{
    if (!Session::instance()->isUnlocked()) {
        emit operationFailed(QStringLiteral("Calendar is locked."));
        return false;
    }

    CalendarRepository repo(Session::instance()->secureKey());
    const bool ok = repo.deleteEntry(id);
    if (ok) {
        emit entriesChanged();
    } else {
        emit operationFailed(QStringLiteral("Could not delete the event."));
    }
    return ok;
}