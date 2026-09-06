#include "richtexthelper.h"

#include <QQuickTextDocument>
#include <QTextDocument>
#include <QTextCursor>
#include <QTextCharFormat>
#include <QFont>
#include <QColor>

static QTextCursor cursorFor(QObject *textEditObj)
{
    if (!textEditObj)
        return {};

    auto *qqdoc = textEditObj->property("textDocument").value<QQuickTextDocument *>();
    if (!qqdoc)
        return {};

    QTextDocument *doc = qqdoc->textDocument();
    if (!doc)
        return {};

    const int start = textEditObj->property("selectionStart").toInt();
    const int end = textEditObj->property("selectionEnd").toInt();

    QTextCursor cur(doc);
    if (start == end) {
        cur.setPosition(start);
    } else {
        cur.setPosition(start);
        cur.setPosition(end, QTextCursor::KeepAnchor);
    }
    return cur;
}

static void mergeFmt(QObject *textEditObj, const QTextCharFormat &extra)
{
    QTextCursor cur = cursorFor(textEditObj);
    if (cur.isNull())
        return;
    cur.mergeCharFormat(extra);

    // Force QML text property update so onTextChanged / save see HTML
    auto *qqdoc = textEditObj->property("textDocument").value<QQuickTextDocument *>();
    if (qqdoc) {
        if (QTextDocument *doc = qqdoc->textDocument())
            textEditObj->setProperty("text", doc->toHtml());
    }
}

RichTextHelper::RichTextHelper(QObject *parent)
    : QObject(parent)
{
}

void RichTextHelper::toggleBold(QObject *o)
{
    QTextCursor cur = cursorFor(o);
    if (cur.isNull())
        return;
    QTextCharFormat f;
    f.setFontWeight(cur.charFormat().fontWeight() == QFont::Bold ? QFont::Normal : QFont::Bold);
    mergeFmt(o, f);
}

void RichTextHelper::toggleItalic(QObject *o)
{
    QTextCursor cur = cursorFor(o);
    if (cur.isNull())
        return;
    QTextCharFormat f;
    f.setFontItalic(!cur.charFormat().fontItalic());
    mergeFmt(o, f);
}

void RichTextHelper::toggleUnderline(QObject *o)
{
    QTextCursor cur = cursorFor(o);
    if (cur.isNull())
        return;
    QTextCharFormat f;
    f.setFontUnderline(!cur.charFormat().fontUnderline());
    mergeFmt(o, f);
}

void RichTextHelper::toggleStrike(QObject *o)
{
    QTextCursor cur = cursorFor(o);
    if (cur.isNull())
        return;
    QTextCharFormat f;
    f.setFontStrikeOut(!cur.charFormat().fontStrikeOut());
    mergeFmt(o, f);
}

void RichTextHelper::setForeground(QObject *o, const QString &colorName)
{
    QTextCharFormat f;
    f.setForeground(QColor(colorName));
    mergeFmt(o, f);
}

void RichTextHelper::setBackground(QObject *o, const QString &colorName)
{
    QTextCharFormat f;
    f.setBackground(QColor(colorName));
    mergeFmt(o, f);
}

void RichTextHelper::clearBackground(QObject *o)
{
    QTextCharFormat f;
    f.setBackground(Qt::transparent);
    mergeFmt(o, f);
}

QVariantMap RichTextHelper::splitAtCursor(QObject *textEditObj)
{
    QVariantMap out;
    out.insert(QStringLiteral("before"), QString());
    out.insert(QStringLiteral("after"), QString());

    if (!textEditObj)
        return out;

    auto *qqdoc = textEditObj->property("textDocument").value<QQuickTextDocument *>();
    if (!qqdoc)
        return out;
    QTextDocument *doc = qqdoc->textDocument();
    if (!doc)
        return out;

    const int pos = textEditObj->property("cursorPosition").toInt();

    QTextCursor cur(doc);
    cur.setPosition(pos);
    cur.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
    QString afterPlain = cur.selectedText();
    afterPlain.replace(QChar::ParagraphSeparator, QLatin1Char('\n'));
    cur.removeSelectedText();

    const QString beforeHtml = doc->toHtml();
    textEditObj->setProperty("text", beforeHtml);

    out.insert(QStringLiteral("before"), beforeHtml);
    out.insert(QStringLiteral("after"), afterPlain);
    return out;
}