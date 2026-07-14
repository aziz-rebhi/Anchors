#ifndef VAULTPAGE_H
#define VAULTPAGE_H

#pragma once

#include "../vault/vaultentry.h"
#include "../vault/vaultrepository.h"

#include <QWidget>

class QLineEdit;
class QVBoxLayout;
class QHBoxLayout;
class QPushButton;
class QFrame;

class VaultPage : public QWidget
{
    Q_OBJECT

public:
    explicit VaultPage(const QByteArray &sessionKey, QWidget *parent = nullptr);

private slots:
    void onAddClicked();
    void onSearchChanged(const QString &text);
    void onCategoryCardClicked(const QString &category);
    void onSortModeChanged(bool sortAlphabetically);

private:
    void setupUi();
    void reloadEntries();
    void rebuildCategoryCards();
    void rebuildEntriesList();
    QFrame *createCategoryCard(const QString &icon, const QString &name,
                               const QString &subtitle, int count);
    void editEntry(const QString &id);
    void deleteEntry(const QString &id);
    void copyPassword(const QString &id);

    VaultRepository m_repository;
    QVector<VaultEntry> m_entries;

    QString m_activeCategory; // empty = no filter
    QString m_searchText;
    bool m_sortAlphabetically = false; // false = most recent first

    QHBoxLayout *m_categoriesLayout = nullptr;
    QVBoxLayout *m_entriesLayout = nullptr;
    QLineEdit *m_searchField = nullptr;
    QPushButton *m_sortAZButton = nullptr;
    QPushButton *m_sortRecentButton = nullptr;
};
#endif // VAULTPAGE_H
