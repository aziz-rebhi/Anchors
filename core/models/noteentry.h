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

    friend class NoteRepository;
    friend class NotePage;

public:
    NoteEntry() = default;

    QJsonObject toJson() const;
    static NoteEntry fromJson(const QJsonObject &obj);

    QString id() const {return m_id;}
    QString title() const { return m_title;}
    QString content() const {return m_content;}
    qint64 createdAt() const {return m_createdAt;}

};

#endif // NOTEENTRY_H
