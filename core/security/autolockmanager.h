#ifndef AUTOLOCKMANAGER_H
#define AUTOLOCKMANAGER_H
#pragma once

#include <QObject>

class QTimer;

class Autolockmanager : public QObject{
    Q_OBJECT

public :
    explicit Autolockmanager(QObject *parent = nullptr);

    void setTimeOutSeconds(int seconds);
    void start();
    void stop();

protected :
    bool eventFilter(QObject *matched, QEvent *event) override;

private slots :
    void onTimeOut();

private :
    QTimer *m_timer;
    int m_timeOutSeconds = 180;
    bool m_filterInstalled = false;
};

#endif // AUTOLOCKMANAGER_H
