#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QIcon>
#include <QDebug>
#include <QWindow>

#include "app/session.h"
#include "app/authcontroller.h"
#include "app/vaultcontroller.h"
#include "app/notecontroller.h"
#include "app/calendarcontroller.h"
#include "app/taskcontroller.h"
#include "core/crypto/cryptomanager.h"
#include "core/storage/notesdatabase.h"
#include "core/storage/FilePaths.h"
#include "app/noteeditorcontroller.h"
#include "core/editor/codehighlightbridge.h"
#include "app/NotesEditorHost.h"
#include "app/settingscontroller.h"
#include "core/security/autolockmanager.h"
#include "core/security/cliboardguard.h"
#include "core/security/remindermanager.h"
#include "core/editor/richtexthelper.h"

#include <QtWebEngineQuick>
#include <QQmlWebChannel>

int main(int argc, char *argv[])
{
    // Chromium's GPU compositor faults on this hybrid NVIDIA/AMD box. Two separate
    // symptoms, two separate causes, and they need opposite fixes:
    //
    //   * GPU compositing enabled  -> Mesa's libgallium segfaults the process.
    //   * --disable-features=Vulkan -> no segfault, but every frame fails with
    //     "dma_buf acquisition failure / Compositor returned null texture" and
    //     the canvas renders blank. Verified by pixel count: 1 distinct colour.
    //
    // So do not disable Vulkan. --disable-gpu-compositing alone dodges both: the
    // compositor goes software (SHM path, no dma-buf) while Skia keeps
    // rasterising on the GPU. Verified 3/3 runs - no crash, ~83-104 distinct
    // colours in a window grab (i.e. real content), bridge round-tripping.
    //
    // Must be set before initialize(), which is what reads the environment -
    // Qt 6.11's initialize() takes no arguments, so this is the only entry point.
    qputenv("QTWEBENGINE_CHROMIUM_FLAGS", "--disable-gpu-compositing");

    // Must precede QGuiApplication construction: this sets up the shared OpenGL
    // context the WebEngine view needs. Calling it later (e.g. just before
    // engine.load) crashes as soon as the view is created.
    QtWebEngineQuick::initialize();

    // QApplication (not QGuiApplication) so QSystemTrayIcon notifications work
    QApplication app(argc, argv);

    QCoreApplication::setOrganizationName(QStringLiteral("Anchors"));
    QCoreApplication::setApplicationName(QStringLiteral("Anchors"));
    QGuiApplication::setDesktopFileName(QStringLiteral("Anchors"));
    app.setWindowIcon(QIcon(QStringLiteral(":/qml/logo.png")));

    if (!CryptoManager::init()) {
        qCritical() << "ERROR: Failed to init libsodium!";
        return 1;
    }

    QQmlApplicationEngine engine;

    AuthController authController;
    VaultController vaultController;
    NoteController noteController;
    CalendarController calendarController;
    TaskController taskController;
    NoteEditorController noteEditorController;
    // Bridge to the embedded notes editor. Owned by the app; the QML side picks
    // it up as `notesEditorHost` and NotesPageEditor wires it to the web view.
    auto *notesEditorHost = new NotesEditorHost(&app);
    // The transport the page talks over. Must be a QQmlWebChannel, not a plain
    // QWebChannel: WebEngineView's `webChannel` property is typed as the former
    // and silently rejects the latter with "Cannot assign QObject* to
    // QQmlWebChannel*". Registering the host under the name the page looks up
    // (HOST_OBJECT_NAME in notes-editor/src/bridge.ts).
    auto *notesWebChannel = new QQmlWebChannel(&app);
    notesWebChannel->registerObject(QStringLiteral("anchorsNotes"), notesEditorHost);

    QString dbPath = FilePaths::dataDir() + "/notes.db";
    if (!NotesDatabase::instance()->initialize(dbPath)) {
        qCritical() << "Failed to initialize notes database!";
        return 1;
    }

    auto *settingsController = new SettingsController(&app);
    auto *autoLock = new Autolockmanager(&app);
    auto *clipboardGuard = new CliboardGuard(&app);
    auto *reminders = new ReminderManager(&app);
    auto *richTextHelper = new RichTextHelper(&app);

    auto applySecuritySettings = [autoLock, settingsController]() {
        autoLock->setTimeoutMinutes(settingsController->autoLockMinutes());
        autoLock->setLockOnMinimize(settingsController->lockOnMinimize());
        if (Session::instance()->isUnlocked())
            autoLock->start();
        else
            autoLock->stop();
    };

    applySecuritySettings();

    QObject::connect(settingsController, &SettingsController::autoLockMinutesChanged,
                     applySecuritySettings);
    QObject::connect(settingsController, &SettingsController::lockOnMinimizeChanged,
                     applySecuritySettings);

    // Auto-lock while unlocked
    QObject::connect(Session::instance(), &Session::unlocked, autoLock,
                     [autoLock, settingsController]() {
                         autoLock->setTimeoutMinutes(settingsController->autoLockMinutes());
                         autoLock->setLockOnMinimize(settingsController->lockOnMinimize());
                         autoLock->start();
                     });
    QObject::connect(Session::instance(), &Session::locked,
                     autoLock, &Autolockmanager::stop);

    // Reminders only while vault/session is unlocked (needs encryption key)
    QObject::connect(Session::instance(), &Session::unlocked,
                     reminders, &ReminderManager::start);
    QObject::connect(Session::instance(), &Session::locked,
                     reminders, &ReminderManager::stop);
    if (Session::instance()->isUnlocked())
        reminders->start();

    engine.rootContext()->setContextProperty(QStringLiteral("settingsController"), settingsController);
    engine.rootContext()->setContextProperty(QStringLiteral("clipboardGuard"), clipboardGuard);
    engine.rootContext()->setContextProperty("authController", &authController);
    engine.rootContext()->setContextProperty("session", Session::instance());
    engine.rootContext()->setContextProperty("vaultController", &vaultController);
    engine.rootContext()->setContextProperty("noteController", &noteController);
    engine.rootContext()->setContextProperty("calendarController", &calendarController);
    engine.rootContext()->setContextProperty("taskController", &taskController);
    engine.rootContext()->setContextProperty("noteEditor", &noteEditorController);
    engine.rootContext()->setContextProperty("notesEditorHost", notesEditorHost);
    engine.rootContext()->setContextProperty("notesWebChannel", notesWebChannel);

    // Where the notes editor bundle comes from. Empty - the default - means
    // "load the bundle compiled into this binary", which is what a released
    // build must do. A build that quietly required http://localhost:5173/ would
    // show a blank editor to every user who is not also running a dev server,
    // and the symptom (empty canvas, no error) points nowhere near the cause.
    //
    // Set ANCHORS_NOTES_DEV_URL=http://localhost:5173/ to use `npm run dev`
    // instead. Read from the environment rather than a qmake define so the
    // override works on an already-built binary - which is the case that
    // actually matters when bisecting a release.
    const QString devEditorUrl =
        QString::fromLocal8Bit(qgetenv("ANCHORS_NOTES_DEV_URL")).trimmed();
    if (devEditorUrl.isEmpty()) {
        qInfo().noquote() << "Anchors: notes editor from qrc:/notes-editor/index.html (packaged)";
    } else {
        qWarning().noquote()
            << "Anchors: notes editor from" << devEditorUrl
            << "- dev override active, NOT the packaged bundle";
    }
    engine.rootContext()->setContextProperty(QStringLiteral("notesEditorDevUrl"), devEditorUrl);

    engine.rootContext()->setContextProperty(QStringLiteral("richTextHelper"), richTextHelper);

    qmlRegisterType<CodeHighlightBridge>("Anchors", 1, 0, "CodeHighlightBridge");

    engine.load(QUrl(QStringLiteral("qrc:/qml/Main.qml")));
    if (engine.rootObjects().isEmpty())
        return -1;

    if (QWindow *win = qobject_cast<QWindow *>(engine.rootObjects().constFirst())) {
        win->setIcon(QIcon(QStringLiteral(":/qml/logo.png")));
    }

    return app.exec();
}