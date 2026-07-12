#include "mainwindow.h"
#include "ui/loginscreen.h"
#include "app/session.h"
#include "core/crypto/cryptomanager.h"
#include "core/security/autolockmanager.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setApplicationName("Anchor");
    a.setQuitOnLastWindowClosed(true);

    if (!CryptoManager::init()) {
        return 1;
    }

    Autolockmanager autoLockManager;
    autoLockManager.setTimeOutSeconds(180); // 3 minutes — adjust to taste

    auto *loginScreen = new LoginScreen();
    loginScreen->setAttribute(Qt::WA_DeleteOnClose);

    MainWindow *mainWindow = nullptr;

    QObject::connect(loginScreen, &LoginScreen::unlocked,
                     [&](const QByteArray &sessionKey) {
                         Session::instance()->unlock(sessionKey);
                         autoLockManager.start();   // ← begin the countdown

                         mainWindow = new MainWindow();
                         mainWindow->setAttribute(Qt::WA_DeleteOnClose);
                         mainWindow->show();

                         loginScreen->close();
                     });

    QObject::connect(Session::instance(), &Session::locked,
                     [&]() {
                         autoLockManager.stop();    // ← pause until next unlock

                         if (mainWindow) {
                             mainWindow->close();
                             mainWindow = nullptr;
                         }

                         loginScreen = new LoginScreen();
                         loginScreen->setAttribute(Qt::WA_DeleteOnClose);

                         QObject::connect(loginScreen, &LoginScreen::unlocked,
                                          [&](const QByteArray &sessionKey) {
                                              Session::instance()->unlock(sessionKey);
                                              autoLockManager.start();

                                              mainWindow = new MainWindow();
                                              mainWindow->setAttribute(Qt::WA_DeleteOnClose);
                                              mainWindow->show();

                                              loginScreen->close();
                                          });

                         loginScreen->show();
                     });

    loginScreen->show();
    return QApplication::exec();
}
