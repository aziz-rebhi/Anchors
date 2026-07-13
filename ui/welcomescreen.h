#ifndef WELCOMESCREEN_H
#define WELCOMESCREEN_H
#pragma once

#include <QDate>
#include <QWidget>

class QLabel;
class QLineEdit;
class QDateEdit;
class QPushButton;


class Welcomescreen : public QWidget
{
    Q_OBJECT

public :
    explicit Welcomescreen(QWidget *parent = nullptr);

signals:
    void continueRequested(const QString &name, const QDate &birthday);

private slots:
    void onContinueClicked();
    void onSkipClicked();

private:
    void setupUi();

    QLabel *m_titleLabel = nullptr;
    QLabel *m_subtitleLabel = nullptr;
    QLineEdit *m_nameField = nullptr;
    QDateEdit *m_birthdayField = nullptr;
    QPushButton *m_continueButton = nullptr;
    QPushButton *m_skipButton = nullptr;
};

#endif // WELCOMESCREEN_H
