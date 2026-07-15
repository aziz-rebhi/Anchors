#include "notepage.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListWidget>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QMessageBox>
#include <QInputDialog>
#include <QDateTime>
#include <QTimer>
#include <QFrame>
#include <QTextCursor>
#include <QTextList>
#include <QGraphicsDropShadowEffect>
#include <QRegularExpression>
#include <algorithm>

/* ── Dark shell palette ── */
static const char *C_BG          = "#0a0a0a";
static const char *C_SIDEBAR_BG  = "#111111";
static const char *C_TEXT_MAIN   = "#e1e3db";
static const char *C_TEXT_MUTED  = "#8c9386";
static const char *C_BORDER      = "#242424";
static const char *C_ACCENT      = "#2E8B57";
static const char *C_ACCENT_DARK = "#0B3D0B";
static const char *C_ACCENT_DIM  = "#9ed492";
static const char *C_INPUT_BG    = "#111111";
static const char *C_HOVER_BG    = "#1a1d1a";
static const char *C_SELECTED_BG = "#1f2a1f";

/* ── Paper palette ── */
static const char *C_PAPER_BG      = "#f5f3ee";
static const char *C_PAPER_BORDER  = "#e2ded2";
static const char *C_PAPER_TEXT    = "#2a2a26";

NotePage::NotePage(const QByteArray &sessionKey, QWidget *parent)
    : QWidget(parent), m_repository(sessionKey)
{
    setupUi();
    loadNotes();
}

