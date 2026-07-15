#include "vaultrepository.h"

#include "../../storage/encryptedfilestore.h"
#include "../../storage/FilePaths.h"

#include <QDateTime>
#include <QJsonArray>
#include <QJsonDocument>
#include <QUuid>

VaultRepository::VaultRepository(const QByteArray &sessionKey)
    : m_sessionKey(sessionKey)
{
}

QVector<VaultEntry> VaultRepository::loadAll(bool *ok) const
{
    bool loadOk = false;
    QJsonDocument doc = EncryptedFileStore::load(FilePaths::vaultFile(), m_sessionKey, &loadOk);

    if (ok) *ok = loadOk;
    if (!loadOk || !doc.isArray()) {
        return {};
    }

    QVector<VaultEntry> entries;
    const QJsonArray arr = doc.array();
    entries.reserve(arr.size());
    for (const QJsonValue &v : arr) {
        entries.append(VaultEntry::fromJson(v.toObject()));
    }
    return entries;
}

bool VaultRepository::saveAll(const QVector<VaultEntry> &entries) const
{
    QJsonArray arr;
    for (const VaultEntry &e : entries) {
        arr.append(e.toJson());
    }
    return EncryptedFileStore::save(FilePaths::vaultFile(), QJsonDocument(arr), m_sessionKey);
}

bool VaultRepository::addEntry(VaultEntry entry) const
{
    if (entry.id.isEmpty()) {
        entry.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    }
    const qint64 now = QDateTime::currentSecsSinceEpoch();
    entry.createdAt = now;
    entry.updatedAt = now;

    QVector<VaultEntry> entries = loadAll();
    entries.append(entry);
    return saveAll(entries);
}

bool VaultRepository::updateEntry(const VaultEntry &entry) const
{
    QVector<VaultEntry> entries = loadAll();
    for (VaultEntry &e : entries) {
        if (e.id == entry.id) {
            e = entry;
            e.updatedAt = QDateTime::currentSecsSinceEpoch();
            return saveAll(entries);
        }
    }
    return false; // no entry with that id existed
}

bool VaultRepository::deleteEntry(const QString &id) const
{
    QVector<VaultEntry> entries = loadAll();
    const int before = entries.size();
    entries.removeIf([&id](const VaultEntry &e) { return e.id == id; });
    if (entries.size() == before) {
        return false; // nothing was removed
    }
    return saveAll(entries);
}