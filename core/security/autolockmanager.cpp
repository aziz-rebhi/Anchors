#include "autolockmanager.h"
#include "../../app/session.h"

#include <QGuiApplication>
#include <QEvent>
#include <QTimer>
#include <QWindow>

Autolockmanager::Autolockmanager(QObject *parent)
    : QObject(parent)
{
    m_timer = new QTimer(this);
    m_timer->setSingleShot(true);
    connect(m_timer, &QTimer::timeout, this, &Autolockmanager::onTimeOut);
}

void Autolockmanager::setTimeoutMinutes(int minutes)
{
    if (minutes <= 0) {
        m_timeOutSeconds = 0;
        m_timer->stop();
        return;
    }
    m_timeOutSeconds = minutes * 60;
    if (m_running)
        m_timer->start(m_timeOutSeconds * 1000);
}

void Autolockmanager::setLockOnMinimize(bool enabled)
{
    m_lockOnMinimize = enabled;
}

void Autolockmanager::start()
{
    m_running = true;
    if (!m_filterInstalled && qGuiApp) {
        qGuiApp->installEventFilter(this);
        m_filterInstalled = true;
    }
    if (m_timeOutSeconds > 0)
        m_timer->start(m_timeOutSeconds * 1000);
    else
        m_timer->stop();
}

void Autolockmanager::stop()
{
    m_running = false;
    m_timer->stop();
}

bool Autolockmanager::eventFilter(QObject *watched, QEvent *event)
{
    if (!m_running)
        return QObject::eventFilter(watched, event);

    switch (event->type()) {
    case QEvent::MouseMove:
    case QEvent::MouseButtonPress:
    case QEvent::MouseButtonRelease:
    case QEvent::KeyPress:
    case QEvent::KeyRelease:
    case QEvent::Wheel:
    case QEvent::TouchBegin:
        if (m_timeOutSeconds > 0)
            m_timer->start(m_timeOutSeconds * 1000);
        break;

    case QEvent::WindowStateChange: {
        if (!m_lockOnMinimize)
            break;
        auto *win = qobject_cast<QWindow *>(watched);
        if (win && (win->windowState() & Qt::WindowMinimized)) {
            if (Session::instance() && Session::instance()->isUnlocked())
                Session::instance()->lock();
        }
        break;
    }
    default:
        break;
    }
    return QObject::eventFilter(watched, event);
}

void Autolockmanager::onTimeOut()
{
    if (Session::instance() && Session::instance()->isUnlocked())
        Session::instance()->lock();
}