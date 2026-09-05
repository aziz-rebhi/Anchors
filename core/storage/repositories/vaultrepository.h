#ifndef VAULTREPOSITORY_H
#define VAULTREPOSITORY_H

#pragma once

#include "../../models/vaultentry.h"
#include "../../crypto/SecureBuffer.h"
#include <QVector>

class VaultRepository
{
public:
    explicit VaultRepository(const SecureBuffer &sessionKey);

    QVector <VaultEntry> loadAll(bool *ok = nullptr ) const ;

    bool saveAll(const QVector<VaultEntry> &entries) const;

    bool addEntry(VaultEntry entry) const;
    bool updateEntry(const VaultEntry &entry) const;
    bool deleteEntry(const QString &id) const;

private:
    const SecureBuffer &m_sessionKey;
};

#endif // VAULTREPOSITORY_H
