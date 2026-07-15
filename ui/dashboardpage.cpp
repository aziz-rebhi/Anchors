#include "dashboardpage.h"

#include <QDateTime>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTextEdit>
#include <QTime>
#include <QVBoxLayout>

static const char *C_BG_CARD    = "#181818";
static const char *C_BORDER     = "#242424";
static const char *C_TEXT_MAIN  = "#e1e3db";
static const char *C_TEXT_MUTED = "#8c9386";
static const char *C_TEXT_DIM   = "#c1c9bb";
static const char *C_ACCENT     = "#9ed492";
static const char *C_PILL_BG    = "#111111";

DashboardPage::DashboardPage(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
    rebuildTasksCard();
    rebuildScheduleCard();
}

void DashboardPage::clearLayout(QVBoxLayout *layout)
{
    if (!layout) return;
    QLayoutItem *item;
    while ((item = layout->takeAt(0)) != nullptr) {
        if (QWidget *w = item->widget()) {
            w->deleteLater();
        }
        delete item;
    }
}

QFrame *DashboardPage::createCard(const QString &iconEmoji, const QString &title,
                                  QWidget *headerExtra, QVBoxLayout **outContentLayout)
{
    auto *card = new QFrame(this);
    // No border – only background and rounded corners
    card->setStyleSheet(QString(R"(
        QFrame { background-color: %1; border: none; border-radius: 16px; }
    )").arg(C_BG_CARD));

    auto *outer = new QVBoxLayout(card);
    outer->setContentsMargins(20, 20, 20, 20);
    outer->setSpacing(16);

    auto *header = new QHBoxLayout();
    header->setSpacing(10);

    auto *icon = new QLabel(iconEmoji, card);
    icon->setStyleSheet("background: transparent; font-size: 18px;");
    header->addWidget(icon);

    auto *titleLabel = new QLabel(title, card);
    titleLabel->setStyleSheet(QString(
                                  "background: transparent; color: %1; font-size: 20px; font-weight: 600;"
                                  ).arg(C_TEXT_MAIN));
    header->addWidget(titleLabel);

    header->addStretch();

    if (headerExtra) {
        header->addWidget(headerExtra);
    }

    outer->addLayout(header);

    auto *contentWidget = new QWidget(card);
    contentWidget->setStyleSheet("background: transparent;");
    auto *contentLayout = new QVBoxLayout(contentWidget);
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(8);

    outer->addWidget(contentWidget, 1);

    *outContentLayout = contentLayout;
    return card;
}

void DashboardPage::setupUi()
{
    setStyleSheet("background-color: #000000;");

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(40, 24, 40, 40);
    mainLayout->setSpacing(24);

    auto *headerLayout = new QHBoxLayout();
    headerLayout->setSpacing(16);

    auto *titleLayout = new QVBoxLayout();
    titleLayout->setSpacing(2);

    int hour = QTime::currentTime().hour();
    QString overview;
    if (hour < 12) overview = "MORNING OVERVIEW";
    else if (hour < 18) overview = "AFTERNOON OVERVIEW";
    else overview = "EVENING OVERVIEW";

    auto *overviewLabel = new QLabel(overview, this);
    overviewLabel->setStyleSheet(QString(
                                     "background: transparent; font-size: 11px; letter-spacing: 0.15em; color: %1;"
                                     ).arg(C_TEXT_MUTED));
    titleLayout->addWidget(overviewLabel);

    m_greetingLabel = new QLabel("Today", this);
    m_greetingLabel->setStyleSheet(QString(
                                       "background: transparent; font-size: 42px; font-weight: 700; letter-spacing: -0.02em; color: %1;"
                                       ).arg(C_TEXT_MAIN));
    titleLayout->addWidget(m_greetingLabel);

    headerLayout->addLayout(titleLayout, 1);

    auto *statusRow = new QHBoxLayout();
    statusRow->setSpacing(8);
    auto *statusDot = new QFrame(this);
    statusDot->setFixedSize(8, 8);
    statusDot->setStyleSheet(QString("background-color: %1; border-radius: 4px;").arg(C_ACCENT));
    statusRow->addWidget(statusDot);
    auto *statusLabel = new QLabel("Vault Unlocked", this);
    statusLabel->setStyleSheet(QString("background: transparent; color: %1; font-size: 12px; font-weight: 500;").arg(C_TEXT_DIM));
    statusRow->addWidget(statusLabel);

    auto *statusContainer = new QFrame(this);
    statusContainer->setLayout(statusRow);
    statusContainer->setStyleSheet(QString(
                                       "background-color: %1; border: 1px solid %2; border-radius: 9999px; padding: 4px 14px;"
                                       ).arg(C_PILL_BG, C_BORDER));
    headerLayout->addWidget(statusContainer, 0, Qt::AlignTop | Qt::AlignRight);

    mainLayout->addLayout(headerLayout);

    auto *grid = new QGridLayout();
    grid->setSpacing(16);
    grid->setColumnStretch(0, 3);
    grid->setColumnStretch(1, 2);

    auto *addTaskBtn = new QPushButton("Add Task", this);
    addTaskBtn->setCursor(Qt::PointingHandCursor);
    addTaskBtn->setStyleSheet(QString(R"(
        QPushButton {
            background-color: #0B3D0B; color: %1; border: none;
            border-radius: 9999px; padding: 6px 16px; font-size: 12px; font-weight: 600;
        }
        QPushButton:hover { background-color: #145214; }
    )").arg(C_ACCENT));
    connect(addTaskBtn, &QPushButton::clicked, this, &DashboardPage::addTaskRequested);

    QFrame *tasksCard = createCard("\u2705", "Priority Tasks", addTaskBtn, &m_tasksContent);
    grid->addWidget(tasksCard, 0, 0, 1, 2);

    QFrame *scheduleCard = createCard("\U0001F4C5", "Schedule", nullptr, &m_scheduleContent);
    grid->addWidget(scheduleCard, 1, 0, 1, 1);

    QVBoxLayout *scratchContent = nullptr;
    QFrame *scratchCard = createCard("\U0001F4DD", "Scratchpad", nullptr, &scratchContent);
    auto *textEdit = new QTextEdit(scratchCard);
    textEdit->setPlaceholderText("Capture thoughts quickly...");
    textEdit->setStyleSheet(QString(R"(
        QTextEdit {
            background-color: %1; border: 1px solid %2; border-radius: 8px;
            color: %3; font-size: 14px; padding: 10px;
        }
        QTextEdit:focus { border-color: %4; }
    )").arg(C_PILL_BG, C_BORDER, C_TEXT_MAIN, C_ACCENT));
    scratchContent->addWidget(textEdit);
    grid->addWidget(scratchCard, 1, 1, 1, 1);

    mainLayout->addLayout(grid);
    mainLayout->addStretch();
}

void DashboardPage::setGreetingName(const QString &name)
{
    int hour = QTime::currentTime().hour();
    QString greeting = (hour < 12) ? "Good morning" : (hour < 18) ? "Good afternoon" : "Good evening";

    m_greetingLabel->setText(name.isEmpty() ? "Today" : QString("%1, %2").arg(greeting, name));
}

void DashboardPage::setTasks(const QVector<TaskItem> &tasks)
{
    m_tasks = tasks;
    rebuildTasksCard();
}

void DashboardPage::setScheduleEvents(const QVector<ScheduleItem> &events)
{
    m_scheduleEvents = events;
    rebuildScheduleCard();
}

void DashboardPage::rebuildTasksCard()
{
    clearLayout(m_tasksContent);

    if (m_tasks.isEmpty()) {
        auto *empty = new QLabel(
            "No tasks yet. Once the To-Do module is built, they'll show up here.", this);
        empty->setWordWrap(true);
        empty->setStyleSheet(QString("background: transparent; color: %1; font-size: 13px;").arg(C_TEXT_MUTED));
        m_tasksContent->addWidget(empty);
        return;
    }

    for (const TaskItem &task : m_tasks) {
        auto *row = new QFrame(this);
        row->setStyleSheet(QString(
                               "QFrame { background: transparent; border-left: 3px solid %1; border-radius: 6px; padding-left: 10px; }"
                               ).arg(C_ACCENT));

        auto *rowLayout = new QHBoxLayout(row);
        rowLayout->setContentsMargins(8, 6, 8, 6);
        rowLayout->setSpacing(10);

        auto *checkbox = new QFrame(row);
        checkbox->setFixedSize(18, 18);
        checkbox->setStyleSheet("background: transparent; border: 2px solid #42493e; border-radius: 4px;");
        rowLayout->addWidget(checkbox);

        auto *titleLabel = new QLabel(task.title, row);
        titleLabel->setStyleSheet(QString("background: transparent; color: %1; font-size: 14px;").arg(C_TEXT_MAIN));
        rowLayout->addWidget(titleLabel, 1);

        auto *dueLabel = new QLabel(task.dueLabel, row);
        dueLabel->setStyleSheet(QString("background: transparent; color: %1; font-size: 11px;").arg(C_TEXT_MUTED));
        rowLayout->addWidget(dueLabel);

        m_tasksContent->addWidget(row);
    }
    m_tasksContent->addStretch();
}

void DashboardPage::rebuildScheduleCard()
{
    clearLayout(m_scheduleContent);

    if (m_scheduleEvents.isEmpty()) {
        auto *empty = new QLabel(
            "No events scheduled. Once the Calendar module is built, they'll show up here.", this);
        empty->setWordWrap(true);
        empty->setStyleSheet(QString("background: transparent; color: %1; font-size: 13px;").arg(C_TEXT_MUTED));
        m_scheduleContent->addWidget(empty);
        return;
    }

    for (const ScheduleItem &ev : m_scheduleEvents) {
        auto *row = new QWidget(this);
        row->setStyleSheet("background: transparent;");
        auto *rowLayout = new QHBoxLayout(row);
        rowLayout->setContentsMargins(0, 0, 0, 0);
        rowLayout->setSpacing(12);

        auto *timeLabel = new QLabel(ev.time, row);
        timeLabel->setFixedWidth(48);
        timeLabel->setStyleSheet(QString("background: transparent; color: %1; font-size: 12px; font-weight: 500;").arg(C_TEXT_DIM));
        rowLayout->addWidget(timeLabel);

        auto *dot = new QFrame(row);
        dot->setFixedSize(8, 8);
        dot->setStyleSheet(QString("background-color: %1; border-radius: 4px;")
                               .arg(ev.isNext ? C_ACCENT : C_TEXT_MUTED));
        rowLayout->addWidget(dot, 0, Qt::AlignTop);

        auto *textLayout = new QVBoxLayout();
        textLayout->setSpacing(2);
        auto *titleLabel = new QLabel(ev.title, row);
        titleLabel->setStyleSheet(QString("background: transparent; color: %1; font-size: 15px; font-weight: 500;")
                                      .arg(ev.isNext ? C_TEXT_MAIN : C_TEXT_MUTED));
        textLayout->addWidget(titleLabel);

        if (!ev.detail.isEmpty()) {
            auto *detailLabel = new QLabel(ev.detail, row);
            detailLabel->setStyleSheet(QString("background: transparent; color: %1; font-size: 12px;").arg(C_TEXT_MUTED));
            textLayout->addWidget(detailLabel);
        }
        rowLayout->addLayout(textLayout, 1);

        m_scheduleContent->addWidget(row);
    }
    m_scheduleContent->addStretch();
}