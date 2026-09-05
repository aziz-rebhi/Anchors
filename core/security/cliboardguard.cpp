#include "cliboardguard.h"

#include <QGuiApplication>
#include <QClipboard>
#include <QTimer>


CliboardGuard::CliboardGuard(QObject *parent) : QObject(parent)
{
}

void CliboardGuard::copyWithAutoClear(const QString &text, int seconds)
{
    QClipboard *clipboard = QGuiApplication::clipboard();
    clipboard->setText(text);

    if (seconds <= 0)
        return;

    QTimer::singleShot(seconds * 1000, this, [this, text]() {
        QClipboard *cb = QGuiApplication::clipboard();
        if (cb->text() == text) {
            cb->clear();
        }
    });
}