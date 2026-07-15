#include "vaultentry.h"

QJsonObject VaultEntry::toJson() const {
    QJsonObject obj;
    obj["id"] = id;
    obj["title"] = title;
    obj["username"] = username;
    obj ["password"] = password;
    obj["url"] = url;
    obj["category"] = category;
    obj["createdAt"] = createdAt;
    obj["updatedAt"] = updatedAt;
    return obj;
}

VaultEntry VaultEntry::fromJson(const QJsonObject &obj){
    VaultEntry e;
    e.id = obj.value("id").toString();
    e.title = obj.value("title").toString();
    e.username = obj.value("username").toString();
    e.password = obj.value("password").toString();
    e.url = obj.value("url").toString();
    e.category = obj.value("category").toString();
    e.createdAt = static_cast<qint64>(obj.value("createdAt").toDouble());
    e.updatedAt = static_cast<qint64>(obj.value("updatedAt").toDouble());
    return e;
}
