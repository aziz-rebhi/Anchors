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

inline QString vaultFile()    { return dataDir() + QStringLiteral("/vault.enc"); }
inline QString notesFile()    { return dataDir() + QStringLiteral("/notes.enc"); }
inline QString tasksFile()    { return dataDir() + QStringLiteral("/tasks.enc"); }
inline QString calendarFile() { return dataDir() + QStringLiteral("/calendar.enc"); }
inline QString resumeFile()   { return dataDir() + QStringLiteral("/resume.enc"); }
inline QString profileFile()  { return dataDir() + QStringLiteral("/profile.enc"); }
}

#endif // FILEPATHS_H
