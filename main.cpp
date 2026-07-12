#include "mainwindow.h"
#include "ui/loginscreen.h"
#include "core/crypto/cryptomanager.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setApplicationName("Anchor"); // important: keeps app data separate
    // from the "Tests" folder used by
    // the test suite

    if (!CryptoManager::init()) {
        return 1; // libsodium failed to initialize — cannot proceed safely
    }

    auto *loginScreen = new LoginScreen();
    MainWindow *mainWindow = nullptr;

    QObject::connect(loginScreen, &::LoginScreen::unlocked,
                     [&](const QByteArray &sessionKey) {
                         mainWindow = new MainWindow();
                         // TODO: pass sessionKey into MainWindow once it needs to use it
                         // (e.g. mainWindow->setSessionKey(sessionKey);) — MainWindow
                         // doesn't need it yet since no repositories are wired into the
                         // UI at this stage.
                         mainWindow -> show();
                         loginScreen -> deleteLater();
                     });

    loginScreen->show();
    return QApplication::exec();
}
