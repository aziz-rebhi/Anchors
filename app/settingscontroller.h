#ifndef SETTINGSCONTROLLER_H
#define SETTINGSCONTROLLER_H
#pragma once

#include <QObject>
#include <QString>
#include <QNetworkAccessManager>
#include <QStringList>

class SettingsController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int autoLockMinutes READ autoLockMinutes WRITE setAutoLockMinutes NOTIFY autoLockMinutesChanged)
    Q_PROPERTY(bool clearClipboard READ clearClipboard WRITE setClearClipboard NOTIFY clearClipboardChanged)
    Q_PROPERTY(int clipboardClearSeconds READ clipboardClearSeconds WRITE setClipboardClearSeconds NOTIFY clipboardClearSecondsChanged)
    Q_PROPERTY(bool lockOnMinimize READ lockOnMinimize WRITE setLockOnMinimize NOTIFY lockOnMinimizeChanged)
    Q_PROPERTY(QString themeId READ themeId WRITE setThemeId NOTIFY themeIdChanged)
    Q_PROPERTY(QString accentColor READ accentColor WRITE setAccentColor NOTIFY accentColorChanged)
    Q_PROPERTY(QString startPage READ startPage WRITE setStartPage NOTIFY startPageChanged)
    Q_PROPERTY(QString calendarDefaultView READ calendarDefaultView WRITE setCalendarDefaultView NOTIFY calendarDefaultViewChanged)
    Q_PROPERTY(bool checkUpdatesOnStartup READ checkUpdatesOnStartup WRITE setCheckUpdatesOnStartup NOTIFY checkUpdatesOnStartupChanged)
    Q_PROPERTY(QString appVersion READ appVersion CONSTANT)
    Q_PROPERTY(QString dataPath READ dataPath CONSTANT)
    Q_PROPERTY(QString latestVersion READ latestVersion NOTIFY updateInfoChanged)
    Q_PROPERTY(bool updateAvailable READ updateAvailable NOTIFY updateInfoChanged)
    Q_PROPERTY(bool checkingUpdate READ checkingUpdate NOTIFY checkingUpdateChanged)
    Q_PROPERTY(QStringList vaultCategories READ vaultCategories WRITE setVaultCategories NOTIFY vaultCategoriesChanged)
    Q_PROPERTY(qreal fontScale READ fontScale WRITE setFontScale NOTIFY fontScaleChanged)
    Q_PROPERTY(QString uiFontFamily READ uiFontFamily WRITE setUiFontFamily NOTIFY uiFontFamilyChanged)

public:
    explicit SettingsController(QObject *parent = nullptr);

    int autoLockMinutes() const { return m_autoLockMinutes; }
    void setAutoLockMinutes(int v);

    bool clearClipboard() const { return m_clearClipboard; }
    void setClearClipboard(bool v);

    int clipboardClearSeconds() const { return m_clipboardClearSeconds; }
    void setClipboardClearSeconds(int v);

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

    bool checkUpdatesOnStartup() const { return m_checkUpdatesOnStartup; }
    void setCheckUpdatesOnStartup(bool v);

    QString appVersion() const;
    QString dataPath() const;

    QString latestVersion() const { return m_latestVersion; }
    bool updateAvailable() const { return m_updateAvailable; }
    bool checkingUpdate() const { return m_checkingUpdate; }

    Q_INVOKABLE void lockNow();
    Q_INVOKABLE void openDataLocation();
    Q_INVOKABLE bool exportBackup(const QString &destPath);
    Q_INVOKABLE bool importBackup(const QString &srcPath);
    Q_INVOKABLE bool wipeAllData();
    Q_INVOKABLE QString changeMasterPassword(const QString &currentPassword,
                                             const QString &newPassword);
    Q_INVOKABLE void checkForUpdates(bool quiet = false);
    Q_INVOKABLE void openLatestRelease();

    QStringList vaultCategories() const { return m_vaultCategories; }
    void setVaultCategories(const QStringList &v);
    Q_INVOKABLE void addVaultCategory(const QString &name);
    Q_INVOKABLE void removeVaultCategory(const QString &name);

    qreal fontScale() const { return m_fontScale; }
    void setFontScale(qreal v);

    QString uiFontFamily() const { return m_uiFontFamily; }
    void setUiFontFamily(const QString &v);

signals:
    void autoLockMinutesChanged();
    void clearClipboardChanged();
    void clipboardClearSecondsChanged();
    void lockOnMinimizeChanged();
    void themeIdChanged();
    void accentColorChanged();
    void startPageChanged();
    void calendarDefaultViewChanged();
    void checkUpdatesOnStartupChanged();
    void operationFailed(QString reason);
    void operationSucceeded(QString message);
    void lockRequested();
    void updateInfoChanged();
    void checkingUpdateChanged();
    void vaultCategoriesChanged();
    void fontScaleChanged();
    void uiFontFamilyChanged();

private:
    void load();
    void save();
    static QString normalizeLocalPath(QString path);
    static QString stripV(QString tag);

    int m_autoLockMinutes = 5;
    bool m_clearClipboard = true;
    int m_clipboardClearSeconds = 15;
    bool m_lockOnMinimize = false;
    QString m_themeId = QStringLiteral("dark");
    QString m_accentColor = QStringLiteral("#89b4fa");
    QString m_startPage = QStringLiteral("dashboard");
    QString m_calendarDefaultView = QStringLiteral("month");
    bool m_checkUpdatesOnStartup = true;

    QNetworkAccessManager m_nam;
    QString m_latestVersion;
    bool m_updateAvailable = false;
    bool m_checkingUpdate = false;
    QStringList m_vaultCategories;
    qreal m_fontScale = 1.0;
    QString m_uiFontFamily;
};

#endif