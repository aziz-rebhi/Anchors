QT += core
QT -= gui

CONFIG += c++17 console cmdline
CONFIG -= app_bundle


LIBS += -lsodium

CONFIG += link_pkgconfig
PKGCONFIG += libsodium

SOURCES += \
    main.cpp \
    ../core/crypto/cryptomanager.cpp\
    ../core/storage/encryptedfilestore.cpp \
    ../core/models/profile.cpp \
    ../core/storage/repositories/profilerepository.cpp \
    ../core/storage/saltstore.cpp


HEADERS += \
    ../core/crypto/cryptomanager.h \
    ../core/crypto/SecureBuffer.h \
    ../core/storage/encryptedfilestore.h \
    ../core/storage/FilePaths.h \
    ../core/models/profile.h \
    ../core/storage/repositories/profilerepository.h \
    ../core/storage/saltstore.h

