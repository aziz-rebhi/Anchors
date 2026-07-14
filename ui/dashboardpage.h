#pragma once

#include <QWidget>
#include <QString>
#include <QVector>

class QFrame;
class QVBoxLayout;
class QLabel;

class DashboardPage : public QWidget
{
    Q_OBJECT

public:
    // Plain data structs — MainWindow (or later, real repositories) feed
    // these in via the setters below. Nothing here is hardcoded demo
    // content anymore; an empty vector correctly shows an empty state
    // instead of fake sample data.
    struct TaskItem {
        QString title;
        QString dueLabel;
    };

    struct ScheduleItem {
        QString time;
        QString title;
        QString detail;
        bool isNext = false;
    };

    explicit DashboardPage(QWidget *parent = nullptr);

    // Called once by MainWindow after loading the Profile, so the
    // greeting can say "Good evening, Aziz" instead of just "Today".
    // Pass an empty string to fall back to the name-less version.
    void setGreetingName(const QString &name);

    // Replaces the Priority Tasks list. Call with an empty vector to
    // show the empty state (correct behavior right now, since there's
    // no Tasks module yet to populate this from).
    void setTasks(const QVector<TaskItem> &tasks);

    // Same idea for the Schedule card.
    void setScheduleEvents(const QVector<ScheduleItem> &events);

signals:
    // MainWindow can connect this once a real "add task" flow exists.
    // For now it's just wired to the button with nothing listening yet.
    void addTaskRequested();

private:
    void setupUi();
    QFrame *createCard(const QString &iconEmoji, const QString &title,
                       QWidget *headerExtra, QVBoxLayout **outContentLayout);
    void rebuildTasksCard();
    void rebuildScheduleCard();
    static void clearLayout(QVBoxLayout *layout);

    QLabel *m_greetingLabel = nullptr;
    QVBoxLayout *m_tasksContent = nullptr;
    QVBoxLayout *m_scheduleContent = nullptr;

    QVector<TaskItem> m_tasks;
    QVector<ScheduleItem> m_scheduleEvents;
};