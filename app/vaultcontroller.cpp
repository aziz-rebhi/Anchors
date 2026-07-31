#include "vaultcontroller.h"
#include "session.h"

#include "../core/models/vaultentry.h"
#include "../core/storage/repositories/vaultrepository.h"
#include "../core/security/passwordgenerator.h"

#include <QVariantMap>

VaultController::VaultController(QObject *parent) : QObject(parent)
{
}

QVariantList VaultController::entries() const
{
    QVariantList list;

    if (!Session::instance()->isUnlocked()) {
        return list;
    }

    VaultRepository repo(Session::instance()->sessionKey());
    bool ok = false;
    const QVector<VaultEntry> all = repo.loadAll(&ok);
    if (!ok) {
        return list;
    }

    list.reserve(all.size());
    for (const VaultEntry &e : all) {
        QVariantMap m;
        m["id"] = e.id;
        m["title"] = e.title;
        m["username"] = e.username;
        m["password"] = e.password;
        m["url"] = e.url;
        m["category"] = e.category;
        m["createdAt"] = e.createdAt;
        m["updatedAt"] = e.updatedAt;
        list.append(m);
    }
    return list;
}

bool VaultController::addEntry(const QString &title, const QString &username,
                                const QString &password, const QString &url,
                                const QString &category)
{
    if (!Session::instance()->isUnlocked()) {
        emit operationFailed(QStringLiteral("Vault is locked."));
        return false;
    }

    VaultRepository repo(Session::instance()->sessionKey());
    VaultEntry e;
    e.title = title;
    e.username = username;
    e.password = password;
    e.url = url;
    e.category = category;

    const bool ok = repo.addEntry(e);
    if (ok) {
        emit entriesChanged();
    } else {
        emit operationFailed(QStringLiteral("Could not save the entry."));
    }
    return ok;
}

bool VaultController::updateEntry(const QString &id, const QString &title,
                                   const QString &username, const QString &password,
                                   const QString &url, const QString &category)
{
    if (!Session::instance()->isUnlocked()) {
        emit operationFailed(QStringLiteral("Vault is locked."));
        return false;
    }

    VaultRepository repo(Session::instance()->sessionKey());
    VaultEntry e;
    e.id = id;
    e.title = title;
    e.username = username;
    e.password = password;
    e.url = url;
    e.category = category;

    const bool ok = repo.updateEntry(e);
    if (ok) {
        emit entriesChanged();
    } else {
        emit operationFailed(QStringLiteral("Could not update the entry."));
    }
    return ok;
}

bool VaultController::deleteEntry(const QString &id)
{
    if (!Session::instance()->isUnlocked()) {
        emit operationFailed(QStringLiteral("Vault is locked."));
        return false;
    }

    VaultRepository repo(Session::instance()->sessionKey());
    const bool ok = repo.deleteEntry(id);
    if (ok) {
        emit entriesChanged();
    } else {
        emit operationFailed(QStringLiteral("Could not delete the entry."));
    }
    return ok;
}

QString VaultController::generatePassword(int length, bool useSymbols, bool useNumbers) const
{
    return PasswordGenerator::generate(length, useSymbols, useNumbers);
}
