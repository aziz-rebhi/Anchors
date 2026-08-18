#ifndef PROJECTENTRY_H
#define PROJECTENTRY_H
#pragma once

#include <QString>
#include <QJsonObject>

class ProjectEntry
{
    friend class TaskRepository;
    friend class TaskController;

    QString m_id;
    QString m_name;
    QString m_emoji;   // e.g. "💼"
    QString m_color;   // e.g. "#89b4fa"
    int     m_order = 0;
    qint64  m_createdAt = 0;
    qint64  m_updatedAt = 0;

public:
    ProjectEntry() = default;

    QJsonObject toJson() const;
    static ProjectEntry fromJson(const QJsonObject &obj);

    QString id() const { return m_id; }
    QString name() const { return m_name; }
    QString emoji() const { return m_emoji; }
    QString color() const { return m_color; }
    int order() const { return m_order; }
    qint64 createdAt() const { return m_createdAt; }
    qint64 updatedAt() const { return m_updatedAt; }
};

#endif