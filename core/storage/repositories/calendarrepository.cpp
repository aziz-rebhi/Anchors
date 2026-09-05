#include "calendarrepository.h"
#include "../encryptedfilestore.h"
#include "../FilePaths.h"
#include "core/models/Calendarentry.h"
#include <QDateTime>
#include <QJsonArray>
#include <QJsonDocument>
#include <QUuid>
#include <algorithm>

CalendarRepository::CalendarRepository(const SecureBuffer &sessionKey)
    : m_sessionKey(sessionKey)
{
}

QVector<CalendarEntry> CalendarRepository::loadAll(bool *ok) const
{
    bool loadOk = false;
    QJsonDocument doc = EncryptedFileStore::load(FilePaths::calendarFile(), m_sessionKey, &loadOk);

    if (ok) *ok = loadOk;
    if (!loadOk || !doc.isArray()) {
        return {};
    }

    QVector<CalendarEntry> entries;
    const QJsonArray arr = doc.array();
    entries.reserve(arr.size());
    for (const QJsonValue &v : arr) {
        entries.append(CalendarEntry::fromJson(v.toObject()));
    }
    return entries;
}

bool CalendarRepository::saveAll(const QVector<CalendarEntry> &entries) const
{
    QJsonArray arr;
    for (const CalendarEntry &e : entries) {
        arr.append(e.toJson());
    }
    return EncryptedFileStore::save(FilePaths::calendarFile(), QJsonDocument(arr), m_sessionKey);
}

bool CalendarRepository::addEntry(CalendarEntry entry) const
{
    if (entry.m_id.isEmpty()) {
        entry.m_id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    }
    QVector<CalendarEntry> entries = loadAll();
    entries.append(entry);
    return saveAll(entries);
}

bool CalendarRepository::updateEntry(const CalendarEntry &entry) const
{
    QVector<CalendarEntry> entries = loadAll();
    for (CalendarEntry &e : entries) {
        if (e.m_id == entry.m_id) {
            e = entry;
            return saveAll(entries);
        }
    }
    return false;
}

bool CalendarRepository::deleteEntry(const QString &id) const
{
    QVector<CalendarEntry> entries = loadAll();
    const int before = entries.size();
    entries.erase(std::remove_if(entries.begin(), entries.end(),
                                 [&id](const CalendarEntry &e) { return e.m_id == id; }),
                  entries.end());
    if (entries.size() == before) {
        return false;
    }
    return saveAll(entries);
}