#ifndef VAULTENTRYDIALOG_H
#define VAULTENTRYDIALOG_H

#pragma once

#include "../core/models/vaultentry.h"

#include <QDialog>

class QLineEdit;
class QComboBox;
class QTextEdit;
class QPushButton;

class VaultEntryDialog : public QDialog
{
    Q_OBJECT

public:
    // Pass an existing entry to edit it, or a default-constructed one
    // (empty id) to create a new entry.
    explicit VaultEntryDialog(const VaultEntry &entry, QWidget *parent = nullptr);

    VaultEntry result() const;

private slots:
    void onGenerateClicked();
    void onTogglePasswordVisibility();

private:
    void setupUi();

    VaultEntry m_entry; // seed data; id preserved on save if editing

    QLineEdit *m_titleField = nullptr;
    QLineEdit *m_usernameField = nullptr;
    QLineEdit *m_passwordField = nullptr;
    QPushButton *m_visibilityToggle = nullptr;
    QLineEdit *m_urlField = nullptr;
    QComboBox *m_categoryBox = nullptr;
};
#endif // VAULTENTRYDIALOG_H
