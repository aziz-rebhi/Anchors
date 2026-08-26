#ifndef AUTOLOCKMANAGER_H
#define AUTOLOCKMANAGER_H
#pragma once

#include <QObject>

class QTimer;

class Autolockmanager : public QObject
{
    Q_OBJECT

public:
    explicit Autolockmanager(QObject *parent = nullptr);

    /** 0 = never auto-lock on idle */
    void setTimeoutMinutes(int minutes);
    void setLockOnMinimize(bool enabled);

    void start();
    void stop();

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private slots:
    void onTimeOut();

private:
    QTimer *m_timer = nullptr;
    int m_timeOutSeconds = 300;   // 5 min default
    bool m_lockOnMinimize = false;
    bool m_filterInstalled = false;
    bool m_running = false;
};

#endif