#include "noterepository.h"
#include "../../core/storage/encryptedfilestore.h"
#include "../../core/storage/FilePaths.h"
#include <QDateTime>
#include <QJsonArray>
#include <QJsonDocument>
#include <QUuid>

NoteRepository::NoteRepository(const QByteArray &sessionKey)
    : m_sessionKey(sessionKey)
{
}

QVector<NoteEntry> NoteRepository::loadAll(bool *ok) const
{
    bool loadOk = false;
    QJsonDocument doc = EncryptedFileStore::load(FilePaths::notesFile(), m_sessionKey, &loadOk);

    if (ok) *ok = loadOk;
    if (!loadOk || !doc.isArray()) {
        return {};
    }

    QVector<NoteEntry> entries;
    const QJsonArray arr = doc.array();
    entries.reserve(arr.size());
    for (const QJsonValue &v : arr) {
        entries.append(NoteEntry::fromJson(v.toObject()));
    }
    return entries;
}

bool NoteRepository::saveAll(const QVector<NoteEntry> &entries) const
{
    QJsonArray arr;
    for (const NoteEntry &e : entries) {
        arr.append(e.toJson());
    }
    return EncryptedFileStore::save(FilePaths::notesFile(), QJsonDocument(arr), m_sessionKey);
}

bool NoteRepository::addEntry(NoteEntry entry) const
{
    if (entry.m_id.isEmpty()) {
        entry.m_id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    }
    const qint64 now = QDateTime::currentSecsSinceEpoch();
    entry.m_createdAt = now;
    entry.m_updatedAt = now;

    QVector<NoteEntry> entries = loadAll();
    entries.append(entry);
    return saveAll(entries);
}

bool NoteRepository::updateEntry(const NoteEntry &entry) const
{
    QVector<NoteEntry> entries = loadAll();
    for (NoteEntry &e : entries) {
        if (e.m_id == entry.m_id) {
            e = entry;
            e.m_updatedAt = QDateTime::currentSecsSinceEpoch();
            return saveAll(entries);
        }
    }
    return false;
}

bool NoteRepository::deleteEntry(const QString &id) const
{
    QVector<NoteEntry> entries = loadAll();
    const int before = entries.size();
    entries.removeIf([&id](const NoteEntry &e) { return e.m_id == id; });
    if (entries.size() == before) {
        return false;
    }
    return saveAll(entries);
}