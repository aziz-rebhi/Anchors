QT += core
QT -= gui

CONFIG += c++17 console cmdline
CONFIG -= app_bundle

CONFIG += link_pkgconfig
PKGCONFIG += libsodium

SOURCES += \
    main.cpp \
    ../core/crypto/cryptomanager.cpp\
    ../core/storage/encryptedfilestore.cpp

HEADERS += \
    ../core/crypto/cryptomanager.h \
    ../core/crypto/SecureBuffer.h \
    ../core/storage/encryptedfilestore.h \
    ../core/storage/FilePaths.h
