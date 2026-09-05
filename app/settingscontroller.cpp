#include "settingscontroller.h"
#include "session.h"

#include "core/storage/notesdatabase.h"
#include "core/storage/FilePaths.h"
#include "core/storage/saltstore.h"
#include "core/storage/encryptedfilestore.h"
#include "core/crypto/cryptomanager.h"

#include <QSettings>
#include <QStandardPaths>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QDateTime>
#include <QUrl>
#include <QDesktopServices>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QVersionNumber>
#include <QSaveFile>
#include <QTimer>
#include <QGuiApplication>
#include <QFont>
#include <QVector>
#include <sodium.h>

static void applyAppFont(qreal scale, const QString &family)
{
    QFont f = QGuiApplication::font();
    if (!family.isEmpty())
        f.setFamily(family);
    // Base ~10.5 pt, scaled by user preference
    f.setPointSizeF(10.5 * scale);
    QGuiApplication::setFont(f);
}

SettingsController::SettingsController(QObject *parent)
    : QObject(parent)
{
    load();
    applyAppFont(m_fontScale, m_uiFontFamily);
    QDir().mkpath(dataPath());

    if (m_checkUpdatesOnStartup) {
        QTimer::singleShot(2500, this, [this]() {
            checkForUpdates(/* quiet */ true);
        });
    }
}

QString SettingsController::appVersion() const
{
#ifdef APP_VERSION
    return QStringLiteral(APP_VERSION);
#else
    return QStringLiteral("0.0.0");
#endif
}

QString SettingsController::stripV(QString tag)
{
    tag = tag.trimmed();
    if (tag.startsWith(QLatin1Char('v')) || tag.startsWith(QLatin1Char('V')))
        tag = tag.mid(1);
    return tag;
}

void SettingsController::load()
{
    QSettings s(QStringLiteral("Anchors"), QStringLiteral("Anchors"));
    m_autoLockMinutes = s.value(QStringLiteral("security/autoLockMinutes"), 5).toInt();
    m_clearClipboard = s.value(QStringLiteral("security/clearClipboard"), true).toBool();
    m_clipboardClearSeconds = s.value(QStringLiteral("security/clipboardClearSeconds"), 15).toInt();
    if (m_clipboardClearSeconds < 3)
        m_clipboardClearSeconds = 3;
    m_lockOnMinimize = s.value(QStringLiteral("security/lockOnMinimize"), false).toBool();
    m_themeId = s.value(QStringLiteral("appearance/themeId"), QStringLiteral("dark")).toString();
    m_accentColor = s.value(QStringLiteral("appearance/accentColor"), QStringLiteral("#89b4fa")).toString();
    m_startPage = s.value(QStringLiteral("general/startPage"), QStringLiteral("dashboard")).toString();
    m_calendarDefaultView = s.value(QStringLiteral("calendar/defaultView"), QStringLiteral("month")).toString();
    m_vaultCategories = s.value(QStringLiteral("vault/categories")).toStringList();
    m_checkUpdatesOnStartup = s.value(QStringLiteral("general/checkUpdatesOnStartup"), true).toBool();
    m_fontScale = s.value(QStringLiteral("appearance/fontScale"), 1.0).toDouble();
    if (m_fontScale < 0.8 || m_fontScale > 1.5)
        m_fontScale = 1.0;
    m_uiFontFamily = s.value(QStringLiteral("appearance/uiFontFamily")).toString();
}

void SettingsController::save()
{
    QSettings s(QStringLiteral("Anchors"), QStringLiteral("Anchors"));
    s.setValue(QStringLiteral("security/autoLockMinutes"), m_autoLockMinutes);
    s.setValue(QStringLiteral("security/clearClipboard"), m_clearClipboard);
    s.setValue(QStringLiteral("security/clipboardClearSeconds"), m_clipboardClearSeconds);
    s.setValue(QStringLiteral("security/lockOnMinimize"), m_lockOnMinimize);
    s.setValue(QStringLiteral("appearance/themeId"), m_themeId);
    s.setValue(QStringLiteral("appearance/accentColor"), m_accentColor);
    s.setValue(QStringLiteral("general/startPage"), m_startPage);
    s.setValue(QStringLiteral("calendar/defaultView"), m_calendarDefaultView);
    s.setValue(QStringLiteral("vault/categories"), m_vaultCategories);
    s.setValue(QStringLiteral("general/checkUpdatesOnStartup"), m_checkUpdatesOnStartup);
    s.setValue(QStringLiteral("appearance/fontScale"), m_fontScale);
    s.setValue(QStringLiteral("appearance/uiFontFamily"), m_uiFontFamily);
}

