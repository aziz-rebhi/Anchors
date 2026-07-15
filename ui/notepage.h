#ifndef NOTEPAGE_H
#define NOTEPAGE_H

#pragma once

#include "../core/models/noteentry.h"
#include "../core/storage/repositories/noterepository.h"

#include <QWidget>

class QListWidget;
class QTextEdit;
class QLineEdit;
class QPushButton;
class QLabel;
class QTimer;

class NotePage : public QWidget
{
    Q_OBJECT

public:
    explicit NotePage(const QByteArray &sessionKey, QWidget *parent = nullptr);

private slots:
    void onNewNote();
    void onDeleteNote();
    void onNoteSelected(int index);
    void onContentChanged();
    void onSearchTextChanged(const QString &text);

    // Toolbar formatting actions
    void onBoldClicked();
    void onItalicClicked();
    void onUnderlineClicked();
    void onBulletListClicked();
    void onNumberedListClicked();
    void onLinkClicked();

private:
    void setupUi();
    void loadNotes();
    void saveCurrentNote();
    void clearEditor();
    void updateNoteList(const QString &filter = QString());
    void setStatus(const QString &text, bool isSaved);
    static QString snippetFor(const QString &markdownContent);

    NoteRepository m_repository;
    QVector<NoteEntry> m_notes;

    QListWidget *m_noteList = nullptr;
    QTextEdit *m_editor = nullptr;
    QLineEdit *m_searchField = nullptr;
    QPushButton *m_newButton = nullptr;
    QPushButton *m_deleteButton = nullptr;
    QLabel *m_statusLabel = nullptr;
    QTimer *m_saveTimer = nullptr;

    QString m_currentNoteId;
    bool m_ignoreContentChange = false;
};

#endif // NOTEPAGE_H