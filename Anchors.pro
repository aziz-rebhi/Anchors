QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    app/session.cpp \
    core/crypto/cryptomanager.cpp \
    core/models/profile.cpp \
    core/security/autolockmanager.cpp \
    core/storage/encryptedfilestore.cpp \
    core/storage/repositories/profilerepository.cpp \
    core/storage/saltstore.cpp \
    main.cpp \
    mainwindow.cpp \
    ui/dashboardpage.cpp \
    ui/loginscreen.cpp \
    ui/vaultentrydialog.cpp \
    ui/vaultpage.cpp \
    ui/welcomescreen.cpp \
    vault/cliboardguard.cpp \
    vault/passwordgenerator.cpp \
    vault/vaultentry.cpp \
    vault/vaultrepository.cpp

HEADERS += \
    app/session.h \
    core/crypto/SecureBuffer.h \
    core/crypto/cryptomanager.h \
    core/models/profile.h \
    core/security/autolockmanager.h \
    core/storage/FilePaths.h \
    core/storage/encryptedfilestore.h \
    core/storage/repositories/profilerepository.h \
    core/storage/saltstore.h \
    mainwindow.h \
    ui/dashboardpage.h \
    ui/loginscreen.h \
    ui/vaultentrydialog.h \
    ui/vaultpage.h \
    ui/welcomescreen.h \
    vault/cliboardguard.h \
    vault/passwordgenerator.h \
    vault/vaultentry.h \
    vault/vaultrepository.h

FORMS += \
    mainwindow.ui

LIBS += -lsodium

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    Ressource.qrc \
    src.qrc