void SettingsController::setCheckUpdatesOnStartup(bool v)
{
    if (m_checkUpdatesOnStartup == v)
        return;
    m_checkUpdatesOnStartup = v;
    save();
    emit checkUpdatesOnStartupChanged();
}

void SettingsController::setFontScale(qreal v)
{
    v = qBound(0.85, v, 1.40);
    v = qRound(v * 20.0) / 20.0; // 0.05 steps
    if (qFuzzyCompare(m_fontScale, v))
        return;
    m_fontScale = v;
    save();
    applyAppFont(m_fontScale, m_uiFontFamily);
    emit fontScaleChanged();
}

void SettingsController::setUiFontFamily(const QString &v)
{
    if (m_uiFontFamily == v)
        return;
    m_uiFontFamily = v;
    save();
    applyAppFont(m_fontScale, m_uiFontFamily);
    emit uiFontFamilyChanged();
}

void SettingsController::setAutoLockMinutes(int v)
{
    if (m_autoLockMinutes == v)
        return;
    m_autoLockMinutes = v;
    save();
    emit autoLockMinutesChanged();
}

void SettingsController::setClearClipboard(bool v)
{
    if (m_clearClipboard == v)
        return;
    m_clearClipboard = v;
    save();
    emit clearClipboardChanged();
}

void SettingsController::setClipboardClearSeconds(int v)
{
    if (v < 3)
        v = 3;
    if (v > 300)
        v = 300;
    if (m_clipboardClearSeconds == v)
        return;
    m_clipboardClearSeconds = v;
    save();
    emit clipboardClearSecondsChanged();
}

void SettingsController::setLockOnMinimize(bool v)
{
    if (m_lockOnMinimize == v)
        return;
    m_lockOnMinimize = v;
    save();
    emit lockOnMinimizeChanged();
}

void SettingsController::setThemeId(const QString &v)
{
    if (m_themeId == v)
        return;
    m_themeId = v;
    save();
    emit themeIdChanged();
}

void SettingsController::setAccentColor(const QString &v)
{
    if (m_accentColor == v)
        return;
    m_accentColor = v;
    save();
    emit accentColorChanged();
}

void SettingsController::setStartPage(const QString &v)
{
    if (m_startPage == v)
        return;
    m_startPage = v;
    save();
    emit startPageChanged();
}

void SettingsController::setCalendarDefaultView(const QString &v)
{
    if (m_calendarDefaultView == v)
        return;
    m_calendarDefaultView = v;
    save();
    emit calendarDefaultViewChanged();
}

QString SettingsController::dataPath() const
{
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
}

QString SettingsController::normalizeLocalPath(QString path)
{
    path = path.trimmed();
    if (path.startsWith(QStringLiteral("file://")))
        path = QUrl(path).toLocalFile();
    while (path.endsWith(QLatin1Char('/')) && path.size() > 1)
        path.chop(1);
    return path;
}

void SettingsController::openDataLocation()
{
    const QString path = dataPath();
    if (!QDir().mkpath(path)) {
        emit operationFailed(QStringLiteral("Could not create data folder."));
        return;
    }
    if (!QDesktopServices::openUrl(QUrl::fromLocalFile(path)))
        emit operationFailed(QStringLiteral("Could not open data folder."));
    else
        emit operationSucceeded(QStringLiteral("Opened data folder."));
}

void SettingsController::lockNow()
{
    emit lockRequested();
    if (Session::instance())
        Session::instance()->lock();
}

