#include "taskcontroller.h"
#include "session.h"
#include "../core/models/taskentry.h"
#include "../core/models/projectentry.h"
#include "../core/storage/repositories/taskrepository.h"

#include <QVariantMap>

TaskController::TaskController(QObject *parent) : QObject(parent) {}

QVariantList TaskController::entries() const
{
    QVariantList list;
    if (!Session::instance()->isUnlocked())
        return list;

    TaskRepository repo(Session::instance()->secureKey());
    bool ok = false;
    const QVector<TaskEntry> all = repo.loadAllTasks(&ok);
    if (!ok) return list;

    list.reserve(all.size());
    for (const TaskEntry &e : all) {
        QVariantMap m;
        m[QStringLiteral("id")] = e.id();
        m[QStringLiteral("title")] = e.title();
        m[QStringLiteral("done")] = e.done();
        m[QStringLiteral("dueAt")] = e.dueAt();
        m[QStringLiteral("createdAt")] = e.createdAt();
        m[QStringLiteral("updatedAt")] = e.updatedAt();
        m[QStringLiteral("projectId")] = e.projectId();
        list.append(m);
    }
    return list;
}

QVariantList TaskController::projects() const
{
    QVariantList list;
    if (!Session::instance()->isUnlocked())
        return list;

    TaskRepository repo(Session::instance()->secureKey());
    bool ok = false;
    const QVector<ProjectEntry> all = repo.loadAllProjects(&ok);
    if (!ok) return list;

    list.reserve(all.size());
    for (const ProjectEntry &p : all) {
        QVariantMap m;
        m[QStringLiteral("id")] = p.id();
        m[QStringLiteral("name")] = p.name();
        m[QStringLiteral("emoji")] = p.emoji();
        m[QStringLiteral("color")] = p.color();
        m[QStringLiteral("order")] = p.order();
        list.append(m);
    }
    return list;
}

bool TaskController::addEntry(const QString &title, qint64 dueAtSecs, const QString &projectId)
{
    if (!Session::instance()->isUnlocked()) {
        emit operationFailed(QStringLiteral("Tasks are locked."));
        return false;
    }
    TaskRepository repo(Session::instance()->secureKey());
    TaskEntry e;
    e.m_title = title;
    e.m_dueAt = dueAtSecs;
    e.m_projectId = projectId;
    const bool ok = repo.addTask(e);
    if (ok) emit entriesChanged();
    else emit operationFailed(QStringLiteral("Could not save the task."));
    return ok;
}

bool TaskController::updateEntry(const QString &id, const QString &title, qint64 dueAtSecs)
{
    if (!Session::instance()->isUnlocked()) {
        emit operationFailed(QStringLiteral("Tasks are locked."));
        return false;
    }
    TaskRepository repo(Session::instance()->secureKey());
    const bool ok = repo.updateTaskTitleDue(id, title, dueAtSecs);
    if (ok) emit entriesChanged();
    else emit operationFailed(QStringLiteral("Could not update the task."));
    return ok;
}

bool TaskController::setDone(const QString &id, bool done)
{
    if (!Session::instance()->isUnlocked()) {
        emit operationFailed(QStringLiteral("Tasks are locked."));
        return false;
    }
    TaskRepository repo(Session::instance()->secureKey());
    const bool ok = repo.setDone(id, done);
    if (ok) emit entriesChanged();
    else emit operationFailed(QStringLiteral("Could not update the task."));
    return ok;
}

bool TaskController::deleteEntry(const QString &id)
{
    if (!Session::instance()->isUnlocked()) {
        emit operationFailed(QStringLiteral("Tasks are locked."));
        return false;
    }
    TaskRepository repo(Session::instance()->secureKey());
    const bool ok = repo.deleteTask(id);
    if (ok) emit entriesChanged();
    else emit operationFailed(QStringLiteral("Could not delete the task."));
    return ok;
}

bool TaskController::moveTaskToProject(const QString &taskId, const QString &projectId)
{
    if (!Session::instance()->isUnlocked()) {
        emit operationFailed(QStringLiteral("Tasks are locked."));
        return false;
    }
    TaskRepository repo(Session::instance()->secureKey());
    const bool ok = repo.moveTaskToProject(taskId, projectId);
    if (ok) emit entriesChanged();
    else emit operationFailed(QStringLiteral("Could not move the task."));
    return ok;
}

bool TaskController::addProject(const QString &name, const QString &emoji, const QString &color)
{
    if (!Session::instance()->isUnlocked()) {
        emit operationFailed(QStringLiteral("Tasks are locked."));
        return false;
    }
    TaskRepository repo(Session::instance()->secureKey());
    ProjectEntry p;
    p.m_name = name;
    p.m_emoji = emoji.isEmpty() ? QStringLiteral("📁") : emoji;
    p.m_color = color.isEmpty() ? QStringLiteral("#89b4fa") : color;
    const bool ok = repo.addProject(p);
    if (ok) emit projectsChanged();
    else emit operationFailed(QStringLiteral("Could not create project."));
    return ok;
}

bool TaskController::updateProject(const QString &id, const QString &name,
                                   const QString &emoji, const QString &color)
{
    if (!Session::instance()->isUnlocked()) {
        emit operationFailed(QStringLiteral("Tasks are locked."));
        return false;
    }
    TaskRepository repo(Session::instance()->secureKey());
    const bool ok = repo.updateProject(id, name, emoji, color);
    if (ok) emit projectsChanged();
    else emit operationFailed(QStringLiteral("Could not update project."));
    return ok;
}

bool TaskController::deleteProject(const QString &id)
{
    if (!Session::instance()->isUnlocked()) {
        emit operationFailed(QStringLiteral("Tasks are locked."));
        return false;
    }
    TaskRepository repo(Session::instance()->secureKey());
    const bool ok = repo.deleteProject(id);
    if (ok) {
        emit projectsChanged();
        emit entriesChanged();
    } else {
        emit operationFailed(QStringLiteral("Could not delete project."));
    }
    return ok;
}