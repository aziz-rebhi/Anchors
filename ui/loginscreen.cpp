#include "loginscreen.h"
#include "../core/crypto/cryptomanager.h"
#include "../core/storage/FilePaths.h"
#include "../core/storage/encryptedfilestore.h"
#include "../core/storage/saltstore.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QProgressBar>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFrame>
#include <QGraphicsDropShadowEffect>
#include <QRegularExpression>

/* ── Colour Palette ─────────────────────────────────────────────── */
static const char *C_CARD_BG       = "#111111";
static const char *C_CARD_BORDER   = "#242424";
static const char *C_PRIMARY_GREEN = "#2E8B57";
static const char *C_PRIMARY_DARK  = "#0B3D0B";
static const char *C_TEXT_MAIN     = "#e1e3db";
static const char *C_TEXT_MUTED    = "#8c9386";
static const char *C_INPUT_BG      = "#0c0f0b";
static const char *C_INPUT_BORDER  = "#42493e";
static const char *C_ERROR_TEXT    = "#e55555";
static const char *C_ERROR_BG      = "rgba(229, 85, 85, 0.08)";
static const char *C_WARNING_TEXT  = "#c9a054";
static const char *C_WARNING_BG    = "rgba(201, 160, 84, 0.08)";

LoginScreen::LoginScreen(QWidget *parent)
    : QWidget{parent}, m_isFirstLaunch(!SaltStore::exists())
{
    setupUi();
}

void LoginScreen::setupUi()
{
    setStyleSheet("background-color: #000000;");
    setMinimumSize(560, 680);

    auto *rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);
    rootLayout->addStretch();

    buildCardUi(rootLayout);

    rootLayout->addStretch();
    buildGlobalFooter(rootLayout);
}

/* Reusable "alert" style helper — used for both error and warning
   messages, so both share the same visual language (icon-ish left
   accent bar + tinted background) instead of being plain colored text. */
