#include "mainwindow.h"
#include "ui/loginscreen.h"
#include "app/session.h"
#include "core/crypto/cryptomanager.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setApplicationName("Anchor");
    a.setQuitOnLastWindowClosed(true);

    if (!CryptoManager::init()) {
        return 1;
    }

    // ── Create windows ──────────────────────────────────────────
    auto *loginScreen = new LoginScreen();
    loginScreen->setAttribute(Qt::WA_DeleteOnClose);

    MainWindow *mainWindow = nullptr;

    // ── Unlock flow: correct password → unlock Session → show app
    QObject::connect(loginScreen, &LoginScreen::unlocked,
                     [&](const QByteArray &sessionKey) {
                         Session::instance()->unlock(sessionKey);

                         mainWindow = new MainWindow();
                         mainWindow->setAttribute(Qt::WA_DeleteOnClose);
                         mainWindow->show();

                         loginScreen->close();  // WA_DeleteOnClose frees it
                     });

    // ── Lock flow: Session locks → show LoginScreen again
    QObject::connect(Session::instance(), &Session::locked,
                     [&]() {
                         // MainWindow is still visible — hide it first
                         if (mainWindow) {
                             mainWindow->close();  // WA_DeleteOnClose frees it
                             mainWindow = nullptr;
                         }

                         // Create a fresh login screen for re-authentication
                         loginScreen = new LoginScreen();
                         loginScreen->setAttribute(Qt::WA_DeleteOnClose);

                         QObject::connect(loginScreen, &LoginScreen::unlocked,
                                          [&](const QByteArray &sessionKey) {
                                              Session::instance()->unlock(sessionKey);

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