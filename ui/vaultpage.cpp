#include "vaultpage.h"
#include "vaultentrydialog.h"
#include "../vault/cliboardguard.h"

#include <QFrame>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QMessageBox>
#include <QScrollArea>
#include <algorithm>

static const char *C_TEXT_MAIN   = "#e1e3db";
static const char *C_TEXT_MUTED  = "#8c9386";
static const char *C_TEXT_DIM    = "#c1c9bb";
static const char *C_CARD_BG     = "#181818";
static const char *C_BORDER      = "#242424";
static const char *C_ACCENT      = "#2E8B57";
static const char *C_ACCENT_DARK = "#0B3D0B";
static const char *C_ACCENT_DIM  = "#9ed492";
static const char *C_INPUT_BG    = "#111111";

// Category -> (icon, subtitle) — matches the Stitch reference's four
// cards. "Other" is a valid category for entries but deliberately has
// no card of its own, same as the mockup.
struct CategoryMeta { QString icon; QString subtitle; };
static const QMap<QString, CategoryMeta> kCategoryMeta = {
    {"Social",   {"\U0001F465", "Personal networks & media"}},
    {"Work",     {"\U0001F4BC", "Enterprise & productivity"}},
    {"Learning", {"\U0001F393", "Courses & educational"}},
    {"Finance",  {"\U0001F3E6", "Banking & crypto"}},
    };

VaultPage::VaultPage(const QByteArray &sessionKey, QWidget *parent)
    : QWidget(parent), m_repository(sessionKey)
{
    setupUi();
    reloadEntries();
}

