#ifndef NOTEREPOSITORY_H
#define NOTEREPOSITORY_H

#pragma once

#include "../../models/noteentry.h"
#include <QByteArray>
#include <QVector>

class NoteRepository
{
public:
    explicit NoteRepository(const QByteArray &sessionKey);

    QVector<NoteEntry> loadAll(bool *ok = nullptr) const ;
    bool saveAll(const QVector<NoteEntry> &entries) const;

    bool addEntry(NoteEntry entry) const ;
    bool updateEntry(const NoteEntry &entry) const ;
    bool deleteEntry(const QString &id) const ;

private:
    QByteArray m_sessionKey;
};

#endif // NOTEREPOSITORY_H
