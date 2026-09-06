#include "codesyntaxhighlighter.h"
#include <QTextDocument>
#include <QJsonDocument>
#include <QJsonParseError>

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
    else if (l == QLatin1String("c#") || l == QLatin1String("cs"))
        l = QStringLiteral("csharp");
    else if (l == QLatin1String("md"))
        l = QStringLiteral("markdown");
    else if (l == QLatin1String("yml"))
        l = QStringLiteral("yaml");
    else if (l == QLatin1String("golang"))
        l = QStringLiteral("go");
    else if (l == QLatin1String("rs"))
        l = QStringLiteral("rust");
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

void CodeSyntaxHighlighter::addRule(const QString& pattern, const QTextCharFormat& fmt,
                                    QRegularExpression::PatternOptions options)
{
    Rule r;
    r.pattern = QRegularExpression(pattern, options);
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
    else if (lang == QLatin1String("sql")) {
        const auto CIO = QRegularExpression::CaseInsensitiveOption;
        addRule(QStringLiteral(
                    R"(\b(select|insert\s+into|update|delete\s+from|drop|create|alter|truncate|)"
                    R"(merge|replace|grant|revoke|with|explain|begin|commit|rollback|transaction|)"
                    R"(declare|execute|call)\b)"),
                m_keywordFmt, CIO);
        addRule(QStringLiteral(
                    R"(\b(from|where|join|left\s+join|right\s+join|inner\s+join|outer\s+join|)"
                    R"(full\s+join|on|union|all|distinct|as|group\s+by|order\s+by|having|limit|)"
                    R"(offset|values|into|set|and|or|not|in|exists|between|like|is\s+null|)"
                    R"(is\s+not\s+null|top|asc|desc|case|when|then|else|end|primary\s+key|)"
                    R"(foreign\s+key|references|constraint|default|cascade|index|table|view|)"
                    R"(procedure|function|trigger)\b)"),
                m_keywordFmt, CIO);
        addRule(QStringLiteral(
                    R"(\b(count|sum|avg|min|max|coalesce|ifnull|nullif|cast|convert|concat|)"
                    R"(substring|now|current_date|current_timestamp)\s*\()"),
                m_functionFmt, CIO);
        addRule(QStringLiteral(R"("([^"\\]|\\.)*"|'([^'\\]|\\.)*')"), m_stringFmt);
        addRule(QStringLiteral(R"(/\*[\s\S]*?\*/)"), m_commentFmt);
        addRule(QStringLiteral(R"(--[^\n]*)"), m_commentFmt);
    }
    else if (lang == QLatin1String("markdown")) {
        const auto MLO = QRegularExpression::MultilineOption;
        addRule(QStringLiteral(R"(^#{1,6}\s.*$)"), m_keywordFmt, MLO);            // headings
        addRule(QStringLiteral(R"(\*\*[^*\n]*\*\*|__[^*\n]*__)"), m_keywordFmt);  // bold
        addRule(QStringLiteral(R"(\*[^*\n]*\*|_[^*\n]*_)"), m_commentFmt);        // italic
        addRule(QStringLiteral(R"(`[^`\n]*`)"), m_stringFmt);                     // inline code
        addRule(QStringLiteral(R"(^\s*(```|~~~)\S*)"), m_stringFmt, MLO);         // fences
        addRule(QStringLiteral(R"(!?\[[^\]\n]*\]\([^)\n]*\))"), m_functionFmt);   // links / images
        addRule(QStringLiteral(R"(^\s*>\s?)"), m_commentFmt, MLO);                // blockquote
        addRule(QStringLiteral(R"(^\s*(\*\s|-{1,3}\s|\d+\.\s+))"), m_typeFmt, MLO); // list markers
        addRule(QStringLiteral(R"((-{3,}|\*{3,})\s*$)"), m_commentFmt, MLO);      // horizontal rule
    }
    else if (lang == QLatin1String("yaml")) {
        const auto MLO = QRegularExpression::MultilineOption;
        const auto CIO = QRegularExpression::CaseInsensitiveOption;
        addRule(QStringLiteral(R"(^[ \t]*[A-Za-z0-9_.-]+\s*:.*$)"), m_keywordFmt, MLO);                // key: value
        addRule(QStringLiteral(R"(^[ \t]*-{1,3}\s+[A-Za-z0-9_.-]+\s*:.+$)"), m_keywordFmt, MLO);       // - item: value
        addRule(QStringLiteral(R"("([^"\\]|\\.)*"|'([^'\\]|\\.)*')"), m_stringFmt);
        addRule(QStringLiteral(R"(\b(true|false|null|yes|no|on|off|~)\b)"), m_typeFmt, CIO);
        addRule(QStringLiteral(R"(!{1,2}[A-Za-z0-9_-]+|&[A-Za-z0-9_-]+|\*[A-Za-z0-9_-]+)"), m_functionFmt);
        addRule(QStringLiteral(R"(#[^\n]*)"), m_commentFmt);
    }
    else if (lang == QLatin1String("xml")) {
        addRule(QStringLiteral(R"(<\?[A-Za-z_]+\b[^>]*\?>|<!DOCTYPE[^>]*>)"), m_preprocessorFmt);
        addRule(QStringLiteral(R"(</?[A-Za-z_][A-Za-z0-9_.-]*)"), m_keywordFmt);
        addRule(QStringLiteral(R"(\b[A-Za-z_:][A-Za-z0-9_:.-]*(?=\s*=))"), m_typeFmt);
        addRule(QStringLiteral(R"("([^"\\]|\\.)*"|'([^'\\]|\\.)*')"), m_stringFmt);
        addRule(QStringLiteral(R"(<!--[\s\S]*?-->)"), m_commentFmt);
    }
    else if (lang == QLatin1String("java")) {
        addRule(QStringLiteral(
                    R"(\b(public|private|protected|static|final|abstract|native|synchronized|)"
                    R"(volatile|transient|strictfp|void|class|interface|enum|extends|implements|)"
                    R"(import|package|return|new|this|super|if|else|for|while|do|switch|case|)"
                    R"(default|break|continue|try|catch|finally|throw|throws|instanceof|true|)"
                    R"(false|null)\b)"),
                m_keywordFmt);
        addRule(QStringLiteral(
                    R"(\b(String|Integer|Long|Double|Float|Boolean|Character|Byte|Short|Object|)"
                    R"(List|ArrayList|LinkedList|HashMap|Map|Set|HashSet|TreeSet|Exception|)"
                    R"(RuntimeException|Throwable|System|Math|Arrays|Collections|Optional)\b)"),
                m_typeFmt);
        addRule(QStringLiteral(R"(\b(class|interface|enum)\s+[A-Za-z_][A-Za-z0-9_]*)"), m_functionFmt);
        addRule(QStringLiteral(R"(\bSystem\.(out|err)\b)"), m_typeFmt);
        addRule(QStringLiteral(R"(@[A-Za-z_][A-Za-z0-9_]*)"), m_preprocessorFmt);
        addRule(QStringLiteral(R"("([^"\\]|\\.)*"|'([^'\\]|\\.)*')"), m_stringFmt);
        addRule(QStringLiteral(R"(//[^\n]*)"), m_commentFmt);
        addRule(QStringLiteral(R"(/\*[\s\S]*?\*/)"), m_commentFmt);
        addRule(QStringLiteral(R"(\b[A-Za-z_][A-Za-z0-9_]*(?=\s*\())"), m_functionFmt);
    }
    else if (lang == QLatin1String("csharp")) {
        addRule(QStringLiteral(
                    R"(\b(using|namespace|class|struct|interface|enum|public|private|protected|)"
                    R"(internal|static|readonly|virtual|override|abstract|sealed|partial|event|)"
                    R"(delegate|void|int|long|float|double|decimal|bool|char|string|var|if|else|)"
                    R"(switch|case|for|foreach|while|do|return|new|this|base|try|catch|finally|)"
                    R"(throw|get|set|value|in|out|ref|params|async|await|null|true|false)\b)"),
                m_keywordFmt);
        addRule(QStringLiteral(
                    R"(\b(List|Dictionary|IEnumerable|IList|ICollection|Action|Func|Task|Console|)"
                    R"(String|Object|Exception|Math|DateTime|StringBuilder|Random)\b)"),
                m_typeFmt);
        addRule(QStringLiteral(R"(\b(class|interface|struct|enum)\s+[A-Za-z_]\w*)"), m_functionFmt);
        addRule(QStringLiteral(R"(\bConsole\.\w+)"), m_functionFmt);
        addRule(QStringLiteral(R"(#(region|endregion|if|else|endif|define|pragma)\b[^\n]*)"), m_preprocessorFmt);
        addRule(QStringLiteral(R"(@"[^"\n]*"|"([^"\\]|\\.)*"|'([^'\\]|\\.)*')"), m_stringFmt);
        addRule(QStringLiteral(R"(//[^\n]*)"), m_commentFmt);
        addRule(QStringLiteral(R"(/\*[\s\S]*?\*/)"), m_commentFmt);
        addRule(QStringLiteral(R"(\b[A-Za-z_][A-Za-z0-9_]*(?=\s*\())"), m_functionFmt);
    }
    else if (lang == QLatin1String("go")) {
        addRule(QStringLiteral(
                    R"(\b(package|import|func|var|const|type|struct|interface|map|chan|go|defer|)"
                    R"(return|if|else|for|range|switch|case|default|break|continue|goto|select|)"
                    R"(fallthrough)\b)"),
                m_keywordFmt);
        addRule(QStringLiteral(
                    R"(\b(string|int|int8|int16|int32|int64|uint|uint8|uint16|uint32|uint64|)"
                    R"(uintptr|float32|float64|bool|byte|rune|error|any|comparable)\b)"),
                m_typeFmt);
        addRule(QStringLiteral(
                    R"(\b(make|new|len|cap|append|copy|delete|panic|recover|close|complex|real|imag)\b)"),
                m_keywordFmt);
        addRule(QStringLiteral(R"(\b(fmt|log)\.(Print|Println|Printf|Sprintf|Errorf|Fprintf)\b)"), m_functionFmt);
        addRule(QStringLiteral(R"(`[^`\n]*`|"[^"\\]*(?:\\.[^"\\]*)*"|'([^'\\]|\\.)*')"), m_stringFmt);
        addRule(QStringLiteral(R"(//[^\n]*)"), m_commentFmt);
        addRule(QStringLiteral(R"(/\*[\s\S]*?\*/)"), m_commentFmt);
        addRule(QStringLiteral(R"(:=)"), m_typeFmt);
        addRule(QStringLiteral(R"(\b[A-Za-z_][A-Za-z0-9_]*(?=\s*\())"), m_functionFmt);
    }
    else if (lang == QLatin1String("rust")) {
        const auto MLO = QRegularExpression::MultilineOption;
        addRule(QStringLiteral(
                    R"(\b(fn|let|mut|pub|struct|enum|impl|trait|mod|use|match|if|else|for|while|)"
                    R"(loop|return|break|continue|move|ref|static|const|unsafe|async|await|dyn|)"
                    R"(where|as|in|type|crate|self|Self|super|extern|macro_rules)\b)"),
                m_keywordFmt);
        addRule(QStringLiteral(
                    R"(\b(i8|i16|i32|i64|i128|isize|u8|u16|u32|u64|u128|usize|f32|f64|bool|char|)"
                    R"(str|String|Vec|Option|Result|Box|Rc|Arc|HashMap|BTreeMap|HashSet|VecDeque)\b)"),
                m_typeFmt);
        addRule(QStringLiteral(R"(\b[A-Za-z_][A-Za-z0-9_]*!\s*\()"), m_functionFmt);      // macros: println!(
        addRule(QStringLiteral(R"(\B'[A-Za-z_]\w*)"), m_typeFmt);                          // lifetimes: &'a
        addRule(QStringLiteral(R"('([^'\\]|\\.)*')"), m_stringFmt);                        // char literals
        addRule(QStringLiteral(R"("[^"\\]*(?:\\.[^"\\]*)*")"), m_stringFmt);
        addRule(QStringLiteral(R"(//[^\n]*)"), m_commentFmt);
        addRule(QStringLiteral(R"(/\*[\s\S]*?\*/)"), m_commentFmt);
        addRule(QStringLiteral(R"(^[ \t]*#!?\[[^\]]*\])"), m_preprocessorFmt, MLO);        // attributes
        addRule(QStringLiteral(R"(\b[A-Za-z_][A-Za-z0-9_]*(?=\s*\())"), m_functionFmt);
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
    const QString s = code.trimmed();
    if (s.isEmpty())
        return QStringLiteral("text");

    int cpp = 0, py = 0, js = 0, bash = 0, qml = 0, html = 0, css = 0, json = 0;
    int sql = 0, md = 0, yaml = 0, xml = 0, java = 0, csharp = 0, go = 0, rust = 0;

    const auto MLO  = QRegularExpression::MultilineOption;
    const auto CIO  = QRegularExpression::CaseInsensitiveOption;
    const auto MCIO = MLO | CIO;

    auto count = [&s](const QString& rx, QRegularExpression::PatternOptions opts) -> int {
        int n = 0;
        auto it = QRegularExpression(rx, opts).globalMatch(s);
        while (it.hasNext()) { it.next(); ++n; }
        return n;
    };

    // JSON: an actual parse is the only infallible signal
    {
        QJsonParseError pe;
        const QJsonDocument d = QJsonDocument::fromJson(code.toUtf8(), &pe);
        if (pe.error == QJsonParseError::NoError && (d.isArray() || d.isObject()))
            json += 6;
    }

    // C / C++
    if (count(QStringLiteral(R"(^[ \t]*#\s*(include|define|ifdef|ifndef|pragma)\b)"), MLO))
        cpp += 4;
    if (s.contains(QLatin1String("std::")))
        cpp += 3;
    if (s.contains(QLatin1String("nullptr")) || s.contains(QLatin1String("constexpr")))
        cpp += 2;
    if (count(QStringLiteral(R"(\b(template\s*<|using\s+namespace\b|namespace\s+\w+\s*\{))"), QRegularExpression::NoPatternOption))
        cpp += 2;
    if (count(QStringLiteral(R"(\b(int|void|char|bool|long|unsigned|auto|static)\b)"), QRegularExpression::NoPatternOption))
        cpp += 1;

    // Python
    if (count(QStringLiteral(R"(^\s*(async\s+)?def\s+\w+\s*\()"), MLO))
        py += 4;
    if (count(QStringLiteral(R"(^\s*from\s+\S+\s+import\b)"), MLO))
        py += 3;
    if (s.contains(QLatin1String("self.")))
        py += 3;
    if (count(QStringLiteral(R"(\b(elif|lambda|yield|None|True|False)\b)"), QRegularExpression::NoPatternOption))
        py += 2;
    if (count(QStringLiteral(R"(^\s+\w+\s*:\s*\w+\s*=)"), MLO)) // "name: Type = val" → Python
        py += 1;

    // JS / TS
    if (count(QStringLiteral(R"(^\s*(const|let|var)\s+[\w$]+\s*=)"), MLO))
        js += 4;
    if (s.contains(QLatin1String("=>")))
        js += 4;
    if (count(QStringLiteral(R"(^\s*(import\s+.*\s+from\s+['"]|export\s+))"), MLO))
        js += 3;
    if (s.contains(QLatin1String("require(")) || s.contains(QLatin1String("module.exports")))
        js += 2;
    if (count(QStringLiteral(R"(\.map\(|\.forEach\(|\.filter\()"), QRegularExpression::NoPatternOption))
        js += 1;

    // Bash / shell
    if (count(QStringLiteral(R"(^#!.*\b(bash|zsh|sh)\b)"), MLO))
        bash += 5;
    if (count(QStringLiteral(R"(\b(sudo|apt(-get)?|pacman|dnf|yum|systemctl|journalctl|chmod|chown|grep|find|xargs|rsync)\b)"), QRegularExpression::NoPatternOption))
        bash += 3;
    if (count(QStringLiteral(R"(^\s*(if\s*\[|for\s+\w+\s+in\b|export\s+\w+=|alias\s+\w+=)"), MLO))
        bash += 3;
    if (count(QStringLiteral(R"(\$[A-Za-z_]\w*|\$\{|\$\(\(|\$@|\$\?)"), QRegularExpression::NoPatternOption))
        bash += 1;

    // QML
    if (s.contains(QLatin1String("import QtQuick")))
        qml += 5;
    if (count(QStringLiteral(R"(\b(Rectangle|Item|ColumnLayout|RowLayout|anchors\.)"), CIO))
        qml += 3;
    if (count(QStringLiteral(R"(^[ \t]*(property|signal|function)\s+\w+)"), MLO))
        qml += 2;

    // HTML
    if (count(QStringLiteral(R"(^<!DOCTYPE|<html|</html>|<head|<body\b)"), MCIO))
        html += 4;
    if (count(QStringLiteral(R"(<(div|span|p|a|img|ul|li|table|section|nav|button)(\s|>))"), CIO))
        html += 3;
    if (s.contains(QLatin1String("&nbsp;")) || s.contains(QLatin1String("&amp;")))
        html += 2;

    // CSS
    if (count(QStringLiteral(R"(^\s*[.#][\w-]+\s*\{)"), MLO))
        css += 4;
    if (count(QStringLiteral(R"(@(media|import|keyframes|font-face|supports)\b)"), QRegularExpression::NoPatternOption))
        css += 3;
    if (count(QStringLiteral(R"(::?(hover|focus|active|before|after|visited)\b)"), QRegularExpression::NoPatternOption))
        css += 2;
    if (s.contains(QLatin1String("!important")))
        css += 1;

    // SQL — a verb gates the clause count so stray "from/where" words can't score
    {
        const int verbs = count(QStringLiteral(
            R"(^\s*(select|insert|update|delete|drop|create|alter|truncate|merge|replace|grant|with|explain)\b)"),
            MCIO);
        const int clauses = count(QStringLiteral(
            R"(\b(from|where|join|on|group\s+by|order\s+by|having|limit|offset|union|values|into|set)\b)"),
            CIO);
        if (verbs)
            sql += 4 + qMin(3, clauses);
        else if (clauses >= 3)
            sql += clauses;
    }

    // Markdown
    if (count(QStringLiteral(R"(^#{1,6}\s+\S)"), MLO))
        md += 4;
    if (count(QStringLiteral(R"(^\s*(```|~~~))"), MLO))
        md += 4;
    if (count(QStringLiteral(R"(\*\*[^*\n]+\*\*|__[^*\n]+__)"), QRegularExpression::NoPatternOption))
        md += 3;
    if (count(QStringLiteral(R"(!?\[[^\]\n]*\]\([^)\n]*\))"), QRegularExpression::NoPatternOption))
        md += 3;
    if (count(QStringLiteral(R"(^\s*>\s)"), MLO))
        md += 2;
    if (count(QStringLiteral(R"(^\s*(\*\s+|\d+\.\s+))"), MLO))
        md += 1;

    // YAML
    {
        const int keys = count(QStringLiteral(R"(^[ \t]*[A-Za-z0-9_.-]+\s*:\s)"), MLO);
        const bool docStart = count(QStringLiteral(R"(^---\s*$)"), MLO) > 0;
        const bool listItems = count(QStringLiteral(R"(^\s*-\s+[\w"'.-]+\s*:)"), MLO) > 0;
        const bool tagged = s.contains(QLatin1String("!!")) || s.contains(QLatin1String("&anchor"));
        // "name: Type = value" blocks are Python type hints, not YAML keys
        const bool annotAssign = count(QStringLiteral(R"(^\s+\w+\s*:\s*\w+\s*=)"), MLO) > 0;

        if (docStart)       yaml += 4;
        if (listItems)      yaml += 2;
        if (tagged)         yaml += 1;
        if (keys >= 2)      yaml += annotAssign ? 2 : (2 + qMin(keys, 3));
    }

    // XML
    if (count(QStringLiteral(R"(<\?xml\b)"), CIO))
        xml += 4;
    if (count(QStringLiteral(R"(xmlns[:=])"), QRegularExpression::NoPatternOption))
        xml += 3;
    if (count(QStringLiteral(R"(^\s*<[\w.-]+[^>]*/>)"), MLO))
        xml += 2;
    if (count(QStringLiteral(R"(<(project|config|root|item|manifest|property|widget|beans)(\s|>))"), CIO))
        xml += 1;

    // Java
    if (count(QStringLiteral(R"(\bpublic\s+(static\s+)?(class|void|int|String|boolean|main)\b)"), QRegularExpression::NoPatternOption))
        java += 5;
    if (s.contains(QLatin1String("System.out")) || s.contains(QLatin1String("System.err")))
        java += 4;
    if (count(QStringLiteral(R"(^\s*import\s+(java\.|javax\.|com\.|org\.|android\.|kotlin\.))"), MLO))
        java += 3;
    if (count(QStringLiteral(R"(@(Override|Test|Autowired|Resource|Inject|RequestMapping|SuppressWarnings|Deprecated)\b)"), QRegularExpression::NoPatternOption))
        java += 2;
    if (count(QStringLiteral(R"(^\s*(public|private|protected)\s+\S+\s+\w+\s*\()"), MLO))
        java += 2;
    if (count(QStringLiteral(R"(\bpublic\s+static\s+void\s+main\s*\(|String\[\]\s+\w+)"), QRegularExpression::NoPatternOption))
        java += 2;

    // C#
    if (count(QStringLiteral(R"(^\s*using\s+(System|Microsoft|UnityEngine)[\w.]*;)"), MLO))
        csharp += 5;
    if (count(QStringLiteral(R"(\bConsole\.(Write|WriteLine|ReadLine|Error|Out)\b)"), QRegularExpression::NoPatternOption))
        csharp += 4;
    if (count(QStringLiteral(R"(\bstatic\s+(async\s+)?void\s+Main\s*\()"), QRegularExpression::NoPatternOption))
        csharp += 4;
    if (count(QStringLiteral(R"(^\s*namespace\s+\w+)"), MLO))
        csharp += 2;
    if (count(QStringLiteral(R"(\b(List<|Dictionary<|IEnumerable|foreach\s*\()"), QRegularExpression::NoPatternOption))
        csharp += 2;
    if (count(QStringLiteral(R"(\bpublic\s+(sealed\s+|static\s+)?class\s+\w+)"), QRegularExpression::NoPatternOption))
        csharp += 2;

    // Go
    if (count(QStringLiteral(R"(^\s*package\s+main\b)"), MLO))
        go += 5;
    if (count(QStringLiteral(R"(\b(fmt|log)\.(Print|Println|Printf|Sprintf|Errorf|Fprintf)\s*\()"), QRegularExpression::NoPatternOption))
        go += 4;
    if (count(QStringLiteral(R"(\b:=)"), QRegularExpression::NoPatternOption))
        go += 3;
    if (count(QStringLiteral(R"(^\s*func\s+(\([^)]*\)\s+)?\w+\s*\()"), MLO))
        go += 3;
    if (count(QStringLiteral(R"(^\s*(import|var|const)\s*\(?)"), MLO))
        go += 2;
    if (count(QStringLiteral(R"(\b(defer|go\s+func|chan\s|map\[|error\b))"), QRegularExpression::NoPatternOption))
        go += 1;

    // Rust
    if (count(QStringLiteral(R"(^\s*fn\s+main\s*\()"), MLO))
        rust += 5;
    if (count(QStringLiteral(R"(^\s*(pub\s+)?(fn|struct|enum|impl|trait|mod|use)\s+\w+)"), MLO))
        rust += 4;
    if (count(QStringLiteral(R"(\b(println|eprintln|format|vec|panic|unwrap|expect|dbg)!)"), QRegularExpression::NoPatternOption))
        rust += 3;
    if (s.contains(QLatin1String("let mut ")))
        rust += 3;
    if (count(QStringLiteral(R"(\b(crate::|self::|super::|match\s+\w+\s*\{))"), QRegularExpression::NoPatternOption))
        rust += 2;
    if (count(QStringLiteral(R"(\b(Result<|Option<|Vec<|Box<|Rc<|Arc<)"), QRegularExpression::NoPatternOption))
        rust += 1;

    struct Pair { int score; const char* id; };
    // Order = tie priority: with equal scores the earlier language wins.
    const Pair pairs[] = {
        { json,    "json"    },
        { sql,     "sql"     },
        { cpp,     "cpp"     },
        { go,      "go"      },
        { rust,    "rust"    },
        { java,    "java"    },
        { csharp,  "csharp"  },
        { py,      "python"  },
        { js,      "js"      },
        { bash,    "bash"    },
        { qml,     "qml"     },
        { html,    "html"    },
        { css,     "css"     },
        { xml,     "xml"     },
        { md,     "markdown"},
        { yaml,    "yaml"    },
    };

    int best = 0;
    const char* bestId = "text";
    for (const Pair& p : pairs) {
        if (p.score > best) {
            best = p.score;
            bestId = p.id;
        }
    }
    return best >= 2 ? QString::fromLatin1(bestId) : QStringLiteral("text");
}