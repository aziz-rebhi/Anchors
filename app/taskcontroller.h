#ifndef TASKCONTROLLER_H
#define TASKCONTROLLER_H
#pragma once

#include <QObject>
#include <QString>
#include <QVariantList>

// QML-facing bridge to TaskRepository. See VaultController for the
// session-key-freshness rationale.
class TaskController : public QObject
{
    Q_OBJECT

public:
    explicit TaskController(QObject *parent = nullptr);

    // Returns all tasks as a list of maps: id, title, done,
    // dueAt (secs since epoch, 0 = none), createdAt, updatedAt.
    Q_INVOKABLE QVariantList entries() const;

    // dueAtSecs: pass 0 for "no due date".
    Q_INVOKABLE bool addEntry(const QString &title, qint64 dueAtSecs);

    Q_INVOKABLE bool updateEntry(const QString &id, const QString &title, qint64 dueAtSecs);

    Q_INVOKABLE bool setDone(const QString &id, bool done);

    Q_INVOKABLE bool deleteEntry(const QString &id);

signals:
    void entriesChanged();
    void operationFailed(QString reason);
};

#endif // TASKCONTROLLER_H
