#ifndef DASHBOARDCONTROLLER_H
#define DASHBOARDCONTROLLER_H

#include <QObject>
#include <QVariantList>

// Simple controller that provides dashboard data to QML.
// Later you can replace static data with repository calls.
class DashboardController : public QObject
{
    Q_OBJECT

    // Expose data as QVariantList so QML can consume it directly
    Q_PROPERTY(QVariantList tasks READ tasks NOTIFY dataChanged)
    Q_PROPERTY(QVariantList schedule READ schedule NOTIFY dataChanged)
    Q_PROPERTY(int urgentCount READ urgentCount NOTIFY dataChanged)

public:
    explicit DashboardController(QObject *parent = nullptr);

    QVariantList tasks() const;
    QVariantList schedule() const;
    int urgentCount() const;

signals:
    void dataChanged();

    // Called from QML when a task is toggled
    Q_INVOKABLE void toggleTask(int index);
    Q_INVOKABLE void addTask(const QString &title);
    Q_INVOKABLE void updateScratchpad(const QString &text);

private:
    struct Task {
        QString title;
        QString due;
        bool done;
    };
    struct Event {
        QString time;
        QString title;
        QString detail;
        QString icon;
        bool active;
    };

    QList<Task> m_tasks;
    QList<Event> m_events;
    QString m_scratchpad;

    void loadData();
    void notifyChanged();
};

#endif // DASHBOARDCONTROLLER_H