#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPropertyAnimation>
#include <QHash>
#include <QPushButton>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void toggleMenu();
    void switchToPage1();
    void switchToPage2();
    void switchToPage3();
    void switchToPage4();
    void switchToPage5();

private:
    Ui::MainWindow *ui;
    QPropertyAnimation *animation;
    bool isExpanded;                     // true = expanded, false = collapsed

    // Constants for sidebar widths
    static constexpr int COLLAPSED_WIDTH = 90;
    static constexpr int EXPANDED_WIDTH  = 350;

    // Store original button texts for later restoration
    QHash<QPushButton*, QString> originalButtonTexts;
};

#endif // MAINWINDOW_H