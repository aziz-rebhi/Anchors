#ifndef FILEPATHS_H
#define FILEPATHS_H

#pragma once

#include <QDir>
#include <QStandardPaths>
#include <QString>

namespace FilePaths
{
inline QString dataDir()
{
    QString path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(path); // ensure it exists before anything tries to read/write into it
    return path;
}

// Directory for note-attachment images. Every image is stored here as an
// *.png.enc blob (encrypted with the session key) — never as plaintext.
inline QString imagesDir()
{
    return dataDir() + QStringLiteral("/Anchors/images");
}

// Scratch directory under the OS temp location where decrypted image bytes
// are spilled so QML can render them while a note is open. Never the on-disk
// home for an attachment: it is wiped on every lock.
inline QString tempImagesDir()
{
    return QStandardPaths::writableLocation(QStandardPaths::TempLocation)
           + QStringLiteral("/Anchors-images");
}

inline QString vaultFile()    { return dataDir() + QStringLiteral("/vault.enc"); }
inline QString notesFile()    { return dataDir() + QStringLiteral("/notes.enc"); }
inline QString tasksFile()    { return dataDir() + QStringLiteral("/tasks.enc"); }
inline QString calendarFile() { return dataDir() + QStringLiteral("/calendar.enc"); }
inline QString resumeFile()   { return dataDir() + QStringLiteral("/resume.enc"); }
inline QString profileFile()  { return dataDir() + QStringLiteral("/profile.enc"); }
inline QString verifyFile ()  { return dataDir() + QStringLiteral("/verify.enc");}
inline QString pinFile()      {return dataDir()  + QStringLiteral("/pin.enc");}
}

#endif // FILEPATHS_H
