#include "vaultpage.h"
#include "vaultentrydialog.h"
#include "../core/security/cliboardguard.h"

#include <QFrame>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QListWidget>
#include <QMessageBox>
#include <algorithm>
#include <QDateTime>
#include <QHash>
#include <utility>   // for std::as_const

static const char *C_BG           = "#111410";
static const char *C_TEXT_MAIN    = "#E1E3DB";
static const char *C_TEXT_MUTED   = "#8C9386";
static const char *C_TEXT_DIM     = "#C1C9BB";
static const char *C_CARD_BG      = "#181818";
static const char *C_BORDER       = "#242424";
static const char *C_ACCENT       = "#2E8B57";
static const char *C_ACCENT_DARK  = "#0B3D0B";
static const char *C_ACCENT_DIM   = "#4F9F4F";
static const char *C_INPUT_BG     = "#111111";
static const char *C_HOVER_BG     = "#1A1D1A";
static const char *C_WEAK         = "#FF6B6B";
static const char *C_MEDIUM       = "#FFB347";
static const char *C_STRONG       = "#4F9F4F";

VaultPage::VaultPage(const QByteArray &sessionKey, QWidget *parent)
    : QWidget(parent), m_repository(sessionKey)
{
    // Initialize category metadata (non‑static)
    m_categoryMeta = {
        {"Social",   "\U0001F465", "Personal networks & media"},
        {"Work",     "\U0001F4BC", "Enterprise & productivity"},
        {"Learning", "\U0001F393", "Courses & educational"},
        {"Finance",  "\U0001F3E6", "Banking & crypto"}
    };

    setupUi();
    reloadEntries();
}

