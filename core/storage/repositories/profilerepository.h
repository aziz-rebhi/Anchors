#ifndef PROFILEREPOSITORY_H
#define PROFILEREPOSITORY_H

#pragma once

#include "../../models/profile.h"
#include <QByteArray>

class profilerepository
{
public:
    explicit profilerepository(const QByteArray &sessionKeey);
    profile load(bool *ok = nullptr) const ;
    bool save (const profile &profile) const ;
private :
    QByteArray m_sessionKey;
};

#endif // PROFILEREPOSITORY_H
