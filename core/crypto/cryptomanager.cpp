#include "cryptomanager.h"

#include <sodium.h>
#include <QDebug>

bool CryptoManager::init()
{
    // sodium_init() returns 0 on success, 1 if already initialized, -1 on failure.
    if (sodium_init() < 0) {
        qCritical() << "CryptoManager: libsodium failed to initialize";
        return false;
    }
    return true;
}

QByteArray CryptoManager::generateSalt()
{
    QByteArray salt(crypto_pwhash_SALTBYTES, 0);
    randombytes_buf(reinterpret_cast<unsigned char *>(salt.data()), salt.size());
    return salt;
}

QByteArray CryptoManager::deriveKey(const QString &masterPassword, const QByteArray &salt)
{
    if (salt.size() != crypto_pwhash_SALTBYTES) {
        qWarning() << "CryptoManager::deriveKey: invalid salt size, expected"
                   << crypto_pwhash_SALTBYTES << "got" << salt.size();
        return QByteArray();
    }

    QByteArray passwordUtf8 = masterPassword.toUtf8();
    QByteArray key(crypto_secretbox_KEYBYTES, 0);

    // OPSLIMIT_MODERATE / MEMLIMIT_MODERATE: deliberately expensive
    // (roughly ~0.7s and ~256MB on typical hardware) to slow down
    // brute-force attempts against a stolen encrypted file, while
    // staying fast enough for normal unlock UX.
    int result = crypto_pwhash(
        reinterpret_cast<unsigned char *>(key.data()), key.size(),
        passwordUtf8.constData(), static_cast<unsigned long long>(passwordUtf8.size()),
        reinterpret_cast<const unsigned char *>(salt.constData()),
        crypto_pwhash_OPSLIMIT_MODERATE,
        crypto_pwhash_MEMLIMIT_MODERATE,
        crypto_pwhash_ALG_ARGON2ID13);

    // Wipe our local copy of the plaintext password from memory now that
    // we're done with it — don't rely on Qt/the OS to do this for us.
    sodium_memzero(passwordUtf8.data(), static_cast<size_t>(passwordUtf8.size()));

    if (result != 0) {
        qCritical() << "CryptoManager::deriveKey: crypto_pwhash failed"
                       "(likely out of memory for the given MEMLIMIT)";
        return QByteArray();
    }

    return key;
}

QByteArray CryptoManager::encrypt(const QByteArray &plaintext, const QByteArray &key)
{
    if (key.size() != crypto_secretbox_KEYBYTES) {
        qWarning() << "CryptoManager::encrypt: invalid key size";
        return QByteArray();
    }

    QByteArray nonce(crypto_secretbox_NONCEBYTES, 0);
    randombytes_buf(reinterpret_cast<unsigned char *>(nonce.data()), nonce.size());

    QByteArray ciphertext(plaintext.size() + crypto_secretbox_MACBYTES, 0);

    crypto_secretbox_easy(
        reinterpret_cast<unsigned char *>(ciphertext.data()),
        reinterpret_cast<const unsigned char *>(plaintext.constData()),
        static_cast<unsigned long long>(plaintext.size()),
        reinterpret_cast<const unsigned char *>(nonce.constData()),
        reinterpret_cast<const unsigned char *>(key.constData()));

    return nonce + ciphertext;
}

QByteArray CryptoManager::decrypt(const QByteArray &nonceAndCiphertext,
                                  const QByteArray &key,
                                  bool *ok)
{
    auto fail = [ok]() {
        if (ok) *ok = false;
        return QByteArray();
    };

    if (key.size() != crypto_secretbox_KEYBYTES) {
        qWarning() << "CryptoManager::decrypt: invalid key size";
        return fail();
    }

    const int minSize = crypto_secretbox_NONCEBYTES + crypto_secretbox_MACBYTES;
    if (nonceAndCiphertext.size() < minSize) {
        qWarning() << "CryptoManager::decrypt: data too short to be valid"
                      "(possibly corrupted file)";
        return fail();
    }

    QByteArray nonce = nonceAndCiphertext.left(crypto_secretbox_NONCEBYTES);
    QByteArray ciphertext = nonceAndCiphertext.mid(crypto_secretbox_NONCEBYTES);

    QByteArray plaintext(ciphertext.size() - crypto_secretbox_MACBYTES, 0);

    int result = crypto_secretbox_open_easy(
        reinterpret_cast<unsigned char *>(plaintext.data()),
        reinterpret_cast<const unsigned char *>(ciphertext.constData()),
        static_cast<unsigned long long>(ciphertext.size()),
        reinterpret_cast<const unsigned char *>(nonce.constData()),
        reinterpret_cast<const unsigned char *>(key.constData()));

    if (result != 0) {
        // Authentication failed: either the key is wrong, or the
        // ciphertext was tampered with. We deliberately don't
        // distinguish between these cases in the return value —
        // both must fail closed the same way.
        return fail();
    }

    if (ok) *ok = true;
    return plaintext;
}


CryptoManager::CryptoManager() {}