void VaultPage::setupUi()
{
    setStyleSheet(QString("background-color: %1;").arg(C_BG));

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(48, 40, 48, 40);
    mainLayout->setSpacing(32);

    // -------- HEADER --------
    auto *headerLayout = new QHBoxLayout();
    headerLayout->setSpacing(32);

    auto *titleLayout = new QVBoxLayout();
    titleLayout->setSpacing(4);

    auto *title = new QLabel("Vault", this);
    title->setStyleSheet(QString("background: transparent; color: %1; font-size: 40px; font-weight: 700; letter-spacing: -0.02em;").arg(C_TEXT_MAIN));
    titleLayout->addWidget(title);

    auto *subtitle = new QLabel("Securely manage your credentials.", this);
    subtitle->setStyleSheet(QString("background: transparent; color: %1; font-size: 16px; font-weight: 400;").arg(C_TEXT_MUTED));
    titleLayout->addWidget(subtitle);

    headerLayout->addLayout(titleLayout, 1);

    // Search
    m_searchField = new QLineEdit(this);
    m_searchField->setPlaceholderText("Search passwords, usernames...");
    m_searchField->setFixedWidth(320);
    m_searchField->setStyleSheet(QString(R"(
        QLineEdit {
            background-color: %1;
            border: 1px solid %2;
            border-radius: 12px;
            padding: 12px 16px 12px 44px;
            color: %3;
            font-size: 14px;
        }
        QLineEdit:focus {
            border-color: %4;
            background-color: %5;
        }
    )").arg(C_INPUT_BG, C_BORDER, C_TEXT_MAIN, C_ACCENT, C_CARD_BG));

    auto *searchContainer = new QHBoxLayout();
    searchContainer->setContentsMargins(0, 0, 0, 0);
    searchContainer->setSpacing(0);
    QLabel *searchIcon = new QLabel("🔍", this);
    searchIcon->setStyleSheet("background: transparent; color: #8C9386; font-size: 18px; padding-left: 16px;");
    searchContainer->addWidget(searchIcon);
    searchContainer->addWidget(m_searchField);

    QWidget *searchWidget = new QWidget();
    searchWidget->setLayout(searchContainer);
    searchWidget->setStyleSheet("background: transparent;");
    headerLayout->addWidget(searchWidget, 0, Qt::AlignTop);

    connect(m_searchField, &QLineEdit::textChanged, this, &VaultPage::onSearchChanged);
    mainLayout->addLayout(headerLayout);

    // -------- FILTERS --------
    auto *filterLayout = new QHBoxLayout();
    filterLayout->setSpacing(16);

    m_categoryCombo = new QComboBox(this);
    m_categoryCombo->setStyleSheet(QString(R"(
        QComboBox {
            background-color: %1;
            border: 1px solid %2;
            border-radius: 10px;
            padding: 8px 16px;
            color: %3;
            font-size: 13px;
            min-width: 140px;
        }
        QComboBox::drop-down { border: none; }
        QComboBox::down-arrow { image: none; }
        QComboBox QAbstractItemView {
            background-color: %1;
            border: 1px solid %2;
            selection-background-color: %4;
            color: %3;
        }
    )").arg(C_INPUT_BG, C_BORDER, C_TEXT_MAIN, C_ACCENT_DARK));
    m_categoryCombo->addItem("All Categories", "");
    connect(m_categoryCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &VaultPage::onCategoryFilterChanged);
    filterLayout->addWidget(m_categoryCombo);

    m_sortCombo = new QComboBox(this);
    m_sortCombo->setStyleSheet(m_categoryCombo->styleSheet());
    m_sortCombo->addItem("Recent", false);
    m_sortCombo->addItem("A–Z", true);
    connect(m_sortCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &VaultPage::onSortModeChanged);
    filterLayout->addWidget(m_sortCombo);

    filterLayout->addStretch();
    mainLayout->addLayout(filterLayout);

    // -------- STATISTICS (no border) --------
    m_statsLayout = new QGridLayout();
    m_statsLayout->setSpacing(16);
    m_statsLayout->setContentsMargins(0, 0, 0, 0);

    const QStringList icons = {"🔑", "🔓", "🔄", "⚠️"};
    const QStringList labels = {"Passwords", "Weak", "Reused", "Compromised"};
    for (int i = 0; i < 4; ++i) {
        auto *card = new QFrame(this);
        card->setStyleSheet(QString(R"(
            QFrame {
                background-color: %1;
                border: none;
                border-radius: 16px;
                padding: 16px;
            }
        )").arg(C_CARD_BG));

        auto *layout = new QHBoxLayout(card);
        layout->setContentsMargins(16, 12, 16, 12);
        layout->setSpacing(12);

        QLabel *iconLabel = new QLabel(icons[i], card);
        iconLabel->setStyleSheet(QString("background: transparent; color: %1; font-size: 24px;").arg(C_ACCENT_DIM));
        layout->addWidget(iconLabel);

        auto *rightLayout = new QVBoxLayout();
        rightLayout->setSpacing(2);

        QLabel *valueLabel = new QLabel("0", card);
        valueLabel->setStyleSheet(QString("background: transparent; color: %1; font-size: 22px; font-weight: 700;").arg(C_TEXT_MAIN));
        rightLayout->addWidget(valueLabel);
        m_statValueLabels.append(valueLabel);

        QLabel *nameLabel = new QLabel(labels[i], card);
        nameLabel->setStyleSheet(QString("background: transparent; color: %1; font-size: 13px; font-weight: 400;").arg(C_TEXT_MUTED));
        rightLayout->addWidget(nameLabel);

        layout->addLayout(rightLayout, 1);
        m_statsLayout->addWidget(card, 0, i);
    }
    mainLayout->addLayout(m_statsLayout);

    // -------- CATEGORIES --------
    auto *categoryLabel = new QLabel("Categories", this);
    categoryLabel->setStyleSheet(QString("background: transparent; color: %1; font-size: 18px; font-weight: 700; margin-top: 8px;").arg(C_TEXT_MAIN));
    mainLayout->addWidget(categoryLabel);

    m_categoriesLayout = new QHBoxLayout();
    m_categoriesLayout->setSpacing(16);
    createCategoryCards();
    mainLayout->addLayout(m_categoriesLayout);

    // -------- ENTRIES LIST (QListWidget) --------
    auto *entriesHeader = new QLabel("All Entries", this);
    entriesHeader->setStyleSheet(QString("background: transparent; color: %1; font-size: 18px; font-weight: 700; margin-top: 16px;").arg(C_TEXT_MAIN));
    mainLayout->addWidget(entriesHeader);

    m_entriesList = new QListWidget(this);
    m_entriesList->setStyleSheet(R"(
        QListWidget {
            background: transparent;
            border: none;
            outline: none;
        }
        QListWidget::item {
            background: transparent;
            border: none;
        }
        QListWidget::item:selected {
            background: transparent;
        }
        QListWidget::item:hover {
            background: transparent;
        }
    )");
    m_entriesList->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    mainLayout->addWidget(m_entriesList, 1);

    // -------- FLOATING ADD BUTTON --------
    auto *fabLayout = new QHBoxLayout();
    fabLayout->addStretch();
    auto *addButton = new QPushButton("+  Add Password", this);
    addButton->setCursor(Qt::PointingHandCursor);
    addButton->setStyleSheet(QString(R"(
        QPushButton {
            background-color: %1;
            color: white;
            border: none;
            border-radius: 9999px;
            padding: 16px 36px;
            font-size: 16px;
            font-weight: 600;
            box-shadow: 0 8px 24px rgba(0,0,0,0.6);
        }
        QPushButton:hover { background-color: %2; transform: scale(1.02); }
    )").arg(C_ACCENT, C_ACCENT_DARK));
    connect(addButton, &QPushButton::clicked, this, &VaultPage::onAddClicked);
    fabLayout->addWidget(addButton);
    mainLayout->addLayout(fabLayout);
}

// -------- CREATE CATEGORY BUTTONS --------
void VaultPage::createCategoryCards()
{
    for (const CategoryMeta &meta : std::as_const(m_categoryMeta)) {
        auto *btn = new QPushButton(this);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setObjectName(meta.name);
        btn->setMinimumWidth(120);
        btn->setMinimumHeight(72);
        btn->setFlat(true);

        auto *layout = new QVBoxLayout(btn);
        layout->setContentsMargins(12, 10, 12, 10);
        layout->setSpacing(4);

        auto *topRow = new QHBoxLayout();
        QLabel *iconLabel = new QLabel(meta.icon, btn);
        iconLabel->setStyleSheet(QString("background: transparent; color: %1; font-size: 16px;").arg(C_ACCENT_DIM));
        topRow->addWidget(iconLabel);
        topRow->addStretch();

        QLabel *countLabel = new QLabel("0", btn);
        countLabel->setStyleSheet(QString("background: transparent; color: %1; font-size: 16px; font-weight: 700;").arg(C_TEXT_MAIN));
        topRow->addWidget(countLabel);
        layout->addLayout(topRow);

        QLabel *nameLabel = new QLabel(meta.name, btn);
        nameLabel->setStyleSheet(QString("background: transparent; color: %1; font-size: 14px; font-weight: 600;").arg(C_TEXT_MAIN));
        layout->addWidget(nameLabel);

        QLabel *subLabel = new QLabel(meta.subtitle, btn);
        subLabel->setStyleSheet(QString("background: transparent; color: %1; font-size: 10px;").arg(C_TEXT_MUTED));
        subLabel->setWordWrap(true);
        layout->addWidget(subLabel);

        m_categoryButtons.append(btn);
        m_categoryCountLabels.append(countLabel);
        m_categoryNames.append(meta.name);

        connect(btn, &QPushButton::clicked, this, [this, meta]() {
            onCategoryClicked(meta.name);
        });

        m_categoriesLayout->addWidget(btn);
    }
}

// -------- UPDATE STATISTICS --------
void VaultPage::updateStatistics()
{
    int total = m_entries.size();
    int weak = 0, reused = 0, compromised = 0;

    QHash<QString, int> passCount;
    for (const VaultEntry &e : std::as_const(m_entries)) {
        if (computeStrength(e.password) == "Weak")
            weak++;
        passCount[e.password]++;
    }
    for (auto it = passCount.constBegin(); it != passCount.constEnd(); ++it) {
        if (it.value() > 1)
            reused += it.value();
    }

    const QList<int> values = {total, weak, reused, compromised};
    for (int i = 0; i < m_statValueLabels.size(); ++i) {
        m_statValueLabels[i]->setText(QString::number(values[i]));
    }
}

// -------- REBUILD CATEGORY CARDS --------
void VaultPage::rebuildCategoryCards()
{
    for (int i = 0; i < m_categoryButtons.size(); ++i) {
        const QString &name = m_categoryNames[i];
        bool isActive = (m_activeCategory == name);

        int count = std::count_if(m_entries.begin(), m_entries.end(),
                                  [&name](const VaultEntry &e) { return e.category == name; });
        m_categoryCountLabels[i]->setText(QString::number(count));

        QPushButton *btn = m_categoryButtons[i];
        QString style = QString(R"(
            QPushButton {
                background-color: %1;
                border: 1px solid %2;
                border-radius: 16px;
                padding: 12px 16px;
                min-width: 120px;
                min-height: 72px;
                text-align: left;
            }
            QPushButton:hover {
                background-color: %3;
                border-color: %4;
            }
        )").arg(C_CARD_BG, isActive ? C_ACCENT : C_BORDER, C_HOVER_BG, C_ACCENT);
        btn->setStyleSheet(style);
    }
}

// -------- REBUILD ENTRIES LIST --------
void VaultPage::rebuildEntriesList()
{
    m_entriesList->clear();

    QVector<VaultEntry> filtered;
    for (const VaultEntry &e : std::as_const(m_entries)) {
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
        auto *emptyItem = new QListWidgetItem(m_entriesList);
        emptyItem->setSizeHint(QSize(200, 200));

        auto *emptyWidget = new QWidget(m_entriesList);
        auto *emptyLayout = new QVBoxLayout(emptyWidget);
        emptyLayout->setAlignment(Qt::AlignCenter);
        emptyLayout->setSpacing(16);

        QLabel *iconLabel = new QLabel("🔐", emptyWidget);
        iconLabel->setStyleSheet("background: transparent; color: #4F9F4F; font-size: 64px;");
        iconLabel->setAlignment(Qt::AlignCenter);
        emptyLayout->addWidget(iconLabel);

        QLabel *titleLabel = new QLabel(
            m_entries.isEmpty() ? "No Passwords Yet" : "No Matches Found",
            emptyWidget);
        titleLabel->setStyleSheet(QString("background: transparent; color: %1; font-size: 24px; font-weight: 700;").arg(C_TEXT_MAIN));
        titleLabel->setAlignment(Qt::AlignCenter);
        emptyLayout->addWidget(titleLabel);

        QLabel *descLabel = new QLabel(
            m_entries.isEmpty() ? "Click the \"Add Password\" button to save your first credential." : "Try adjusting your search or category filter.",
            emptyWidget);
        descLabel->setStyleSheet(QString("background: transparent; color: %1; font-size: 16px;").arg(C_TEXT_MUTED));
        descLabel->setAlignment(Qt::AlignCenter);
        emptyLayout->addWidget(descLabel);

        m_entriesList->setItemWidget(emptyItem, emptyWidget);
        return;
    }

    for (const VaultEntry &entry : std::as_const(filtered)) {
        auto *item = new QListWidgetItem(m_entriesList);
        item->setSizeHint(QSize(0, 80));

        auto *row = new QFrame(m_entriesList);
        row->setStyleSheet(QString(R"(
            QFrame {
                background: transparent;
                border: none;
                border-radius: 16px;
                margin: 2px 0;
            }
            QFrame:hover {
                background-color: %1;
            }
        )").arg(C_HOVER_BG));

        auto *rowLayout = new QHBoxLayout(row);
        rowLayout->setContentsMargins(16, 12, 16, 12);
        rowLayout->setSpacing(16);

        QLabel *iconLabel = new QLabel(entry.title.isEmpty() ? "●" : QString(entry.title[0]).toUpper(), row);
        iconLabel->setFixedSize(40, 40);
        iconLabel->setAlignment(Qt::AlignCenter);
        iconLabel->setStyleSheet(QString(R"(
            background-color: %1;
            color: %2;
            border-radius: 10px;
            font-size: 18px;
            font-weight: 700;
        )").arg(C_ACCENT_DARK, C_TEXT_MAIN));
        rowLayout->addWidget(iconLabel);

        auto *infoLayout = new QVBoxLayout();
        infoLayout->setSpacing(2);

        QLabel *titleLabel = new QLabel(entry.title, row);
        titleLabel->setStyleSheet(QString("background: transparent; color: %1; font-size: 16px; font-weight: 600;").arg(C_TEXT_MAIN));
        infoLayout->addWidget(titleLabel);

        QHBoxLayout *subInfo = new QHBoxLayout();
        subInfo->setSpacing(12);
        QLabel *userLabel = new QLabel(entry.username, row);
        userLabel->setStyleSheet(QString("background: transparent; color: %1; font-size: 13px;").arg(C_TEXT_DIM));
        subInfo->addWidget(userLabel);

        if (!entry.url.isEmpty()) {
            QLabel *urlLabel = new QLabel(entry.url, row);
            urlLabel->setStyleSheet(QString("background: transparent; color: %1; font-size: 12px;").arg(C_TEXT_MUTED));
            subInfo->addWidget(urlLabel);
        }

        QDateTime dt = QDateTime::fromSecsSinceEpoch(entry.updatedAt);
        QLabel *dateLabel = new QLabel(dt.toString("MMM d, yyyy"), row);
        dateLabel->setStyleSheet(QString("background: transparent; color: %1; font-size: 11px;").arg(C_TEXT_MUTED));
        subInfo->addWidget(dateLabel);

        infoLayout->addLayout(subInfo);
        rowLayout->addLayout(infoLayout, 1);

        QLabel *pwLabel = new QLabel(QString(qMax(entry.password.size(), 8), QChar(0x2022)), row);
        pwLabel->setStyleSheet(QString("background: transparent; color: %1; font-size: 14px; letter-spacing: 2px;").arg(C_TEXT_MUTED));
        rowLayout->addWidget(pwLabel);

        QString strength = computeStrength(entry.password);
        QString color = (strength == "Strong") ? C_STRONG : (strength == "Medium" ? C_MEDIUM : C_WEAK);
        QLabel *strengthLabel = new QLabel(strength, row);
        strengthLabel->setFixedWidth(70);
        strengthLabel->setAlignment(Qt::AlignCenter);
        strengthLabel->setStyleSheet(QString(R"(
            background-color: %1;
            color: white;
            border-radius: 12px;
            padding: 2px 8px;
            font-size: 11px;
            font-weight: 600;
        )").arg(color));
        rowLayout->addWidget(strengthLabel);

        auto *actionsLayout = new QHBoxLayout();
        actionsLayout->setSpacing(4);

        auto makeActionButton = [&](const QString &symbol, const QString &hoverColor) {
            auto *btn = new QPushButton(symbol, row);
            btn->setCursor(Qt::PointingHandCursor);
            btn->setFixedSize(32, 32);
            btn->setStyleSheet(QString(R"(
                QPushButton {
                    background: transparent;
                    border: none;
                    border-radius: 8px;
                    font-size: 16px;
                    color: %1;
                }
                QPushButton:hover {
                    background-color: %2;
                    color: %3;
                }
            )").arg(C_TEXT_MUTED, C_HOVER_BG, hoverColor));
            return btn;
        };

        const QString id = entry.id;

        auto *copyBtn = makeActionButton("⎘", C_ACCENT);
        connect(copyBtn, &QPushButton::clicked, this, [this, id]() { copyPassword(id); });
        actionsLayout->addWidget(copyBtn);

        auto *editBtn = makeActionButton("✎", C_ACCENT_DIM);
        connect(editBtn, &QPushButton::clicked, this, [this, id]() { editEntry(id); });
        actionsLayout->addWidget(editBtn);

        auto *deleteBtn = makeActionButton("✕", "#FF6B6B");
        connect(deleteBtn, &QPushButton::clicked, this, [this, id]() { deleteEntry(id); });
        actionsLayout->addWidget(deleteBtn);

        rowLayout->addLayout(actionsLayout);

        m_entriesList->setItemWidget(item, row);
    }
}

// -------- STRENGTH --------
QString VaultPage::computeStrength(const QString &password) const
{
    int len = password.length();
    bool hasUpper = false, hasLower = false, hasDigit = false, hasSymbol = false;
    for (QChar ch : password) {
        if (ch.isUpper()) hasUpper = true;
        else if (ch.isLower()) hasLower = true;
        else if (ch.isDigit()) hasDigit = true;
        else hasSymbol = true;
    }

    int score = 0;
    if (len >= 12) score += 2;
    else if (len >= 8) score += 1;
    if (hasUpper && hasLower) score += 1;
    if (hasDigit) score += 1;
    if (hasSymbol) score += 1;

    if (score >= 4) return "Strong";
    if (score >= 2) return "Medium";
    return "Weak";
}

// -------- RELOAD --------
void VaultPage::reloadEntries()
{
    bool ok = false;
    m_entries = m_repository.loadAll(&ok);
    if (!ok) {
        QMessageBox::warning(this, "Vault",
                             "Could not load your vault entries. The session key may be invalid.");
        m_entries.clear();
    }
    updateStatistics();
    rebuildCategoryCards();
    rebuildEntriesList();

    // Update dropdown
    m_categoryCombo->clear();
    m_categoryCombo->addItem("All Categories", "");
    QStringList categories;
    for (const VaultEntry &e : std::as_const(m_entries)) {
        if (!categories.contains(e.category))
            categories.append(e.category);
    }
    std::sort(categories.begin(), categories.end());
    for (const QString &cat : std::as_const(categories)) {
        m_categoryCombo->addItem(cat, cat);
    }
    int idx = m_categoryCombo->findData(m_activeCategory);
    if (idx >= 0) m_categoryCombo->setCurrentIndex(idx);
    else m_categoryCombo->setCurrentIndex(0);
}

// -------- SLOTS --------
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

void VaultPage::onCategoryClicked(const QString &category)
{
    m_activeCategory = (m_activeCategory == category) ? QString() : category;
    int idx = m_categoryCombo->findData(m_activeCategory);
    if (idx >= 0) m_categoryCombo->setCurrentIndex(idx);
    else m_categoryCombo->setCurrentIndex(0);
    rebuildCategoryCards();
    rebuildEntriesList();
}

void VaultPage::onCategoryFilterChanged(int index)
{
    QString category = m_categoryCombo->itemData(index).toString();
    m_activeCategory = category;
    rebuildCategoryCards();
    rebuildEntriesList();
}

void VaultPage::onSortModeChanged(int index)
{
    bool alpha = m_sortCombo->itemData(index).toBool();
    m_sortAlphabetically = alpha;
    rebuildEntriesList();
}

// -------- EDIT / DELETE / COPY --------
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