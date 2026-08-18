#include "taskrepository.h"
#include "core/storage/encryptedfilestore.h"
#include "core/storage/FilePaths.h"

#include <QDateTime>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUuid>
#include <algorithm>

TaskRepository::TaskRepository(const QByteArray &sessionKey)
    : m_sessionKey(sessionKey)
{
}

bool TaskRepository::load(QVector<ProjectEntry> *projects, QVector<TaskEntry> *tasks) const
{
    if (projects) projects->clear();
    if (tasks) tasks->clear();

    bool loadOk = false;
    QJsonDocument doc = EncryptedFileStore::load(FilePaths::tasksFile(), m_sessionKey, &loadOk);
    if (!loadOk)
        return false;

    // New format: { "projects": [...], "tasks": [...] }
    if (doc.isObject()) {
        const QJsonObject root = doc.object();
        if (projects) {
            const QJsonArray arr = root.value(QStringLiteral("projects")).toArray();
            projects->reserve(arr.size());
            for (const QJsonValue &v : arr)
                projects->append(ProjectEntry::fromJson(v.toObject()));
            std::sort(projects->begin(), projects->end(),
                      [](const ProjectEntry &a, const ProjectEntry &b) {
                          return a.order() < b.order();
                      });
        }
        if (tasks) {
            const QJsonArray arr = root.value(QStringLiteral("tasks")).toArray();
            tasks->reserve(arr.size());
            for (const QJsonValue &v : arr)
                tasks->append(TaskEntry::fromJson(v.toObject()));
        }
        return true;
    }

    // Legacy: plain array of tasks
    if (doc.isArray()) {
        if (tasks) {
            const QJsonArray arr = doc.array();
            tasks->reserve(arr.size());
            for (const QJsonValue &v : arr)
                tasks->append(TaskEntry::fromJson(v.toObject()));
        }
        return true;
    }

    return false;
}

bool TaskRepository::save(const QVector<ProjectEntry> &projects,
                          const QVector<TaskEntry> &tasks) const
{
    QJsonArray pArr;
    for (const ProjectEntry &p : projects)
        pArr.append(p.toJson());

    QJsonArray tArr;
    for (const TaskEntry &t : tasks)
        tArr.append(t.toJson());

    QJsonObject root;
    root[QStringLiteral("projects")] = pArr;
    root[QStringLiteral("tasks")] = tArr;
    return EncryptedFileStore::save(FilePaths::tasksFile(), QJsonDocument(root), m_sessionKey);
}

QVector<TaskEntry> TaskRepository::loadAllTasks(bool *ok) const
{
    QVector<ProjectEntry> projects;
    QVector<TaskEntry> tasks;
    const bool good = load(&projects, &tasks);
    if (ok) *ok = good;
    return tasks;
}

QVector<ProjectEntry> TaskRepository::loadAllProjects(bool *ok) const
{
    QVector<ProjectEntry> projects;
    QVector<TaskEntry> tasks;
    const bool good = load(&projects, &tasks);
    if (ok) *ok = good;
    return projects;
}

bool TaskRepository::addTask(TaskEntry entry) const
{
    QVector<ProjectEntry> projects;
    QVector<TaskEntry> tasks;
    if (!load(&projects, &tasks))
        return false;

    if (entry.m_id.isEmpty())
        entry.m_id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    const qint64 now = QDateTime::currentSecsSinceEpoch();
    entry.m_createdAt = now;
    entry.m_updatedAt = now;

    tasks.append(entry);
    return save(projects, tasks);
}

bool TaskRepository::updateTaskTitleDue(const QString &id, const QString &title, qint64 dueAt) const
{
    QVector<ProjectEntry> projects;
    QVector<TaskEntry> tasks;
    if (!load(&projects, &tasks))
        return false;

    for (TaskEntry &e : tasks) {
        if (e.m_id == id) {
            e.m_title = title;
            e.m_dueAt = dueAt;
            e.m_updatedAt = QDateTime::currentSecsSinceEpoch();
            return save(projects, tasks);
        }
    }
    return false;
}

bool TaskRepository::setDone(const QString &id, bool done) const
{
    QVector<ProjectEntry> projects;
    QVector<TaskEntry> tasks;
    if (!load(&projects, &tasks))
        return false;

    for (TaskEntry &e : tasks) {
        if (e.m_id == id) {
            e.m_done = done;
            e.m_updatedAt = QDateTime::currentSecsSinceEpoch();
            return save(projects, tasks);
        }
    }
    return false;
}

bool TaskRepository::deleteTask(const QString &id) const
{
    QVector<ProjectEntry> projects;
    QVector<TaskEntry> tasks;
    if (!load(&projects, &tasks))
        return false;

    const int before = tasks.size();
    tasks.erase(std::remove_if(tasks.begin(), tasks.end(),
                               [&id](const TaskEntry &e) { return e.m_id == id; }),
                tasks.end());
    if (tasks.size() == before)
        return false;
    return save(projects, tasks);
}

bool TaskRepository::moveTaskToProject(const QString &taskId, const QString &projectId) const
{
    QVector<ProjectEntry> projects;
    QVector<TaskEntry> tasks;
    if (!load(&projects, &tasks))
        return false;

    for (TaskEntry &e : tasks) {
        if (e.m_id == taskId) {
            e.m_projectId = projectId;
            e.m_updatedAt = QDateTime::currentSecsSinceEpoch();
            return save(projects, tasks);
        }
    }
    return false;
}

bool TaskRepository::addProject(ProjectEntry entry) const
{
    QVector<ProjectEntry> projects;
    QVector<TaskEntry> tasks;
    if (!load(&projects, &tasks)) {
        // First run: empty store
        projects.clear();
        tasks.clear();
    }

    if (entry.m_id.isEmpty())
        entry.m_id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    const qint64 now = QDateTime::currentSecsSinceEpoch();
    entry.m_createdAt = now;
    entry.m_updatedAt = now;
    if (entry.m_order == 0)
        entry.m_order = projects.size();

    projects.append(entry);
    return save(projects, tasks);
}

bool TaskRepository::updateProject(const QString &id, const QString &name,
                                   const QString &emoji, const QString &color) const
{
    QVector<ProjectEntry> projects;
    QVector<TaskEntry> tasks;
    if (!load(&projects, &tasks))
        return false;

    for (ProjectEntry &p : projects) {
        if (p.m_id == id) {
            p.m_name = name;
            p.m_emoji = emoji;
            p.m_color = color;
            p.m_updatedAt = QDateTime::currentSecsSinceEpoch();
            return save(projects, tasks);
        }
    }
    return false;
}

bool TaskRepository::deleteProject(const QString &id) const
{
    if (id.isEmpty())
        return false;

    QVector<ProjectEntry> projects;
    QVector<TaskEntry> tasks;
    if (!load(&projects, &tasks))
        return false;

    const int before = projects.size();
    projects.erase(std::remove_if(projects.begin(), projects.end(),
                                  [&id](const ProjectEntry &p) { return p.m_id == id; }),
                   projects.end());
    if (projects.size() == before)
        return false;

    // Move tasks to Inbox
    for (TaskEntry &t : tasks) {
        if (t.m_projectId == id) {
            t.m_projectId.clear();
            t.m_updatedAt = QDateTime::currentSecsSinceEpoch();
        }
    }
    return save(projects, tasks);
}