void NotePage::setupUi()
{
    setStyleSheet(QString("background-color: %1;").arg(C_BG));

    auto *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // ─── LEFT SIDEBAR ───────────────────────────────────────────────
    auto *sidebar = new QFrame(this);
    sidebar->setFixedWidth(280);
    sidebar->setStyleSheet(QString("background-color: %1; border-right: 1px solid %2;").arg(C_SIDEBAR_BG, C_BORDER));

    auto *sideLayout = new QVBoxLayout(sidebar);
    sideLayout->setContentsMargins(16, 20, 16, 16);
    sideLayout->setSpacing(10);

    m_searchField = new QLineEdit(this);
    m_searchField->setPlaceholderText("Search notes...");
    m_searchField->setStyleSheet(QString(R"(
        QLineEdit { background-color: %1; border: 1px solid %2; border-radius: 10px; padding: 8px 12px; color: %3; font-size: 13px; }
        QLineEdit:focus { border-color: %4; }
    )").arg(C_INPUT_BG, C_BORDER, C_TEXT_MAIN, C_ACCENT));
    connect(m_searchField, &QLineEdit::textChanged, this, &NotePage::onSearchTextChanged);
    sideLayout->addWidget(m_searchField);

    m_newButton = new QPushButton("+  New Note", this);
    m_newButton->setCursor(Qt::PointingHandCursor);
    m_newButton->setStyleSheet(QString(R"(
        QPushButton { background-color: %1; color: white; border: none; border-radius: 10px; padding: 9px; font-size: 13px; font-weight: 600; }
        QPushButton:hover { background-color: %2; }
    )").arg(C_ACCENT, C_ACCENT_DARK));
    connect(m_newButton, &QPushButton::clicked, this, &NotePage::onNewNote);
    sideLayout->addWidget(m_newButton);

    m_noteList = new QListWidget(this);
    m_noteList->setStyleSheet(QString(R"(
        QListWidget { background: transparent; border: none; }
        QListWidget::item { border-radius: 8px; padding: 10px; margin-bottom: 2px; color: %1; }
        QListWidget::item:selected { background: %2; }
        QListWidget::item:hover { background: %3; }
    )").arg(C_TEXT_MAIN, C_SELECTED_BG, C_HOVER_BG));
    connect(m_noteList, &QListWidget::currentRowChanged, this, &NotePage::onNoteSelected);
    sideLayout->addWidget(m_noteList, 1);

    m_deleteButton = new QPushButton("Delete note", this);
    m_deleteButton->setCursor(Qt::PointingHandCursor);
    m_deleteButton->setEnabled(false);
    m_deleteButton->setStyleSheet(QString(R"(
        QPushButton { background: transparent; color: %1; border: 1px solid %2; border-radius: 10px; padding: 8px; font-size: 12px; }
        QPushButton:hover { color: #e55555; border-color: #e55555; }
        QPushButton:disabled { color: %3; border-color: %2; }
    )").arg(C_TEXT_MUTED, C_BORDER, C_TEXT_MUTED));
    connect(m_deleteButton, &QPushButton::clicked, this, &NotePage::onDeleteNote);
    sideLayout->addWidget(m_deleteButton);

    mainLayout->addWidget(sidebar);

    // ─── RIGHT PANEL ────────────────────────────────────────────────
    auto *rightPanel = new QWidget(this);
    rightPanel->setStyleSheet("background: transparent;");
    auto *rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setContentsMargins(24, 16, 24, 24);
    rightLayout->setSpacing(16);

    // Toolbar
    auto *toolbar = new QHBoxLayout();
    toolbar->setSpacing(4);

    auto makeToolbarButton = [&](const QString &text, bool bold = false) {
        auto *btn = new QPushButton(text, this);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setFixedSize(32, 32);
        QString weight = bold ? "font-weight: 700;" : "";
        btn->setStyleSheet(QString(
                               "QPushButton { background: transparent; border: none; border-radius: 6px; color: %1; font-size: 14px; %2 }"
                               "QPushButton:hover { background-color: %3; }"
                               ).arg(C_TEXT_MAIN, weight, C_BORDER));
        return btn;
    };

    auto *boldBtn = makeToolbarButton("B", true);
    connect(boldBtn, &QPushButton::clicked, this, &NotePage::onBoldClicked);
    toolbar->addWidget(boldBtn);

    auto *italicBtn = makeToolbarButton("I");
    connect(italicBtn, &QPushButton::clicked, this, &NotePage::onItalicClicked);
    toolbar->addWidget(italicBtn);

    auto *underlineBtn = makeToolbarButton("U");
    connect(underlineBtn, &QPushButton::clicked, this, &NotePage::onUnderlineClicked);
    toolbar->addWidget(underlineBtn);

    auto *sep1 = new QFrame(this);
    sep1->setFixedSize(1, 20);
    sep1->setStyleSheet(QString("background-color: %1;").arg(C_BORDER));
    toolbar->addWidget(sep1);
    toolbar->addSpacing(4);

    auto *bulletBtn = makeToolbarButton("\u2022\u2261");
    connect(bulletBtn, &QPushButton::clicked, this, &NotePage::onBulletListClicked);
    toolbar->addWidget(bulletBtn);

    auto *numberedBtn = makeToolbarButton("1.");
    connect(numberedBtn, &QPushButton::clicked, this, &NotePage::onNumberedListClicked);
    toolbar->addWidget(numberedBtn);

    auto *sep2 = new QFrame(this);
    sep2->setFixedSize(1, 20);
    sep2->setStyleSheet(QString("background-color: %1;").arg(C_BORDER));
    toolbar->addWidget(sep2);
    toolbar->addSpacing(4);

    auto *linkBtn = makeToolbarButton("\U0001F517");
    connect(linkBtn, &QPushButton::clicked, this, &NotePage::onLinkClicked);
    toolbar->addWidget(linkBtn);

    toolbar->addStretch();

    m_statusLabel = new QLabel("", this);
    m_statusLabel->setStyleSheet(QString("background: transparent; color: %1; font-size: 12px;").arg(C_TEXT_MUTED));
    toolbar->addWidget(m_statusLabel);

    toolbar->addSpacing(12);
    auto *moreBtn = makeToolbarButton("\u22EF");
    toolbar->addWidget(moreBtn);

    rightLayout->addLayout(toolbar);

    // ─── PAPER FRAME ────────────────────────────────────────────────
    auto *paperOuter = new QFrame(this);
    paperOuter->setStyleSheet(QString(R"(
        QFrame {
            background-color: %1;
            border: 1px solid %2;
            border-radius: 14px;
        }
    )").arg(C_PAPER_BG, C_PAPER_BORDER));

    auto *paperShadow = new QGraphicsDropShadowEffect(paperOuter);
    paperShadow->setBlurRadius(40);
    paperShadow->setColor(QColor(0, 0, 0, 120));
    paperShadow->setOffset(0, 6);
    paperOuter->setGraphicsEffect(paperShadow);

    auto *paperLayout = new QVBoxLayout(paperOuter);
    paperLayout->setContentsMargins(0, 0, 0, 0);
    paperLayout->setSpacing(0);

    m_editor = new QTextEdit(paperOuter);
    m_editor->setAcceptRichText(true);
    m_editor->setPlaceholderText("Start writing...");
    m_editor->setFrameShape(QFrame::NoFrame);
    m_editor->setStyleSheet(QString(R"(
        QTextEdit {
            background-color: transparent;
            color: %1;
            font-size: 14px;
            padding: 40px 56px;
            border: none;
        }
    )").arg(C_PAPER_TEXT));

    // Stylesheet for block elements (headings, code, tables, etc.)
    m_editor->document()->setDefaultStyleSheet(R"(
        h1 { font-size: 22px; font-weight: 700; color: #1a1a1a; border-bottom: 1px solid #ddd6c8; padding-bottom: 6px; }
        h2, h3 { font-weight: 700; color: #1a1a1a; }
        p, li { color: #333029; line-height: 1.6; }
        code { background-color: #ebe7dc; padding: 2px 5px; border-radius: 4px; font-family: monospace; color: #b03a2e; }
        pre { background-color: #ebe7dc; padding: 12px; border-radius: 8px; font-family: monospace; }
        blockquote { border-left: 3px solid #2E8B57; background-color: #e8f0e6; padding: 10px 14px; color: #2f4f2f; margin: 8px 0; }
        table, th, td { border: 1px solid #ddd6c8; border-collapse: collapse; padding: 6px 10px; }
        th { background-color: #ece8dc; font-weight: 700; }
    )");

    connect(m_editor, &QTextEdit::textChanged, this, &NotePage::onContentChanged);

    paperLayout->addWidget(m_editor, 1);
    rightLayout->addWidget(paperOuter, 1);

    mainLayout->addWidget(rightPanel, 1);
}

// ─── DATA LOADING ──────────────────────────────────────────────────

void NotePage::loadNotes()
{
    bool ok = false;
    m_notes = m_repository.loadAll(&ok);
    if (!ok) {
        QMessageBox::warning(this, "Notes", "Could not load your notes. The session key may be invalid.");
        m_notes.clear();
    }

    std::sort(m_notes.begin(), m_notes.end(),
              [](const NoteEntry &a, const NoteEntry &b) { return a.m_updatedAt > b.m_updatedAt; });

    updateNoteList();
    clearEditor();
    m_currentNoteId.clear();
}

QString NotePage::snippetFor(const QString &markdownContent)
{
    QString flat = markdownContent;
    flat.replace('\n', ' ').replace(QRegularExpression("[#*_`>-]"), "");
    flat = flat.trimmed();
    return flat.left(50) + (flat.size() > 50 ? "..." : "");
}

void NotePage::updateNoteList(const QString &filter)
{
    m_noteList->clear();

    for (const NoteEntry &note : m_notes) {
        if (!filter.isEmpty() && !note.m_title.contains(filter, Qt::CaseInsensitive)) {
            continue;
        }
        QString display = note.m_title.isEmpty() ? "Untitled" : note.m_title;
        QDateTime dt = QDateTime::fromSecsSinceEpoch(note.m_updatedAt);
        QString dateStr = (dt.date() == QDate::currentDate())
                              ? dt.toString("h:mm AP")
                              : dt.toString("MMM d");

        auto *item = new QListWidgetItem(m_noteList);
        auto *row = new QWidget(this);
        auto *rowLayout = new QVBoxLayout(row);
        rowLayout->setContentsMargins(6, 4, 6, 4);
        rowLayout->setSpacing(2);

        auto *titleLbl = new QLabel(display, row);
        titleLbl->setStyleSheet(QString("background: transparent; color: %1; font-size: 13px; font-weight: 600;").arg(C_TEXT_MAIN));
        rowLayout->addWidget(titleLbl);

        auto *snippetLbl = new QLabel(snippetFor(note.m_content), row);
        snippetLbl->setStyleSheet(QString("background: transparent; color: %1; font-size: 11px;").arg(C_TEXT_MUTED));
        snippetLbl->setWordWrap(false);
        rowLayout->addWidget(snippetLbl);

        auto *dateLbl = new QLabel(dateStr, row);
        dateLbl->setStyleSheet(QString("background: transparent; color: %1; font-size: 10px;").arg(C_TEXT_MUTED));
        rowLayout->addWidget(dateLbl);

        item->setSizeHint(row->sizeHint());
        m_noteList->setItemWidget(item, row);
        item->setData(Qt::UserRole, note.m_id);
    }

    if (m_noteList->count() == 0) {
        auto *item = new QListWidgetItem("No notes", m_noteList);
        item->setFlags(Qt::NoItemFlags);
    }
}

void NotePage::clearEditor()
{
    m_ignoreContentChange = true;
    m_editor->clear();
    m_ignoreContentChange = false;
    setStatus("", true);
}

void NotePage::onNoteSelected(int index)
{
    if (index < 0 || index >= m_noteList->count()) {
        clearEditor();
        m_currentNoteId.clear();
        m_deleteButton->setEnabled(false);
        return;
    }

    // Save any pending changes before switching
    saveCurrentNote();

    QListWidgetItem *item = m_noteList->item(index);
    QString id = item->data(Qt::UserRole).toString();
    if (id.isEmpty()) return; // "No notes" placeholder

    auto it = std::find_if(m_notes.begin(), m_notes.end(),
                           [&id](const NoteEntry &n) { return n.m_id == id; });
    if (it == m_notes.end()) return;

    m_currentNoteId = it->m_id;
    m_deleteButton->setEnabled(true);

    m_ignoreContentChange = true;
    m_editor->setMarkdown(it->m_content);
    m_ignoreContentChange = false;
    setStatus("Saved just now", true);
}

void NotePage::onContentChanged()
{
    if (m_ignoreContentChange || m_currentNoteId.isEmpty()) return;

    if (!m_saveTimer) {
        m_saveTimer = new QTimer(this);
        m_saveTimer->setSingleShot(true);
        m_saveTimer->setInterval(600);
        connect(m_saveTimer, &QTimer::timeout, this, [this]() { saveCurrentNote(); });
    }
    m_saveTimer->start();
    setStatus("Saving...", false);
}

void NotePage::saveCurrentNote()
{
    if (m_currentNoteId.isEmpty()) return;

    // Prevent re‑entrant calls (e.g. during list refresh)
    static bool isSaving = false;
    if (isSaving) return;
    isSaving = true;

    for (NoteEntry &note : m_notes) {
        if (note.m_id == m_currentNoteId) {
            QString newContent = m_editor->toMarkdown();
            if (note.m_content != newContent) {
                note.m_content = newContent;

                // Auto‑title from first heading if title is empty
                if (note.m_title.isEmpty() && !newContent.isEmpty()) {
                    QStringList lines = newContent.split('\n');
                    if (!lines.isEmpty() && lines.first().startsWith("# ")) {
                        note.m_title = lines.first().mid(2).trimmed();
                    } else {
                        note.m_title = QString("Note %1").arg(QDateTime::currentDateTime().toString("MMM d"));
                    }
                }

                m_repository.updateEntry(note);

                // Refresh the list only if the title changed (so the list shows the new title)
                updateNoteList(m_searchField->text());

                // Restore selection
                for (int i = 0; i < m_noteList->count(); ++i) {
                    if (m_noteList->item(i)->data(Qt::UserRole).toString() == m_currentNoteId) {
                        m_noteList->setCurrentRow(i);
                        break;
                    }
                }
            }
            setStatus("Saved just now", true);
            break;
        }
    }

    isSaving = false;
}

void NotePage::onNewNote()
{
    NoteEntry newNote;
    newNote.m_title = "";
    newNote.m_content = "";

    if (m_repository.addEntry(newNote)) {
        loadNotes();
        if (m_noteList->count() > 0) {
            m_noteList->setCurrentRow(0);
            m_editor->setFocus();
        }
    }
}

void NotePage::onDeleteNote()
{
    if (m_currentNoteId.isEmpty()) return;

    auto reply = QMessageBox::question(this, "Delete Note", "Delete this note permanently?");
    if (reply == QMessageBox::Yes) {
        if (m_repository.deleteEntry(m_currentNoteId)) {
            loadNotes();
            clearEditor();
            m_currentNoteId.clear();
            m_deleteButton->setEnabled(false);
        }
    }
}

void NotePage::onSearchTextChanged(const QString &text)
{
    updateNoteList(text);

    bool currentStillVisible = false;
    for (const NoteEntry &note : m_notes) {
        if (note.m_id == m_currentNoteId) {
            currentStillVisible = text.isEmpty() || note.m_title.contains(text, Qt::CaseInsensitive);
            break;
        }
    }

    if (!currentStillVisible) {
        clearEditor();
        m_currentNoteId.clear();
        m_deleteButton->setEnabled(false);
    }
}

void NotePage::setStatus(const QString &text, bool /*isSaved*/)
{
    m_statusLabel->setText(text);
}

// ─── FORMATTING TOOLBAR SLOTS ─────────────────────────────────────

void NotePage::onBoldClicked()
{
    QTextCursor cursor = m_editor->textCursor();
    QTextCharFormat fmt;
    fmt.setFontWeight(cursor.charFormat().fontWeight() == QFont::Bold ? QFont::Normal : QFont::Bold);
    cursor.mergeCharFormat(fmt);
    m_editor->mergeCurrentCharFormat(fmt);
}

void NotePage::onItalicClicked()
{
    QTextCursor cursor = m_editor->textCursor();
    QTextCharFormat fmt;
    fmt.setFontItalic(!cursor.charFormat().fontItalic());
    cursor.mergeCharFormat(fmt);
    m_editor->mergeCurrentCharFormat(fmt);
}

void NotePage::onUnderlineClicked()
{
    QTextCursor cursor = m_editor->textCursor();
    QTextCharFormat fmt;
    fmt.setFontUnderline(!cursor.charFormat().fontUnderline());
    cursor.mergeCharFormat(fmt);
    m_editor->mergeCurrentCharFormat(fmt);
}

void NotePage::onBulletListClicked()
{
    QTextCursor cursor = m_editor->textCursor();
    cursor.insertList(QTextListFormat::ListDisc);
}

void NotePage::onNumberedListClicked()
{
    QTextCursor cursor = m_editor->textCursor();
    cursor.insertList(QTextListFormat::ListDecimal);
}

void NotePage::onLinkClicked()
{
    bool ok = false;
    QString url = QInputDialog::getText(this, "Insert Link", "URL:", QLineEdit::Normal, "https://", &ok);
    if (!ok || url.isEmpty()) return;

    QTextCursor cursor = m_editor->textCursor();
    QString linkText = cursor.hasSelection() ? cursor.selectedText() : url;
    cursor.insertHtml(QString("<a href=\"%1\">%2</a>").arg(url, linkText));
}