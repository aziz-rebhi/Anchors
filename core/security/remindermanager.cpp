#include "remindermanager.h"

#include "app/session.h"
#include "core/models/Calendarentry.h"
#include "core/models/taskentry.h"
#include "core/storage/repositories/calendarrepository.h"
#include "core/storage/repositories/taskrepository.h"

#include <QDate>
#include <QDateTime>
#include <QIcon>
#include <QSystemTrayIcon>

ReminderManager::ReminderManager(QObject *parent)
    : QObject(parent)
{
    m_tray = new QSystemTrayIcon(QIcon(QStringLiteral(":/qml/logo.png")), this);
    m_tray->setToolTip(QStringLiteral("Anchors"));
    m_tray->show();

    m_timer.setInterval(30 * 1000); // every 30s
    connect(&m_timer, &QTimer::timeout, this, &ReminderManager::tick);
}

void ReminderManager::start()
{
    m_timer.start();
    tick();
}

void ReminderManager::stop()
{
    m_timer.stop();
}

QString ReminderManager::dayKey() const
{
    return QDate::currentDate().toString(Qt::ISODate);
}

void ReminderManager::notify(const QString &title, const QString &body, const QString &fireKey)
{
    if (m_fired.contains(fireKey))
        return;
    m_fired.insert(fireKey);

    if (m_tray && m_tray->isSystemTrayAvailable()) {
        m_tray->showMessage(title, body, QSystemTrayIcon::Information, 8000);
    }
}

void ReminderManager::tick()
{
    if (!Session::instance()->isUnlocked())
        return;

    const qint64 nowSecs = QDateTime::currentSecsSinceEpoch();
    const QString today = dayKey();

    // --- Tasks (due) ---
    {
        TaskRepository repo(Session::instance()->secureKey());
        bool ok = false;
        const auto tasks = repo.loadAllTasks(&ok);
        if (ok) {
            for (const TaskEntry &t : tasks) {
                if (t.done() || t.dueAt() <= 0)
                    continue;
                // Fire when due time has passed (same day key avoids spam forever)
                if (t.dueAt() <= nowSecs) {
                    const QString key = QStringLiteral("task:%1:%2").arg(t.id(), today);
                    notify(QStringLiteral("Task due"), t.title(), key);
                }
            }
        }
    }

    // --- Calendar (start − reminderMinutes; default 15 if field missing) ---
    {
        CalendarRepository repo(Session::instance()->secureKey());
        bool ok = false;
        const auto events = repo.loadAll(&ok);
        if (ok) {
            for (const CalendarEntry &e : events) {
                if (!e.start().isValid())
                    continue;
                int mins = e.reminderMinutes(); // add getter; 0 = no reminder, -1 = use default
                if (mins < 0)
                    mins = 15;
                if (mins == 0)
                    continue;

                const qint64 fireAt = e.start().toSecsSinceEpoch() - mins * 60;
                if (nowSecs >= fireAt && nowSecs < e.start().toSecsSinceEpoch() + 3600) {
                    const QString key = QStringLiteral("event:%1:%2").arg(e.id(), today);
                    notify(QStringLiteral("Upcoming event"),
                           e.title() + QStringLiteral(" · ") + e.start().toString(QStringLiteral("hh:mm")),
                           key);
                }
            }
        }
    }
}