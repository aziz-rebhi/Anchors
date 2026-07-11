#include "profilerepository.h"

#include "../encryptedfilestore.h"
#include "../FilePaths.h"

#include <QJsonDocument>
#include <QJsonObject>

profilerepository::profilerepository(const QByteArray &sessionKey) : m_sessionKey(sessionKey){

}

profile profilerepository::load(bool *ok) const {
    bool loadOk = false;
    QJsonDocument doc = EncryptedFileStore::load(FilePaths::profileFile(), m_sessionKey, &loadOk);
    if (ok) *ok  = loadOk;
    if (!loadOk){
        return profile();
    }
    if (doc.isObject()){
        return profile::fromJson(doc.object());
    }
    return profile();
}

bool profilerepository::save(const profile &profile ) const{
    QJsonDocument doc(profile.toJson());
    return EncryptedFileStore::save(FilePaths::profileFile(), doc, m_sessionKey);
}
