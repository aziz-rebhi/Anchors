#ifndef TASKCONTROLLER_H
#define TASKCONTROLLER_H
#pragma once

#include <QObject>
#include <QString>
#include <QVariantList>

class TaskController : public QObject
{
    Q_OBJECT
public:
    explicit TaskController(QObject *parent = nullptr);

    Q_INVOKABLE QVariantList entries() const;
    Q_INVOKABLE QVariantList projects() const;

    Q_INVOKABLE bool addEntry(const QString &title, qint64 dueAtSecs, const QString &projectId = QString());
    Q_INVOKABLE bool updateEntry(const QString &id, const QString &title, qint64 dueAtSecs);
    Q_INVOKABLE bool setDone(const QString &id, bool done);
    Q_INVOKABLE bool deleteEntry(const QString &id);
    Q_INVOKABLE bool moveTaskToProject(const QString &taskId, const QString &projectId);

    Q_INVOKABLE bool addProject(const QString &name, const QString &emoji, const QString &color);
    Q_INVOKABLE bool updateProject(const QString &id, const QString &name,
                                   const QString &emoji, const QString &color);
    Q_INVOKABLE bool deleteProject(const QString &id);

signals:
    void entriesChanged();
    void projectsChanged();
    void operationFailed(QString reason);
};

#endif