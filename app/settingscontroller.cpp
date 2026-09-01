#include "settingscontroller.h"
#include "session.h"

#include <QSettings>
#include <QStandardPaths>
#include <QDir>
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

SettingsController::SettingsController(QObject *parent)
    : QObject(parent)
{
    load();
    QDir().mkpath(dataPath());
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
    m_lockOnMinimize = s.value(QStringLiteral("security/lockOnMinimize"), false).toBool();
    m_themeId = s.value(QStringLiteral("appearance/themeId"), QStringLiteral("dark")).toString();
    m_accentColor = s.value(QStringLiteral("appearance/accentColor"), QStringLiteral("#89b4fa")).toString();
    m_startPage = s.value(QStringLiteral("general/startPage"), QStringLiteral("dashboard")).toString();
    m_calendarDefaultView = s.value(QStringLiteral("calendar/defaultView"), QStringLiteral("month")).toString();
}

void SettingsController::save()
{
    QSettings s(QStringLiteral("Anchors"), QStringLiteral("Anchors"));
    s.setValue(QStringLiteral("security/autoLockMinutes"), m_autoLockMinutes);
    s.setValue(QStringLiteral("security/clearClipboard"), m_clearClipboard);
    s.setValue(QStringLiteral("security/lockOnMinimize"), m_lockOnMinimize);
    s.setValue(QStringLiteral("appearance/themeId"), m_themeId);
    s.setValue(QStringLiteral("appearance/accentColor"), m_accentColor);
    s.setValue(QStringLiteral("general/startPage"), m_startPage);
    s.setValue(QStringLiteral("calendar/defaultView"), m_calendarDefaultView);
}

void SettingsController::setAutoLockMinutes(int v)
{
    if (m_autoLockMinutes == v) return;
    m_autoLockMinutes = v;
    save();
    emit autoLockMinutesChanged();
}

void SettingsController::setClearClipboard(bool v)
{
    if (m_clearClipboard == v) return;
    m_clearClipboard = v;
    save();
    emit clearClipboardChanged();
}

void SettingsController::setLockOnMinimize(bool v)
{
    if (m_lockOnMinimize == v) return;
    m_lockOnMinimize = v;
    save();
    emit lockOnMinimizeChanged();
}

void SettingsController::setThemeId(const QString &v)
{
    if (m_themeId == v) return;
    m_themeId = v;
    save();
    emit themeIdChanged();
}

void SettingsController::setAccentColor(const QString &v)
{
    if (m_accentColor == v) return;
    m_accentColor = v;
    save();
    emit accentColorChanged();
}

void SettingsController::setStartPage(const QString &v)
{
    if (m_startPage == v) return;
    m_startPage = v;
    save();
    emit startPageChanged();
}

void SettingsController::setCalendarDefaultView(const QString &v)
{
    if (m_calendarDefaultView == v) return;
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

    const QStringList files = srcDir.entryList(QDir::Files);
    if (files.isEmpty()) {
        emit operationFailed(QStringLiteral("Data folder has no files to export."));
        return false;
    }

    int copied = 0;
    for (const QString &f : files) {
        const QString from = srcDir.filePath(f);
        const QString to = QDir(dest).filePath(f);
        QFile::remove(to);
        if (!QFile::copy(from, to)) {
            emit operationFailed(QStringLiteral("Failed to copy %1").arg(f));
            return false;
        }
        ++copied;
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

    int copied = 0;
    for (const QString &f : files) {
        const QString from = srcDir.filePath(f);
        const QString to = QDir(dest).filePath(f);
        QFile::remove(to);
        if (!QFile::copy(from, to)) {
            emit operationFailed(QStringLiteral("Failed to restore %1").arg(f));
            return false;
        }
        ++copied;
    }

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
    QDir dir(src);
    if (dir.exists()) {
        const QStringList files = dir.entryList(QDir::Files);
        for (const QString &f : files)
            QFile::remove(dir.filePath(f));
    }

    Session::instance()->lock();
    emit lockRequested();
    emit operationSucceeded(QStringLiteral("Local data removed. Session locked."));
    return true;
}

QString SettingsController::changeMasterPassword(const QString &currentPassword,
                                                 const QString &newPassword)
{
    Q_UNUSED(currentPassword);
    Q_UNUSED(newPassword);
    return QStringLiteral("Change password is not implemented yet.");
}

void SettingsController::checkForUpdates()
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

    QNetworkReply *reply = m_nam.get(req);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        m_checkingUpdate = false;
        emit checkingUpdateChanged();

        if (reply->error() != QNetworkReply::NoError) {
            emit operationFailed(QStringLiteral("Could not check for updates."));
            return;
        }

        const QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        QString tag = doc.object().value(QStringLiteral("tag_name")).toString();
        tag = stripV(tag);
        if (tag.isEmpty()) {
            emit operationFailed(QStringLiteral("No release information found."));
            return;
        }

        m_latestVersion = tag;
        const QVersionNumber current = QVersionNumber::fromString(appVersion());
        const QVersionNumber latest = QVersionNumber::fromString(tag);
        m_updateAvailable = !latest.isNull() && latest > current;
        emit updateInfoChanged();

        if (m_updateAvailable)
            emit operationSucceeded(
                QStringLiteral("Update available: v%1").arg(m_latestVersion));
        else
            emit operationSucceeded(QStringLiteral("You're up to date."));
    });
}

void SettingsController::openLatestRelease()
{
    QDesktopServices::openUrl(
        QUrl(QStringLiteral("https://github.com/aziz-rebhi/Anchors/releases/latest")));
}