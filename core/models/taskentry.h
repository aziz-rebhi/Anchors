#ifndef TASKENTRY_H
#define TASKENTRY_H
#pragma once

#include <QString>
#include <QJsonObject>

class TaskEntry
{
    QString m_id;
    QString m_title;
    bool m_done = false;
    qint64 m_dueAt = 0;
    qint64 m_createdAt = 0;
    qint64 m_updatedAt = 0;
    QString m_projectId; // "" = Inbox

    friend class TaskRepository;
    friend class TaskController;

public:
    TaskEntry() = default;

    QJsonObject toJson() const;
    static TaskEntry fromJson(const QJsonObject &obj);

    QString id() const { return m_id; }
    QString title() const { return m_title; }
    bool done() const { return m_done; }
    qint64 dueAt() const { return m_dueAt; }
    qint64 createdAt() const { return m_createdAt; }
    qint64 updatedAt() const { return m_updatedAt; }
    QString projectId() const { return m_projectId; }
};

#endif