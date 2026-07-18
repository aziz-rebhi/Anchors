#include "notepage.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListWidget>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>
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
#include <QStackedLayout>
#include <QMenu>
#include <QToolButton>
#include <QUuid>
#include <QSplitter>
#include <QStatusBar>
#include <QScrollBar>
#include <QColorDialog>

/* ── Colour palette ── */
static const char *C_BG           = "#000000";
static const char *C_PANEL_BG     = "#111111";
static const char *C_CARD_BG      = "#181818";
static const char *C_BORDER       = "#242424";
static const char *C_TEXT_PRIMARY = "#F5F5F5";
static const char *C_TEXT_MUTED   = "#8A8A8A";
static const char *C_ACCENT       = "#2E8B57";
static const char *C_HIGHLIGHT    = "#4F9F4F";
static const char *C_HOVER_BG     = "#1A1A1A";
static const char *C_SELECTED_BG  = "#1E2A1E";

NotePage::NotePage(const QByteArray &sessionKey, QWidget *parent)
    : QWidget(parent), m_repository(sessionKey)
{
    setupUi();
    loadNotes();
}

void NotePage::setupUi()
{
    setStyleSheet(QString("background-color: %1;").arg(C_BG));
    setMinimumSize(1024, 640);

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // ─── MAIN SPLITTER (left + center) ────────────────────────────
    m_mainSplitter = new QSplitter(Qt::Horizontal, this);
    m_mainSplitter->setStyleSheet("QSplitter::handle { background-color: #242424; width: 1px; }");

    // ─── LEFT SIDEBAR ──────────────────────────────────────────────
    m_leftPanel = new QWidget(m_mainSplitter);
    m_leftPanel->setFixedWidth(280);
    m_leftPanel->setStyleSheet(QString("background-color: %1;").arg(C_PANEL_BG));
    auto *leftLayout = new QVBoxLayout(m_leftPanel);
    leftLayout->setContentsMargins(12, 16, 12, 16);
    leftLayout->setSpacing(8);

    // Notebook selector
    m_notebookSelector = new QComboBox(m_leftPanel);
    m_notebookSelector->addItem("Notes");
    m_notebookSelector->setStyleSheet(QString(R"(
        QComboBox { background: transparent; border: none; color: %1; font-size: 16px; font-weight: 600; padding: 4px; }
        QComboBox::drop-down { border: none; }
        QComboBox QAbstractItemView { background: %2; border: 1px solid %3; color: %1; selection-background-color: %4; }
    )").arg(C_TEXT_PRIMARY, C_PANEL_BG, C_BORDER, C_SELECTED_BG));
    leftLayout->addWidget(m_notebookSelector);

    // Search
    m_searchField = new QLineEdit(m_leftPanel);
    m_searchField->setPlaceholderText("Search notes...");
    m_searchField->setStyleSheet(QString(R"(
        QLineEdit { background-color: %1; border: 1px solid %2; border-radius: 10px; padding: 8px 12px 8px 36px; color: %3; font-size: 13px; }
        QLineEdit:focus { border-color: %4; }
    )").arg(C_CARD_BG, C_BORDER, C_TEXT_PRIMARY, C_ACCENT));

    QHBoxLayout *searchLayout = new QHBoxLayout();
    searchLayout->setContentsMargins(0, 0, 0, 0);
    QLabel *searchIcon = new QLabel("🔍", m_leftPanel);
    searchIcon->setStyleSheet("background: transparent; color: #8A8A8A; font-size: 14px; padding-left: 10px;");
    searchLayout->addWidget(searchIcon);
    searchLayout->addWidget(m_searchField);
    QWidget *searchWidget = new QWidget(m_leftPanel);
    searchWidget->setLayout(searchLayout);
    leftLayout->addWidget(searchWidget);

    connect(m_searchField, &QLineEdit::textChanged, this, &NotePage::onSearchTextChanged);

    // Action row
    auto *actionLayout = new QHBoxLayout();
    actionLayout->setSpacing(6);

    m_newNoteBtn = new QPushButton("+ New", m_leftPanel);
    m_newNoteBtn->setCursor(Qt::PointingHandCursor);
    m_newNoteBtn->setStyleSheet(QString(R"(
        QPushButton { background-color: %1; color: white; border: none; border-radius: 10px; padding: 6px 12px; font-weight: 600; }
        QPushButton:hover { background-color: %2; }
    )").arg(C_ACCENT, C_HIGHLIGHT));
    connect(m_newNoteBtn, &QPushButton::clicked, this, &NotePage::onNewNote);
    actionLayout->addWidget(m_newNoteBtn);

    m_sortBtn = new QToolButton(m_leftPanel);
    m_sortBtn->setText("⇅");
    m_sortBtn->setToolTip("Sort");
    m_sortBtn->setStyleSheet("QToolButton { background: transparent; border: none; color: #B8B8B8; font-size: 16px; }");
    actionLayout->addWidget(m_sortBtn);

    m_filterBtn = new QToolButton(m_leftPanel);
    m_filterBtn->setText("⌘");
    m_filterBtn->setToolTip("Filter");
    m_filterBtn->setStyleSheet(m_sortBtn->styleSheet());
    actionLayout->addWidget(m_filterBtn);

    actionLayout->addStretch();
    leftLayout->addLayout(actionLayout);

    // Note list
    m_noteList = new QListWidget(m_leftPanel);
    m_noteList->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_noteList, &QListWidget::customContextMenuRequested, this, &NotePage::onNoteListContextMenu);
    m_noteList->setStyleSheet(QString(R"(
        QListWidget { background: transparent; border: none; }
        QListWidget::item { border-radius: 8px; padding: 8px; margin-bottom: 2px; }
        QListWidget::item:selected { background: %1; border-left: 3px solid %2; }
        QListWidget::item:hover { background: %3; }
    )").arg(C_SELECTED_BG, C_ACCENT, C_HOVER_BG));
    connect(m_noteList, &QListWidget::currentRowChanged, this, &NotePage::onNoteSelected);
    leftLayout->addWidget(m_noteList, 1);

    m_mainSplitter->addWidget(m_leftPanel);

    // ─── CENTER PANEL ──────────────────────────────────────────────
    m_centerPanel = new QWidget(m_mainSplitter);
    m_centerPanel->setStyleSheet("background: transparent;");
    auto *centerLayout = new QVBoxLayout(m_centerPanel);
    centerLayout->setContentsMargins(0, 0, 0, 0);
    centerLayout->setSpacing(0);

    // Top: title & metadata row
    m_topWidget = new QWidget(m_centerPanel);
    m_topWidget->setStyleSheet("background: transparent; padding: 16px 24px 0 24px;");
    auto *topLayout = new QVBoxLayout(m_topWidget);
    topLayout->setSpacing(4);

    m_titleEdit = new QLineEdit(m_topWidget);
    m_titleEdit->setPlaceholderText("Untitled");
    m_titleEdit->setStyleSheet(QString("QLineEdit { background: transparent; border: none; color: %1; font-size: 28px; font-weight: 700; padding: 0; }").arg(C_TEXT_PRIMARY));
    connect(m_titleEdit, &QLineEdit::textChanged, this, &NotePage::onTitleChanged);
    topLayout->addWidget(m_titleEdit);

    m_metaLabel = new QLabel(m_topWidget);
    m_metaLabel->setStyleSheet(QString("background: transparent; color: %1; font-size: 12px;").arg(C_TEXT_MUTED));
    topLayout->addWidget(m_metaLabel);

    centerLayout->addWidget(m_topWidget);

    // Toolbar
    m_toolbarWidget = new QWidget(m_centerPanel);
    m_toolbarWidget->setStyleSheet("background: transparent; padding: 4px 24px;");
    m_toolbarLayout = new QHBoxLayout(m_toolbarWidget);
    m_toolbarLayout->setContentsMargins(0, 0, 0, 0);
    m_toolbarLayout->setSpacing(2);

    auto addSeparator = [&]() {
        QFrame *sep = new QFrame(m_toolbarWidget);
        sep->setFixedSize(1, 20);
        sep->setStyleSheet("background-color: #242424;");
        m_toolbarLayout->addWidget(sep);
    };

    // Group 1: Undo/Redo
    QToolButton *undoBtn = new QToolButton(m_toolbarWidget);
    undoBtn->setText("↩");
    undoBtn->setToolTip("Undo (Ctrl+Z)");
    undoBtn->setStyleSheet("QToolButton { background: transparent; border: none; color: #B8B8B8; padding: 4px; } QToolButton:hover { background: #1A1A1A; border-radius: 4px; }");
    connect(undoBtn, &QToolButton::clicked, this, &NotePage::onUndoClicked);
    m_toolbarLayout->addWidget(undoBtn);

    QToolButton *redoBtn = new QToolButton(m_toolbarWidget);
    redoBtn->setText("↪");
    redoBtn->setToolTip("Redo (Ctrl+Y)");
    redoBtn->setStyleSheet(undoBtn->styleSheet());
    connect(redoBtn, &QToolButton::clicked, this, &NotePage::onRedoClicked);
    m_toolbarLayout->addWidget(redoBtn);
    addSeparator();

    // Group 2: Bold, Italic, Underline, Strike, Inline Code
    QToolButton *boldBtn = new QToolButton(m_toolbarWidget);
    boldBtn->setText("B");
    boldBtn->setToolTip("Bold (Ctrl+B)");
    boldBtn->setStyleSheet(undoBtn->styleSheet());
    connect(boldBtn, &QToolButton::clicked, this, &NotePage::onBoldClicked);
    m_toolbarLayout->addWidget(boldBtn);

    QToolButton *italicBtn = new QToolButton(m_toolbarWidget);
    italicBtn->setText("I");
    italicBtn->setToolTip("Italic (Ctrl+I)");
    italicBtn->setStyleSheet(undoBtn->styleSheet());
    connect(italicBtn, &QToolButton::clicked, this, &NotePage::onItalicClicked);
    m_toolbarLayout->addWidget(italicBtn);

    QToolButton *underlineBtn = new QToolButton(m_toolbarWidget);
    underlineBtn->setText("U");
    underlineBtn->setToolTip("Underline (Ctrl+U)");
    underlineBtn->setStyleSheet(undoBtn->styleSheet());
    connect(underlineBtn, &QToolButton::clicked, this, &NotePage::onUnderlineClicked);
    m_toolbarLayout->addWidget(underlineBtn);

    QToolButton *strikeBtn = new QToolButton(m_toolbarWidget);
    strikeBtn->setText("S");
    strikeBtn->setToolTip("Strike");
    strikeBtn->setStyleSheet(undoBtn->styleSheet());
    connect(strikeBtn, &QToolButton::clicked, this, &NotePage::onStrikeClicked);
    m_toolbarLayout->addWidget(strikeBtn);

    QToolButton *codeBtn = new QToolButton(m_toolbarWidget);
    codeBtn->setText("<>");
    codeBtn->setToolTip("Inline Code (Ctrl+Shift+C)");
    codeBtn->setStyleSheet(undoBtn->styleSheet());
    connect(codeBtn, &QToolButton::clicked, this, &NotePage::onInlineCodeClicked);
    m_toolbarLayout->addWidget(codeBtn);
    addSeparator();

    // Group 3: Heading selector (QComboBox)
    m_headingCombo = new QComboBox(m_toolbarWidget);
    m_headingCombo->addItems({"Paragraph", "Heading 1", "Heading 2", "Heading 3"});
    m_headingCombo->setToolTip("Heading level");
    m_headingCombo->setStyleSheet(QString(R"(
        QComboBox { background: transparent; border: 1px solid %1; border-radius: 6px; padding: 2px 6px; color: %2; font-size: 12px; }
        QComboBox::drop-down { border: none; }
        QComboBox QAbstractItemView { background: %3; border: 1px solid %1; color: %2; selection-background-color: %4; }
    )").arg(C_BORDER, C_TEXT_PRIMARY, C_PANEL_BG, C_SELECTED_BG));
    connect(m_headingCombo, QOverload<int>::of(&QComboBox::activated), this, &NotePage::onHeadingChanged);
    m_toolbarLayout->addWidget(m_headingCombo);
    addSeparator();

    // Group 4: Lists
    QToolButton *bulletBtn = new QToolButton(m_toolbarWidget);
    bulletBtn->setText("•");
    bulletBtn->setToolTip("Bullet List");
    bulletBtn->setStyleSheet(undoBtn->styleSheet());
    connect(bulletBtn, &QToolButton::clicked, this, &NotePage::onBulletListClicked);
    m_toolbarLayout->addWidget(bulletBtn);

    QToolButton *numberedBtn = new QToolButton(m_toolbarWidget);
    numberedBtn->setText("1.");
    numberedBtn->setToolTip("Numbered List");
    numberedBtn->setStyleSheet(undoBtn->styleSheet());
    connect(numberedBtn, &QToolButton::clicked, this, &NotePage::onNumberedListClicked);
    m_toolbarLayout->addWidget(numberedBtn);

    QToolButton *checklistBtn = new QToolButton(m_toolbarWidget);
    checklistBtn->setText("✓");
    checklistBtn->setToolTip("Checklist");
    checklistBtn->setStyleSheet(undoBtn->styleSheet());
    connect(checklistBtn, &QToolButton::clicked, this, &NotePage::onChecklistClicked);
    m_toolbarLayout->addWidget(checklistBtn);

    QToolButton *toggleBtn = new QToolButton(m_toolbarWidget);
    toggleBtn->setText("►");
    toggleBtn->setToolTip("Toggle List");
    toggleBtn->setStyleSheet(undoBtn->styleSheet());
    connect(toggleBtn, &QToolButton::clicked, this, &NotePage::onToggleListClicked);
    m_toolbarLayout->addWidget(toggleBtn);
    addSeparator();

    // Group 5: Quote, Callout, Code Block, Divider, Table
    QToolButton *quoteBtn = new QToolButton(m_toolbarWidget);
    quoteBtn->setText("“");
    quoteBtn->setToolTip("Quote");
    quoteBtn->setStyleSheet(undoBtn->styleSheet());
    connect(quoteBtn, &QToolButton::clicked, this, &NotePage::onQuoteClicked);
    m_toolbarLayout->addWidget(quoteBtn);

    QToolButton *calloutBtn = new QToolButton(m_toolbarWidget);
    calloutBtn->setText("💡");
    calloutBtn->setToolTip("Callout");
    calloutBtn->setStyleSheet(undoBtn->styleSheet());
    connect(calloutBtn, &QToolButton::clicked, this, &NotePage::onCalloutClicked);
    m_toolbarLayout->addWidget(calloutBtn);

    QToolButton *codeBlockBtn = new QToolButton(m_toolbarWidget);
    codeBlockBtn->setText("{}");
    codeBlockBtn->setToolTip("Code Block");
    codeBlockBtn->setStyleSheet(undoBtn->styleSheet());
    connect(codeBlockBtn, &QToolButton::clicked, this, &NotePage::onCodeBlockClicked);
    m_toolbarLayout->addWidget(codeBlockBtn);

    QToolButton *dividerBtn = new QToolButton(m_toolbarWidget);
    dividerBtn->setText("—");
    dividerBtn->setToolTip("Divider");
    dividerBtn->setStyleSheet(undoBtn->styleSheet());
    connect(dividerBtn, &QToolButton::clicked, this, &NotePage::onDividerClicked);
    m_toolbarLayout->addWidget(dividerBtn);

    QToolButton *tableBtn = new QToolButton(m_toolbarWidget);
    tableBtn->setText("⊞");
    tableBtn->setToolTip("Table");
    tableBtn->setStyleSheet(undoBtn->styleSheet());
    connect(tableBtn, &QToolButton::clicked, this, &NotePage::onTableClicked);
    m_toolbarLayout->addWidget(tableBtn);
    addSeparator();

    // Group 6: Image, Link, File, Math, Diagram
    QToolButton *imageBtn = new QToolButton(m_toolbarWidget);
    imageBtn->setText("🖼");
    imageBtn->setToolTip("Image");
    imageBtn->setStyleSheet(undoBtn->styleSheet());
    connect(imageBtn, &QToolButton::clicked, this, &NotePage::onImageClicked);
    m_toolbarLayout->addWidget(imageBtn);

    QToolButton *linkBtn = new QToolButton(m_toolbarWidget);
    linkBtn->setText("🔗");
    linkBtn->setToolTip("Link (Ctrl+K)");
    linkBtn->setStyleSheet(undoBtn->styleSheet());
    connect(linkBtn, &QToolButton::clicked, this, &NotePage::onLinkClicked);
    m_toolbarLayout->addWidget(linkBtn);

    QToolButton *fileBtn = new QToolButton(m_toolbarWidget);
    fileBtn->setText("📎");
    fileBtn->setToolTip("File");
    fileBtn->setStyleSheet(undoBtn->styleSheet());
    connect(fileBtn, &QToolButton::clicked, this, &NotePage::onFileClicked);
    m_toolbarLayout->addWidget(fileBtn);

    QToolButton *mathBtn = new QToolButton(m_toolbarWidget);
    mathBtn->setText("∑");
    mathBtn->setToolTip("Math");
    mathBtn->setStyleSheet(undoBtn->styleSheet());
    connect(mathBtn, &QToolButton::clicked, this, &NotePage::onMathClicked);
    m_toolbarLayout->addWidget(mathBtn);

    QToolButton *diagramBtn = new QToolButton(m_toolbarWidget);
    diagramBtn->setText("◊");
    diagramBtn->setToolTip("Diagram");
    diagramBtn->setStyleSheet(undoBtn->styleSheet());
    connect(diagramBtn, &QToolButton::clicked, this, &NotePage::onDiagramClicked);
    m_toolbarLayout->addWidget(diagramBtn);
    addSeparator();

    // Group 7: Text Color, Highlight
    QToolButton *textColorBtn = new QToolButton(m_toolbarWidget);
    textColorBtn->setText("A");
    textColorBtn->setToolTip("Text Color");
    textColorBtn->setStyleSheet(undoBtn->styleSheet());
    connect(textColorBtn, &QToolButton::clicked, this, &NotePage::onTextColorClicked);
    m_toolbarLayout->addWidget(textColorBtn);

    QToolButton *highlightBtn = new QToolButton(m_toolbarWidget);
    highlightBtn->setText("◉");
    highlightBtn->setToolTip("Highlight");
    highlightBtn->setStyleSheet(undoBtn->styleSheet());
    connect(highlightBtn, &QToolButton::clicked, this, &NotePage::onHighlightClicked);
    m_toolbarLayout->addWidget(highlightBtn);
    addSeparator();

    // Group 8: Reading Mode, Focus Mode, Export PDF
    QToolButton *readingModeBtn = new QToolButton(m_toolbarWidget);
    readingModeBtn->setText("📖");
    readingModeBtn->setToolTip("Reading Mode");
    readingModeBtn->setCheckable(true);
    readingModeBtn->setStyleSheet(undoBtn->styleSheet());
    connect(readingModeBtn, &QToolButton::toggled, this, &NotePage::onReadingModeToggled);
    m_toolbarLayout->addWidget(readingModeBtn);

    QToolButton *focusModeBtn = new QToolButton(m_toolbarWidget);
    focusModeBtn->setText("◐");
    focusModeBtn->setToolTip("Focus Mode");
    focusModeBtn->setCheckable(true);
    focusModeBtn->setStyleSheet(undoBtn->styleSheet());
    connect(focusModeBtn, &QToolButton::toggled, this, &NotePage::onFocusModeToggled);
    m_toolbarLayout->addWidget(focusModeBtn);

    QToolButton *exportBtn = new QToolButton(m_toolbarWidget);
    exportBtn->setText("⬇");
    exportBtn->setToolTip("Export PDF");
    exportBtn->setStyleSheet(undoBtn->styleSheet());
    connect(exportBtn, &QToolButton::clicked, this, &NotePage::onExportPdf);
    m_toolbarLayout->addWidget(exportBtn);

    m_toolbarLayout->addStretch();

    centerLayout->addWidget(m_toolbarWidget);

    // Editor area with placeholder
    m_editor = new QTextEdit(m_centerPanel);
    m_editor->setAcceptRichText(true);
    m_editor->setPlaceholderText("Start writing...");
    m_editor->setFrameShape(QFrame::NoFrame);
    m_editor->setStyleSheet(QString(R"(
        QTextEdit {
            background-color: transparent;
            color: %1;
            font-size: 16px;
            padding: 24px 48px;
            line-height: 1.7;
        }
    )").arg(C_TEXT_PRIMARY));
    m_editor->document()->setDefaultStyleSheet(R"(
        h1 { font-size: 26px; font-weight: 700; color: #F5F5F5; margin-top: 16px; margin-bottom: 8px; }
        h2 { font-size: 22px; font-weight: 600; color: #F5F5F5; margin-top: 14px; margin-bottom: 6px; }
        h3 { font-size: 18px; font-weight: 600; color: #F5F5F5; margin-top: 12px; margin-bottom: 4px; }
        p { color: #F5F5F5; line-height: 1.7; margin: 8px 0; }
        blockquote { border-left: 3px solid #2E8B57; background-color: #1A1A1A; padding: 8px 16px; margin: 8px 0; color: #B8B8B8; }
        code { background-color: #1A1A1A; padding: 2px 6px; border-radius: 4px; font-family: monospace; color: #4F9F4F; }
        pre { background-color: #1A1A1A; padding: 12px; border-radius: 8px; font-family: monospace; color: #F5F5F5; }
        table, th, td { border: 1px solid #242424; border-collapse: collapse; padding: 6px 10px; }
        th { background-color: #1A1A1A; }
    )");
    connect(m_editor, &QTextEdit::textChanged, this, &NotePage::onContentChanged);
    connect(m_editor, &QTextEdit::cursorPositionChanged, this, &NotePage::updateStatusBar);

    // Empty state
    m_placeholderContainer = new QWidget(m_centerPanel);
    m_placeholderContainer->setStyleSheet("background: transparent;");
    auto *emptyLayout = new QVBoxLayout(m_placeholderContainer);
    emptyLayout->setAlignment(Qt::AlignCenter);
    emptyLayout->setSpacing(12);

    auto *emptyIcon = new QLabel("\U0001F4D3", m_placeholderContainer);
    emptyIcon->setAlignment(Qt::AlignCenter);
    emptyIcon->setStyleSheet("background: transparent; font-size: 56px;");
    emptyLayout->addWidget(emptyIcon);

    emptyLayout->addSpacing(4);

    auto *emptyTitle = new QLabel("No note selected", m_placeholderContainer);
    emptyTitle->setAlignment(Qt::AlignCenter);
    emptyTitle->setStyleSheet(QString(
                                  "background: transparent; color: %1; font-size: 20px; font-weight: 700;"
                                  ).arg(C_TEXT_PRIMARY));
    emptyLayout->addWidget(emptyTitle);

    auto *emptyDesc = new QLabel("Select a note from the list, or create a new one.", m_placeholderContainer);
    emptyDesc->setAlignment(Qt::AlignCenter);
    emptyDesc->setStyleSheet(QString(
                                 "background: transparent; color: %1; font-size: 13px;"
                                 ).arg(C_TEXT_MUTED));
    emptyLayout->addWidget(emptyDesc);

    emptyLayout->addSpacing(12);

    auto *emptyNewButton = new QPushButton("+  New Note", m_placeholderContainer);
    emptyNewButton->setCursor(Qt::PointingHandCursor);
    emptyNewButton->setFixedWidth(180);
    emptyNewButton->setStyleSheet(QString(R"(
        QPushButton { background-color: %1; color: white; border: none; border-radius: 10px; padding: 12px; font-size: 14px; font-weight: 600; }
        QPushButton:hover { background-color: %2; }
    )").arg(C_ACCENT, C_HIGHLIGHT));
    connect(emptyNewButton, &QPushButton::clicked, this, &NotePage::onNewNote);

    auto *buttonRow = new QHBoxLayout();
    buttonRow->addStretch();
    buttonRow->addWidget(emptyNewButton);
    buttonRow->addStretch();
    emptyLayout->addLayout(buttonRow);

    m_placeholderContainer->hide();

    centerLayout->addWidget(m_editor, 1);
    centerLayout->addWidget(m_placeholderContainer);

    m_mainSplitter->addWidget(m_centerPanel);

    // ─── STATUS BAR ──────────────────────────────────────────────────
    m_statusBarWidget = new QWidget(this);
    m_statusBarWidget->setStyleSheet(QString("background-color: %1; border-top: 1px solid %2; padding: 4px 12px;").arg(C_PANEL_BG, C_BORDER));
    auto *statusLayout = new QHBoxLayout(m_statusBarWidget);
    statusLayout->setContentsMargins(0, 0, 0, 0);
    statusLayout->setSpacing(12);

    m_wordCountLabel = new QLabel("0 words", m_statusBarWidget);
    m_wordCountLabel->setStyleSheet(QString("color: %1; font-size: 11px;").arg(C_TEXT_MUTED));
    statusLayout->addWidget(m_wordCountLabel);

    m_charCountLabel = new QLabel("0 chars", m_statusBarWidget);
    m_charCountLabel->setStyleSheet(m_wordCountLabel->styleSheet());
    statusLayout->addWidget(m_charCountLabel);

    m_readingTimeLabel = new QLabel("0 min read", m_statusBarWidget);
    m_readingTimeLabel->setStyleSheet(m_wordCountLabel->styleSheet());
    statusLayout->addWidget(m_readingTimeLabel);

    statusLayout->addStretch();

    m_cursorPosLabel = new QLabel("Ln 1 Col 1", m_statusBarWidget);
    m_cursorPosLabel->setStyleSheet(m_wordCountLabel->styleSheet());
    statusLayout->addWidget(m_cursorPosLabel);

    m_zoomLabel = new QLabel("100%", m_statusBarWidget);
    m_zoomLabel->setStyleSheet(m_wordCountLabel->styleSheet());
    statusLayout->addWidget(m_zoomLabel);

    m_saveStatusLabel = new QLabel("Saved", m_statusBarWidget);
    m_saveStatusLabel->setStyleSheet(m_wordCountLabel->styleSheet());
    statusLayout->addWidget(m_saveStatusLabel);

    // ─── LAYOUT ─────────────────────────────────────────────────────
    mainLayout->addWidget(m_mainSplitter, 1);
    mainLayout->addWidget(m_statusBarWidget);

    // Set splitter sizes: left 280, center takes the rest
    m_mainSplitter->setSizes({280, width() - 280});

    // Initially, show placeholder for no note selected
    showPlaceholder(true);
    m_editor->hide();
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
    updateStatusBar();
    clearEditor();
    m_currentNoteId.clear();
    showPlaceholder(true);
    m_editor->hide();
}

// ─── SIMPLIFIED NOTE LIST: only title + pin icon ────────────────
void NotePage::updateNoteList(const QString &filter)
{
    m_noteList->clear();

    QVector<NoteEntry> pinned, unpinned;
    for (const NoteEntry &note : m_notes) {
        if (m_pinnedNotes.contains(note.m_id))
            pinned.append(note);
        else
            unpinned.append(note);
    }

    auto addItem = [&](const NoteEntry &note) {
        if (!filter.isEmpty() && !note.m_title.contains(filter, Qt::CaseInsensitive)) {
            return;
        }
        QString display = note.m_title.isEmpty() ? "Untitled" : note.m_title;

        auto *item = new QListWidgetItem(m_noteList);
        auto *row = new QWidget(m_noteList);
        auto *rowLayout = new QHBoxLayout(row);
        rowLayout->setContentsMargins(8, 6, 8, 6);   // more padding
        rowLayout->setSpacing(6);

        QLabel *titleLbl = new QLabel(display, row);
        // Bigger, bolder title
        titleLbl->setStyleSheet(QString("background: transparent; color: %1; font-size: 14px; font-weight: 700;").arg(C_TEXT_PRIMARY));
        rowLayout->addWidget(titleLbl, 1);

        if (m_pinnedNotes.contains(note.m_id)) {
            QLabel *pinIcon = new QLabel("📌", row);
            pinIcon->setStyleSheet("background: transparent; color: #B8B8B8; font-size: 12px;");
            rowLayout->addWidget(pinIcon);
        }

        // Increase row height for better visibility
        item->setSizeHint(QSize(0, 40));
        m_noteList->setItemWidget(item, row);
        item->setData(Qt::UserRole, note.m_id);
    };

    for (const NoteEntry &note : pinned) addItem(note);
    for (const NoteEntry &note : unpinned) addItem(note);

    if (m_noteList->count() == 0) {
        auto *item = new QListWidgetItem("No notes", m_noteList);
        item->setFlags(Qt::NoItemFlags);
    }
}

QString NotePage::snippetFor(const QString &markdownContent)
{
    Q_UNUSED(markdownContent);
    return QString();
}

void NotePage::clearEditor()
{
    m_ignoreContentChange = true;
    m_editor->clear();
    m_titleEdit->clear();
    m_ignoreContentChange = false;
    setStatus("", true);
    updateStatusBar();
}

void NotePage::showPlaceholder(bool show)
{
    if (show) {
        m_topWidget->hide();
        m_toolbarWidget->hide();
        m_editor->hide();
        m_placeholderContainer->show();
    } else {
        m_topWidget->show();
        m_toolbarWidget->show();
        m_editor->show();
        m_placeholderContainer->hide();
    }
}

void NotePage::switchToEditMode(const QString &noteId)
{
    if (!noteId.isEmpty()) {
        for (int i = 0; i < m_noteList->count(); ++i) {
            if (m_noteList->item(i)->data(Qt::UserRole).toString() == noteId) {
                m_noteList->setCurrentRow(i);
                break;
            }
        }
    } else {
        clearEditor();
        m_currentNoteId.clear();
        showPlaceholder(true);
    }
}

void NotePage::onNoteSelected(int index)
{
    if (index < 0 || index >= m_noteList->count()) {
        clearEditor();
        m_currentNoteId.clear();
        showPlaceholder(true);
        return;
    }

    saveCurrentNote();

    QListWidgetItem *item = m_noteList->item(index);
    QString id = item->data(Qt::UserRole).toString();
    if (id.isEmpty()) return;

    auto it = std::find_if(m_notes.begin(), m_notes.end(),
                           [&id](const NoteEntry &n) { return n.m_id == id; });
    if (it == m_notes.end()) return;

    m_currentNoteId = it->m_id;
    showPlaceholder(false);

    m_ignoreContentChange = true;
    m_ignoreTitleChange = true;
    m_titleEdit->setText(it->m_title);
    m_editor->setMarkdown(it->m_content);
    m_ignoreTitleChange = false;
    m_ignoreContentChange = false;
    setStatus("Saved just now", true);
    updateStatusBar();
}

void NotePage::onTitleChanged()
{
    if (m_ignoreTitleChange || m_currentNoteId.isEmpty()) return;

    if (!m_saveTimer) {
        m_saveTimer = new QTimer(this);
        m_saveTimer->setSingleShot(true);
        m_saveTimer->setInterval(600);
        connect(m_saveTimer, &QTimer::timeout, this, [this]() { saveCurrentNote(); });
    }
    m_saveTimer->start();
    setStatus("Saving...", false);
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
    updateStatusBar();
}

void NotePage::saveCurrentNote()
{
    if (m_currentNoteId.isEmpty()) return;
    if (m_isSaving) return;
    m_isSaving = true;

    for (NoteEntry &note : m_notes) {
        if (note.m_id == m_currentNoteId) {
            QString newContent = m_editor->toMarkdown();
            QString newTitle = m_titleEdit->text().trimmed();
            bool changed = false;

            if (note.m_content != newContent) {
                note.m_content = newContent;
                changed = true;
            }
            if (note.m_title != newTitle) {
                note.m_title = newTitle;
                changed = true;
            }

            if (changed) {
                note.m_updatedAt = QDateTime::currentSecsSinceEpoch();
                m_repository.updateEntry(note);

                m_noteList->blockSignals(true);
                updateNoteList(m_searchField->text());
                for (int i = 0; i < m_noteList->count(); ++i) {
                    if (m_noteList->item(i)->data(Qt::UserRole).toString() == m_currentNoteId) {
                        m_noteList->setCurrentRow(i);
                        break;
                    }
                }
                m_noteList->blockSignals(false);
            }
            setStatus("Saved just now", true);
            updateStatusBar();
            break;
        }
    }

    m_isSaving = false;
}

void NotePage::onNewNote()
{
    QString newId = QUuid::createUuid().toString(QUuid::WithoutBraces);

    NoteEntry newNote;
    newNote.m_id = newId;
    newNote.m_title = "";
    newNote.m_content = "";

    if (m_repository.addEntry(newNote)) {
        loadNotes();
        switchToEditMode(newId);
        m_titleEdit->setFocus();
        m_titleEdit->selectAll();
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
            showPlaceholder(true);
        }
    }
}

void NotePage::onSearchTextChanged(const QString &text)
{
    updateNoteList(text);
    bool visible = false;
    for (int i = 0; i < m_noteList->count(); ++i) {
        if (m_noteList->item(i)->data(Qt::UserRole).toString() == m_currentNoteId) {
            visible = true;
            break;
        }
    }
    if (!visible && !m_currentNoteId.isEmpty()) {
        clearEditor();
        m_currentNoteId.clear();
        showPlaceholder(true);
    }
}

void NotePage::onNoteListContextMenu(const QPoint &pos)
{
    QListWidgetItem *item = m_noteList->itemAt(pos);
    if (!item) return;

    QString noteId = item->data(Qt::UserRole).toString();
    if (noteId.isEmpty()) return;

    bool isPinned = m_pinnedNotes.contains(noteId);

    QMenu menu(this);
    QAction *pinAction = menu.addAction(isPinned ? "Unpin note" : "Pin note");
    connect(pinAction, &QAction::triggered, this, [this, noteId]() { onTogglePinned(noteId); });

    menu.exec(m_noteList->mapToGlobal(pos));
}

void NotePage::onTogglePinned(const QString &noteId)
{
    if (m_pinnedNotes.contains(noteId))
        m_pinnedNotes.remove(noteId);
    else
        m_pinnedNotes.insert(noteId);
    updateNoteList(m_searchField->text());
}

void NotePage::updateStatusBar()
{
    if (!m_editor) return;
    QString text = m_editor->toPlainText();
    int wordCount = text.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts).count();
    int charCount = text.length();
    int readingTime = (wordCount == 0) ? 0 : qMax(1, wordCount / 200);

    m_wordCountLabel->setText(QString("%1 words").arg(wordCount));
    m_charCountLabel->setText(QString("%1 chars").arg(charCount));
    m_readingTimeLabel->setText(QString("%1 min read").arg(readingTime));

    QTextCursor cursor = m_editor->textCursor();
    int line = cursor.blockNumber() + 1;
    int col = cursor.columnNumber() + 1;
    m_cursorPosLabel->setText(QString("Ln %1 Col %2").arg(line).arg(col));
}

void NotePage::setStatus(const QString &text, bool isSaved)
{
    m_saveStatusLabel->setText(text);
    if (isSaved) {
        m_saveStatusLabel->setStyleSheet(QString("color: %1; font-size: 11px;").arg(C_TEXT_MUTED));
    } else {
        m_saveStatusLabel->setStyleSheet(QString("color: %1; font-size: 11px;").arg(C_ACCENT));
    }
}

// ─── READING / FOCUS MODE ─────────────────────────────────────────

void NotePage::onReadingModeToggled(bool enabled)
{
    m_readingMode = enabled;
    if (enabled) {
        m_leftPanel->hide();
        m_statusBarWidget->hide();
        m_toolbarLayout->parentWidget()->hide();
        m_metaLabel->hide();
        m_mainSplitter->setSizes({0, width()});
        m_editor->setStyleSheet(m_editor->styleSheet() + "padding: 40px 80px; max-width: 850px; margin: 0 auto;");
    } else {
        m_leftPanel->show();
        m_statusBarWidget->show();
        m_toolbarLayout->parentWidget()->show();
        m_metaLabel->show();
        m_mainSplitter->setSizes({280, width() - 280});
        m_editor->setStyleSheet(m_editor->styleSheet().replace("padding: 40px 80px; max-width: 850px; margin: 0 auto;", ""));
    }
}

void NotePage::onFocusModeToggled(bool enabled)
{
    m_focusMode = enabled;
    if (enabled) {
        m_statusBarWidget->hide();
    } else {
        if (!m_readingMode) {
            m_statusBarWidget->show();
        }
    }
}

// ─── FORMATTING TOOLBAR SLOTS ────────────────────────────────────

void NotePage::onUndoClicked() { if (m_editor) m_editor->undo(); }
void NotePage::onRedoClicked() { if (m_editor) m_editor->redo(); }

void NotePage::onHeadingChanged(int index)
{
    if (!m_editor) return;
    QTextCursor cursor = m_editor->textCursor();
    QTextBlockFormat fmt;
    switch (index) {
    case 1: fmt.setHeadingLevel(1); break;
    case 2: fmt.setHeadingLevel(2); break;
    case 3: fmt.setHeadingLevel(3); break;
    default: fmt.setHeadingLevel(0); break;
    }
    cursor.mergeBlockFormat(fmt);
}

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

void NotePage::onStrikeClicked()
{
    QTextCursor cursor = m_editor->textCursor();
    QTextCharFormat fmt;
    fmt.setFontStrikeOut(!cursor.charFormat().fontStrikeOut());
    cursor.mergeCharFormat(fmt);
    m_editor->mergeCurrentCharFormat(fmt);
}

void NotePage::onInlineCodeClicked()
{
    QTextCursor cursor = m_editor->textCursor();
    if (cursor.hasSelection()) {
        QString selected = cursor.selectedText();
        cursor.insertText("`" + selected + "`");
    } else {
        cursor.insertText("``");
        cursor.movePosition(QTextCursor::Left);
        m_editor->setTextCursor(cursor);
    }
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

void NotePage::onChecklistClicked()
{
    QTextCursor cursor = m_editor->textCursor();
    QTextBlockFormat fmt = cursor.blockFormat();
    fmt.setMarker(QTextBlockFormat::MarkerType::Unchecked);
    cursor.mergeBlockFormat(fmt);
}

void NotePage::onToggleListClicked()
{
    QMessageBox::information(this, "Toggle List", "Toggle lists will be added later.");
}

void NotePage::onQuoteClicked()
{
    QTextCursor cursor = m_editor->textCursor();
    cursor.movePosition(QTextCursor::StartOfBlock);
    cursor.insertText("> ");
    QTextBlockFormat fmt;
    fmt.setLeftMargin(20);
    cursor.mergeBlockFormat(fmt);
}

void NotePage::onCalloutClicked()
{
    QTextCursor cursor = m_editor->textCursor();
    cursor.movePosition(QTextCursor::StartOfBlock);
    cursor.insertText("> 💡 ");
    QTextBlockFormat fmt;
    fmt.setLeftMargin(20);
    fmt.setBackground(QColor("#1A1A1A"));
    cursor.mergeBlockFormat(fmt);
}

void NotePage::onCodeBlockClicked()
{
    if (!m_editor) return;
    QTextCursor cursor = m_editor->textCursor();
    if (cursor.hasSelection()) {
        QString selected = cursor.selectedText();
        cursor.insertText("```\n" + selected + "\n```");
    } else {
        cursor.insertText("```\n\n```");
        cursor.movePosition(QTextCursor::Left, QTextCursor::MoveAnchor, 1);
        m_editor->setTextCursor(cursor);
    }
}

void NotePage::onDividerClicked()
{
    m_editor->insertPlainText("\n---\n");
}

void NotePage::onTableClicked()
{
    m_editor->insertPlainText("\n| Header 1 | Header 2 |\n|----------|----------|\n| Cell 1   | Cell 2   |\n");
}

void NotePage::onImageClicked()
{
    QMessageBox::information(this, "Image", "Image insertion will be added later.");
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

void NotePage::onFileClicked() { QMessageBox::information(this, "File", "File attachment will be added later."); }
void NotePage::onMathClicked() { QMessageBox::information(this, "Math", "Math insertion will be added later."); }
void NotePage::onDiagramClicked() { QMessageBox::information(this, "Diagram", "Diagram insertion will be added later."); }

void NotePage::onTextColorClicked()
{
    QColor color = QColorDialog::getColor(Qt::white, this, "Select text color");
    if (color.isValid()) {
        QTextCursor cursor = m_editor->textCursor();
        QTextCharFormat fmt;
        fmt.setForeground(color);
        cursor.mergeCharFormat(fmt);
        m_editor->mergeCurrentCharFormat(fmt);
    }
}

void NotePage::onHighlightClicked()
{
    QColor color = QColorDialog::getColor(Qt::yellow, this, "Select highlight color");
    if (color.isValid()) {
        QTextCursor cursor = m_editor->textCursor();
        QTextCharFormat fmt;
        fmt.setBackground(color);
        cursor.mergeCharFormat(fmt);
        m_editor->mergeCurrentCharFormat(fmt);
    }
}

void NotePage::onExportPdf()
{
    QMessageBox::information(this, "Export PDF", "PDF export will be added later.");
}