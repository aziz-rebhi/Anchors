#pragma once

#include <QObject>
#include <QString>
#include <QVariantMap>

class RichTextHelper : public QObject
{
    Q_OBJECT
public:
    explicit RichTextHelper(QObject *parent = nullptr);

    // start/end < 0 → use TextArea selectionStart/selectionEnd
    Q_INVOKABLE void toggleBold(QObject *textEditObj, int start = -1, int end = -1);
    Q_INVOKABLE void toggleItalic(QObject *textEditObj, int start = -1, int end = -1);
    Q_INVOKABLE void toggleUnderline(QObject *textEditObj, int start = -1, int end = -1);
    Q_INVOKABLE void toggleStrike(QObject *textEditObj, int start = -1, int end = -1);

    Q_INVOKABLE void setForeground(QObject *textEditObj, const QString &colorName);
    Q_INVOKABLE void setBackground(QObject *textEditObj, const QString &colorName);
    Q_INVOKABLE void clearBackground(QObject *textEditObj);

    Q_INVOKABLE QVariantMap splitAtCursor(QObject *textEditObj);
};