#ifndef NOTEREPOSITORY_H
#define NOTEREPOSITORY_H

#pragma once

#include "../../models/noteentry.h"
#include "../../crypto/SecureBuffer.h"
#include <QVector>

class NoteRepository
{
public:
    explicit NoteRepository(const SecureBuffer &sessionKey);

    QVector<NoteEntry> loadAll(bool *ok = nullptr) const ;
    bool saveAll(const QVector<NoteEntry> &entries) const;

    bool addEntry(NoteEntry &entry) const ;
    bool updateEntry(const NoteEntry &entry) const ;
    bool deleteEntry(const QString &id) const ;

private:
    const SecureBuffer &m_sessionKey;
};

#endif // NOTEREPOSITORY_H