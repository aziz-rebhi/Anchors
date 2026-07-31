#include "authcontroller.h"
#include "session.h"

#include "../core/crypto/cryptomanager.h"
#include "../core/storage/saltstore.h"
#include "../core/storage/FilePaths.h"
#include "../core/storage/encryptedfilestore.h"
#include "../core/storage/repositories/profilerepository.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>
#include <sodium.h>
namespace {
// Fixed plaintext we encrypt on account creation and try to decrypt on
// every later unlock attempt. crypto_secretbox authenticates on decrypt,
// so a wrong key or a tampered file both fail closed - we don't need to
// compare the recovered string, but we do anyway as a cheap sanity check
// against a future format change.
constexpr auto kVerifyMarker = "ANCHORS_VERIFY_V1";
}

AuthController::AuthController(QObject *parent) : QObject(parent)
{
}

bool AuthController::isFirstRun() const
{
    return !SaltStore::exists();
}

bool AuthController::createAccount(const QString &name, const QString &password)
{
    if (password.isEmpty()) {
        emit accountCreationFailed(QStringLiteral("Password cannot be empty."));
        return false;
    }

    if (SaltStore::exists()) {
        qWarning() << "AuthController::createAccount: an account already exists";
        emit accountCreationFailed(QStringLiteral("An account already exists on this device."));
        return false;
    }

    // 1. Generate salt in memory only – don't write to disk yet
    const QByteArray salt = CryptoManager::generateSalt();
    if (salt.isEmpty()) {
        emit accountCreationFailed(QStringLiteral("Could not generate secure salt."));
        return false;
    }

    // 2. Derive the session key from the password and salt
    const QByteArray key = CryptoManager::deriveKey(password, salt);
    if (key.isEmpty()) {
        emit accountCreationFailed(QStringLiteral("Could not derive encryption key."));
        return false;
    }

    // 3. Write the verification file
    QJsonObject verifyObj;
    verifyObj["v"] = QString::fromUtf8(kVerifyMarker);
    bool verifySaved = EncryptedFileStore::save(
        FilePaths::verifyFile(),
        QJsonDocument(verifyObj),
        key
        );

    // 4. Write the profile file
    profile p;
    p.name = name;
    profilerepository repo(key);
    bool profileSaved = repo.save(p);

    // 5. If EITHER file failed to write, clean up and abort
    if (!verifySaved || !profileSaved) {
        // Remove any partial files so the device isn't left in a half-created state
        QFile::remove(FilePaths::verifyFile());
        QFile::remove(FilePaths::profileFile());

        emit accountCreationFailed(
            verifySaved ? QStringLiteral("Could not save profile.")
                        : QStringLiteral("Could not write verification file.")
            );
        return false;
    }

    // 6. ONLY NOW – persist the salt to disk. If this fails, roll back.
    if (!SaltStore::save(salt)) {
        QFile::remove(FilePaths::verifyFile());
        QFile::remove(FilePaths::profileFile());
        emit accountCreationFailed(QStringLiteral("Could not initialize secure storage."));
        return false;
    }

    // 7. Success – unlock the session and notify
    Session::instance()->unlock(key);
    emit accountCreated();
    return true;
}
bool AuthController::tryUnlock(const QString &password)
{
    if (!SaltStore::exists()) {
        qWarning() << "AuthController::tryUnlock: no account exists yet";
        emit unlockFailed();
        return false;
    }

    const QByteArray salt = SaltStore::load();
    if (salt.isEmpty()) {
        emit unlockFailed();
        return false;
    }

    const QByteArray key = CryptoManager::deriveKey(password, salt);
    if (key.isEmpty()) {
        emit unlockFailed();
        return false;
    }

    bool ok = false;
    const QJsonDocument doc = EncryptedFileStore::load(FilePaths::verifyFile(), key, &ok);

    const bool markerMatches = ok && doc.isObject()
                               && doc.object().value("v").toString() == QString::fromUtf8(kVerifyMarker);

    if (!markerMatches) {
        // Wrong password, or verify.enc is missing/corrupted. Don't
        // distinguish - same failure path either way.
        emit unlockFailed();
        return false;
    }

    Session::instance()->unlock(key);
    emit unlockSucceeded();
    return true;
}

QString AuthController::currentUserName() const
{
    if (!Session::instance()->isUnlocked()) {
        return QString();
    }

    profilerepository repo(Session::instance()->sessionKey());
    bool ok = false;
    const profile p = repo.load(&ok);
    return ok ? p.name : QString();
}


