#ifndef ENCRYPTEDFILESTORE_H
#define ENCRYPTEDFILESTORE_H

#include "core/crypto/SecureBuffer.h"
#include <QByteArray>
#include <QJsonDocument>
#include <QString>

class EncryptedFileStore
{
public:
    EncryptedFileStore();

    static QJsonDocument load(const QString &filePath, const QByteArray &sessionKey, bool *ok = nullptr);
    static bool save (const QString &filePath, const QJsonDocument &doc, const QByteArray &sessionKey);

    // Buffer-guarded variants — prefer these anywhere the key lives in the
    // Session's SecureBuffer so no copy is ever made into a plain heap
    // QByteArray.
    static QJsonDocument load(const QString &filePath, const SecureBuffer &sessionKey, bool *ok = nullptr);
    static bool save (const QString &filePath, const QJsonDocument &doc, const SecureBuffer &sessionKey);

private:
    static QJsonDocument loadWithKey(const QString &filePath,
                                     const unsigned char *keyData, size_t keySize,
                                     bool *ok);
    static bool saveWithKey(const QString &filePath, const QJsonDocument &doc,
                            const unsigned char *keyData, size_t keySize);
};

#endif // ENCRYPTEDFILESTORE_H
