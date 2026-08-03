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
        m["folder"] = e.m_folder;
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
    bool ok;
    auto all = repo.loadAll(&ok);
    if (!ok) {
        emit operationFailed(QStringLiteral("Could not load notes."));
        return false;
    }

    // Find the existing note to preserve its folder
    NoteEntry e;
    bool found = false;
    for (const auto &existing : all) {
        if (existing.m_id == id) {
            e = existing; // copy all fields
            e.m_title = title;
            e.m_content = content;
            found = true;
            break;
        }
    }
    if (!found) {
        emit operationFailed(QStringLiteral("Note not found."));
        return false;
    }

    const bool updated = repo.updateEntry(e);
    if (updated) {
        emit entriesChanged();
    } else {
        emit operationFailed(QStringLiteral("Could not update the note."));
    }
    return updated;
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


QStringList NoteController::getFolders() const
{
    if (!Session::instance()->isUnlocked()) return {};
    NoteRepository repo(Session::instance()->sessionKey());
    bool ok; auto entries = repo.loadAll(&ok);
    if (!ok) return {};
    QStringList folders;
    for (const auto &e : entries) {
        if (!e.m_folder.isEmpty() && !folders.contains(e.m_folder))
            folders << e.m_folder;
    }
    folders.sort();
    return folders;
}

QVariantList NoteController::entriesForFolder(const QString &folder) const
{
    QVariantList list;
    if (!Session::instance()->isUnlocked()) return list;
    NoteRepository repo(Session::instance()->sessionKey());
    bool ok; auto all = repo.loadAll(&ok);
    if (!ok) return list;
    for (const auto &e : all) {
        if (e.m_folder != folder) continue;
        QVariantMap m;
        m["id"] = e.id();
        m["title"] = e.title();
        m["content"] = e.content();
        m["createdAt"] = e.createdAt();
        m["updatedAt"] = e.updatedAt();
        m["folder"] = e.m_folder;
        list.append(m);
    }
    return list;
}

bool NoteController::moveNoteToFolder(const QString &noteId, const QString &folderName)
{
    if (!Session::instance()->isUnlocked()) return false;
    NoteRepository repo(Session::instance()->sessionKey());
    bool ok; auto all = repo.loadAll(&ok);
    if (!ok) return false;
    for (auto &e : all) {
        if (e.id() == noteId) {
            e.m_folder = folderName;
            return repo.saveAll(all);
        }
    }
    return false;
}

bool NoteController::renameFolder(const QString &oldName, const QString &newName)
{
    if (oldName.isEmpty() || newName.isEmpty() || oldName == newName) return false;
    if (!Session::instance()->isUnlocked()) return false;
    NoteRepository repo(Session::instance()->sessionKey());
    bool ok; auto all = repo.loadAll(&ok);
    if (!ok) return false;
    bool changed = false;
    for (auto &e : all) {
        if (e.m_folder == oldName) {
            e.m_folder = newName;
            changed = true;
        }
    }
    return changed && repo.saveAll(all);
}

bool NoteController::deleteFolder(const QString &folderName)
{
    if (folderName.isEmpty()) return false;
    if (!Session::instance()->isUnlocked()) return false;
    NoteRepository repo(Session::instance()->sessionKey());
    bool ok; auto all = repo.loadAll(&ok);
    if (!ok) return false;
    // Move all notes in this folder to "" (no folder)
    bool changed = false;
    for (auto &e : all) {
        if (e.m_folder == folderName) {
            e.m_folder = "";
            changed = true;
        }
    }
    return changed && repo.saveAll(all);
}

bool NoteController::addEntryInFolder(const QString &title, const QString &content, const QString &folderName)
{
    if (!Session::instance()->isUnlocked()) {
        emit operationFailed(QStringLiteral("Notes are locked."));
        return false;
    }

    NoteRepository repo(Session::instance()->sessionKey());
    NoteEntry e;
    e.m_title = title;
    e.m_content = content;
    e.m_folder = folderName;

    const bool ok = repo.addEntry(e);
    if (ok) {
        emit entriesChanged();
    } else {
        emit operationFailed(QStringLiteral("Could not save the note."));
    }
    return ok;
}