bool SettingsController::exportBackup(const QString &destPath)
{
    const QString src = dataPath();
    QDir srcDir(src);
    if (!srcDir.exists()) {
        emit operationFailed(QStringLiteral("No data directory found."));
        return false;
    }

    QString dest = normalizeLocalPath(destPath);
    if (dest.isEmpty()) {
        emit operationFailed(QStringLiteral("No destination selected."));
        return false;
    }

    QFileInfo fi(dest);
    if (fi.isDir() || !fi.exists()) {
        if (!QDir(dest).exists() && !QDir().mkpath(dest)) {
            emit operationFailed(QStringLiteral("Could not create destination folder."));
            return false;
        }
        dest = QDir(dest).filePath(
            QStringLiteral("anchors-backup-%1").arg(
                QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd-HHmmss"))));
    }

    if (!QDir().mkpath(dest)) {
        emit operationFailed(QStringLiteral("Could not create backup folder."));
        return false;
    }

    // Flush the SQLite journal/handle before snapshotting the folder so we
    // never copy a half-written notes.db.
    NotesDatabase::instance()->reset();

    QDirIterator it(src, QDir::Files, QDirIterator::Subdirectories);
    int copied = 0;
    while (it.hasNext()) {
        it.next();
        const QString from = it.filePath();
        const QString rel = srcDir.relativeFilePath(from);
        const QString to = QDir(dest).filePath(rel);

        QFileInfo di(to);
        if (!QDir().mkpath(di.absolutePath())) {
            emit operationFailed(QStringLiteral("Could not create backup subfolder."));
            return false;
        }

        QFile::remove(to);
        if (!QFile::copy(from, to)) {
            emit operationFailed(QStringLiteral("Failed to copy %1").arg(rel));
            return false;
        }
        ++copied;
    }

    if (copied == 0) {
        emit operationFailed(QStringLiteral("Data folder has no files to export."));
        return false;
    }

    emit operationSucceeded(
        QStringLiteral("Backup saved (%1 files) to %2").arg(copied).arg(dest));
    return true;
}

bool SettingsController::importBackup(const QString &srcPath)
{
    QString src = normalizeLocalPath(srcPath);
    if (src.isEmpty()) {
        emit operationFailed(QStringLiteral("No backup folder selected."));
        return false;
    }

    QDir srcDir(src);
    if (!srcDir.exists()) {
        emit operationFailed(QStringLiteral("Backup folder does not exist."));
        return false;
    }

    QStringList files = srcDir.entryList(QDir::Files);
    if (files.isEmpty()) {
        const QStringList subdirs = srcDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
        QString best;
        for (const QString &d : subdirs) {
            if (d.startsWith(QStringLiteral("anchors-backup-")))
                best = d;
        }
        if (!best.isEmpty()) {
            srcDir = QDir(srcDir.filePath(best));
            files = srcDir.entryList(QDir::Files);
        }
    }

    if (files.isEmpty()) {
        emit operationFailed(QStringLiteral("No files found in backup folder."));
        return false;
    }

    const bool looksLikeBackup =
        files.contains(QStringLiteral("verify.enc"))
        || files.contains(QStringLiteral("salt.bin"))
        || files.contains(QStringLiteral("notes.db"))
        || files.contains(QStringLiteral("vault.enc"))
        || files.contains(QStringLiteral("notes.enc"))
        || files.contains(QStringLiteral("tasks.enc"))
        || files.contains(QStringLiteral("calendar.enc"));

    if (!looksLikeBackup) {
        emit operationFailed(
            QStringLiteral("Folder does not look like an Anchors backup."));
        return false;
    }

    const QString dest = dataPath();
    if (!QDir().mkpath(dest)) {
        emit operationFailed(QStringLiteral("Could not prepare data folder."));
        return false;
    }

    // Close the SQLite handle first: notes.db is about to be replaced on
    // disk and a stale open handle would keep pointing at the old inode.
    NotesDatabase::instance()->close();

    int copied = 0;
    QDirIterator it(srcDir.absolutePath(), QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        it.next();
        const QString from = it.filePath();
        const QString rel = srcDir.relativeFilePath(from);
        const QString to = QDir(dest).filePath(rel);

        QFileInfo di(to);
        if (!QDir().mkpath(di.absolutePath())) {
            emit operationFailed(QStringLiteral("Could not create data subfolder."));
            NotesDatabase::instance()->reset();
            return false;
        }

        QFile::remove(to);
        if (!QFile::copy(from, to)) {
            emit operationFailed(QStringLiteral("Failed to restore %1").arg(rel));
            NotesDatabase::instance()->reset();
            return false;
        }
        ++copied;
    }

    // Re-open against the restored file so the running session stays usable.
    NotesDatabase::instance()->reset();

    if (Session::instance())
        Session::instance()->lock();
    emit lockRequested();

    emit operationSucceeded(
        QStringLiteral("Restored %1 files. Unlock with the backup password.").arg(copied));
    return true;
}

bool SettingsController::wipeAllData()
{
    if (!Session::instance() || !Session::instance()->isUnlocked()) {
        emit operationFailed(QStringLiteral("Unlock the app before wiping data."));
        return false;
    }

    const QString src = dataPath();

    // Close the SQLite handle first so the file can actually be removed.
    NotesDatabase::instance()->close();

    QDir dir(src);
    if (dir.exists() && !dir.removeRecursively()) {
        NotesDatabase::instance()->reset();
        emit operationFailed(QStringLiteral("Could not remove all data files."));
        return false;
    }

    if (!QDir().mkpath(src)) {
        emit operationFailed(QStringLiteral("Could not recreate data folder."));
        return false;
    }

    // Start fresh: an empty notes.db (tables recreated) + clean images dir.
    NotesDatabase::instance()->reset();

    Session::instance()->lock();
    emit lockRequested();
    emit operationSucceeded(QStringLiteral("Local data removed. Session locked."));
    return true;
}

namespace {
struct PasswordChangeFile
{
    QString path;
    QByteArray ciphertext; // re-encrypted with the new key
    QByteArray original;    // pre-change bytes, for rollback
};

bool writeFileAtomic(const QString &path, const QByteArray &data)
{
    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly))
        return false;
    if (file.write(data) != data.size()) {
        file.cancelWriting();
        return false;
    }
    return file.commit();
}

// Best-effort restoration of the pre-change ciphertexts after a partial
// password change. Returns the supplied error message.
QString rollbackChangePassword(const QVector<PasswordChangeFile> &pending,
                               const QString &reason)
{
    for (const PasswordChangeFile &p : pending)
        writeFileAtomic(p.path, p.original);
    return reason;
}
}

