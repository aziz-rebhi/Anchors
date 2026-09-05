#ifndef CRYPTOMANAGER_H
#define CRYPTOMANAGER_H

#pragma once
#include "SecureBuffer.h"
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

    // Same operations, but reading the key straight out of a guarded
    // SecureBuffer. Pass this instead of sessionKey() on any hot path so
    // the key bytes never get copied into an ordinary (un-guarded) heap
    // buffer via QByteArray.
    static QByteArray encrypt(const QByteArray &plaintext, const SecureBuffer &key);
    static QByteArray decrypt(const QByteArray &nonceAndCipherText, const SecureBuffer &key, bool *ok = nullptr);

private:
    static QByteArray encryptWithKey(const QByteArray &plaintext,
                                     const unsigned char *keyData, size_t keySize);
    static QByteArray decryptWithKey(const QByteArray &nonceAndCiphertext,
                                     const unsigned char *keyData, size_t keySize, bool *ok);
};

#endif // CRYPTOMANAGER_H
