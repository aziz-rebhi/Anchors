#include "dashboardcontroller.h"
#include <QDebug>

DashboardController::DashboardController(QObject *parent)
    : QObject(parent)
{
    loadData();
}

void DashboardController::loadData()
{
    // Static data matching the HTML design
    m_tasks = {
        {"Finalize Q3 Security Audit", "Due 14:00", false},
        {"Review Tahoe Design System PR", "Due 16:30", false},
        {"Update Cryptography Keys", "EOD", false}
    };

    m_events = {
        {"10:00", "Sync: Project Stealth", "Secure Channel Alpha", "🎥", true},
        {"14:30", "Architecture Review", "Room 404", "📍", false}
    };

    m_scratchpad = "";
}

QVariantList DashboardController::tasks() const
{
    QVariantList list;
    for (const Task &t : m_tasks) {
        QVariantMap map;
        map["title"] = t.title;
        map["due"] = t.due;
        map["done"] = t.done;
        list.append(map);
    }
    return list;
}

QVariantList DashboardController::schedule() const
{
    QVariantList list;
    for (const Event &e : m_events) {
        QVariantMap map;
        map["time"] = e.time;
        map["title"] = e.title;
        map["detail"] = e.detail;
        map["icon"] = e.icon;
        map["active"] = e.active;
        list.append(map);
    }
    return list;
}

int DashboardController::urgentCount() const
{
    int count = 0;
    for (const Task &t : m_tasks) {
        if (!t.done) count++;
    }
    return count;
}

void DashboardController::toggleTask(int index)
{
    if (index >= 0 && index < m_tasks.size()) {
        m_tasks[index].done = !m_tasks[index].done;
        notifyChanged();
    }
}

void DashboardController::addTask(const QString &title)
{
    if (title.trimmed().isEmpty()) return;
    m_tasks.append({title.trimmed(), "Today", false});
    notifyChanged();
}

void DashboardController::updateScratchpad(const QString &text)
{
    m_scratchpad = text;
    // You could save this to a file here
}

void DashboardController::notifyChanged()
{
    emit dataChanged();
    emit dataChanged();
    emit scheduleChanged();
    emit urgentCountChanged();
}