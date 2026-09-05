#include "encryptedfilestore.h"

#include "../crypto/cryptomanager.h"

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonParseError>
#include <QSaveFile>

EncryptedFileStore::EncryptedFileStore() {}

QJsonDocument EncryptedFileStore::load(const QString &filePath,
                                       const QByteArray &sessionKey,
                                       bool *ok)
{
    return loadWithKey(filePath,
                       reinterpret_cast<const unsigned char *>(sessionKey.constData()),
                       static_cast<size_t>(sessionKey.size()),
                       ok);
}

bool EncryptedFileStore::save (const QString &filePath, const QJsonDocument &doc, const QByteArray &sessionKey)
{
    return saveWithKey(filePath, doc,
                       reinterpret_cast<const unsigned char *>(sessionKey.constData()),
                       static_cast<size_t>(sessionKey.size()));
}

QJsonDocument EncryptedFileStore::load(const QString &filePath,
                                       const SecureBuffer &sessionKey,
                                       bool *ok)
{
    if (!sessionKey.isValid())
        return loadWithKey(filePath, nullptr, 0, ok);
    return loadWithKey(filePath, sessionKey.data(), sessionKey.size(), ok);
}

bool EncryptedFileStore::save(const QString &filePath, const QJsonDocument &doc,
                              const SecureBuffer &sessionKey)
{
    if (!sessionKey.isValid())
        return saveWithKey(filePath, doc, nullptr, 0);
    return saveWithKey(filePath, doc, sessionKey.data(), sessionKey.size());
}

QJsonDocument EncryptedFileStore::loadWithKey(const QString &filePath,
                                              const unsigned char *keyData, size_t keySize,
                                              bool *ok)
{
    auto setOk = [ok](bool value) {
        if (ok) *ok = value;
    };

    QFile file(filePath);

    if (!file.exists()) {
        // Expected on first launch, or the first time this particular
        // module is used. Not an error.
        setOk(true);
        return QJsonDocument(QJsonArray());
    }

    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "EncryptedFileStore::load: could not open" << filePath
                   << "-" << file.errorString();
        setOk(false);
        return QJsonDocument();
    }

    QByteArray encrypted = file.readAll();
    file.close();

    bool decryptOk = false;
    // fromRawData wraps the caller's key memory without copying it — the
    // CryptoManager call below only reads the bytes, never writes them.
    const QByteArray keyView = QByteArray::fromRawData(
        reinterpret_cast<const char *>(keyData), static_cast<int>(keySize));
    QByteArray plaintext = CryptoManager::decrypt(encrypted, keyView, &decryptOk);

    if (!decryptOk) {
        // Wrong session key, or the file has been corrupted/tampered
        // with. Either way, fail closed — never hand back partial or
        // unauthenticated data.
        qWarning() << "EncryptedFileStore::load: decryption failed for" << filePath
                   << "(wrong key, or file is corrupted/tampered)";
        setOk(false);
        return QJsonDocument();
    }

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(plaintext, &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        qWarning() << "EncryptedFileStore::load: JSON parse error in" << filePath
                   << "-" << parseError.errorString();
        setOk(false);
        return QJsonDocument();
    }

    setOk(true);
    return doc;
}

bool EncryptedFileStore::saveWithKey(const QString &filePath, const QJsonDocument &doc,
                                     const unsigned char *keyData, size_t keySize)
{
    QByteArray plaintext = doc.toJson(QJsonDocument::Compact);
    const QByteArray keyView = QByteArray::fromRawData(
        reinterpret_cast<const char *>(keyData), static_cast<int>(keySize));
    QByteArray encrypted = CryptoManager::encrypt(plaintext, keyView);

    if (encrypted.isEmpty()) {
        qWarning() << "EncryptedFileStore::save: encryption failed for" << filePath;
        return false;
    }

    // Make sure the parent directory exists before we try to write into it.
    QFileInfo info(filePath);
    QDir dir = info.dir();
    if (!dir.exists() && !dir.mkpath(".")) {
        qWarning() << "EncryptedFileStore::save: could not create directory"
                   << dir.path();
        return false;
    }

    // QSaveFile writes to a temporary file and only replaces the real
    // file on commit() — this is what makes the write atomic. If the app
    // crashes or loses power mid-write, filePath itself is never left in
    // a half-written state.
    QSaveFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "EncryptedFileStore::save: could not open for writing"
                   << filePath << "-" << file.errorString();
        return false;
    }

    qint64 written = file.write(encrypted);
    if (written != encrypted.size()) {
        qWarning() << "EncryptedFileStore::save: incomplete write to" << filePath;
        file.cancelWriting();
        return false;
    }

    if (!file.commit()) {
        qWarning() << "EncryptedFileStore::save: commit failed for" << filePath
                   << "-" << file.errorString();
        return false;
    }

    return true;
}
