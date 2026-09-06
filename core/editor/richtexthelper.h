#pragma once

#include <QObject>
#include <QString>
#include <QVariantMap>   // required for Q_INVOKABLE return type + moc

class RichTextHelper : public QObject
{
    Q_OBJECT
public:
    explicit RichTextHelper(QObject *parent = nullptr);

    Q_INVOKABLE void toggleBold(QObject *textEditObj);
    Q_INVOKABLE void toggleItalic(QObject *textEditObj);
    Q_INVOKABLE void toggleUnderline(QObject *textEditObj);
    Q_INVOKABLE void toggleStrike(QObject *textEditObj);

    Q_INVOKABLE void setForeground(QObject *textEditObj, const QString &colorName);
    Q_INVOKABLE void setBackground(QObject *textEditObj, const QString &colorName);
    Q_INVOKABLE void clearBackground(QObject *textEditObj);

    Q_INVOKABLE QVariantMap splitAtCursor(QObject *textEditObj);
};