# Notes editor assets live in notes-editor/dist/ and are compiled into the
# binary via notes_editor.qrc, so a released build needs no Node/Vite/server.
# That qrc is GENERATED - run `npm run build` in notes-editor/ to refresh it.
# Regenerated rather than hand-maintained because Vite content-hashes its
# filenames; a hand-edited list would go stale silently, and rcc failing on a
# missing file is the failure mode we want (at build time, loudly) rather than a
# blank editor in a shipped release.
exists(notes-editor/notes_editor.qrc) {
    message("Anchors: packaging notes editor from notes-editor/notes_editor.qrc")
} else {
    warning("notes-editor/notes_editor.qrc is missing - run 'npm run build' in notes-editor/ first. The notes canvas will have no bundle to load.")
}

QT += widgets core quick qml sql

# Notes editor canvas is an embedded web app (notes-editor/) hosted in a
# WebEngineView. WebChannel carries the page -> host calls.
QT += webenginequick webchannel

CONFIG += c++17

VERSION = 1.0.37
DEFINES += APP_VERSION=\\\"$$VERSION\\\"

QT += network



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

win32 {
    RC_ICONS = ui/app.ico
}

unix {
    LIBS += -lsodium
}

SOURCES += \
    app/calendarcontroller.cpp \
    app/noteeditorcontroller.cpp \
    app/NotesEditorHost.cpp \
    app/settingscontroller.cpp \
    core/editor/codesyntaxhighlighter.cpp \
    core/editor/richtexthelper.cpp \
    core/models/block.cpp \
    core/models/blockcommands.cpp \
    core/models/blockmodel.cpp \
    core/models/calendarentry.cpp \
    core/models/document.cpp \
    core/models/projectentry.cpp \
    core/security/remindermanager.cpp \
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
    app/NotesEditorHost.h \
    app/session.h \
    app/settingscontroller.h \
    app/vaultcontroller.h \
    app/notecontroller.h \
    app/taskcontroller.h \
    core/crypto/SecureBuffer.h \
    core/crypto/cryptomanager.h \
    core/editor/codehighlightbridge.h \
    core/editor/codesyntaxhighlighter.h \
    core/editor/richtexthelper.h \
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
    core/security/remindermanager.h \
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
    ui/qml.qrc \
    notes-editor/notes_editor.qrc