#include "vaultentrydialog.h"
#include "../core/security/passwordgenerator.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTextEdit>
#include <QVBoxLayout>

static const char *C_BG          = "#111111";
static const char *C_TEXT_MAIN   = "#e1e3db";
static const char *C_TEXT_MUTED  = "#8c9386";
static const char *C_INPUT_BG    = "#0c0f0b";
static const char *C_INPUT_BORDER= "#42493e";
static const char *C_ACCENT      = "#2E8B57";
static const char *C_ACCENT_DARK = "#0B3D0B";

VaultEntryDialog::VaultEntryDialog(const VaultEntry &entry, QWidget *parent)
    : QDialog(parent), m_entry(entry)
{
    setupUi();
}

void VaultEntryDialog::setupUi()
{
    setWindowTitle(m_entry.id.isEmpty() ? "Add Password" : "Edit Password");
    setStyleSheet(QString("background-color: %1;").arg(C_BG));
    setMinimumWidth(420);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(28, 28, 28, 20);
    layout->setSpacing(12);

    QString fieldStyle = QString(R"(
        QLineEdit, QComboBox, QTextEdit {
            background-color: %1; border: 1px solid %2; border-radius: 10px;
            color: %3; padding: 10px 12px; font-size: 14px;
        }
        QLineEdit:focus, QComboBox:focus, QTextEdit:focus { border-color: %4; }
    )").arg(C_INPUT_BG, C_INPUT_BORDER, C_TEXT_MAIN, C_ACCENT);

    auto addLabel = [&](const QString &text) {
        auto *l = new QLabel(text, this);
        l->setStyleSheet(QString("background: transparent; color: %1; font-size: 12px; font-weight: 600;").arg(C_TEXT_MUTED));
        layout->addWidget(l);
    };

    addLabel("Title");
    m_titleField = new QLineEdit(m_entry.title, this);
    m_titleField->setPlaceholderText("e.g. ProtonMail");
    m_titleField->setStyleSheet(fieldStyle);
    layout->addWidget(m_titleField);

    addLabel("Username / Email");
    m_usernameField = new QLineEdit(m_entry.username, this);
    m_usernameField->setStyleSheet(fieldStyle);
    layout->addWidget(m_usernameField);

    addLabel("Password");
    auto *pwRow = new QHBoxLayout();
    m_passwordField = new QLineEdit(m_entry.password, this);
    m_passwordField->setEchoMode(QLineEdit::Password);
    m_passwordField->setStyleSheet(fieldStyle);
    pwRow->addWidget(m_passwordField, 1);

    m_visibilityToggle = new QPushButton("\U0001F441", this);
    m_visibilityToggle->setCursor(Qt::PointingHandCursor);
    m_visibilityToggle->setFixedWidth(40);
    m_visibilityToggle->setStyleSheet(QString(
                                          "QPushButton { background: transparent; border: 1px solid %1; border-radius: 10px; color: %2; }"
                                          ).arg(C_INPUT_BORDER, C_TEXT_MUTED));
    connect(m_visibilityToggle, &QPushButton::clicked, this, &VaultEntryDialog::onTogglePasswordVisibility);
    pwRow->addWidget(m_visibilityToggle);

    auto *generateBtn = new QPushButton("Generate", this);
    generateBtn->setCursor(Qt::PointingHandCursor);
    generateBtn->setStyleSheet(QString(R"(
        QPushButton { background-color: %1; color: white; border: none; border-radius: 10px; padding: 0 14px; font-size: 13px; font-weight: 600; }
        QPushButton:hover { background-color: %2; }
    )").arg(C_ACCENT, C_ACCENT_DARK));
    connect(generateBtn, &QPushButton::clicked, this, &VaultEntryDialog::onGenerateClicked);
    pwRow->addWidget(generateBtn);

    layout->addLayout(pwRow);

    addLabel("Website (optional)");
    m_urlField = new QLineEdit(m_entry.url, this);
    m_urlField->setStyleSheet(fieldStyle);
    layout->addWidget(m_urlField);

    addLabel("Category");
    m_categoryBox = new QComboBox(this);
    m_categoryBox->addItems({"Social", "Work", "Learning", "Finance", "Other"});
    int idx = m_categoryBox->findText(m_entry.category);
    m_categoryBox->setCurrentIndex(idx >= 0 ? idx : m_categoryBox->count() - 1);
    m_categoryBox->setStyleSheet(fieldStyle);
    layout->addWidget(m_categoryBox);



    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel, this);
    buttons->setStyleSheet(QString(R"(
        QPushButton {
            background-color: %1; color: white; border: none; border-radius: 10px;
            padding: 8px 18px; font-size: 13px; font-weight: 600;
        }
        QPushButton:hover { background-color: %2; }
    )").arg(C_ACCENT, C_ACCENT_DARK));
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addSpacing(8);
    layout->addWidget(buttons);
}

void VaultEntryDialog::onGenerateClicked()
{
    m_passwordField->setEchoMode(QLineEdit::Normal);
    m_visibilityToggle->setText("\U0001F576");
    m_passwordField->setText(PasswordGenerator::generate(16, true, true));
}

void VaultEntryDialog::onTogglePasswordVisibility()
{
    bool isPassword = (m_passwordField->echoMode() == QLineEdit::Password);
    m_passwordField->setEchoMode(isPassword ? QLineEdit::Normal : QLineEdit::Password);
    m_visibilityToggle->setText(isPassword ? "\U0001F576" : "\U0001F441");
}

VaultEntry VaultEntryDialog::result() const
{
    VaultEntry e = m_entry; // preserves id/createdAt if editing
    e.title = m_titleField->text().trimmed();
    e.username = m_usernameField->text().trimmed();
    e.password = m_passwordField->text();
    e.url = m_urlField->text().trimmed();
    e.category = m_categoryBox->currentText();
    return e;
}