#include "settingscontroller.h"
#include "session.h"

#include <QSettings>
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QDateTime>

#ifdef Q_OS_LINUX
#include <QProcess>
#endif

SettingsController::SettingsController(QObject *parent)
    : QObject(parent)
{
    load();
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
    // TODO: AutoLockManager::instance()->setTimeoutMinutes(v);
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

void SettingsController::lockNow()
{
    emit lockRequested();
    if (Session::instance())
        Session::instance()->lock();
}

bool SettingsController::exportBackup(const QString &destPath)
{
    // Copies the app data directory into a timestamped folder at destPath.
    // Full encrypted single-file backup can replace this later.
    const QString src = dataPath();
    QDir srcDir(src);
    if (!srcDir.exists()) {
        emit operationFailed(QStringLiteral("No data directory found."));
        return false;
    }

    QString dest = destPath;
    if (dest.isEmpty()) {
        emit operationFailed(QStringLiteral("No destination selected."));
        return false;
    }

    QFileInfo fi(dest);
    if (fi.isDir()) {
        dest = QDir(dest).filePath(
            QStringLiteral("anchors-backup-%1").arg(
                QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd-HHmmss"))));
    }

    if (!QDir().mkpath(dest)) {
        emit operationFailed(QStringLiteral("Could not create backup folder."));
        return false;
    }

    const QStringList files = srcDir.entryList(QDir::Files);
    for (const QString &f : files) {
        const QString from = srcDir.filePath(f);
        const QString to = QDir(dest).filePath(f);
        QFile::remove(to);
        if (!QFile::copy(from, to)) {
            emit operationFailed(QStringLiteral("Failed to copy %1").arg(f));
            return false;
        }
    }

    emit operationSucceeded(QStringLiteral("Backup saved to %1").arg(dest));
    return true;
}

bool SettingsController::importBackup(const QString &srcPath)
{
    Q_UNUSED(srcPath);
    // Requires session lock + re-open flow; implement after backup format is fixed.
    emit operationFailed(QStringLiteral("Import is not implemented yet."));
    return false;
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
    emit operationSucceeded(QStringLiteral("Local data removed. Session locked."));
    return true;
}

QString SettingsController::changeMasterPassword(const QString &currentPassword,
                                                 const QString &newPassword)
{
    Q_UNUSED(currentPassword);
    Q_UNUSED(newPassword);
    // Wire to CryptoManager: verify current, re-wrap keys, re-save verify.enc + stores.
    return QStringLiteral("Change password is not implemented yet.");
}