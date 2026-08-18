#ifndef TASKREPOSITORY_H
#define TASKREPOSITORY_H
#pragma once

#include "../../models/taskentry.h"
#include "../../models/projectentry.h"
#include <QByteArray>
#include <QVector>

class TaskRepository
{
public:
    explicit TaskRepository(const QByteArray &sessionKey);

    // Full snapshot
    bool load(QVector<ProjectEntry> *projects, QVector<TaskEntry> *tasks) const;
    bool save(const QVector<ProjectEntry> &projects, const QVector<TaskEntry> &tasks) const;

    QVector<TaskEntry> loadAllTasks(bool *ok = nullptr) const;
    QVector<ProjectEntry> loadAllProjects(bool *ok = nullptr) const;

    bool addTask(TaskEntry entry) const;
    bool updateTaskTitleDue(const QString &id, const QString &title, qint64 dueAt) const;
    bool setDone(const QString &id, bool done) const;
    bool deleteTask(const QString &id) const;
    bool moveTaskToProject(const QString &taskId, const QString &projectId) const;

    bool addProject(ProjectEntry entry) const;
    bool updateProject(const QString &id, const QString &name,
                       const QString &emoji, const QString &color) const;
    bool deleteProject(const QString &id) const; // tasks → Inbox

private:
    QByteArray m_sessionKey;
};

#endif