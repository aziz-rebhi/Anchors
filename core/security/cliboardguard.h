#ifndef CLIBOARDGUARD_H
#define CLIBOARDGUARD_H

#pragma once
#include <QObject>
#include <QString>

class CliboardGuard : public QObject
{
    Q_OBJECT
public:
    explicit CliboardGuard(QObject *parent = nullptr);

    // Copies `text` to the system clipboard and schedules a background
    // clear `seconds` later. Pass seconds <= 0 for no auto-clear.
    Q_INVOKABLE void copyWithAutoClear(const QString &text, int seconds = 0);
};
#endif // CLIBOARDGUARD_H