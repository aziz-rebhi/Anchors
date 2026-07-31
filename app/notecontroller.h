#ifndef NOTECONTROLLER_H
#define NOTECONTROLLER_H
#pragma once

#include <QObject>
#include <QString>
#include <QVariantList>

// QML-facing bridge to NoteRepository. See VaultController for the
// session-key-freshness rationale.
class NoteController : public QObject
{
    Q_OBJECT

public:
    explicit NoteController(QObject *parent = nullptr);

    // Returns all notes as a list of maps: id, title, content,
    // createdAt, updatedAt.
    Q_INVOKABLE QVariantList entries() const;

    Q_INVOKABLE bool addEntry(const QString &title, const QString &content);
    Q_INVOKABLE bool updateEntry(const QString &id, const QString &title, const QString &content);
    Q_INVOKABLE bool deleteEntry(const QString &id);

signals:
    void entriesChanged();
    void operationFailed(QString reason);
};

#endif // NOTECONTROLLER_H
