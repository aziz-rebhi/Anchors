#ifndef CODESYNTAXHIGHLIGHTER_H
#define CODESYNTAXHIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QRegularExpression>
#include <QVector>
#include <QString>

class CodeSyntaxHighlighter : public QSyntaxHighlighter {
    Q_OBJECT
public:
    explicit CodeSyntaxHighlighter(QTextDocument* parent = nullptr);

    void setLanguage(const QString& lang);   // "auto","cpp","python","js","bash",...
    QString language() const { return m_language; }

    // Heuristic detect; returns canonical id: cpp, python, js, bash, qml, json, html, css, text
    static QString detectLanguage(const QString& code);

protected:
    void highlightBlock(const QString& text) override;

private:
    struct Rule {
        QRegularExpression pattern;
        QTextCharFormat format;
    };

    void buildRules(const QString& lang);
    void addRule(const QString& pattern, const QTextCharFormat& fmt);

    QString m_language = QStringLiteral("text");
    QVector<Rule> m_rules;

    QTextCharFormat m_keywordFmt;
    QTextCharFormat m_typeFmt;
    QTextCharFormat m_stringFmt;
    QTextCharFormat m_commentFmt;
    QTextCharFormat m_numberFmt;
    QTextCharFormat m_functionFmt;
    QTextCharFormat m_preprocessorFmt;
};

#endif