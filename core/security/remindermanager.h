#pragma once

#include <QObject>
#include <QSet>
#include <QTimer>

class QSystemTrayIcon;

class ReminderManager : public QObject
{
    Q_OBJECT
public:
    explicit ReminderManager(QObject *parent = nullptr);

    void start();
    void stop();

private slots:
    void tick();

private:
    void notify(const QString &title, const QString &body, const QString &fireKey);
    QString dayKey() const;

    QTimer m_timer;
    QSystemTrayIcon *m_tray = nullptr;
    QSet<QString> m_fired; // fireKey already shown
};