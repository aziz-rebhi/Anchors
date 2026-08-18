#include "codesyntaxhighlighter.h"
#include <QTextDocument>

CodeSyntaxHighlighter::CodeSyntaxHighlighter(QTextDocument* parent)
    : QSyntaxHighlighter(parent)
{
    m_keywordFmt.setForeground(QColor("#81A1C1"));
    m_keywordFmt.setFontWeight(QFont::Bold);

    m_typeFmt.setForeground(QColor("#8FBCBB"));

    m_stringFmt.setForeground(QColor("#A3BE8C"));

    m_commentFmt.setForeground(QColor("#6B756B"));
    m_commentFmt.setFontItalic(true);

    m_numberFmt.setForeground(QColor("#B48EAD"));

    m_functionFmt.setForeground(QColor("#88C0D0"));

    m_preprocessorFmt.setForeground(QColor("#EBCB8B"));

    buildRules(QStringLiteral("text"));
}

void CodeSyntaxHighlighter::setLanguage(const QString& lang)
{
    QString l = lang.trimmed().toLower();
    if (l == QLatin1String("c++") || l == QLatin1String("c"))
        l = QStringLiteral("cpp");
    else if (l == QLatin1String("javascript") || l == QLatin1String("typescript") || l == QLatin1String("ts"))
        l = QStringLiteral("js");
    else if (l == QLatin1String("shell") || l == QLatin1String("sh") || l == QLatin1String("zsh")
             || l == QLatin1String("linux") || l == QLatin1String("command")
             || l == QLatin1String("bash / linux"))
        l = QStringLiteral("bash");
    else if (l == QLatin1String("auto"))
        l = QStringLiteral("text");

    if (l.isEmpty())
        l = QStringLiteral("text");

    if (l != m_language || m_rules.isEmpty()) {
        m_language = l;
        buildRules(m_language);
    }

    // Always rehighlight — document text may have been reset by QML binding
    rehighlight();
}

void CodeSyntaxHighlighter::addRule(const QString& pattern, const QTextCharFormat& fmt)
{
    Rule r;
    r.pattern = QRegularExpression(pattern);
    r.format = fmt;
    m_rules.append(r);
}

