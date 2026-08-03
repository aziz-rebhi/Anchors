#ifndef NOTEENTRY_H
#define NOTEENTRY_H

#pragma once

#include <QString>
#include <QJsonObject>

class NoteEntry
{
    QString m_id;
    QString m_title;
    QString m_content;
    qint64 m_createdAt;
    qint64 m_updatedAt;
    QString m_folder;

    friend class NoteRepository;
    friend class NotePage;
    friend class NoteController;

public:
    NoteEntry() = default;

    QJsonObject toJson() const;
    static NoteEntry fromJson(const QJsonObject &obj);

    QString id() const {return m_id;}
    QString title() const { return m_title;}
    QString content() const {return m_content;}
    qint64 createdAt() const {return m_createdAt;}
    qint64 updatedAt() const {return m_updatedAt;}

};

#endif // NOTEENTRY_H