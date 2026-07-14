#ifndef VAULTENTRY_H
#define VAULTENTRY_H

#pragma once

#include <QString>
#include <QJsonObject>

class VaultEntry
{
    QString id;
    QString title;
    QString username;
    QString password;
    QString url;
    QString category;
    qint64 createdAt = 0;
    qint64 updatedAt = 0;

    friend class VaultRepository;
    friend class VaultEntryDialog;
    friend class VaultPage;

    QJsonObject toJson() const;
    static VaultEntry fromJson(const QJsonObject &obj);
};

#endif // VAULTENTRY_H
