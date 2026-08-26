#ifndef SETTINGSCONTROLLER_H
#define SETTINGSCONTROLLER_H
#pragma once

#include <QObject>
#include <QString>

class SettingsController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int autoLockMinutes READ autoLockMinutes WRITE setAutoLockMinutes NOTIFY autoLockMinutesChanged)
    Q_PROPERTY(bool clearClipboard READ clearClipboard WRITE setClearClipboard NOTIFY clearClipboardChanged)
    Q_PROPERTY(bool lockOnMinimize READ lockOnMinimize WRITE setLockOnMinimize NOTIFY lockOnMinimizeChanged)
    Q_PROPERTY(QString themeId READ themeId WRITE setThemeId NOTIFY themeIdChanged)
    Q_PROPERTY(QString accentColor READ accentColor WRITE setAccentColor NOTIFY accentColorChanged)
    Q_PROPERTY(QString startPage READ startPage WRITE setStartPage NOTIFY startPageChanged)
    Q_PROPERTY(QString calendarDefaultView READ calendarDefaultView WRITE setCalendarDefaultView NOTIFY calendarDefaultViewChanged)
    Q_PROPERTY(QString appVersion READ appVersion CONSTANT)
    Q_PROPERTY(QString dataPath READ dataPath CONSTANT)

public:
    explicit SettingsController(QObject *parent = nullptr);

    int autoLockMinutes() const { return m_autoLockMinutes; }
    void setAutoLockMinutes(int v);

    bool clearClipboard() const { return m_clearClipboard; }
    void setClearClipboard(bool v);

    bool lockOnMinimize() const { return m_lockOnMinimize; }
    void setLockOnMinimize(bool v);

    QString themeId() const { return m_themeId; }
    void setThemeId(const QString &v);

    QString accentColor() const { return m_accentColor; }
    void setAccentColor(const QString &v);

    QString startPage() const { return m_startPage; }
    void setStartPage(const QString &v);

    QString calendarDefaultView() const { return m_calendarDefaultView; }
    void setCalendarDefaultView(const QString &v);

    QString appVersion() const { return QStringLiteral("1.0.0"); }
    QString dataPath() const;

    Q_INVOKABLE void lockNow();
    Q_INVOKABLE void openDataLocation();
    Q_INVOKABLE bool exportBackup(const QString &destPath);
    Q_INVOKABLE bool importBackup(const QString &srcPath);
    Q_INVOKABLE bool wipeAllData();
    Q_INVOKABLE QString changeMasterPassword(const QString &currentPassword,
                                             const QString &newPassword);

signals:
    void autoLockMinutesChanged();
    void clearClipboardChanged();
    void lockOnMinimizeChanged();
    void themeIdChanged();
    void accentColorChanged();
    void startPageChanged();
    void calendarDefaultViewChanged();
    void operationFailed(QString reason);
    void operationSucceeded(QString message);
    void lockRequested();

private:
    void load();
    void save();
    static QString normalizeLocalPath(QString path);

    int m_autoLockMinutes = 5;
    bool m_clearClipboard = true;
    bool m_lockOnMinimize = false;
    QString m_themeId = QStringLiteral("dark");
    QString m_accentColor = QStringLiteral("#89b4fa");
    QString m_startPage = QStringLiteral("dashboard");
    QString m_calendarDefaultView = QStringLiteral("month");
};

#endif