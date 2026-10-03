import QtQuick
import QtQuick.Controls
// The QML module is "QtWebEngine"; "QtWebEngineQuick" is only the C++
// namespace and is not a valid QML import.
import QtWebEngine

// Wraps the embedded BlockNote (TypeScript) notes editor in a WebEngineView.
//
// The host -> page direction (loadNote / setTheme) is pushed with runJavaScript
// rather than over the WebChannel. WebChannel only lets the *page* pull host
// signals, so pushing would otherwise need a connectToSignal handshake that the
// page has to opt into - more protocol code for no benefit, since nothing here
// needs a reply.
//
// Readiness is handled by an explicit handshake: the bundle calls
// anchorsNotes.ready() once window.anchorsNotes exists and React has mounted.
// That arrives as NotesEditorHost.editorReady, and everything the host asked for
// in the meantime is replayed from the host's pending* properties. So a note
// selected while the view is still painting is never lost.
//
// Usage from NotesPage.qml:
//   NotesPageEditor {
//       host: notesEditorHost
//       themeMode: theme.isDark ? "dark" : "light"
//       // editorUrl comes from ANCHORS_NOTES_DEV_URL, or defaults to the
//       // packaged bundle at qrc:/notes-editor/index.html
//       onSaveRequested: (title, json) => ...
//       onTitleEdited: (title) => ...
//   }
Item {
    id: root

    // NotesEditorHost. Required - without it there is nothing to bridge to.
    property var host: null

    // QWebChannel the page talks over, created in main.cpp. Without it Qt never
    // injects window.qt.webChannelTransport into the page and the editor stays
    // permanently in standalone mode (renders, but never receives or sends a
    // note). Must be assigned before the view's first navigation, so it is a
    // plain binding rather than something done in onCompleted.
    property var channel: typeof notesWebChannel !== "undefined" ? notesWebChannel : null

    // Where the editor bundle is loaded from.
    //
    // Production default is the qrc path - the built bundle compiled into the
    // binary, so a released app needs no Node, no Vite and no port 5173. The
    // relative asset URLs in that bundle resolve against qrc:/notes-editor/,
    // which is why the prefix here must match the one emit-qrc.mjs writes.
    //
    // Set ANCHORS_NOTES_DEV_URL to point at a running `npm run dev` instead.
    // main.cpp reads it and exposes it as `notesEditorDevUrl`; when it is empty
    // this stays on the packaged bundle. That keeps the dev loop available
    // without a rebuild, and - more importantly - keeps the shipped default from
    // ever silently depending on a developer machine having a server up.
    // Read-only on purpose. It used to be an assignable property that this file
    // *and* NotesPage.qml each set to localhost:5173 - two places deciding the
    // same thing, neither checked against the other, which is how a release build
    // ended up pointing at a dev server. Callers change the flag, not this.
    readonly property string editorUrl:
        (typeof notesEditorDevUrl !== "undefined" && notesEditorDevUrl.length > 0)
            ? notesEditorDevUrl
            : "qrc:/notes-editor/index.html"

    property string themeMode: "dark"

    // True once the page has reported ready(). Outgoing calls are dropped until
    // then and replayed by syncFromHost().
    property bool pageReady: false

    // Whether the view is actually on screen. Pushing into a hidden view wastes
    // work and can race with a page that is about to be torn down.
    readonly property bool active: visible && width > 0 && height > 0

    signal saveRequested(string title, string json)
    signal titleEdited(string title)
    signal ready()

    Component.onCompleted: engine.syncHostState()

    onActiveChanged: if (active) engine.syncHostState()

    // Theme changes are recorded on the host immediately; the page picks them up
    // on the next sync. Nothing else to do here.
    onThemeModeChanged: if (active && pageReady) engine.pushSetTheme(themeMode)

    // Open a note. Safe before the page is ready - the host keeps the payload
    // and syncHostState() replays it.
    function loadNote(title, json) {
        if (!host)
            return
        host.loadNote(title === undefined ? "" : title, json === undefined ? "" : json)
        if (active && pageReady)
            Qt.callLater(function () { engine.pushLoadNote(title, json) })
    }

    function setTheme(mode) {
        if (!host)
            return
        host.setTheme(mode === undefined ? "dark" : mode)
        if (active && pageReady)
            Qt.callLater(function () { engine.pushSetTheme(mode) })
    }

    // Ask the page to emit any debounced edit immediately, so switching notes or
    // deleting one cannot lose the last <800ms of typing.
    function flushSave() {
        if (pageReady)
            engine.pushFlushSave()
    }

    WebEngineView {
        id: engine

        anchors.fill: parent

        // The page only ever needs its own bundle. Deny the cross-origin escape
        // hatches so a compromised dependency cannot reach back into the shell.
        settings.allowRunningInsecureContent: false
        settings.localContentCanAccessRemoteUrls: false
        settings.localContentCanAccessFileUrls: false

        // Declared before `url` on purpose: Qt only injects the transport into
        // documents created after the channel is attached.
        webChannel: root.channel

        url: root.editorUrl

        onLoadingChanged: function (loadRequest) {
            if (!loadRequest)
                return
            // A reload (including Vite HMR full-reloads) tears down the page's
            // bridge, so readiness has to be re-established before we push again.
            if (loadRequest.isLoading && loadRequest.url == engine.url) {
                root.pageReady = false
                engine.scheduleSync()
            }
            if (loadRequest.isError)
                console.warn("NotesPageEditor: failed to load",
                             loadRequest.url, loadRequest.errorString)
        }

        // --- host -> page ---------------------------------------------------

        // Called by the host to deliver the current note and theme. Also runs
        // after every page load, once the page reports ready().
        function syncHostState() {
            if (!root.host || !root.active || !root.pageReady)
                return
            pushSetTheme(root.host.themeMode)
            pushLoadNote(root.host.pendingTitle, root.host.pendingJson)
        }

        function scheduleSync() {
            // The page installs its bridge asynchronously after the document
            // loads, so wait a beat for the ready() handshake instead of pushing
            // into an empty window.
            syncTimer.restart()
        }

        function pushLoadNote(title, json) {
            // Encoded as JSON literals so quotes and newlines in the document
            // cannot break out of the injected script.
            var payload = JSON.stringify({
                title: title === undefined ? "" : title,
                json: json === undefined ? "" : json
            })
            runJavaScript("window.anchorsNotes && window.anchorsNotes.loadNote("
                          + payload + ");")
        }

        function pushSetTheme(mode) {
            runJavaScript("window.anchorsNotes && window.anchorsNotes.setTheme("
                          + JSON.stringify(mode === "light" ? "light" : "dark") + ");")
        }

        function pushFlushSave() {
            runJavaScript("window.anchorsNotes && window.anchorsNotes.flushSave();")
        }

        Timer {
            id: syncTimer
            interval: 150
            onTriggered: engine.syncHostState()
        }

        // --- page -> host ---------------------------------------------------

        Connections {
            target: root.host
            ignoreUnknownSignals: true

            function onEditorReady() {
                root.pageReady = true
                // The page just booted - push whatever the host was holding.
                engine.scheduleSync()
                root.ready()
            }

            function onNoteSaveRequested(title, json) {
                root.saveRequested(title, json)
            }

            function onNoteTitleChanged(title) {
                root.titleEdited(title)
            }
        }
    }
}