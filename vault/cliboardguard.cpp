#include "cliboardguard.h"

#include <QApplication>
#include <QClipboard>
#include <QTimer>


void CliboardGuard::copyWithAutoClear(const QString &text, int seconds){
    QClipboard *clipboard = QApplication::clipboard();
    clipboard -> setText(text);

    QTimer::singleShot(seconds * 1000, qApp, [text](){
        QClipboard *cb = QApplication::clipboard();
        if (cb -> text() == text){
            cb -> clear();
        }
    });
}