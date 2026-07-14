#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ui/dashboardpage.h"
#include "ui/vaultpage.h"
#include "app/session.h"
#include <QLabel>
#include <QIcon>
#include <QButtonGroup>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , isExpanded(true)
{
    ui->setupUi(this);

    // ---- Navigation connections ----
    connect(ui->btnDashboard, &QPushButton::clicked, this, &MainWindow::switchToPage1);
    connect(ui->btnVault,     &QPushButton::clicked, this, &MainWindow::switchToPage2);
    connect(ui->btnNotes,     &QPushButton::clicked, this, &MainWindow::switchToPage3);
    connect(ui->btnCalendar,  &QPushButton::clicked, this, &MainWindow::switchToPage4);
    connect(ui->btnTodo,      &QPushButton::clicked, this, &MainWindow::switchToPage5);

    // --------DashboardPage --------
    DashboardPage *dashboard = new DashboardPage(this);
    ui -> stackedWidget -> removeWidget(ui ->pageDashboard);
    ui -> stackedWidget -> insertWidget(0, dashboard);
    delete ui -> pageDashboard;
    ui -> pageDashboard = dashboard;

    // ---------VaultPage -----------
    QByteArray sessionKey = Session::instance() -> sessionKey();
    VaultPage *vaultPage = new VaultPage(sessionKey, this);
    ui -> stackedWidget -> removeWidget (ui -> pageVault);
    ui -> stackedWidget -> insertWidget(1, vaultPage);
    delete  ui -> pageVault;
    ui -> pageVault = vaultPage;

    // ---------FrirstPage -----------
    ui -> stackedWidget -> setCurrentIndex(0);


    // ------Groupe button ----------
    QButtonGroup *navGroup = new QButtonGroup(this);
    navGroup -> setExclusive(true);

    navGroup -> addButton( ui -> btnDashboard);
    navGroup -> addButton( ui -> btnVault);
    navGroup -> addButton( ui -> btnNotes);
    navGroup -> addButton( ui -> btnCalendar);
    navGroup -> addButton( ui -> btnTodo);
    navGroup -> addButton( ui -> btnSettings);
    navGroup -> addButton( ui -> btnSupport);

    ui -> btnDashboard -> setChecked(true);

    // ---- Toggle button setup ----
    ui->leftPanelClose->setCheckable(true);
    QIcon toggleIcon;
    toggleIcon.addFile(":/src/assets/left_panel_open.svg", QSize(), QIcon::Normal, QIcon::Off);
    toggleIcon.addFile(":/src/assets/left_panel_close.svg",  QSize(), QIcon::Normal, QIcon::On);
    ui->leftPanelClose->setIcon(toggleIcon);
    ui->leftPanelClose->setChecked(true);   // expanded initially
    connect(ui->leftPanelClose, &QPushButton::clicked, this, &MainWindow::toggleMenu);

    // ---- Sidebar initial setup ----
    // We'll animate maximumWidth, so set minimumWidth to 0 to avoid constraints
    ui->sideBar->setMinimumWidth(0);
    ui->sideBar->setMaximumWidth(EXPANDED_WIDTH);
    ui->sideBar->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);

    // ---- Animation ----
    animation = new QPropertyAnimation(ui->sideBar, "maximumWidth", this);
    animation->setDuration(400);                     // slightly longer for smoothness
    animation->setEasingCurve(QEasingCurve::InOutCubic);  // cleaner than elastic

    // ---- Store original button texts (for restoring later) ----
    const auto buttons = ui->sideBar->findChildren<QPushButton*>();
    for (QPushButton *btn : buttons) {
        originalButtonTexts[btn] = btn->text();
    }

    // ---- Show/hide button texts and labels based on width ----
    connect(animation, &QPropertyAnimation::valueChanged, [this](const QVariant &value) {
        int width = value.toInt();
        bool showText = (width > 120);   // threshold: show text when width > 120

        // Toggle button texts (for all push buttons in the sidebar)
        const auto btns = ui->sideBar->findChildren<QPushButton*>();
        for (QPushButton *btn : btns) {
            if (showText) {
                btn->setText(originalButtonTexts.value(btn));
                btn -> setToolTip(originalButtonTexts.value(btn));
            } else {
                btn->setText(QString());   // clear text, keep icon
            }
        }

        // Toggle QLabel visibility (if you have any)
        const auto labels = ui->sideBar->findChildren<QLabel*>();
        for (QLabel *label : labels) {
            if ( label == ui -> avatarLabel){
                label -> setVisible(true);
                continue;
            }
            label->setVisible(showText);
        }
    });


}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::toggleMenu()
{
    animation->stop();

    int startWidth = ui->sideBar->width();       // current actual width
    int endWidth   = isExpanded ? COLLAPSED_WIDTH : EXPANDED_WIDTH;

    animation->setStartValue(startWidth);
    animation->setEndValue(endWidth);
    animation->start();

    isExpanded = !isExpanded;
    ui->leftPanelClose->setChecked(isExpanded);
}

// ---- Page switching slots ----
void MainWindow::switchToPage1() { ui->stackedWidget->setCurrentWidget(ui->pageDashboard); }
void MainWindow::switchToPage2() { ui->stackedWidget->setCurrentWidget(ui->pageVault); }
void MainWindow::switchToPage3() { ui->stackedWidget->setCurrentWidget(ui->pageNotes); }
void MainWindow::switchToPage4() { ui->stackedWidget->setCurrentWidget(ui->pageCalendar); }
void MainWindow::switchToPage5() { ui->stackedWidget->setCurrentWidget(ui->pageTodo); }