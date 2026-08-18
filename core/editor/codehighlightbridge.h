#ifndef CODEHIGHLIGHTBRIDGE_H
#define CODEHIGHLIGHTBRIDGE_H

#include <QObject>
#include <QQuickTextDocument>
#include <QDebug>
#include "codesyntaxhighlighter.h"

class CodeHighlightBridge : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString language READ language WRITE setLanguage NOTIFY languageChanged)
public:
    explicit CodeHighlightBridge(QObject* parent = nullptr)
        : QObject(parent) {}

    QString language() const { return m_lang; }

    void setLanguage(const QString& lang) {
        QString normalized = lang.trimmed().isEmpty() ? QStringLiteral("auto") : lang.trimmed();
        const bool changed = (m_lang != normalized);
        m_lang = normalized;
        apply();
        if (changed)
            emit languageChanged();
    }

    Q_INVOKABLE void attach(QQuickTextDocument* quickDoc) {
        if (!quickDoc)
            return;
        QTextDocument* doc = quickDoc->textDocument();
        if (!doc)
            return;

        m_doc = doc;
        delete m_highlighter;
        m_highlighter = new CodeSyntaxHighlighter(doc);

        qDebug() << "attach doc" << doc << "text" << doc->toPlainText().left(60);
        apply();
        qDebug() << "after attach, highlighter lang rules for"
                 << CodeSyntaxHighlighter::detectLanguage(doc->toPlainText());
    }

    Q_INVOKABLE void redetect() {
        if (!m_doc || !m_highlighter)
            return;
        if (m_lang != QLatin1String("auto")) {
            m_highlighter->rehighlight();
            return;
        }
        const QString detected = CodeSyntaxHighlighter::detectLanguage(m_doc->toPlainText());
        qDebug() << "redetect text=" << m_doc->toPlainText().left(60) << "->" << detected;
        m_highlighter->setLanguage(detected); // ends with rehighlight()
    }

    Q_INVOKABLE QString detectCode(const QString& code) const {
        return CodeSyntaxHighlighter::detectLanguage(code);
    }

    Q_INVOKABLE static QString detect(const QString& code) {
        return CodeSyntaxHighlighter::detectLanguage(code);
    }

signals:
    void languageChanged();

private:
    void apply() {
        if (!m_highlighter || !m_doc)
            return;
        if (m_lang == QLatin1String("auto"))
            m_highlighter->setLanguage(CodeSyntaxHighlighter::detectLanguage(m_doc->toPlainText()));
        else
            m_highlighter->setLanguage(m_lang);
    }

    QString m_lang = QStringLiteral("auto");
    QTextDocument* m_doc = nullptr;
    CodeSyntaxHighlighter* m_highlighter = nullptr;
};

#endif