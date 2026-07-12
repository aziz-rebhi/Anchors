#ifndef LOGINSCREEN_H
#define LOGINSCREEN_H

#pragma once

#include <QByteArray>
#include <QWidget>

class QLabel;
class QLineEdit;
class QPushButton;
class QVBoxLayout;
class QProgressBar;

class LoginScreen : public QWidget
{
    Q_OBJECT
public:
    explicit LoginScreen(QWidget *parent = nullptr);

signals:
    void unlocked(const QByteArray &sessionKey);

private slots:
    void onSubmit();
    void togglePasswordVisibility();
    void updatePasswordStrength(const QString &text);

private:
    void setupUi();
    void showError(const QString &message);
    void buildCardUi(QVBoxLayout *parentLayout);
    void buildSecurityFooter(QVBoxLayout *parentLayout);
    void buildGlobalFooter(QVBoxLayout *parentLayout);

    bool m_isFirstLaunch;

    QLabel *m_lockIconLabel = nullptr;
    QLabel *m_titleLabel = nullptr;
    QLabel *m_subtitleLabel = nullptr;
    QLineEdit *m_passwordField = nullptr;
    QLineEdit *m_confirmField = nullptr;
    QPushButton *m_visibilityToggle = nullptr;
    QProgressBar *m_strengthBar = nullptr;
    QLabel *m_strengthLabel = nullptr;
    QLabel *m_warningLabel = nullptr;
    QLabel *m_errorLabel = nullptr;
    QPushButton *m_submitButton = nullptr;
};

#endif // LOGINSCREEN_H