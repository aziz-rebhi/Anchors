#include "saltstore.h"
#include "FilePaths.h"
#include "../crypto/cryptomanager.h"

#include <QDebug>
#include <QFile>


namespace
{
QString saltFilePath()
{
    return FilePaths::dataDir() + QStringLiteral("/salt.bin");
}
}

bool SaltStore::exists()
{
    return QFile::exists(saltFilePath());
}

bool SaltStore::generateAndSave()
{
    QByteArray salt = CryptoManager::generateSalt();

    QFile file(saltFilePath());
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "SaltStore::generateAndSave: could not open" << saltFilePath()
        << "-" << file.errorString();
        return false;
    }

    qint64 written = file.write(salt);
    file.close();

    if (written != salt.size()) {
        qWarning() << "SaltStore::generateAndSave: incomplete write to" << saltFilePath();
        return false;
    }

    return true;
}

QByteArray SaltStore::load()
{
    QFile file(saltFilePath());
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "SaltStore::load: could not open" << saltFilePath()
        << "-" << file.errorString();
        return QByteArray();
    }

    QByteArray salt = file.readAll();
    file.close();
    return salt;
}

