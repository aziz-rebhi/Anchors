#include "autolockmanager.h"
#include "../../app/session.h"

#include <QApplication>
#include <QEvent>
#include <QTimer>

Autolockmanager::Autolockmanager(QObject *parent) : QObject(parent){
    m_timer = new QTimer(this);
    m_timer -> setSingleShot(true);
    connect (m_timer, &QTimer::timeout, this, &Autolockmanager::onTimeOut);
}

void Autolockmanager::setTimeOutSeconds(int seconds){
    m_timeOutSeconds = seconds;
}

void Autolockmanager::start(){
    if (!m_filterInstalled) {
        qApp -> installEventFilter(this);
        m_filterInstalled = true;

    }
    m_timer -> start(m_timeOutSeconds * 1000);
}

void Autolockmanager::stop(){
    m_timer -> stop();
}

bool Autolockmanager::eventFilter(QObject *watched, QEvent *event){
    switch (event -> type()){
    case QEvent::MouseMove:
    case QEvent::MouseButtonPress:
    case QEvent::MouseButtonRelease:
    case QEvent::KeyPress:
    case QEvent::KeyRelease:
    case QEvent::Wheel:
        if(m_timer -> isActive()){
            m_timer -> start(m_timeOutSeconds * 1000);
        }
        break;
    default: break;
    }
    return QObject::eventFilter(watched, event);
}

void Autolockmanager::onTimeOut(){
    Session::instance() -> lock();
}
