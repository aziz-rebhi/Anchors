#ifndef CRYPTOMANAGER_H
#define CRYPTOMANAGER_H

#pragma once
#include <QByteArray>
#include <QString>
class CryptoManager
{
public:
    CryptoManager();

    static bool init(); //init libsodium
    static QByteArray generateSalt(); //Generate a random salt (crypto_pwhash_saltbytes long)
    static QByteArray deriveKey(const QString &masterPassword, const QByteArray &salt); //create a 32-byte encryption key
    static QByteArray encrypt(const QByteArray &plaintext, const QByteArray &key); //encrypt plaintext with the key
    static QByteArray decrypt(const QByteArray &nonceAndCipherText, const QByteArray &key, bool *ok = nullptr); //Decrypts data produced by encrypt
};

#endif // CRYPTOMANAGER_H
