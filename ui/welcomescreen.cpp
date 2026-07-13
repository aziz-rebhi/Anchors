#include "welcomescreen.h"

#include <QDateEdit>
#include <QFrame>
#include <QGraphicsDropShadowEffect>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

/* Same palette as LoginScreen. Duplicated here rather than shared from
   a common header for now — worth factoring into a shared Theme.h once
   a third screen needs these same constants, but not worth the
   refactor for just two screens yet. */
static const char *C_CARD_BG       = "#111111";
static const char *C_CARD_BORDER   = "#242424";
static const char *C_PRIMARY_GREEN = "#2E8B57";
static const char *C_PRIMARY_DARK  = "#0B3D0B";
static const char *C_TEXT_MAIN     = "#e1e3db";
static const char *C_TEXT_MUTED    = "#8c9386";
static const char *C_INPUT_BG      = "#0c0f0b";
static const char *C_INPUT_BORDER  = "#42493e";

Welcomescreen::Welcomescreen(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
}

void Welcomescreen::setupUi()
{
    setStyleSheet("background-color: #000000;");
    setMinimumSize(560, 560);

    auto *rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->addStretch();

    auto *card = new QFrame(this);
    card->setObjectName("welcomeCard");
    card->setStyleSheet(QString(R"(
        QFrame#welcomeCard {
            background-color: %1;
            border: 1px solid %2;
            border-radius: 20px;
        }
    )").arg(C_CARD_BG, C_CARD_BORDER));
    card->setMaximumWidth(480);
    card->setMinimumWidth(420);

    auto *shadow = new QGraphicsDropShadowEffect(card);
    shadow->setBlurRadius(60);
    shadow->setColor(QColor(46, 139, 87, 60));
    shadow->setOffset(0, 0);
    card->setGraphicsEffect(shadow);

    auto *cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(48, 48, 48, 40);
    cardLayout->setSpacing(16);

    m_titleLabel = new QLabel("Welcome to Anchor", this);
    m_titleLabel->setAlignment(Qt::AlignCenter);
    m_titleLabel->setStyleSheet(QString(
                                    "background: transparent; color: %1; font-size: 24px; font-weight: 700;"
                                    ).arg(C_TEXT_MAIN));
    cardLayout->addWidget(m_titleLabel);

    m_subtitleLabel = new QLabel(
        "Tell us a little about yourself. Both fields are optional — "
        "you can skip this entirely.", this);
    m_subtitleLabel->setAlignment(Qt::AlignCenter);
    m_subtitleLabel->setWordWrap(true);
    m_subtitleLabel->setMinimumHeight(40);
    m_subtitleLabel->setStyleSheet(QString(
                                       "background: transparent; color: %1; font-size: 13px;"
                                       ).arg(C_TEXT_MUTED));
    cardLayout->addWidget(m_subtitleLabel);

    cardLayout->addSpacing(8);

    QString fieldStyle = QString(R"(
        QLineEdit, QDateEdit {
            background-color: %1;
            border: 1px solid %2;
            border-radius: 12px;
            color: %3;
            padding: 14px 16px;
            font-size: 15px;
        }
        QLineEdit:focus, QDateEdit:focus { border-color: %4; }
    )").arg(C_INPUT_BG, C_INPUT_BORDER, C_TEXT_MAIN, C_PRIMARY_GREEN);

    m_nameField = new QLineEdit(this);
    m_nameField->setPlaceholderText("Your name (optional)");
    m_nameField->setStyleSheet(fieldStyle);
    cardLayout->addWidget(m_nameField);

    m_birthdayField = new QDateEdit(this);
    m_birthdayField->setCalendarPopup(true);
    m_birthdayField->setDisplayFormat("MMMM d, yyyy");
    m_birthdayField->setStyleSheet(fieldStyle);
    // Using the minimum date as a sentinel for "not set" — showing
    // special text instead of a real date until the user actually
    // picks one, so we're not defaulting them to some arbitrary date
    // that looks like a real answer.
    m_birthdayField->setMinimumDate(QDate(1900, 1, 1));
    m_birthdayField->setMaximumDate(QDate::currentDate());
    m_birthdayField->setSpecialValueText("Your birthday (optional)");
    m_birthdayField->setDate(m_birthdayField->minimumDate());
    cardLayout->addWidget(m_birthdayField);

    cardLayout->addSpacing(8);

    m_continueButton = new QPushButton("Continue", this);
    m_continueButton->setCursor(Qt::PointingHandCursor);
    m_continueButton->setMinimumHeight(48);
    m_continueButton->setStyleSheet(QString(R"(
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
    cardLayout->addWidget(m_continueButton);

    m_skipButton = new QPushButton("Skip for now", this);
    m_skipButton->setCursor(Qt::PointingHandCursor);
    m_skipButton->setStyleSheet(QString(R"(
        QPushButton { background: transparent; border: none; color: %1; font-size: 13px; padding: 8px; }
        QPushButton:hover { color: %2; }
    )").arg(C_TEXT_MUTED, C_TEXT_MAIN));
    cardLayout->addWidget(m_skipButton, 0, Qt::AlignHCenter);

    connect(m_continueButton, &QPushButton::clicked, this, &Welcomescreen::onContinueClicked);
    connect(m_skipButton, &QPushButton::clicked, this, &Welcomescreen::onSkipClicked);

    auto *centerLayout = new QHBoxLayout();
    centerLayout->addStretch();
    centerLayout->addWidget(card);
    centerLayout->addStretch();
    rootLayout->addLayout(centerLayout);
    rootLayout->addStretch();
}

void Welcomescreen::onContinueClicked()
{
    const QString name = m_nameField->text().trimmed();

    // If the date is still sitting on the sentinel minimum, treat it as
    // "not set" rather than a real birthday of January 1st, 1900.
    const QDate birthday = (m_birthdayField->date() == m_birthdayField->minimumDate())
                               ? QDate()
                               : m_birthdayField->date();

    emit continueRequested(name, birthday);
}

void Welcomescreen::onSkipClicked()
{
    emit continueRequested(QString(), QDate());
}