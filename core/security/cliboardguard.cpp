#include "cliboardguard.h"

#include <QGuiApplication>
#include <QClipboard>
#include <QTimer>


void CliboardGuard::copyWithAutoClear(const QString &text, int seconds){
    QClipboard *clipboard = QGuiApplication::clipboard();
    clipboard -> setText(text);

    QTimer::singleShot(seconds * 1000, qApp, [text](){
        QClipboard *cb = QGuiApplication::clipboard();
        if (cb -> text() == text){
            cb -> clear();
        }
    });
}