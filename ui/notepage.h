#ifndef NOTEPAGE_H
#define NOTEPAGE_H

#pragma once

#include "../core/models/noteentry.h"
#include "../core/storage/repositories/noterepository.h"

#include <QWidget>
#include <QSet>

class QListWidget;
class QTextEdit;
class QLineEdit;
class QPushButton;
class QLabel;
class QTimer;
class QComboBox;
class QSplitter;
class QToolButton;
class QHBoxLayout;

class NotePage : public QWidget
{
    Q_OBJECT

public:
    explicit NotePage(const QByteArray &sessionKey, QWidget *parent = nullptr);

private slots:
    // ---- core actions ----
    void onNewNote();
    void onDeleteNote();
    void onNoteSelected(int index);
    void onContentChanged();
    void onSearchTextChanged(const QString &text);
    void onTitleChanged();
    void onNoteListContextMenu(const QPoint &pos);
    void onTogglePinned(const QString &noteId);

    // ---- toolbar formatting ----
    void onUndoClicked();
    void onRedoClicked();
    void onHeadingChanged(int index);
    void onBoldClicked();
    void onItalicClicked();
    void onUnderlineClicked();
    void onStrikeClicked();
    void onInlineCodeClicked();
    void onBulletListClicked();
    void onNumberedListClicked();
    void onChecklistClicked();
    void onToggleListClicked();
    void onQuoteClicked();
    void onCalloutClicked();
    void onCodeBlockClicked();
    void onDividerClicked();
    void onTableClicked();
    void onImageClicked();
    void onLinkClicked();
    void onFileClicked();
    void onMathClicked();
    void onDiagramClicked();
    void onTextColorClicked();
    void onHighlightClicked();
    void onReadingModeToggled(bool enabled);
    void onFocusModeToggled(bool enabled);
    void onExportPdf();

private:
    void setupUi();
    void loadNotes();
    void saveCurrentNote();
    void clearEditor();
    void updateNoteList(const QString &filter = QString());
    void updateStatusBar();
    void setStatus(const QString &text, bool isSaved);
    void showPlaceholder(bool show);
    void switchToEditMode(const QString &noteId = QString());
    static QString snippetFor(const QString &markdownContent);

    NoteRepository m_repository;
    QVector<NoteEntry> m_notes;
    QSet<QString> m_pinnedNotes;

    // ---- UI elements ----
    QSplitter *m_mainSplitter = nullptr;
    QWidget *m_leftPanel = nullptr;
    QWidget *m_centerPanel = nullptr;
    QWidget *m_statusBarWidget = nullptr;

    // Left panel
    QComboBox *m_notebookSelector = nullptr;
    QLineEdit *m_searchField = nullptr;
    QPushButton *m_newNoteBtn = nullptr;
    QToolButton *m_sortBtn = nullptr;
    QToolButton *m_filterBtn = nullptr;
    QListWidget *m_noteList = nullptr;

    // Center panel
    QWidget *m_topWidget = nullptr;
    QLineEdit *m_titleEdit = nullptr;
    QLabel *m_metaLabel = nullptr;
    QWidget *m_toolbarWidget = nullptr;
    QHBoxLayout *m_toolbarLayout = nullptr;
    QTextEdit *m_editor = nullptr;
    QWidget *m_placeholderContainer = nullptr;
    QComboBox *m_headingCombo = nullptr;

    // Status bar
    QLabel *m_wordCountLabel = nullptr;
    QLabel *m_charCountLabel = nullptr;
    QLabel *m_readingTimeLabel = nullptr;
    QLabel *m_cursorPosLabel = nullptr;
    QLabel *m_zoomLabel = nullptr;
    QLabel *m_saveStatusLabel = nullptr;

    // State
    bool m_readingMode = false;
    bool m_focusMode = false;

    QString m_currentNoteId;
    bool m_ignoreContentChange = false;
    bool m_ignoreTitleChange = false;
    bool m_isSaving = false;
    QTimer *m_saveTimer = nullptr;
};

#endif // NOTEPAGE_H