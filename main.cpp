#include "core/security/autolockmanager.h"
#include "mainwindow.h"
#include "ui/loginscreen.h"
#include "ui/welcomescreen.h"
#include "app/session.h"
#include "core/crypto/cryptomanager.h"
#include "core/security/autolockmanager.h"
#include "core/models/profile.h"
#include "core/storage/repositories/profilerepository.h"
#include "core/storage/saltstore.h"

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
    autoLockManager.setTimeOutSeconds(180);

    MainWindow *mainWindow = nullptr;
    LoginScreen *loginScreen = nullptr;

    profile pendingProfile;


    auto showLoginScreen = [&]() {
        loginScreen = new LoginScreen();
        loginScreen->setAttribute(Qt::WA_DeleteOnClose);

        QObject::connect(loginScreen, &LoginScreen::unlocked,
                         [&](const QByteArray &sessionKey) {
                             Session::instance()->unlock(sessionKey);
                             autoLockManager.start();

                             if (!pendingProfile.name.isEmpty() || pendingProfile.birthday.isValid()) {
                                 profilerepository(sessionKey).save(pendingProfile);
                             }

                             mainWindow = new MainWindow();
                             mainWindow->setAttribute(Qt::WA_DeleteOnClose);
                             mainWindow->show();

                             loginScreen->close();
                         });

        loginScreen->show();
    };

    QObject::connect(Session::instance(), &Session::locked,
                     [&]() {
                         autoLockManager.stop();

                         if (mainWindow) {
                             mainWindow->close();
                             mainWindow = nullptr;
                         }

                         showLoginScreen();
                     });

    if (!SaltStore::exists()) {
        auto *welcomeScreen = new Welcomescreen();
        welcomeScreen->setAttribute(Qt::WA_DeleteOnClose);

        QObject::connect(welcomeScreen, &Welcomescreen::continueRequested,
                         [&, welcomeScreen](const QString &name, const QDate &birthday) {
                             pendingProfile.name = name;
                             pendingProfile.birthday = birthday;
                             welcomeScreen->close();
                             showLoginScreen();
                         });

        welcomeScreen->show();
    } else {
        showLoginScreen();
    }

    return QApplication::exec();
}
