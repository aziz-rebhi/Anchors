#ifndef NOTESEDITORHOST_H
#define NOTESEDITORHOST_H
#pragma once

#include <QObject>
#include <QString>

// QObject exposed to the embedded TypeScript editor over QWebChannel under the
// name "anchorsNotes".
//
// The bridge is bidirectional, but the two directions travel differently:
//
//   page -> host  saveNote / titleChanged / ready
//                  Slots invoked by the web page over the WebChannel transport
//                  (window.qt.webChannelTransport). They are re-emitted as Qt
//                  signals so NotesPage.qml can react without knowing that a web
//                  view exists.
//
//   host -> page  loadNote / setTheme
//                  Q_INVOKABLEs called from QML. Pushing a message *into* the
//                  page over WebChannel requires the page to connect to a host
//                  signal first, which needs the whole connectToSignal dance. We
//                  skip that: NotesPageEditor.qml injects these two calls with
//                  runJavaScript instead, so this class stays free of any web
//                  view dependency and can be unit-tested on its own.
//
// Crypto and disk persistence stay in C++; this class never touches the note
// repository.
class NotesEditorHost : public QObject
{
    Q_OBJECT

    // Last payload handed down, replayed by NotesPageEditor once the page
    // reports ready(). Needed because a note can be selected before the first
    // paint of the WebEngine view.
    Q_PROPERTY(QString pendingTitle READ pendingTitle NOTIFY loadNoteRequested)
    Q_PROPERTY(QString pendingJson READ pendingJson NOTIFY loadNoteRequested)
    Q_PROPERTY(QString themeMode READ themeMode NOTIFY themeRequested)
    Q_PROPERTY(bool pageReady READ pageReady NOTIFY editorReady)

public:
    explicit NotesEditorHost(QObject *parent = nullptr);

    QString pendingTitle() const { return m_pendingTitle; }
    QString pendingJson() const { return m_pendingJson; }
    QString themeMode() const { return m_themeMode; }
    bool pageReady() const { return m_pageReady; }

    // --- host -> page -------------------------------------------------------

    // Opens a note in the editor. Empty title/json clears the canvas.
    Q_INVOKABLE void loadNote(const QString &title, const QString &json);

    // "dark" or "light"; anything else falls back to dark.
    Q_INVOKABLE void setTheme(const QString &mode);

    // --- page -> host -------------------------------------------------------

public slots:
    // Carries an edited note back to the host. Emits noteSaveRequested.
    void saveNote(const QString &title, const QString &json);

    // Reports a title edit made in the page. Emits noteTitleChanged.
    void titleChanged(const QString &title);

    // The page has booted and can accept loadNote/setTheme calls.
    void ready();

signals:
    // Consumed by NotesPageEditor to drive runJavaScript into the page.
    void loadNoteRequested(const QString &title, const QString &json);
    void themeRequested(const QString &mode);

    // Consumed by NotesPage.qml for persistence.
    void noteSaveRequested(const QString &title, const QString &json);
    void noteTitleChanged(const QString &title);
    void editorReady();

private:
    QString m_pendingTitle;
    QString m_pendingJson;
    QString m_themeMode = QStringLiteral("dark");
    bool m_pageReady = false;
};

#endif // NOTESEDITORHOST_H