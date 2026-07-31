#ifndef TASKREPOSITORY_H
#define TASKREPOSITORY_H

#pragma once

#include "../../models/taskentry.h"
#include <QByteArray>
#include <QVector>

class TaskRepository
{
public:
    explicit TaskRepository(const QByteArray &sessionKey);

    QVector<TaskEntry> loadAll(bool *ok = nullptr) const;
    bool saveAll(const QVector<TaskEntry> &entries) const;

    bool addEntry(TaskEntry entry) const;
    bool updateEntry(const TaskEntry &entry) const;
    bool deleteEntry(const QString &id) const;
    bool setDone(const QString &id, bool done) const;

private:
    QByteArray m_sessionKey;
};

#endif // TASKREPOSITORY_H
