#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QDebug>

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
#include "app/settingscontroller.h"
#include "core/security/autolockmanager.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    // Consistent QSettings + QStandardPaths on every OS
    QCoreApplication::setOrganizationName(QStringLiteral("Anchors"));
    QCoreApplication::setApplicationName(QStringLiteral("Anchors"));

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

    QString dbPath = FilePaths::dataDir() + "/notes.db";
    if (!NotesDatabase::instance()->initialize(dbPath)) {
        qCritical() << "Failed to initialize notes database!";
        return 1;
    }

    auto *settingsController = new SettingsController(&app);
    auto *autoLock = new Autolockmanager(&app);

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

    QObject::connect(Session::instance(), &Session::unlocked, autoLock, [autoLock, settingsController]() {
        autoLock->setTimeoutMinutes(settingsController->autoLockMinutes());
        autoLock->setLockOnMinimize(settingsController->lockOnMinimize());
        autoLock->start();
    });
    QObject::connect(Session::instance(), &Session::locked,
                     autoLock, &Autolockmanager::stop);

    engine.rootContext()->setContextProperty(QStringLiteral("settingsController"), settingsController);
    engine.rootContext()->setContextProperty("authController", &authController);
    engine.rootContext()->setContextProperty("session", Session::instance());
    engine.rootContext()->setContextProperty("vaultController", &vaultController);
    engine.rootContext()->setContextProperty("noteController", &noteController);
    engine.rootContext()->setContextProperty("calendarController", &calendarController);
    engine.rootContext()->setContextProperty("taskController", &taskController);
    engine.rootContext()->setContextProperty("noteEditor", &noteEditorController);

    qmlRegisterType<CodeHighlightBridge>("Anchors", 1, 0, "CodeHighlightBridge");

    engine.load(QUrl(QStringLiteral("qrc:/qml/Main.qml")));
    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}