import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Page {
    id: root

    Theme { id: theme }

    property var allTasks: []

    function refresh() {
        allTasks = taskController.entries()
    }

    Component.onCompleted: refresh()

    Connections {
        target: taskController
        function onEntriesChanged() { root.refresh() }
    }

    function startOfDay(d) {
        var x = new Date(d)
        x.setHours(0, 0, 0, 0)
        return x
    }

    readonly property date todayStart: startOfDay(new Date())
    readonly property date tomorrowStart: {
        var d = new Date(todayStart)
        d.setDate(d.getDate() + 1)
        return d
    }

    readonly property var openTasks: allTasks.filter(function (t) { return !t.done })
    readonly property var completedTasks: allTasks.filter(function (t) { return t.done })

    readonly property var todayTasks: openTasks.filter(function (t) {
        return t.dueAt === 0 || (t.dueAt * 1000 >= todayStart.getTime() && t.dueAt * 1000 < tomorrowStart.getTime())
    }).sort(function (a, b) { return (a.dueAt || Infinity) - (b.dueAt || Infinity) })

    readonly property var upcomingTasks: openTasks.filter(function (t) {
        return t.dueAt > 0 && t.dueAt * 1000 >= tomorrowStart.getTime()
    }).sort(function (a, b) { return a.dueAt - b.dueAt })

    background: Rectangle { color: theme.background }

    ScrollView {
        anchors.fill: parent
        contentWidth: availableWidth
        clip: true

        ColumnLayout {
            width: parent.width
            spacing: theme.spacingLarge
            anchors.margins: 24

            RowLayout {
                Layout.fillWidth: true
                Layout.margins: 24

                Label {
                    text: "To-Do"
                    color: theme.textPrimary
                    font.family: theme.headlineFont
                    font.pixelSize: 30
                    font.bold: true
                }
                Item { Layout.fillWidth: true }
                Rectangle {
                    radius: theme.radiusPill
                    color: theme.surfaceAlt
                    Layout.preferredHeight: 28
                    Layout.preferredWidth: pillLabel.implicitWidth + 24
                    Label {
                        id: pillLabel
                        anchors.centerIn: parent
                        text: root.openTasks.length + " Active Tasks"
                        color: theme.textSecondary
                        font.pixelSize: 11
                    }
                }
            }

            // --- Add task row ---
            RowLayout {
                Layout.fillWidth: true
                Layout.leftMargin: 24
                Layout.rightMargin: 24
                spacing: 8

                StyledTextField {
                    id: newTaskField
                    Layout.fillWidth: true
                    placeholderText: "Add a new task..."
                    onAccepted: addButton.clicked()
                }
                PrimaryButton {
                    id: addButton
                    text: "Create  \u2192"
                    onClicked: {
                        var title = newTaskField.text.trim()
                        if (title.length === 0) return
                        taskController.addEntry(title, 0)
                        newTaskField.text = ""
                    }
                }
            }

            RowLayout {
                Layout.fillWidth: true
                Layout.leftMargin: 24
                Layout.rightMargin: 24
                spacing: 16

                // --- Today ---
                Rectangle {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    Layout.minimumHeight: 260
                    radius: theme.radiusMedium
                    color: theme.surfaceAlt
                    border.width: 1
                    border.color: theme.border

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 16
                        spacing: 10

                        RowLayout {
                            Layout.fillWidth: true
                            Label { text: "Today"; color: theme.textPrimary; font.family: theme.headlineFont; font.pixelSize: 16; font.bold: true }
                            Item { Layout.fillWidth: true }
                            Label { text: root.todayTasks.length + " Remaining"; color: theme.textMuted; font.pixelSize: 11 }
                        }

                        Repeater {
                            model: root.todayTasks
                            delegate: RowLayout {
                                Layout.fillWidth: true
                                spacing: 10

                                Rectangle {
                                    width: 18; height: 18; radius: 4
                                    border.width: 1
                                    border.color: theme.neutral
                                    color: "transparent"
                                    MouseArea {
                                        anchors.fill: parent
                                        onClicked: taskController.setDone(modelData.id, true)
                                    }
                                }
                                Label {
                                    text: modelData.title
                                    color: theme.textPrimary
                                    font.pixelSize: 14
                                    Layout.fillWidth: true
                                    elide: Text.ElideRight
                                }
                                Label {
                                    text: modelData.dueAt > 0 ? Qt.formatDateTime(new Date(modelData.dueAt * 1000), "h:mm AP") : ""
                                    color: theme.textMuted
                                    font.pixelSize: 11
                                }
                                Rectangle {
                                    width: 6; height: 6; radius: 3
                                    color: theme.tertiary
                                }
                            }
                        }

                        Label {
                            text: "Nothing due today."
                            visible: root.todayTasks.length === 0
                            color: theme.textMuted
                            font.pixelSize: 13
                        }

                        Item { Layout.fillHeight: true }
                    }
                }

                // --- Upcoming + Completed ---
                ColumnLayout {
                    Layout.preferredWidth: 260
                    spacing: 16

                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: upcomingCol.implicitHeight + 32
                        radius: theme.radiusMedium
                        color: theme.surfaceAlt
                        border.width: 1
                        border.color: theme.border

                        ColumnLayout {
                            id: upcomingCol
                            anchors.fill: parent
                            anchors.margins: 16
                            spacing: 8

                            RowLayout {
                                Layout.fillWidth: true
                                Label { text: "Upcoming"; color: theme.textPrimary; font.family: theme.headlineFont; font.pixelSize: 15; font.bold: true }
                                Item { Layout.fillWidth: true }
                            }

                            Repeater {
                                model: root.upcomingTasks.slice(0, 5)
                                delegate: RowLayout {
                                    Layout.fillWidth: true
                                    spacing: 8
                                    Rectangle {
                                        width: 16; height: 16; radius: 4
                                        border.width: 1
                                        border.color: theme.neutral
                                        color: "transparent"
                                        MouseArea { anchors.fill: parent; onClicked: taskController.setDone(modelData.id, true) }
                                    }
                                    ColumnLayout {
                                        spacing: 0
                                        Layout.fillWidth: true
                                        Label { text: modelData.title; color: theme.textPrimary; font.pixelSize: 13; elide: Text.ElideRight; Layout.fillWidth: true }
                                        Label {
                                            text: Qt.formatDate(new Date(modelData.dueAt * 1000), "MMM d")
                                            color: theme.textMuted
                                            font.pixelSize: 10
                                        }
                                    }
                                    Rectangle { width: 6; height: 6; radius: 3; color: theme.tertiary }
                                }
                            }

                            Label {
                                text: "Nothing scheduled."
                                visible: root.upcomingTasks.length === 0
                                color: theme.textMuted
                                font.pixelSize: 12
                            }
                        }
                    }

                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: completedCol.implicitHeight + 32
                        radius: theme.radiusMedium
                        color: theme.surfaceAlt
                        border.width: 1
                        border.color: theme.border

                        ColumnLayout {
                            id: completedCol
                            anchors.fill: parent
                            anchors.margins: 16
                            spacing: 8

                            Label { text: "Completed"; color: theme.textPrimary; font.family: theme.headlineFont; font.pixelSize: 15; font.bold: true }

                            Repeater {
                                model: root.completedTasks.slice(0, 5)
                                delegate: RowLayout {
                                    Layout.fillWidth: true
                                    spacing: 8
                                    Rectangle {
                                        width: 16; height: 16; radius: 4
                                        color: theme.tertiary
                                        Label { anchors.centerIn: parent; text: "\u2713"; color: "#0A140A"; font.pixelSize: 10 }
                                        MouseArea { anchors.fill: parent; onClicked: taskController.setDone(modelData.id, false) }
                                    }
                                    Label {
                                        text: modelData.title
                                        color: theme.textMuted
                                        font.pixelSize: 13
                                        font.strikeout: true
                                        Layout.fillWidth: true
                                        elide: Text.ElideRight
                                    }
                                }
                            }

                            Label {
                                text: "Nothing completed yet."
                                visible: root.completedTasks.length === 0
                                color: theme.textMuted
                                font.pixelSize: 12
                            }
                        }
                    }
                }
            }

            Item { Layout.preferredHeight: 24 }
        }
    }
}