bool AuthController::hashVaultPin() const
{
    if (!Session::instance()->isUnlocked()) {
        return false;
    }

    const QByteArray key = Session::instance()->sessionKey();
    bool ok = false;
    QJsonDocument doc = EncryptedFileStore::load(FilePaths::pinFile(), key, &ok);

    // If the file exists and contains both a salt and a hash, a PIN is set.
    if (!ok || !doc.isObject()) {
        return false;
    }
    QJsonObject obj = doc.object();
    return obj.contains("salt") && obj.contains("hash");
}

bool AuthController::setVaultPin(const QString &pin)
{
    if (!Session::instance()->isUnlocked()) {
        qWarning() << "setVaultPin: Session is locked";
        return false;
    }

    // Enforce a minimum PIN length (4 digits is typical)
    if (pin.length() < 4) {
        qWarning() << "setVaultPin: PIN must be at least 4 characters";
        return false;
    }

    // 1. Generate a random salt (same size as used for master password)
    const QByteArray salt = CryptoManager::generateSalt();
    if (salt.size() != crypto_pwhash_SALTBYTES) {
        return false;
    }

    // 2. Convert PIN to UTF-8 and hash it using Argon2id (fast interactive variant)
    const QByteArray pinUtf8 = pin.toUtf8();
    QByteArray hash(crypto_generichash_BYTES, 0); // 32 bytes

    // Use INTERACTIVE limits – PIN verification must feel instant
    int result = crypto_pwhash(
        reinterpret_cast<unsigned char *>(hash.data()),
        static_cast<unsigned long long>(hash.size()),
        pinUtf8.constData(),
        static_cast<unsigned long long>(pinUtf8.size()),
        reinterpret_cast<const unsigned char *>(salt.constData()),
        crypto_pwhash_OPSLIMIT_INTERACTIVE,
        crypto_pwhash_MEMLIMIT_INTERACTIVE,
        crypto_pwhash_ALG_DEFAULT
        );

    // Wipe the plaintext PIN from memory immediately
    sodium_memzero(const_cast<char *>(pinUtf8.constData()), static_cast<size_t>(pinUtf8.size()));

    if (result != 0) {
        qCritical() << "setVaultPin: crypto_pwhash_raw failed";
        return false;
    }

    // 3. Build JSON payload: salt + hash (stored as Base64 strings)
    QJsonObject obj;
    obj["salt"] = QString::fromUtf8(salt.toBase64());
    obj["hash"] = QString::fromUtf8(hash.toBase64());

    // 4. Save this payload inside pin.enc (encrypted with the master key)
    const QByteArray key = Session::instance()->sessionKey();
    return EncryptedFileStore::save(FilePaths::pinFile(), QJsonDocument(obj), key);
}

bool AuthController::verifyVaultPin(const QString &pin) const
{
    if (!Session::instance()->isUnlocked()) {
        return false;
    }

    // 1. Load the encrypted PIN file
    const QByteArray key = Session::instance()->sessionKey();
    bool ok = false;
    QJsonDocument doc = EncryptedFileStore::load(FilePaths::pinFile(), key, &ok);
    if (!ok || !doc.isObject()) {
        return false;
    }

    QJsonObject obj = doc.object();
    QByteArray salt = QByteArray::fromBase64(obj.value("salt").toString().toUtf8());
    QByteArray storedHash = QByteArray::fromBase64(obj.value("hash").toString().toUtf8());

    if (salt.size() != crypto_pwhash_SALTBYTES || storedHash.size() != crypto_generichash_BYTES) {
        qWarning() << "verifyVaultPin: corrupt PIN file (wrong salt/hash size)";
        return false;
    }

    // 2. Hash the entered PIN with the same salt
    const QByteArray pinUtf8 = pin.toUtf8();
    QByteArray hash(crypto_generichash_BYTES, 0);

    int result = crypto_pwhash(
        reinterpret_cast<unsigned char *>(hash.data()),
        static_cast<unsigned long long>(hash.size()),
        pinUtf8.constData(),
        static_cast<unsigned long long>(pinUtf8.size()),
        reinterpret_cast<const unsigned char *>(salt.constData()),
        crypto_pwhash_OPSLIMIT_INTERACTIVE,
        crypto_pwhash_MEMLIMIT_INTERACTIVE,
        crypto_pwhash_ALG_DEFAULT
        );

    sodium_memzero(const_cast<char *>(pinUtf8.constData()), static_cast<size_t>(pinUtf8.size()));

    if (result != 0) {
        return false;
    }

    // 3. Constant‑time comparison to avoid timing side‑channels
    return sodium_memcmp(hash.constData(), storedHash.constData(), hash.size()) == 0;
}