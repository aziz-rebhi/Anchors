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

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    // Libsodium must be initialized before anything touches CryptoManager,
    // SaltStore, or AuthController.
    if (!CryptoManager::init()) {
        qCritical() << "ERROR: Failed to init libsodium!";
        return 1;
    }

    QQmlApplicationEngine engine;

    // These are plain local objects (not singletons) - none of them hold
    // state of their own. Each re-reads Session::instance()->sessionKey()
    // fresh on every call, so a single instance stays correct across
    // lock/unlock cycles for the lifetime of the app.
    AuthController authController;
    VaultController vaultController;
    NoteController noteController;
    CalendarController calendarController;
    TaskController taskController;

    engine.rootContext()->setContextProperty("authController", &authController);
    engine.rootContext()->setContextProperty("session", Session::instance());
    engine.rootContext()->setContextProperty("vaultController", &vaultController);
    engine.rootContext()->setContextProperty("noteController", &noteController);
    engine.rootContext()->setContextProperty("calendarController", &calendarController);
    engine.rootContext()->setContextProperty("taskController", &taskController);

    // Removed the objectCreationFailed connection – it does not exist in Qt6.
    // The check for engine.rootObjects().isEmpty() below handles failure.

    engine.load(QUrl(QStringLiteral("qrc:/qml/Main.qml")));

    if (engine.rootObjects().isEmpty()) {
        return -1;
    }

    return app.exec();
}