QString SettingsController::changeMasterPassword(const QString &currentPassword,
                                                 const QString &newPassword)
{
    if (currentPassword.isEmpty() || newPassword.isEmpty())
        return QStringLiteral("Passwords cannot be empty.");

    if (!Session::instance() || !Session::instance()->isUnlocked())
        return QStringLiteral("Session is locked.");

    // 1. Verify the current password against the stored salt + verify.enc.
    const QByteArray oldSalt = SaltStore::load();
    if (oldSalt.isEmpty())
        return QStringLiteral("Secure storage is uninitialized.");

    const QByteArray oldKey = CryptoManager::deriveKey(currentPassword, oldSalt);
    if (oldKey.isEmpty())
        return QStringLiteral("Could not derive a key from the current password.");

    bool verifyOk = false;
    const QJsonDocument verifyDoc = EncryptedFileStore::load(
        FilePaths::verifyFile(), oldKey, &verifyOk);
    if (!verifyOk || !verifyDoc.isObject()
        || verifyDoc.object().value(QLatin1String("v")).toString()
               != QStringLiteral("ANCHORS_VERIFY_V1")) {
        QByteArray wipedOldKey = oldKey;
        sodium_memzero(wipedOldKey.data(), static_cast<size_t>(wipedOldKey.size()));
        return QStringLiteral("Current password is incorrect.");
    }
    QByteArray wipedOldKey = oldKey;
    sodium_memzero(wipedOldKey.data(), static_cast<size_t>(wipedOldKey.size()));

    // 2. Derive the new key from a fresh salt. All stores are re-encrypted
    //    in memory first, and only then written out — salt.bin is written
    //    LAST so the keying material is always consistent with the files.
    const QByteArray newSalt = CryptoManager::generateSalt();
    const QByteArray newKey = CryptoManager::deriveKey(newPassword, newSalt);
    if (newSalt.isEmpty() || newKey.isEmpty())
        return QStringLiteral("Could not derive the new encryption key.");

    const QStringList targets = {
        FilePaths::verifyFile(),
        FilePaths::profileFile(),
        FilePaths::vaultFile(),
        FilePaths::notesFile(),
        FilePaths::tasksFile(),
        FilePaths::calendarFile(),
        FilePaths::pinFile(),
        FilePaths::resumeFile(),
    };

    QVector<PasswordChangeFile> pending;
    pending.reserve(targets.size());

    for (const QString &path : targets) {
        if (!QFile::exists(path))
            continue;

        QFile f(path);
        if (!f.open(QIODevice::ReadOnly))
            return QStringLiteral("Could not read %1.").arg(path);
        const QByteArray cipher = f.readAll();
        f.close();

        bool ok = false;
        const QByteArray plain = CryptoManager::decrypt(cipher, oldKey, &ok);
        if (!ok)
            return QStringLiteral("Could not decrypt %1 with the current key.").arg(path);

        const QByteArray re = CryptoManager::encrypt(plain, newKey);
        if (re.isEmpty())
            return QStringLiteral("Could not re-encrypt %1.").arg(path);

        pending.append({path, re, cipher});
    }

    // 3. Write every store atomically. If any write fails, put the
    //    originals back so the vault is never half-migrated.
    for (const PasswordChangeFile &p : pending) {
        if (!writeFileAtomic(p.path, p.ciphertext))
            return rollbackChangePassword(pending,
                QStringLiteral("Failed to write %1 — nothing was changed.").arg(p.path));
    }

    // 4. Persist the new salt last. Everything above is incompatible with
    //    the old salt, so this commit point is the actual switch-over.
    if (!SaltStore::save(newSalt))
        return rollbackChangePassword(pending,
            QStringLiteral("Failed to save the new salt — nothing was changed."));

    QByteArray wipedNewKey = newKey;
    sodium_memzero(wipedNewKey.data(), static_cast<size_t>(wipedNewKey.size()));

    // 5. Re-key the live session so the app keeps working without a lockout.
    Session::instance()->unlock(newKey);
    return QString();
}

