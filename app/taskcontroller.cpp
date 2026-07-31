#include "taskcontroller.h"
#include "session.h"

#include "../core/models/taskentry.h"
#include "../core/storage/repositories/taskrepository.h"

#include <QVariantMap>

TaskController::TaskController(QObject *parent) : QObject(parent)
{
}

QVariantList TaskController::entries() const
{
    QVariantList list;

    if (!Session::instance()->isUnlocked()) {
        return list;
    }

    TaskRepository repo(Session::instance()->sessionKey());
    bool ok = false;
    const QVector<TaskEntry> all = repo.loadAll(&ok);
    if (!ok) {
        return list;
    }

    list.reserve(all.size());
    for (const TaskEntry &e : all) {
        QVariantMap m;
        m["id"] = e.id();
        m["title"] = e.title();
        m["done"] = e.done();
        m["dueAt"] = e.dueAt();
        m["createdAt"] = e.createdAt();
        m["updatedAt"] = e.updatedAt();
        list.append(m);
    }
    return list;
}

bool TaskController::addEntry(const QString &title, qint64 dueAtSecs)
{
    if (!Session::instance()->isUnlocked()) {
        emit operationFailed(QStringLiteral("Tasks are locked."));
        return false;
    }

    TaskRepository repo(Session::instance()->sessionKey());
    TaskEntry e;
    e.m_title = title;
    e.m_dueAt = dueAtSecs;

    const bool ok = repo.addEntry(e);
    if (ok) {
        emit entriesChanged();
    } else {
        emit operationFailed(QStringLiteral("Could not save the task."));
    }
    return ok;
}

bool TaskController::updateEntry(const QString &id, const QString &title, qint64 dueAtSecs)
{
    if (!Session::instance()->isUnlocked()) {
        emit operationFailed(QStringLiteral("Tasks are locked."));
        return false;
    }

    TaskRepository repo(Session::instance()->sessionKey());
    TaskEntry e;
    e.m_id = id;
    e.m_title = title;
    e.m_dueAt = dueAtSecs;

    const bool ok = repo.updateEntry(e);
    if (ok) {
        emit entriesChanged();
    } else {
        emit operationFailed(QStringLiteral("Could not update the task."));
    }
    return ok;
}

bool TaskController::setDone(const QString &id, bool done)
{
    if (!Session::instance()->isUnlocked()) {
        emit operationFailed(QStringLiteral("Tasks are locked."));
        return false;
    }

    TaskRepository repo(Session::instance()->sessionKey());
    const bool ok = repo.setDone(id, done);
    if (ok) {
        emit entriesChanged();
    } else {
        emit operationFailed(QStringLiteral("Could not update the task."));
    }
    return ok;
}

bool TaskController::deleteEntry(const QString &id)
{
    if (!Session::instance()->isUnlocked()) {
        emit operationFailed(QStringLiteral("Tasks are locked."));
        return false;
    }

    TaskRepository repo(Session::instance()->sessionKey());
    const bool ok = repo.deleteEntry(id);
    if (ok) {
        emit entriesChanged();
    } else {
        emit operationFailed(QStringLiteral("Could not delete the task."));
    }
    return ok;
}
