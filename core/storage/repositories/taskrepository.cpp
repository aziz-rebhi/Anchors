#include "taskrepository.h"
#include "core/storage/encryptedfilestore.h"
#include "core/storage/FilePaths.h"
#include <QDateTime>
#include <QJsonArray>
#include <QJsonDocument>
#include <QUuid>
#include <algorithm>

TaskRepository::TaskRepository(const QByteArray &sessionKey)
    : m_sessionKey(sessionKey)
{
}

QVector<TaskEntry> TaskRepository::loadAll(bool *ok) const
{
    bool loadOk = false;
    QJsonDocument doc = EncryptedFileStore::load(FilePaths::tasksFile(), m_sessionKey, &loadOk);

    if (ok) *ok = loadOk;
    if (!loadOk || !doc.isArray()) {
        return {};
    }

    QVector<TaskEntry> entries;
    const QJsonArray arr = doc.array();
    entries.reserve(arr.size());
    for (const QJsonValue &v : arr) {
        entries.append(TaskEntry::fromJson(v.toObject()));
    }
    return entries;
}

bool TaskRepository::saveAll(const QVector<TaskEntry> &entries) const
{
    QJsonArray arr;
    for (const TaskEntry &e : entries) {
        arr.append(e.toJson());
    }
    return EncryptedFileStore::save(FilePaths::tasksFile(), QJsonDocument(arr), m_sessionKey);
}

bool TaskRepository::addEntry(TaskEntry entry) const
{
    if (entry.m_id.isEmpty()) {
        entry.m_id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    }
    const qint64 now = QDateTime::currentSecsSinceEpoch();
    entry.m_createdAt = now;
    entry.m_updatedAt = now;

    QVector<TaskEntry> entries = loadAll();
    entries.append(entry);
    return saveAll(entries);
}

bool TaskRepository::updateEntry(const TaskEntry &entry) const
{
    QVector<TaskEntry> entries = loadAll();
    for (TaskEntry &e : entries) {
        if (e.m_id == entry.m_id) {
            e = entry;
            e.m_updatedAt = QDateTime::currentSecsSinceEpoch();
            return saveAll(entries);
        }
    }
    return false;
}

bool TaskRepository::deleteEntry(const QString &id) const
{
    QVector<TaskEntry> entries = loadAll();
    const int before = entries.size();
    entries.erase(std::remove_if(entries.begin(), entries.end(),
                                 [&id](const TaskEntry &e) { return e.m_id == id; }),
                  entries.end());
    if (entries.size() == before) {
        return false;
    }
    return saveAll(entries);
}

bool TaskRepository::setDone(const QString &id, bool done) const
{
    QVector<TaskEntry> entries = loadAll();
    for (TaskEntry &e : entries) {
        if (e.m_id == id) {
            e.m_done = done;
            e.m_updatedAt = QDateTime::currentSecsSinceEpoch();
            return saveAll(entries);
        }
    }
    return false;
}