void SettingsController::checkForUpdates(bool quiet)
{
    if (m_checkingUpdate)
        return;

    m_checkingUpdate = true;
    emit checkingUpdateChanged();

    QNetworkRequest req(QUrl(
        QStringLiteral("https://api.github.com/repos/aziz-rebhi/Anchors/releases/latest")));
    req.setHeader(QNetworkRequest::UserAgentHeader,
                  QStringLiteral("Anchors/%1").arg(appVersion()));
    req.setRawHeader("Accept", "application/vnd.github+json");
    req.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                     QNetworkRequest::NoLessSafeRedirectPolicy);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    req.setTransferTimeout(15000);
#endif

    QNetworkReply *reply = m_nam.get(req);
    connect(reply, &QNetworkReply::finished, this, [this, reply, quiet]() {
        reply->deleteLater();
        m_checkingUpdate = false;
        emit checkingUpdateChanged();

        if (reply->error() != QNetworkReply::NoError) {
            if (!quiet) {
                const int http = reply->attribute(
                                          QNetworkRequest::HttpStatusCodeAttribute).toInt();
                emit operationFailed(
                    QStringLiteral("Update check failed: %1 (HTTP %2)")
                        .arg(reply->errorString())
                        .arg(http));
            }
            return;
        }

        const QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        QString tag = stripV(doc.object().value(QStringLiteral("tag_name")).toString());
        if (tag.isEmpty()) {
            if (!quiet)
                emit operationFailed(QStringLiteral("Update check: no tag_name in release."));
            return;
        }

        m_latestVersion = tag;
        const QVersionNumber current = QVersionNumber::fromString(appVersion());
        const QVersionNumber latest = QVersionNumber::fromString(tag);
        m_updateAvailable = !latest.isNull() && !current.isNull() && latest > current;
        emit updateInfoChanged();

        if (m_updateAvailable) {
            emit operationSucceeded(
                QStringLiteral("Update available: v%1 (you have v%2)")
                    .arg(m_latestVersion, appVersion()));
        } else if (!quiet) {
            emit operationSucceeded(
                QStringLiteral("You're up to date (v%1).").arg(appVersion()));
        }
    });
}

void SettingsController::openLatestRelease()
{
    QDesktopServices::openUrl(
        QUrl(QStringLiteral("https://github.com/aziz-rebhi/Anchors/releases/latest")));
}

void SettingsController::setVaultCategories(const QStringList &v)
{
    if (m_vaultCategories == v)
        return;
    m_vaultCategories = v;
    save();
    emit vaultCategoriesChanged();
}

void SettingsController::addVaultCategory(const QString &name)
{
    const QString n = name.trimmed();
    if (n.isEmpty())
        return;
    for (const QString &c : m_vaultCategories) {
        if (c.compare(n, Qt::CaseInsensitive) == 0)
            return;
    }
    m_vaultCategories.append(n);
    save();
    emit vaultCategoriesChanged();
}

void SettingsController::removeVaultCategory(const QString &name)
{
    const QString n = name.trimmed();
    QStringList next;
    for (const QString &c : m_vaultCategories) {
        if (c.compare(n, Qt::CaseInsensitive) != 0)
            next.append(c);
    }
    if (next.size() == m_vaultCategories.size())
        return;
    m_vaultCategories = next;
    save();
    emit vaultCategoriesChanged();
}