void VaultPage::setupUi()
{
    setStyleSheet("background-color: #000000;");

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(40, 32, 40, 32);
    mainLayout->setSpacing(24);

    // --- Header: title + subtitle, search on the right ---
    auto *headerRow = new QHBoxLayout();
    auto *titleCol = new QVBoxLayout();
    titleCol->setSpacing(4);

    auto *title = new QLabel("Vault", this);
    title->setStyleSheet(QString("background: transparent; color: %1; font-size: 32px; font-weight: 700;").arg(C_TEXT_MAIN));
    titleCol->addWidget(title);

    auto *subtitle = new QLabel("Securely manage your credentials across categories.", this);
    subtitle->setStyleSheet(QString("background: transparent; color: %1; font-size: 13px;").arg(C_TEXT_MUTED));
    titleCol->addWidget(subtitle);

    headerRow->addLayout(titleCol, 1);

    m_searchField = new QLineEdit(this);
    m_searchField->setPlaceholderText("Search entries...");
    m_searchField->setFixedWidth(260);
    m_searchField->setStyleSheet(QString(R"(
        QLineEdit {
            background-color: %1; border: 1px solid %2; border-radius: 10px;
            color: %3; padding: 10px 14px; font-size: 13px;
        }
        QLineEdit:focus { border-color: %4; }
    )").arg(C_INPUT_BG, C_BORDER, C_TEXT_MAIN, C_ACCENT));
    connect(m_searchField, &QLineEdit::textChanged, this, &VaultPage::onSearchChanged);
    headerRow->addWidget(m_searchField, 0, Qt::AlignTop);

    mainLayout->addLayout(headerRow);

    // --- Categories row ---
    auto *categoriesLabel = new QLabel("Categories", this);
    categoriesLabel->setStyleSheet(QString("background: transparent; color: %1; font-size: 18px; font-weight: 600;").arg(C_TEXT_MAIN));
    mainLayout->addWidget(categoriesLabel);

    m_categoriesLayout = new QHBoxLayout();
    m_categoriesLayout->setSpacing(16);
    mainLayout->addLayout(m_categoriesLayout);

    // --- "All Entries" section header + sort toggle ---
    auto *entriesHeaderRow = new QHBoxLayout();
    auto *entriesLabel = new QLabel("All Entries", this);
    entriesLabel->setStyleSheet(QString("background: transparent; color: %1; font-size: 18px; font-weight: 600;").arg(C_TEXT_MAIN));
    entriesHeaderRow->addWidget(entriesLabel);
    entriesHeaderRow->addStretch();

    auto sortButtonStyle = [](bool active) {
        return QString(R"(
            QPushButton { background: transparent; border: none; color: %1; font-size: 12px; font-weight: %2; padding: 4px 8px; }
        )").arg(active ? C_ACCENT_DIM : C_TEXT_MUTED, active ? "700" : "500");
    };

    m_sortAZButton = new QPushButton("A-Z", this);
    m_sortAZButton->setCursor(Qt::PointingHandCursor);
    m_sortAZButton->setStyleSheet(sortButtonStyle(false));
    connect(m_sortAZButton, &QPushButton::clicked, this, [this]() { onSortModeChanged(true); });
    entriesHeaderRow->addWidget(m_sortAZButton);

    m_sortRecentButton = new QPushButton("Recent", this);
    m_sortRecentButton->setCursor(Qt::PointingHandCursor);
    m_sortRecentButton->setStyleSheet(sortButtonStyle(true));
    connect(m_sortRecentButton, &QPushButton::clicked, this, [this]() { onSortModeChanged(false); });
    entriesHeaderRow->addWidget(m_sortRecentButton);

    mainLayout->addLayout(entriesHeaderRow);

    // --- Entries table-like header row ---
    auto *columnHeader = new QFrame(this);
    columnHeader->setStyleSheet(QString("background: transparent; border-bottom: 1px solid %1;").arg(C_BORDER));
    auto *columnHeaderLayout = new QHBoxLayout(columnHeader);
    columnHeaderLayout->setContentsMargins(16, 0, 16, 8);
    auto addColumnLabel = [&](const QString &text, int stretch) {
        auto *l = new QLabel(text, this);
        l->setStyleSheet(QString("background: transparent; color: %1; font-size: 11px; font-weight: 700; letter-spacing: 0.05em;").arg(C_TEXT_MUTED));
        columnHeaderLayout->addWidget(l, stretch);
    };
    addColumnLabel("SERVICE", 3);
    addColumnLabel("USERNAME", 3);
    addColumnLabel("PASSWORD", 2);
    addColumnLabel("ACTIONS", 2);
    mainLayout->addWidget(columnHeader);

    // --- Scrollable entries list ---
    auto *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setStyleSheet("QScrollArea { background: transparent; border: none; }");
    auto *scrollContent = new QWidget();
    scrollContent->setStyleSheet("background: transparent;");
    m_entriesLayout = new QVBoxLayout(scrollContent);
    m_entriesLayout->setContentsMargins(0, 0, 0, 0);
    m_entriesLayout->setSpacing(4);
    scrollArea->setWidget(scrollContent);
    mainLayout->addWidget(scrollArea, 1);

    // --- Add Password button, bottom-right ---
    auto *addRow = new QHBoxLayout();
    addRow->addStretch();
    auto *addButton = new QPushButton("+  Add Password", this);
    addButton->setCursor(Qt::PointingHandCursor);
    addButton->setStyleSheet(QString(R"(
        QPushButton {
            background-color: %1; color: white; border: none; border-radius: 9999px;
            padding: 12px 24px; font-size: 14px; font-weight: 600;
        }
        QPushButton:hover { background-color: %2; }
    )").arg(C_ACCENT, C_ACCENT_DARK));
    connect(addButton, &QPushButton::clicked, this, &VaultPage::onAddClicked);
    addRow->addWidget(addButton);
    mainLayout->addLayout(addRow);
}

QFrame *VaultPage::createCategoryCard(const QString &icon, const QString &name,
                                      const QString &subtitle, int count)
{
    bool isActive = (m_activeCategory == name);

    auto *card = new QFrame(this);
    card->setCursor(Qt::PointingHandCursor);
    card->setStyleSheet(QString(R"(
        QFrame {
            background-color: %1;
            border: 1px solid %2;
            border-radius: 16px;
        }
    )").arg(C_CARD_BG, isActive ? C_ACCENT : C_BORDER));
    card->setMinimumWidth(180);
    card->setMinimumHeight(110);

    auto *layout = new QVBoxLayout(card);
    layout->setContentsMargins(18, 16, 18, 16);
    layout->setSpacing(6);

    auto *topRow = new QHBoxLayout();
    auto *iconLabel = new QLabel(icon, card);
    iconLabel->setStyleSheet(QString(
                                 "background-color: %1; border-radius: 8px; font-size: 18px; padding: 6px;"
                                 ).arg(C_ACCENT_DARK));
    topRow->addWidget(iconLabel);
    topRow->addStretch();

    auto *countLabel = new QLabel(QString::number(count), card);
    countLabel->setStyleSheet(QString("background: transparent; color: %1; font-size: 22px; font-weight: 700;").arg(C_TEXT_MAIN));
    topRow->addWidget(countLabel);
    layout->addLayout(topRow);

    auto *nameLabel = new QLabel(name, card);
    nameLabel->setStyleSheet(QString("background: transparent; color: %1; font-size: 15px; font-weight: 600;").arg(C_TEXT_MAIN));
    layout->addWidget(nameLabel);

    auto *subLabel = new QLabel(subtitle, card);
    subLabel->setStyleSheet(QString("background: transparent; color: %1; font-size: 11px;").arg(C_TEXT_MUTED));
    subLabel->setWordWrap(true);
    layout->addWidget(subLabel);

    // A QFrame doesn't emit clicked() natively — install a click handler
    // via an event filter would be more "correct", but for a single,
    // simple case, a mouse press handler on a small subclass isn't
    // worth the extra file. Simplest working approach: wrap the whole
    // card in a transparent QPushButton-like click zone using
    // mousePressEvent via a lambda-friendly trick — instead, we just
    // make the card itself clickable by installing this widget's own
    // mousePressEvent through an event filter on `this` page.
    card->installEventFilter(this);
    card->setProperty("categoryName", name);

    return card;
}

bool VaultPageEventFilterHelper(); // unused forward decl placeholder removed below

void VaultPage::reloadEntries()
{
    bool ok = false;
    m_entries = m_repository.loadAll(&ok);
    if (!ok) {
        QMessageBox::warning(this, "Vault",
                             "Could not load your vault entries. The session key may be invalid.");
        m_entries.clear();
    }
    rebuildCategoryCards();
    rebuildEntriesList();
}

void VaultPage::rebuildCategoryCards()
{
    QLayoutItem *item;
    while ((item = m_categoriesLayout->takeAt(0)) != nullptr) {
        if (QWidget *w = item->widget()) w->deleteLater();
        delete item;
    }

    for (auto it = kCategoryMeta.constBegin(); it != kCategoryMeta.constEnd(); ++it) {
        const QString &category = it.key();
        int count = std::count_if(m_entries.begin(), m_entries.end(),
                                  [&category](const VaultEntry &e) { return e.category == category; });
        m_categoriesLayout->addWidget(
            createCategoryCard(it.value().icon, category, it.value().subtitle, count));
    }
}

void VaultPage::rebuildEntriesList()
{
    QLayoutItem *item;
    while ((item = m_entriesLayout->takeAt(0)) != nullptr) {
        if (QWidget *w = item->widget()) w->deleteLater();
        delete item;
    }

    QVector<VaultEntry> filtered;
    for (const VaultEntry &e : m_entries) {
        if (!m_activeCategory.isEmpty() && e.category != m_activeCategory) continue;
        if (!m_searchText.isEmpty()
            && !e.title.contains(m_searchText, Qt::CaseInsensitive)
            && !e.username.contains(m_searchText, Qt::CaseInsensitive)) {
            continue;
        }
        filtered.append(e);
    }

    if (m_sortAlphabetically) {
        std::sort(filtered.begin(), filtered.end(),
                  [](const VaultEntry &a, const VaultEntry &b) { return a.title.toLower() < b.title.toLower(); });
    } else {
        std::sort(filtered.begin(), filtered.end(),
                  [](const VaultEntry &a, const VaultEntry &b) { return a.updatedAt > b.updatedAt; });
    }

    if (filtered.isEmpty()) {
        auto *empty = new QLabel(
            m_entries.isEmpty()
                ? "No passwords saved yet. Click \"Add Password\" to create your first entry."
                : "No entries match your search/filter.",
            this);
        empty->setStyleSheet(QString("background: transparent; color: %1; font-size: 13px; padding: 24px;").arg(C_TEXT_MUTED));
        empty->setAlignment(Qt::AlignCenter);
        m_entriesLayout->addWidget(empty);
        return;
    }

    for (const VaultEntry &entry : filtered) {
        auto *row = new QFrame(this);
        row->setStyleSheet(QString(
                               "QFrame { background: transparent; border-bottom: 1px solid %1; }"
                               "QFrame:hover { background-color: #141614; }"
                               ).arg(C_BORDER));
        auto *rowLayout = new QHBoxLayout(row);
        rowLayout->setContentsMargins(16, 12, 16, 12);

        auto *titleLabel = new QLabel(entry.title, row);
        titleLabel->setStyleSheet(QString("background: transparent; color: %1; font-size: 14px; font-weight: 500;").arg(C_TEXT_MAIN));
        rowLayout->addWidget(titleLabel, 3);

        auto *userLabel = new QLabel(entry.username, row);
        userLabel->setStyleSheet(QString("background: transparent; color: %1; font-size: 13px;").arg(C_TEXT_DIM));
        rowLayout->addWidget(userLabel, 3);

        auto *pwLabel = new QLabel(QString(qMax(entry.password.size(), 8), QChar(0x2022)), row);
        pwLabel->setStyleSheet(QString("background: transparent; color: %1; font-size: 13px; letter-spacing: 2px;").arg(C_TEXT_MUTED));
        rowLayout->addWidget(pwLabel, 2);

        auto *actionsRow = new QHBoxLayout();
        actionsRow->setSpacing(4);

        auto makeActionButton = [&](const QString &emoji) {
            auto *btn = new QPushButton(emoji, row);
            btn->setCursor(Qt::PointingHandCursor);
            btn->setFixedSize(28, 28);
            btn->setStyleSheet(QString(
                                   "QPushButton { background: transparent; border: none; border-radius: 6px; font-size: 13px; }"
                                   "QPushButton:hover { background-color: %1; }"
                                   ).arg(C_BORDER));
            return btn;
        };

        const QString id = entry.id;

        auto *copyBtn = makeActionButton("\U0001F4CB");
        connect(copyBtn, &QPushButton::clicked, this, [this, id]() { copyPassword(id); });
        actionsRow->addWidget(copyBtn);

        auto *editBtn = makeActionButton("\U0000270F");
        connect(editBtn, &QPushButton::clicked, this, [this, id]() { editEntry(id); });
        actionsRow->addWidget(editBtn);

        auto *deleteBtn = makeActionButton("\U0001F5D1");
        connect(deleteBtn, &QPushButton::clicked, this, [this, id]() { deleteEntry(id); });
        actionsRow->addWidget(deleteBtn);

        rowLayout->addLayout(actionsRow, 2);

        m_entriesLayout->addWidget(row);
    }
}

void VaultPage::onAddClicked()
{
    VaultEntryDialog dialog(VaultEntry(), this);
    if (dialog.exec() == QDialog::Accepted) {
        VaultEntry entry = dialog.result();
        if (entry.title.isEmpty()) {
            QMessageBox::warning(this, "Vault", "Please enter a title for this entry.");
            return;
        }
        m_repository.addEntry(entry);
        reloadEntries();
    }
}

void VaultPage::onSearchChanged(const QString &text)
{
    m_searchText = text;
    rebuildEntriesList();
}

void VaultPage::onCategoryCardClicked(const QString &category)
{
    m_activeCategory = (m_activeCategory == category) ? QString() : category;
    rebuildCategoryCards();
    rebuildEntriesList();
}

void VaultPage::onSortModeChanged(bool sortAlphabetically)
{
    m_sortAlphabetically = sortAlphabetically;

    auto activeStyle = QString("QPushButton { background: transparent; border: none; color: %1; font-size: 12px; font-weight: 700; padding: 4px 8px; }").arg(C_ACCENT_DIM);
    auto inactiveStyle = QString("QPushButton { background: transparent; border: none; color: %1; font-size: 12px; font-weight: 500; padding: 4px 8px; }").arg(C_TEXT_MUTED);

    m_sortAZButton->setStyleSheet(sortAlphabetically ? activeStyle : inactiveStyle);
    m_sortRecentButton->setStyleSheet(sortAlphabetically ? inactiveStyle : activeStyle);

    rebuildEntriesList();
}

void VaultPage::editEntry(const QString &id)
{
    auto it = std::find_if(m_entries.begin(), m_entries.end(),
                           [&id](const VaultEntry &e) { return e.id == id; });
    if (it == m_entries.end()) return;

    VaultEntryDialog dialog(*it, this);
    if (dialog.exec() == QDialog::Accepted) {
        m_repository.updateEntry(dialog.result());
        reloadEntries();
    }
}

void VaultPage::deleteEntry(const QString &id)
{
    auto it = std::find_if(m_entries.begin(), m_entries.end(),
                           [&id](const VaultEntry &e) { return e.id == id; });
    if (it == m_entries.end()) return;

    auto reply = QMessageBox::question(this, "Delete entry",
                                       QString("Delete \"%1\"? This cannot be undone.").arg(it->title));
    if (reply == QMessageBox::Yes) {
        m_repository.deleteEntry(id);
        reloadEntries();
    }
}

void VaultPage::copyPassword(const QString &id)
{
    auto it = std::find_if(m_entries.begin(), m_entries.end(),
                           [&id](const VaultEntry &e) { return e.id == id; });
    if (it == m_entries.end()) return;

    CliboardGuard::copyWithAutoClear(it->password, 15);
}