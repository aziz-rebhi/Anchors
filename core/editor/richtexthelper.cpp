#include "richtexthelper.h"

#include <QQuickTextDocument>
#include <QTextDocument>
#include <QTextCursor>
#include <QTextCharFormat>
#include <QFont>
#include <QColor>
#include <QtGlobal>

static QTextCursor cursorFor(QObject *textEditObj, int start = -1, int end = -1)
{
    if (!textEditObj)
        return {};

    auto *qqdoc = textEditObj->property(QByteArrayLiteral("textDocument")).value<QQuickTextDocument *>();
    if (!qqdoc)
        return {};

    QTextDocument *doc = qqdoc->textDocument();
    if (!doc)
        return {};

    if (start < 0 || end < 0) {
        start = textEditObj->property(QByteArrayLiteral("selectionStart")).toInt();
        end   = textEditObj->property(QByteArrayLiteral("selectionEnd")).toInt();
    }

    start = qBound(0, start, doc->characterCount() - 1);
    end   = qBound(0, end,   doc->characterCount() - 1);

    QTextCursor cur(doc);
    if (start == end) {
        // Empty selection → format applies to the next typed characters
        cur.setPosition(start);
    } else {
        cur.setPosition(qMin(start, end));
        cur.setPosition(qMax(start, end), QTextCursor::KeepAnchor);
    }
    return cur;
}

static void mergeFmt(QObject *textEditObj, const QTextCharFormat &extra, int start = -1, int end = -1)
{
    QTextCursor cur = cursorFor(textEditObj, start, end);
    if (cur.isNull())
        return;
    cur.mergeCharFormat(extra);
    // Do not assign text = toHtml() here: it clears selection and breaks toolbar use.
}

RichTextHelper::RichTextHelper(QObject *parent)
    : QObject(parent)
{
}

void RichTextHelper::toggleBold(QObject *o, int start, int end)
{
    QTextCursor cur = cursorFor(o, start, end);
    if (cur.isNull())
        return;
    QTextCharFormat f;
    const bool isBold = cur.charFormat().fontWeight() >= QFont::Bold;
    f.setFontWeight(isBold ? QFont::Normal : QFont::Bold);
    mergeFmt(o, f, start, end);
}

void RichTextHelper::toggleItalic(QObject *o, int start, int end)
{
    QTextCursor cur = cursorFor(o, start, end);
    if (cur.isNull())
        return;
    QTextCharFormat f;
    f.setFontItalic(!cur.charFormat().fontItalic());
    mergeFmt(o, f, start, end);
}

void RichTextHelper::toggleUnderline(QObject *o, int start, int end)
{
    QTextCursor cur = cursorFor(o, start, end);
    if (cur.isNull())
        return;
    QTextCharFormat f;
    f.setFontUnderline(!cur.charFormat().fontUnderline());
    mergeFmt(o, f, start, end);
}

void RichTextHelper::toggleStrike(QObject *o, int start, int end)
{
    QTextCursor cur = cursorFor(o, start, end);
    if (cur.isNull())
        return;
    QTextCharFormat f;
    f.setFontStrikeOut(!cur.charFormat().fontStrikeOut());
    mergeFmt(o, f, start, end);
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

    auto *qqdoc = textEditObj->property(QByteArrayLiteral("textDocument")).value<QQuickTextDocument *>();
    if (!qqdoc)
        return out;
    QTextDocument *doc = qqdoc->textDocument();
    if (!doc)
        return out;

    const int pos = textEditObj->property(QByteArrayLiteral("cursorPosition")).toInt();

    QTextCursor cur(doc);
    cur.setPosition(pos);
    cur.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
    QString afterPlain = cur.selectedText();
    afterPlain.replace(QChar::ParagraphSeparator, QLatin1Char('\n'));
    cur.removeSelectedText();

    const QString beforeHtml = doc->toHtml();
    textEditObj->setProperty(QByteArrayLiteral("text"), beforeHtml);

    out.insert(QStringLiteral("before"), beforeHtml);
    out.insert(QStringLiteral("after"), afterPlain);
    return out;
}