#ifndef VAULTPAGE_H
#define VAULTPAGE_H

#pragma once

#include "../vault/vaultentry.h"
#include "../vault/vaultrepository.h"

#include <QWidget>
#include <QList>

class QLineEdit;
class QVBoxLayout;
class QHBoxLayout;
class QPushButton;
class QFrame;
class QComboBox;
class QGridLayout;
class QLabel;
class QListWidget;

struct CategoryMeta { QString name; QString icon; QString subtitle; };

class VaultPage : public QWidget
{
    Q_OBJECT

public:
    explicit VaultPage(const QByteArray &sessionKey, QWidget *parent = nullptr);

private slots:
    void onAddClicked();
    void onSearchChanged(const QString &text);
    void onCategoryClicked(const QString &category);
    void onSortModeChanged(int index);
    void onCategoryFilterChanged(int index);

private:
    void setupUi();
    void reloadEntries();
    void rebuildCategoryCards();
    void rebuildEntriesList();
    void updateStatistics();
    void createCategoryCards();
    void editEntry(const QString &id);
    void deleteEntry(const QString &id);
    void copyPassword(const QString &id);
    QString computeStrength(const QString &password) const;

    VaultRepository m_repository;
    QVector<VaultEntry> m_entries;

    QString m_activeCategory;
    QString m_searchText;
    bool m_sortAlphabetically = false;

    // Category metadata (non‑static)
    QList<CategoryMeta> m_categoryMeta;

    // UI elements
    QHBoxLayout *m_categoriesLayout = nullptr;
    QListWidget *m_entriesList = nullptr;
    QLineEdit *m_searchField = nullptr;
    QComboBox *m_categoryCombo = nullptr;
    QComboBox *m_sortCombo = nullptr;
    QGridLayout *m_statsLayout = nullptr;

    // Stat cards
    QList<QLabel*> m_statValueLabels;

    // Category buttons
    QList<QPushButton*> m_categoryButtons;
    QList<QLabel*> m_categoryCountLabels;
    QList<QString> m_categoryNames;
};

#endif // VAULTPAGE_H