void CodeSyntaxHighlighter::buildRules(const QString& lang)
{
    m_rules.clear();

    // numbers (shared)
    addRule(QStringLiteral(R"(\b[0-9]+(\.[0-9]+)?\b)"), m_numberFmt);

    if (lang == QLatin1String("cpp")) {
        addRule(QStringLiteral(R"(^\s*#\s*\w+.*)"), m_preprocessorFmt);
        addRule(QStringLiteral(
                    R"(\b(alignas|alignof|and|and_eq|asm|auto|bitand|bitor|bool|break|case|catch|char|)"
                    R"(class|compl|concept|const|consteval|constexpr|constinit|const_cast|continue|)"
                    R"(co_await|co_return|co_yield|decltype|default|delete|do|double|dynamic_cast|else|)"
                    R"(enum|explicit|export|extern|false|float|for|friend|goto|if|inline|int|long|)"
                    R"(mutable|namespace|new|noexcept|not|not_eq|nullptr|operator|or|or_eq|private|)"
                    R"(protected|public|register|reinterpret_cast|requires|return|short|signed|sizeof|)"
                    R"(static|static_assert|static_cast|struct|switch|template|this|thread_local|throw|)"
                    R"(true|try|typedef|typeid|typename|union|unsigned|using|virtual|void|volatile|)"
                    R"(wchar_t|while|xor|xor_eq|override|final|QString|QObject|QList|QVector|QVariant)\b)"),
                m_keywordFmt);
        addRule(QStringLiteral(R"("([^"\\]|\\.)*")"), m_stringFmt);
        addRule(QStringLiteral(R"('([^'\\]|\\.)*')"), m_stringFmt);
        addRule(QStringLiteral(R"(//[^\n]*)"), m_commentFmt);
        addRule(QStringLiteral(R"(/\*[\s\S]*?\*/)"), m_commentFmt);
        addRule(QStringLiteral(R"(\b[A-Za-z_][A-Za-z0-9_]*(?=\s*\())"), m_functionFmt);
    }
    else if (lang == QLatin1String("python")) {
        addRule(QStringLiteral(
                    R"(\b(False|None|True|and|as|assert|async|await|break|class|continue|def|del|)"
                    R"(elif|else|except|finally|for|from|global|if|import|in|is|lambda|nonlocal|)"
                    R"(not|or|pass|raise|return|try|while|with|yield|match|case|type|print|self|cls)\b)"),
                m_keywordFmt);
        addRule(QStringLiteral(R"("([^"\\]|\\.)*")"), m_stringFmt);
        addRule(QStringLiteral(R"('([^'\\]|\\.)*')"), m_stringFmt);
        addRule(QStringLiteral(R"(#[^\n]*)"), m_commentFmt);
        addRule(QStringLiteral(R"(\b[A-Za-z_][A-Za-z0-9_]*(?=\s*\())"), m_functionFmt);
    }
    else if (lang == QLatin1String("js")) {
        addRule(QStringLiteral(
                    R"(\b(abstract|arguments|await|boolean|break|byte|case|catch|char|class|const|)"
                    R"(continue|debugger|default|delete|do|double|else|enum|eval|export|extends|)"
                    R"(false|final|finally|float|for|function|goto|if|implements|import|in|)"
                    R"(instanceof|int|interface|let|long|native|new|null|of|package|private|)"
                    R"(protected|public|return|short|static|super|switch|synchronized|this|throw|)"
                    R"(throws|transient|true|try|typeof|var|void|volatile|while|with|yield|)"
                    R"(async|from|as|type|interface|undefined|NaN|Infinity)\b)"),
                m_keywordFmt);
        addRule(QStringLiteral(R"("([^"\\]|\\.)*")"), m_stringFmt);
        addRule(QStringLiteral(R"('([^'\\]|\\.)*')"), m_stringFmt);
        addRule(QStringLiteral(R"(`([^`\\]|\\.)*`)"), m_stringFmt);
        addRule(QStringLiteral(R"(//[^\n]*)"), m_commentFmt);
        addRule(QStringLiteral(R"(/\*[\s\S]*?\*/)"), m_commentFmt);
        addRule(QStringLiteral(R"(\b[A-Za-z_][A-Za-z0-9_]*(?=\s*\())"), m_functionFmt);
    }
    else if (lang == QLatin1String("bash")) {
        // Linux / shell commands & keywords
        addRule(QStringLiteral(
                    R"(\b(if|then|else|elif|fi|for|while|until|do|done|case|esac|function|in|)"
                    R"(select|time|coproc|\[|\]|test|exit|return|shift|break|continue|)"
                    R"(local|export|readonly|declare|typeset|unset|alias|unalias|eval|exec|)"
                    R"(source|trap|wait|jobs|bg|fg|kill|pwd|cd|echo|printf|read|set|unset)\b)"),
                m_keywordFmt);
        addRule(QStringLiteral(
                    R"(\b(ls|ll|la|cd|pwd|cat|less|more|head|tail|grep|rg|find|xargs|sed|awk|)"
                    R"(cp|mv|rm|mkdir|rmdir|touch|chmod|chown|chgrp|ln|stat|du|df|mount|umount|)"
                    R"(ps|top|htop|kill|killall|pkill|nice|nohup|time|which|whereis|type|file|)"
                    R"(tar|gzip|gunzip|zip|unzip|curl|wget|ssh|scp|rsync|ping|ip|ifconfig|)"
                    R"(systemctl|journalctl|service|apt|apt-get|pacman|yay|dnf|yum|zypper|)"
                    R"(git|docker|podman|kubectl|sudo|su|whoami|id|uname|hostname|env|printenv|)"
                    R"(bash|sh|zsh|fish|python|python3|node|npm|pip|make|cmake|gcc|g\+\+|clang)\b)"),
                m_functionFmt);
        addRule(QStringLiteral(R"("([^"\\]|\\.)*")"), m_stringFmt);
        addRule(QStringLiteral(R"('([^'\\]|\\.)*')"), m_stringFmt);
        addRule(QStringLiteral(R"(#[^\n]*)"), m_commentFmt);
        addRule(QStringLiteral(R"(\$\{?[A-Za-z_][A-Za-z0-9_]*\}?)"), m_typeFmt); // $VAR
        addRule(QStringLiteral(R"(^\s*function\s+[A-Za-z_][A-Za-z0-9_]*)"), m_functionFmt);
    }
    else if (lang == QLatin1String("qml")) {
        addRule(QStringLiteral(
                    R"(\b(import|as|property|readonly|required|signal|function|default|)"
                    R"(alias|id|true|false|null|undefined|if|else|for|while|return|switch|case|)"
                    R"(break|continue|typeof|new|this|var|let|const)\b)"),
                m_keywordFmt);
        addRule(QStringLiteral(
                    R"(\b(Item|Rectangle|Text|TextInput|TextEdit|TextArea|Image|MouseArea|)"
                    R"(Column|Row|Grid|Flow|Loader|ListView|GridView|PathView|Repeater|)"
                    R"(Window|ApplicationWindow|Page|ScrollView|Flickable|Anchors|Timer)\b)"),
                m_typeFmt);
        addRule(QStringLiteral(R"("([^"\\]|\\.)*")"), m_stringFmt);
        addRule(QStringLiteral(R"(//[^\n]*)"), m_commentFmt);
        addRule(QStringLiteral(R"(/\*[\s\S]*?\*/)"), m_commentFmt);
    }
    else if (lang == QLatin1String("json")) {
        addRule(QStringLiteral(R"("([^"\\]|\\.)*"\s*:)"), m_keywordFmt);
        addRule(QStringLiteral(R"("([^"\\]|\\.)*")"), m_stringFmt);
        addRule(QStringLiteral(R"(\b(true|false|null)\b)"), m_typeFmt);
    }
    else if (lang == QLatin1String("html")) {
        addRule(QStringLiteral(R"(</?[A-Za-z][A-Za-z0-9]*\b[^>]*>)"), m_keywordFmt);
        addRule(QStringLiteral(R"("([^"\\]|\\.)*")"), m_stringFmt);
        addRule(QStringLiteral(R"(<!--[\s\S]*?-->)"), m_commentFmt);
    }
    else if (lang == QLatin1String("css")) {
        addRule(QStringLiteral(R"([.#]?[A-Za-z_][A-Za-z0-9_-]*(?=\s*\{))"), m_functionFmt);
        addRule(QStringLiteral(R"(\b[a-z-]+(?=\s*:))"), m_keywordFmt);
        addRule(QStringLiteral(R"("([^"\\]|\\.)*")"), m_stringFmt);
        addRule(QStringLiteral(R"(/\*[\s\S]*?\*/)"), m_commentFmt);
    }
    // "text" → numbers only (already added)
}

void CodeSyntaxHighlighter::highlightBlock(const QString& text)
{
    for (const Rule& rule : m_rules) {
        auto it = rule.pattern.globalMatch(text);
        while (it.hasNext()) {
            const auto m = it.next();
            setFormat(m.capturedStart(), m.capturedLength(), rule.format);
        }
    }
}

QString CodeSyntaxHighlighter::detectLanguage(const QString& code)
{
    if (code.trimmed().isEmpty())
        return QStringLiteral("text");

    const QString s = code.left(4000); // enough signal
    int scoreCpp = 0, scorePy = 0, scoreJs = 0, scoreBash = 0;
    int scoreQml = 0, scoreJson = 0, scoreHtml = 0, scoreCss = 0;

    auto bump = [](int& score, int n = 1) { score += n; };

    // C / C++
    if (s.contains(QStringLiteral("#include"))) bump(scoreCpp, 4);
    if (s.contains(QStringLiteral("std::"))) bump(scoreCpp, 3);
    if (s.contains(QStringLiteral("nullptr"))) bump(scoreCpp, 2);
    if (s.contains(QStringLiteral("QString")) || s.contains(QStringLiteral("QObject"))) bump(scoreCpp, 3);
    if (QRegularExpression(QStringLiteral(R"(\b(int|void|class|template)\b)")).match(s).hasMatch())
        bump(scoreCpp, 1);

    // Python
    if (QRegularExpression(QStringLiteral(R"(^\s*def\s+\w+\s*\()"), QRegularExpression::MultilineOption).match(s).hasMatch())
        bump(scorePy, 4);
    if (QRegularExpression(QStringLiteral(R"(^\s*import\s+\w+)"), QRegularExpression::MultilineOption).match(s).hasMatch())
        bump(scorePy, 3);
    if (s.contains(QStringLiteral("self."))) bump(scorePy, 2);
    if (s.contains(QStringLiteral("elif "))) bump(scorePy, 2);

    // JS / TS
    if (s.contains(QStringLiteral("console.log"))) bump(scoreJs, 3);
    if (s.contains(QStringLiteral("=>"))) bump(scoreJs, 2);
    if (s.contains(QStringLiteral("const ")) || s.contains(QStringLiteral("let "))) bump(scoreJs, 1);
    if (s.contains(QStringLiteral("function "))) bump(scoreJs, 1);
    if (s.contains(QStringLiteral("export ")) || s.contains(QStringLiteral("import "))) bump(scoreJs, 1);

    // Bash / Linux commands
    if (QRegularExpression(QStringLiteral(R"(^\s*#!/(usr/)?bin/(env\s+)?(bash|sh|zsh))"),
                           QRegularExpression::MultilineOption).match(s).hasMatch())
        bump(scoreBash, 5);
    if (QRegularExpression(QStringLiteral(R"(\b(sudo|apt|pacman|systemctl|journalctl|grep|chmod|chown)\b)")).match(s).hasMatch())
        bump(scoreBash, 3);
    if (QRegularExpression(QStringLiteral(R"(^\s*if\s*\[\s*)"), QRegularExpression::MultilineOption).match(s).hasMatch())
        bump(scoreBash, 3);
    if (s.contains(QStringLiteral("#!/bin/"))) bump(scoreBash, 3);
    if (QRegularExpression(QStringLiteral(R"(\$\{?[A-Za-z_][A-Za-z0-9_]*\}?)")).match(s).hasMatch())
        bump(scoreBash, 1);

    // QML
    if (s.contains(QStringLiteral("import QtQuick"))) bump(scoreQml, 5);
    if (QRegularExpression(QStringLiteral(R"(\b(Rectangle|Item|ColumnLayout|anchors\.)\b)")).match(s).hasMatch())
        bump(scoreQml, 3);

    // JSON
    {
        const QString t = s.trimmed();
        if ((t.startsWith(QLatin1Char('{')) || t.startsWith(QLatin1Char('[')))
            && t.contains(QStringLiteral("\":")))
            bump(scoreJson, 4);
    }

    // HTML
    if (QRegularExpression(QStringLiteral(R"(</?(html|div|span|body|head|script)\b)"),
                           QRegularExpression::CaseInsensitiveOption).match(s).hasMatch())
        bump(scoreHtml, 4);

    // CSS
    if (QRegularExpression(QStringLiteral(R"([.#][\w-]+\s*\{)")).match(s).hasMatch())
        bump(scoreCss, 3);
    if (s.contains(QStringLiteral("px;")) || s.contains(QStringLiteral("color:")))
        bump(scoreCss, 1);

    struct Pair { int score; const char* id; };
    const Pair pairs[] = {
                           { scoreCpp,  "cpp" },
                           { scorePy,   "python" },
                           { scoreJs,   "js" },
                           { scoreBash, "bash" },
                           { scoreQml,  "qml" },
                           { scoreJson, "json" },
                           { scoreHtml, "html" },
                           { scoreCss,  "css" },
                           };

    int best = 0;
    const char* bestId = "text";
    for (const auto& p : pairs) {
        if (p.score > best) {
            best = p.score;
            bestId = p.id;
        }
    }
    return best >= 2 ? QString::fromLatin1(bestId) : QStringLiteral("text");
}