#include "profile.h"

QJsonObject profile::toJson() const {
    QJsonObject obj;
    obj["name"]=name;
    obj["birthday"] = birthday.isValid() ? birthday.toString(Qt::ISODate) : QString();
    return obj;
}

profile profile::fromJson(const QJsonObject &obj){
    profile p;
    p.name = obj.value("name").toString();

    const QString birthdayStr = obj.value("birthday").toString();
    p.birthday= birthdayStr.isEmpty() ? QDate() : QDate::fromString(birthdayStr, Qt::ISODate);
    return p;
}