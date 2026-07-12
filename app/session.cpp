#include "session.h"
#include <sodium.h>
#include <QDebug>

Session *Session::instance(){
    static Session s_instance;
    return &s_instance;
}

Session::Session(QObject *parent): QObject(parent), m_keyBuffer(0){

}

void Session::unlock(const QByteArray &key){
    if(key.isEmpty()){
        qWarning() <<"Session unlock: refusing to unlock with empty key";
        return;
    }

    if(m_state == state::unlocked){
        lock();
    }

    m_keyBuffer = SecureBuffer(static_cast<size_t>(key.size()));
    if(!m_keyBuffer.isValid()){
        qCritical() <<"Session unlock : failed to allocate secure memory for session key";
        return;
    }

    std::copy(key.constBegin(), key.constEnd(), m_keyBuffer.data());

    m_state = state::unlocked;
    emit unlocked();
    emit stateChange(m_state);
    qDebug() <<"Session unlocked";
}

void Session::lock(){
    if (m_state == state::locked){
        return;
    }

    m_keyBuffer = SecureBuffer(0);

    m_state = state::locked;
    emit locked();
    emit stateChange(m_state);
    qDebug() <<"Session locked";
}


bool Session::isLocked() const {
    return m_state == state::locked;
}

bool Session::isUnlocked() const{
    return m_state == state::unlocked;
}

Session::state Session::State() const {
    return m_state;
}

QByteArray Session::sessionKey() const{
    if(m_state == state::locked || !m_keyBuffer.isValid())
    {
        return QByteArray();
    }

    return QByteArray(reinterpret_cast<const char *>(m_keyBuffer.data()),
                      static_cast<int>(m_keyBuffer.size()));
}