static void styleAlertLabel(QLabel *label, const char *textColor, const char *bgColor)
{
    label->setStyleSheet(QString(R"(
        QLabel {
            color: %1;
            background-color: %2;
            border-left: 3px solid %1;
            border-radius: 6px;
            padding: 10px 12px;
            font-size: 12px;
        }
    )").arg(textColor, bgColor));
    label->setWordWrap(true);
}

/* ── Card UI ──────────────────────────────────────────────────── */
void LoginScreen::buildCardUi(QVBoxLayout *rootLayout)
{
    auto *card = new QFrame(this);
    card->setObjectName("loginCard");
    card->setStyleSheet(QString(R"(
        QFrame#loginCard {
            background-color: %1;
            border: 1px solid %2;
            border-radius: 20px;
        }
    )").arg(C_CARD_BG, C_CARD_BORDER));
    // Widened from 480 -> 540 max, 400 -> 440 min. This alone fixes most
    // of the secondary-action overflow, combined with shorter labels below.
    card->setMaximumWidth(540);
    card->setMinimumWidth(440);

    auto *shadow = new QGraphicsDropShadowEffect(card);
    shadow->setBlurRadius(60);
    shadow->setColor(QColor(46, 139, 87, 60));
    shadow->setOffset(0, 0);
    card->setGraphicsEffect(shadow);

    auto *cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(0, 0, 0, 0);
    cardLayout->setSpacing(0);

    /* ── Header Section ────────────────────────────────────────── */
    auto *headerLayout = new QVBoxLayout();
    headerLayout->setSpacing(12);
    // Extra bottom margin here (was 0) is what fixes the subtitle
    // clipping — the wrapped second line now has room to actually render
    // instead of being cut off against the form section below it.
    headerLayout->setContentsMargins(48, 48, 48, 16);
    headerLayout->setAlignment(Qt::AlignHCenter);

    m_lockIconLabel = new QLabel(this);
    m_lockIconLabel->setText("\U0001F512");
    m_lockIconLabel->setAlignment(Qt::AlignCenter);
    m_lockIconLabel->setStyleSheet(QString(R"(
        QLabel {
            font-size: 28px;
            background-color: %1;
            border-radius: 32px;
            border: 1px solid rgba(158, 212, 146, 0.2);
            min-width: 64px;
            max-width: 64px;
            min-height: 64px;
            max-height: 64px;
        }
    )").arg(C_PRIMARY_DARK));
    headerLayout->addWidget(m_lockIconLabel, 0, Qt::AlignHCenter);

    m_titleLabel = new QLabel("Anchor", this);
    m_titleLabel->setAlignment(Qt::AlignCenter);
    m_titleLabel->setStyleSheet(QString(R"(
        QLabel { background: transparent; color: %1; font-size: 28px; font-weight: 700; letter-spacing: -0.5px; }
    )").arg(C_TEXT_MAIN));
    headerLayout->addWidget(m_titleLabel);

    m_subtitleLabel = new QLabel(this);
    m_subtitleLabel->setText(m_isFirstLaunch
                                 ? "Create your master password to secure the vault"
                                 : "Unlock your encrypted workspace");
    m_subtitleLabel->setAlignment(Qt::AlignCenter);
    m_subtitleLabel->setWordWrap(true);
    // Explicit minimum height reserves space for two lines up front,
    // rather than letting the layout guess based on a single-line hint.
    m_subtitleLabel->setMinimumHeight(40);
    m_subtitleLabel->setStyleSheet(QString(R"(
        QLabel { background: transparent; color: %1; font-size: 14px; font-weight: 400; }
    )").arg(C_TEXT_MUTED));
    headerLayout->addWidget(m_subtitleLabel);

    cardLayout->addLayout(headerLayout);

    /* ── Form Section ─────────────────────────────────────────── */
    auto *formLayout = new QVBoxLayout();
    formLayout->setSpacing(14);
    formLayout->setContentsMargins(48, 8, 48, 32);

    QString fieldStyle = QString(R"(
        QLineEdit {
            background-color: %1;
            border: 1px solid %2;
            border-radius: 12px;
            color: %3;
            padding: 14px 16px;
            font-size: 15px;
        }
        QLineEdit:focus { border-color: %4; }
    )").arg(C_INPUT_BG, C_INPUT_BORDER, C_TEXT_MAIN, C_PRIMARY_GREEN);

    // Password row: key icon + field + visibility toggle
    auto *keyIcon = new QLabel("\U0001F511", this);
    keyIcon->setStyleSheet(QString("background: transparent; color: %1; font-size: 16px; padding: 0 12px;").arg(C_TEXT_MUTED));
    keyIcon->setAlignment(Qt::AlignCenter);

    m_passwordField = new QLineEdit(this);
    m_passwordField->setEchoMode(QLineEdit::Password);
    m_passwordField->setPlaceholderText("Master password");
    m_passwordField->setStyleSheet(fieldStyle);

    m_visibilityToggle = new QPushButton("\U0001F441", this);
    m_visibilityToggle->setCursor(Qt::PointingHandCursor);
    m_visibilityToggle->setStyleSheet(QString(R"(
        QPushButton { background: transparent; border: none; color: %1; font-size: 16px; padding: 0 12px; }
        QPushButton:hover { color: %2; }
    )").arg(C_TEXT_MUTED, C_TEXT_MAIN));
    m_visibilityToggle->setMaximumWidth(44);
    connect(m_visibilityToggle, &QPushButton::clicked, this, &LoginScreen::togglePasswordVisibility);

    auto *passwordContainer = new QFrame(this);
    passwordContainer->setStyleSheet(QString(R"(
        QFrame { background-color: %1; border: 1px solid %2; border-radius: 12px; }
    )").arg(C_INPUT_BG, C_INPUT_BORDER));
    auto *passwordContainerLayout = new QHBoxLayout(passwordContainer);
    passwordContainerLayout->setContentsMargins(0, 0, 0, 0);
    passwordContainerLayout->setSpacing(0);
    passwordContainerLayout->addWidget(keyIcon);
    passwordContainerLayout->addWidget(m_passwordField, 1);
    passwordContainerLayout->addWidget(m_visibilityToggle);
    passwordContainer->setFocusProxy(m_passwordField);

    formLayout->addWidget(passwordContainer);

    if (m_isFirstLaunch) {
        connect(m_passwordField, &QLineEdit::textChanged, this, &LoginScreen::updatePasswordStrength);

        // Password strength meter — only shown during setup, since it's
        // only meaningful when the user is choosing a new password, not
        // when re-entering an existing one.
        m_strengthBar = new QProgressBar(this);
        m_strengthBar->setRange(0, 4);
        m_strengthBar->setValue(0);
        m_strengthBar->setTextVisible(false);
        m_strengthBar->setFixedHeight(4);
        m_strengthBar->setStyleSheet(QString(R"(
            QProgressBar { background-color: %1; border: none; border-radius: 2px; }
            QProgressBar::chunk { background-color: %2; border-radius: 2px; }
        )").arg(C_INPUT_BORDER, C_PRIMARY_GREEN));
        formLayout->addWidget(m_strengthBar);

        m_strengthLabel = new QLabel("", this);
        m_strengthLabel->setStyleSheet(QString("background: transparent; color: %1; font-size: 11px;").arg(C_TEXT_MUTED));
        formLayout->addWidget(m_strengthLabel);

        m_confirmField = new QLineEdit(this);
        m_confirmField->setEchoMode(QLineEdit::Password);
        m_confirmField->setPlaceholderText("Confirm master password");
        m_confirmField->setStyleSheet(fieldStyle);
        formLayout->addWidget(m_confirmField);

        m_warningLabel = new QLabel(
            "There is no way to recover this password. If you lose it, "
            "your data cannot be decrypted — by anyone, including you.",
            this);
        styleAlertLabel(m_warningLabel, C_WARNING_TEXT, C_WARNING_BG);
        formLayout->addWidget(m_warningLabel);
    }

    m_errorLabel = new QLabel(this);
    styleAlertLabel(m_errorLabel, C_ERROR_TEXT, C_ERROR_BG);
    m_errorLabel->hide();
    formLayout->addWidget(m_errorLabel);

    m_submitButton = new QPushButton(m_isFirstLaunch ? "Create Vault" : "Unlock Workspace", this);
    m_submitButton->setCursor(Qt::PointingHandCursor);
    m_submitButton->setMinimumHeight(48);
    m_submitButton->setStyleSheet(QString(R"(
        QPushButton {
            background-color: %1;
            color: white;
            border: none;
            border-radius: 12px;
            font-size: 15px;
            font-weight: 600;
        }
        QPushButton:hover { background-color: %2; }
    )").arg(C_PRIMARY_GREEN, C_PRIMARY_DARK));
    formLayout->addWidget(m_submitButton);

    connect(m_submitButton, &QPushButton::clicked, this, &LoginScreen::onSubmit);
    connect(m_passwordField, &QLineEdit::returnPressed, this, &LoginScreen::onSubmit);
    if (m_confirmField) {
        connect(m_confirmField, &QLineEdit::returnPressed, this, &LoginScreen::onSubmit);
    }

    cardLayout->addLayout(formLayout);
    buildSecurityFooter(cardLayout);

    auto *centerLayout = new QHBoxLayout();
    centerLayout->addStretch();
    centerLayout->addWidget(card);
    centerLayout->addStretch();
    rootLayout->addLayout(centerLayout);
}

/* ── Security Footer ──────────────────────────────────────────── */
void LoginScreen::buildSecurityFooter(QVBoxLayout *cardLayout)
{
    auto *footerFrame = new QFrame(this);
    footerFrame->setStyleSheet(QString(R"(
        QFrame {
            background-color: rgba(25, 28, 24, 0.5);
            border-top: 1px solid %1;
            border-bottom-left-radius: 20px;
            border-bottom-right-radius: 20px;
        }
    )").arg(C_CARD_BORDER));

    auto *footerLayout = new QHBoxLayout(footerFrame);
    footerLayout->setContentsMargins(32, 20, 32, 20);
    footerLayout->setSpacing(16);

    auto *shieldIcon = new QLabel("\U0001F6E1", this);
    shieldIcon->setStyleSheet(QString(R"(
        QLabel {
            font-size: 20px;
            background-color: rgba(11, 61, 11, 0.2);
            border-radius: 8px;
            min-width: 32px; min-height: 32px; max-width: 32px; max-height: 32px;
        }
    )"));
    shieldIcon->setAlignment(Qt::AlignCenter);
    footerLayout->addWidget(shieldIcon);

    auto *gridLayout = new QGridLayout();
    gridLayout->setHorizontalSpacing(20);
    gridLayout->setVerticalSpacing(8);

    auto addFeature = [&](int row, int col, const QString &text) {
        auto *container = new QWidget(this);
        container->setStyleSheet("background: transparent;");
        auto *hLayout = new QHBoxLayout(container);
        hLayout->setContentsMargins(0, 0, 0, 0);
        hLayout->setSpacing(8);

        auto *dot = new QLabel(this);
        dot->setFixedSize(6, 6);
        dot->setStyleSheet("background-color: rgba(158, 212, 146, 0.4); border-radius: 3px;");
        hLayout->addWidget(dot, 0, Qt::AlignVCenter);

        auto *label = new QLabel(text, this);
        label->setStyleSheet(QString("background: transparent; color: %1; font-size: 12px;").arg(C_TEXT_MUTED));
        hLayout->addWidget(label);

        gridLayout->addWidget(container, row, col);
    };

    addFeature(0, 0, "Local encrypted storage");
    addFeature(0, 1, "Zero cloud sync");
    addFeature(1, 0, "Argon2id key derivation");
    addFeature(1, 1, "Device-only access");

    footerLayout->addLayout(gridLayout);
    cardLayout->addWidget(footerFrame);
}

/* ── Global Footer ────────────────────────────────────────────── */
void LoginScreen::buildGlobalFooter(QVBoxLayout *rootLayout)
{
    auto *footer = new QLabel(
        "Version 1.0    |    Offline Mode    |    Encrypted Database Ready", this);
    footer->setAlignment(Qt::AlignCenter);
    footer->setStyleSheet(R"(
        QLabel {
            background: transparent;
            color: rgba(140, 147, 134, 0.4);
            font-size: 11px;
            letter-spacing: 2px;
            padding: 16px;
        }
    )");
    rootLayout->addWidget(footer, 0, Qt::AlignBottom | Qt::AlignHCenter);
}

/* ── Password Strength ────────────────────────────────────────── */
void LoginScreen::updatePasswordStrength(const QString &text)
{
    if (!m_strengthBar) return;

    int score = 0;
    if (text.length() >= 8) score++;
    if (text.length() >= 12) score++;
    if (text.contains(QRegularExpression("[0-9]")) && text.contains(QRegularExpression("[A-Za-z]"))) score++;
    if (text.contains(QRegularExpression("[^A-Za-z0-9]"))) score++;

    m_strengthBar->setValue(score);

    static const char *labels[] = {"Too short", "Weak", "Okay", "Good", "Strong"};
    static const char *colors[] = {"#e55555", "#e55555", "#c9a054", "#4F9F4F", "#2E8B57"};

    QString chunkStyle = QString(R"(
        QProgressBar { background-color: %1; border: none; border-radius: 2px; }
        QProgressBar::chunk { background-color: %2; border-radius: 2px; }
    )").arg(C_INPUT_BORDER, colors[score]);
    m_strengthBar->setStyleSheet(chunkStyle);

    m_strengthLabel->setText(text.isEmpty() ? "" : labels[score]);
    m_strengthLabel->setStyleSheet(QString("background: transparent; color: %1; font-size: 11px;").arg(colors[score]));
}

/* ── Visibility Toggle ────────────────────────────────────────── */
void LoginScreen::togglePasswordVisibility()
{
    bool isPassword = (m_passwordField->echoMode() == QLineEdit::Password);
    m_passwordField->setEchoMode(isPassword ? QLineEdit::Normal : QLineEdit::Password);
    m_visibilityToggle->setText(isPassword ? "\U0001F576" : "\U0001F441");

    if (m_confirmField) {
        m_confirmField->setEchoMode(isPassword ? QLineEdit::Normal : QLineEdit::Password);
    }
}

/* ── Error Display ────────────────────────────────────────────── */
void LoginScreen::showError(const QString &message)
{
    m_errorLabel->setText(message);
    m_errorLabel->show();
}

/* ── Submit Handler ───────────────────────────────────────────── */
void LoginScreen::onSubmit()
{
    m_errorLabel->hide();

    const QString password = m_passwordField->text();

    if (password.isEmpty()) {
        showError("Please enter a master password");
        return;
    }

    if (m_isFirstLaunch) {
        const QString confirm = m_confirmField->text();

        if (password.length() < 8) {
            showError("Use at least 8 characters for your master password");
            return;
        }
        if (password != confirm) {
            showError("Passwords do not match");
            return;
        }
        if (!SaltStore::generateAndSave()) {
            showError("Could not set up your vault. Please try again.");
            return;
        }

        QByteArray salt = SaltStore::load();
        QByteArray key = CryptoManager::deriveKey(password, salt);

        QJsonObject canary;
        canary["check"] = "anchor-ok";
        QJsonDocument canaryDoc(canary);

        if (!EncryptedFileStore::save(FilePaths::verifyFile(), canaryDoc, key)) {
            showError("Could not finish vault setup. Please try again.");
            return;
        }

        emit unlocked(key);
        return;
    }

    QByteArray salt = SaltStore::load();
    if (salt.isEmpty()) {
        showError("Could not read vault setup data");
        return;
    }

    QByteArray key = CryptoManager::deriveKey(password, salt);

    bool verifyOk = false;
    EncryptedFileStore::load(FilePaths::verifyFile(), key, &verifyOk);

    if (!verifyOk) {
        showError("Incorrect master password");
        m_passwordField->clear();
        m_passwordField->setFocus();
        return;
    }
    emit unlocked(key);
}