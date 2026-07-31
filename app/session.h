#ifndef SESSION_H
#define SESSION_H

#pragma once

#include "../core/crypto/SecureBuffer.h"
#include <QByteArray>
#include <QObject>


class Session : public QObject
{
    Q_OBJECT
public:
    enum class state { locked, unlocked};
    Q_ENUM(state)

    static Session *instance();

    Session (const Session &) = delete;
    Session &operator = (const Session &) = delete;

    void unlock (const QByteArray &key);

    Q_INVOKABLE void lock();
    Q_INVOKABLE bool isUnlocked() const;
    Q_INVOKABLE bool isLocked() const;

    state State() const;

    QByteArray sessionKey() const;

signals:
    void unlocked();
    void locked();
    void stateChange(Session::state newState);

private:
    explicit Session(QObject *parent = nullptr);

    SecureBuffer m_keyBuffer;
    state m_state = state::locked;

};

#endif // SESSION_H