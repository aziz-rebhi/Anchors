#ifndef PROFILE_H
#define PROFILE_H

#pragma once

#include <QString>
#include <QDate>
#include <QJsonObject>


struct profile
{
    QString name;
    QDate birthday;

    QJsonObject toJson() const;
    static profile fromJson(const QJsonObject &obj);
};

#endif // PROFILE_H
