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
    QByteArray plaintext = CryptoManager::decrypt(encrypted, sessionKey, &decryptOk);

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

bool EncryptedFileStore::save (const QString &filePath, const QJsonDocument &doc, const QByteArray &sessionKey)
{
    QByteArray plaintext = doc.toJson(QJsonDocument::Compact);
    QByteArray encrypted = CryptoManager::encrypt(plaintext, sessionKey);

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
