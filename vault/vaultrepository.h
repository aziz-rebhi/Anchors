#ifndef VAULTREPOSITORY_H
#define VAULTREPOSITORY_H

#pragma once

#include "vaultentry.h"
#include <QByteArray>
#include <QVector>

class VaultRepository
{
public:
    explicit VaultRepository(const QByteArray &sessionKey);

    QVector <VaultEntry> loadAll(bool *ok = nullptr ) const ;

    bool saveAll(const QVector<VaultEntry> &entries) const;

    bool addEntry(VaultEntry entry) const;
    bool updateEntry(const VaultEntry &entry) const;
    bool deleteEntry(const QString &id) const;

private:
    QByteArray m_sessionKey;
};

#endif // VAULTREPOSITORY_H
