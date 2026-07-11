#ifndef ENCRYPTEDFILESTORE_H
#define ENCRYPTEDFILESTORE_H

#include <QByteArray>
#include <QJsonDocument>
#include <QString>

class EncryptedFileStore
{
public:
    EncryptedFileStore();

    static QJsonDocument load(const QString &filePath, const QByteArray &sessionKey, bool *ok = nullptr);
    static bool save (const QString &filePath, const QJsonDocument &doc, const QByteArray &sessionKey);
};

#endif // ENCRYPTEDFILESTORE_H
