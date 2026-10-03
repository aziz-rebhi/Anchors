#include "NotesEditorHost.h"

NotesEditorHost::NotesEditorHost(QObject *parent) : QObject(parent) {}

// --- host -> page ------------------------------------------------------------
// The matching push into the web view lives in NotesPageEditor.qml, which
// watches these signals and injects the call via runJavaScript.

void NotesEditorHost::loadNote(const QString &title, const QString &json)
{
    m_pendingTitle = title;
    m_pendingJson = json;
    emit loadNoteRequested(title, json);
}

void NotesEditorHost::setTheme(const QString &mode)
{
    m_themeMode = (mode == QLatin1String("light")) ? QStringLiteral("light")
                                                   : QStringLiteral("dark");
    emit themeRequested(m_themeMode);
}

// --- page -> host ------------------------------------------------------------

void NotesEditorHost::saveNote(const QString &title, const QString &json)
{
    emit noteSaveRequested(title, json);
}

void NotesEditorHost::titleChanged(const QString &title)
{
    emit noteTitleChanged(title);
}

void NotesEditorHost::ready()
{
    m_pageReady = true;
    emit editorReady();
}