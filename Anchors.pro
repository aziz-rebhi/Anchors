QT += widgets core quick qml sql

CONFIG += c++17

win32-msvc {
    VCPKG_ROOT = $$(VCPKG_ROOT)
    isEmpty(VCPKG_ROOT) {
        VCPKG_ROOT = $$(VCPKG_INSTALLATION_ROOT)
    }
    !isEmpty(VCPKG_ROOT) {
        INCLUDEPATH += $$VCPKG_ROOT/installed/x64-windows/include
        LIBS += -L$$VCPKG_ROOT/installed/x64-windows/lib -llibsodium
    }
}

win32-g++ {
    VCPKG_ROOT = $$(VCPKG_ROOT)
    isEmpty(VCPKG_ROOT) {
        VCPKG_ROOT = $$(VCPKG_INSTALLATION_ROOT)
    }
    !isEmpty(VCPKG_ROOT) {
        INCLUDEPATH += $$VCPKG_ROOT/installed/x64-mingw-dynamic/include
        LIBS += -L$$VCPKG_ROOT/installed/x64-mingw-dynamic/lib -lsodium
    }
}

unix {
    LIBS += -lsodium
}

SOURCES += \
    app/calendarcontroller.cpp \
    app/noteeditorcontroller.cpp \
    app/settingscontroller.cpp \
    core/editor/codesyntaxhighlighter.cpp \
    core/models/block.cpp \
    core/models/blockcommands.cpp \
    core/models/blockmodel.cpp \
    core/models/calendarentry.cpp \
    core/models/document.cpp \
    core/models/projectentry.cpp \
    core/storage/notesdatabase.cpp \
    main.cpp \
    app/session.cpp \
    app/authcontroller.cpp \
    app/vaultcontroller.cpp \
    app/notecontroller.cpp \
    app/taskcontroller.cpp \
    core/crypto/cryptomanager.cpp \
    core/models/noteentry.cpp \
    core/models/profile.cpp \
    core/models/vaultentry.cpp \
    core/models/taskentry.cpp \
    core/security/autolockmanager.cpp \
    core/security/cliboardguard.cpp \
    core/security/passwordgenerator.cpp \
    core/storage/encryptedfilestore.cpp \
    core/storage/saltstore.cpp \
    core/storage/repositories/calendarrepository.cpp \
    core/storage/repositories/noterepository.cpp \
    core/storage/repositories/profilerepository.cpp \
    core/storage/repositories/vaultrepository.cpp \
    core/storage/repositories/taskrepository.cpp

HEADERS += \
    app/authcontroller.h \
    app/calendarcontroller.h \
    app/noteeditorcontroller.h \
    app/session.h \
    app/settingscontroller.h \
    app/vaultcontroller.h \
    app/notecontroller.h \
    app/taskcontroller.h \
    core/crypto/SecureBuffer.h \
    core/crypto/cryptomanager.h \
    core/editor/codehighlightbridge.h \
    core/editor/codesyntaxhighlighter.h \
    core/models/Calendarentry.h \
    core/models/block.h \
    core/models/blockcommands.h \
    core/models/blockdata.h \
    core/models/blockmodel.h \
    core/models/document.h \
    core/models/noteentry.h \
    core/models/profile.h \
    core/models/projectentry.h \
    core/models/vaultentry.h \
    core/models/taskentry.h \
    core/security/autolockmanager.h \
    core/security/cliboardguard.h \
    core/security/passwordgenerator.h \
    core/storage/FilePaths.h \
    core/storage/encryptedfilestore.h \
    core/storage/notesdatabase.h \
    core/storage/saltstore.h \
    core/storage/repositories/calendarrepository.h \
    core/storage/repositories/noterepository.h \
    core/storage/repositories/profilerepository.h \
    core/storage/repositories/vaultrepository.h \
    core/storage/repositories/taskrepository.h

RESOURCES += \
    ui/icons.qrc \
    ui/qml.qrc