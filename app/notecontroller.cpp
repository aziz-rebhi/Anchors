#include "notecontroller.h"
#include "session.h"

#include "../core/models/noteentry.h"
#include "../core/storage/repositories/noterepository.h"

#include <QVariantMap>

NoteController::NoteController(QObject *parent) : QObject(parent)
{
}

QVariantList NoteController::entries() const
{
    QVariantList list;

    if (!Session::instance()->isUnlocked()) {
        return list;
    }

    NoteRepository repo(Session::instance()->sessionKey());
    bool ok = false;
    const QVector<NoteEntry> all = repo.loadAll(&ok);
    if (!ok) {
        return list;
    }

    list.reserve(all.size());
    for (const NoteEntry &e : all) {
        QVariantMap m;
        m["id"] = e.id();
        m["title"] = e.title();
        m["content"] = e.content();
        m["createdAt"] = e.createdAt();
        m["updatedAt"] = e.updatedAt();
        list.append(m);
    }
    return list;
}

bool NoteController::addEntry(const QString &title, const QString &content)
{
    if (!Session::instance()->isUnlocked()) {
        emit operationFailed(QStringLiteral("Notes are locked."));
        return false;
    }

    NoteRepository repo(Session::instance()->sessionKey());
    NoteEntry e;
    e.m_title = title;
    e.m_content = content;

    const bool ok = repo.addEntry(e);
    if (ok) {
        emit entriesChanged();
    } else {
        emit operationFailed(QStringLiteral("Could not save the note."));
    }
    return ok;
}

bool NoteController::updateEntry(const QString &id, const QString &title, const QString &content)
{
    if (!Session::instance()->isUnlocked()) {
        emit operationFailed(QStringLiteral("Notes are locked."));
        return false;
    }

    NoteRepository repo(Session::instance()->sessionKey());
    NoteEntry e;
    e.m_id = id;
    e.m_title = title;
    e.m_content = content;

    const bool ok = repo.updateEntry(e);
    if (ok) {
        emit entriesChanged();
    } else {
        emit operationFailed(QStringLiteral("Could not update the note."));
    }
    return ok;
}

bool NoteController::deleteEntry(const QString &id)
{
    if (!Session::instance()->isUnlocked()) {
        emit operationFailed(QStringLiteral("Notes are locked."));
        return false;
    }

    NoteRepository repo(Session::instance()->sessionKey());
    const bool ok = repo.deleteEntry(id);
    if (ok) {
        emit entriesChanged();
    } else {
        emit operationFailed(QStringLiteral("Could not delete the note."));
    }
    return ok;
}
