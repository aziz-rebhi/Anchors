#ifndef VAULTCONTROLLER_H
#define VAULTCONTROLLER_H
#pragma once

#include <QObject>
#include <QString>
#include <QVariantList>

// QML-facing bridge to VaultRepository. Every call re-reads the current
// session key from Session::instance() rather than caching it, so this
// stays correct across lock/unlock cycles without needing to be
// recreated. All methods fail closed (return false / empty) if the
// Session is locked.
class VaultController : public QObject
{
    Q_OBJECT

public:
    explicit VaultController(QObject *parent = nullptr);

    // Returns all entries as a list of maps: id, title, username,
    // password, url, category, createdAt, updatedAt.
    Q_INVOKABLE QVariantList entries() const;

    Q_INVOKABLE bool addEntry(const QString &title, const QString &username,
                               const QString &password, const QString &url,
                               const QString &category);

    Q_INVOKABLE bool updateEntry(const QString &id, const QString &title,
                                  const QString &username, const QString &password,
                                  const QString &url, const QString &category);

    Q_INVOKABLE bool deleteEntry(const QString &id);

    // Convenience wrapper around PasswordGenerator, so QML doesn't need
    // its own binding for it.
    Q_INVOKABLE QString generatePassword(int length = 16, bool useSymbols = true,
                                          bool useNumbers = true) const;

signals:
    void entriesChanged();
    void operationFailed(QString reason);
};

#endif // VAULTCONTROLLER_H
