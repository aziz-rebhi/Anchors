import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Page {
    id: dashboardPage

    Theme { id: theme }

    readonly property color cardBg: theme.surfaceAlt
    readonly property color cardBorder: theme.border
    readonly property color cardShadow: Qt.rgba(0, 0, 0, 0.5)
    readonly property color subtleBg: "#111311"

    property var openTasks: []
    property var upcomingEvents: []
    property int overdueCount: 0

    signal navigateRequested(string pageName)

    function refreshTasks() {
        var all = taskController.entries()
        var open = all.filter(function (t) { return !t.done })
        open.sort(function (a, b) { return (a.dueAt || Infinity) - (b.dueAt || Infinity) })
        openTasks = open.slice(0, 4)

        var nowSecs = Date.now() / 1000
        overdueCount = open.filter(function (t) { return t.dueAt > 0 && t.dueAt < nowSecs }).length
    }

    function refreshEvents() {
        var all = calendarController.entries()
        var nowSecs = Date.now() / 1000
        var upcoming = all.filter(function (e) {
            return new Date(e.start).getTime() / 1000 >= nowSecs - 3600
        })
        upcoming.sort(function (a, b) { return new Date(a.start) - new Date(b.start) })
        upcomingEvents = upcoming.slice(0, 3)
    }

    Component.onCompleted: {
        refreshTasks()
        refreshEvents()
    }

    Connections {
        target: taskController
        function onEntriesChanged() { dashboardPage.refreshTasks() }
    }

    Connections {
        target: calendarController
        function onEntriesChanged() { dashboardPage.refreshEvents() }
    }

    background: Rectangle { color: theme.background }

    ScrollView {
        anchors.fill: parent
        contentWidth: availableWidth
        clip: true
        ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

        ColumnLayout {
            width: parent.width
            spacing: theme.spacingLarge
            Layout.margins: 20

            RowLayout {
                Layout.fillWidth: true
                spacing: 0

                ColumnLayout {
                    spacing: 4

                    Label {
                        text: "MORNING OVERVIEW"
                        color: theme.textMuted
                        font.family: theme.labelFont
                        font.pixelSize: 11
                        font.letterSpacing: 1.5
                        textFormat: Text.PlainText
                        opacity: 0.6
                    }
                    Label {
                        text: "Today"
                        color: theme.textPrimary
                        font.family: theme.headlineFont
                        font.pixelSize: 32
                        font.weight: Font.Bold
                        font.letterSpacing: -0.5
                    }
                }

                Item { Layout.fillWidth: true }

                RowLayout {
                    spacing: 8
                    // padding removed – RowLayout does not have that property
                    Layout.alignment: Qt.AlignBottom
                    Rectangle {
                        width: 8
                        height: 8
                        radius: 4
                        color: theme.tertiary
                        layer.enabled: true
                        layer.effect: null
                    }
                    Label {
                        text: "System Active"
                        color: theme.textSecondary
                        font.family: theme.bodyFont
                        font.pixelSize: 12
                        font.weight: Font.Medium
                    }
                }
            }

            GridLayout {
                Layout.fillWidth: true
                columnSpacing: 20
                rowSpacing: 20
                columns: 12

                // Priority Tasks
                Rectangle {
                    Layout.columnSpan: 8
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    Layout.minimumHeight: 320
                    color: cardBg
                    border.color: cardBorder
                    border.width: 1
                    radius: 16

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 16
                        spacing: 16

                        RowLayout {
                            Layout.fillWidth: true

                            RowLayout {
                                spacing: 8
                                Label {
                                    text: "✓"
                                    color: theme.tertiary
                                    font.pixelSize: 18
                                }
                                Label {
                                    text: "Priority Tasks"
                                    color: theme.textPrimary
                                    font.family: theme.headlineFont
                                    font.pixelSize: 18
                                    font.weight: Font.Medium
                                }
                            }

                            Item { Layout.fillWidth: true }

                            Button {
                                text: "Add Task"
                                font.family: theme.labelFont
                                font.pixelSize: 12
                                font.weight: Font.Medium
                                background: Rectangle {
                                    radius: 999
                                    color: "#0B3D0B"
                                }
                                contentItem: Text {
                                    text: parent.text
                                    color: theme.tertiary
                                    font: parent.font
                                    horizontalAlignment: Text.AlignHCenter
                                    verticalAlignment: Text.AlignVCenter
                                }
                                padding: 10
                                Layout.preferredHeight: 32
                                onClicked: {
                                    dashboardPage.navigateRequested("todo")
                                }
                            }
                        }

                        ColumnLayout {
                            spacing: 8
                            Layout.fillWidth: true
                            Layout.fillHeight: true

                            Repeater {
                                model: dashboardPage.openTasks

                                delegate: Rectangle {
                                    Layout.fillWidth: true
                                    height: 48
                                    color: "transparent"
                                    radius: 8

                                    RowLayout {
                                        anchors.fill: parent
                                        anchors.leftMargin: 4
                                        spacing: 12

                                        Rectangle {
                                            width: 20
                                            height: 20
                                            radius: 4
                                            border.color: "#42493e"
                                            border.width: 1
                                            color: "transparent"

                                            MouseArea {
                                                anchors.fill: parent
                                                onClicked: {
                                                    taskController.setDone(modelData.id, true)
                                                }
                                            }
                                        }

                                        Label {
                                            text: modelData.title
                                            color: index === 0 ? theme.textPrimary : theme.textSecondary
                                            font.family: theme.bodyFont
                                            font.pixelSize: 15
                                            Layout.fillWidth: true
                                            elide: Text.ElideRight
                                        }

                                        Label {
                                            text: modelData.dueAt > 0
                                                  ? "Due " + Qt.formatDateTime(new Date(modelData.dueAt * 1000), "h:mm AP")
                                                  : ""
                                            color: theme.textMuted
                                            font.family: theme.labelFont
                                            font.pixelSize: 11
                                            opacity: 0.7
                                        }
                                    }

                                    MouseArea {
                                        anchors.fill: parent
                                        hoverEnabled: true
                                        cursorShape: Qt.PointingHandCursor
                                        z: -1
                                        onEntered: parent.color = Qt.rgba(0.2, 0.2, 0.2, 0.3)
                                        onExited: parent.color = "transparent"
                                    }
                                }
                            }

                            Label {
                                text: "No open tasks. Nice."
                                visible: dashboardPage.openTasks.length === 0
                                color: theme.textMuted
                                font.family: theme.bodyFont
                                font.pixelSize: 13
                            }

                            Item { Layout.fillHeight: true }
                        }
                    }
                }

                // Action Required
                Rectangle {
                    Layout.columnSpan: 4
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    Layout.minimumHeight: 320
                    color: cardBg
                    border.color: cardBorder
                    border.width: 1
                    radius: 16

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 16
                        spacing: 12

                        Rectangle {
                            Layout.fillWidth: true
                            height: 4
                            color: "#E0574C"
                            radius: 2
                        }

                        RowLayout {
                            spacing: 8
                            Label {
                                text: "⚠"
                                color: theme.danger
                                font.pixelSize: 18
                            }
                            Label {
                                text: "Action Required"
                                color: theme.textPrimary
                                font.family: theme.headlineFont
                                font.pixelSize: 18
                                font.weight: Font.Medium
                            }
                        }

                        Item { Layout.fillHeight: true }

                        ColumnLayout {
                            Layout.alignment: Qt.AlignCenter
                            spacing: 4

                            Label {
                                text: dashboardPage.overdueCount
                                color: theme.textPrimary
                                font.family: theme.headlineFont
                                font.pixelSize: 56
                                font.weight: Font.Bold
                                Layout.alignment: Qt.AlignHCenter
                            }
                            Label {
                                text: dashboardPage.overdueCount === 1
                                      ? "task is overdue."
                                      : "tasks are overdue."
                                color: theme.textSecondary
                                font.family: theme.bodyFont
                                font.pixelSize: 13
                                wrapMode: Text.WordWrap
                                horizontalAlignment: Text.AlignHCenter
                                Layout.maximumWidth: parent.width
                            }
                        }

                        Item { Layout.fillHeight: true }

                        Button {
                            text: "Review Actions"
                            flat: true
                            Layout.fillWidth: true
                            font.family: theme.labelFont
                            font.pixelSize: 12
                            font.weight: Font.Medium
                            background: Rectangle {
                                color: "transparent"
                                border.color: cardBorder
                                border.width: 1
                                radius: 8
                            }
                            contentItem: Text {
                                text: parent.text
                                color: theme.textPrimary
                                font: parent.font
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                            }
                            padding: 10
                            onClicked: {
                                dashboardPage.navigateRequested("todo")
                            }
                        }
                    }
                }

                // Schedule
                Rectangle {
                    Layout.columnSpan: 7
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    Layout.minimumHeight: 280
                    color: cardBg
                    border.color: cardBorder
                    border.width: 1
                    radius: 16

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 16
                        spacing: 16

                        RowLayout {
                            Layout.fillWidth: true
                            RowLayout {
                                spacing: 8
                                Label {
                                    text: "📅"
                                    color: theme.textPrimary
                                    font.pixelSize: 18
                                }
                                Label {
                                    text: "Schedule"
                                    color: theme.textPrimary
                                    font.family: theme.headlineFont
                                    font.pixelSize: 18
                                    font.weight: Font.Medium
                                }
                            }
                            Item { Layout.fillWidth: true }
                            Label {
                                text: {
                                    if (dashboardPage.upcomingEvents.length === 0) return ""
                                    var mins = Math.max(0, Math.round((new Date(dashboardPage.upcomingEvents[0].start) - new Date()) / 60000))
                                    return "Next: " + mins + "m"
                                }
                                color: theme.textMuted
                                font.family: theme.labelFont
                                font.pixelSize: 11
                                font.weight: Font.Medium
                            }
                        }

                        Item {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            clip: true

                            ColumnLayout {
                                anchors.fill: parent
                                spacing: 16

                                Repeater {
                                    model: dashboardPage.upcomingEvents

                                    delegate: RowLayout {
                                        spacing: 12
                                        Layout.fillWidth: true

                                        Label {
                                            text: Qt.formatDateTime(new Date(modelData.start), "h:mm AP")
                                            color: theme.textSecondary
                                            font.family: theme.bodyFont
                                            font.pixelSize: 12
                                            font.weight: Font.Medium
                                            width: 60
                                            horizontalAlignment: Text.AlignRight
                                        }

                                        Item {
                                            width: 16
                                            height: 16
                                            Rectangle {
                                                width: 8
                                                height: 8
                                                radius: 4
                                                color: index === 0 ? theme.tertiary : theme.textMuted
                                                anchors.centerIn: parent
                                                Rectangle {
                                                    width: 16
                                                    height: 16
                                                    radius: 8
                                                    color: "transparent"
                                                    border.color: Qt.rgba(1,1,1,0.1)
                                                    border.width: 4
                                                    anchors.centerIn: parent
                                                }
                                            }
                                            Rectangle {
                                                visible: index === 0 && dashboardPage.upcomingEvents.length > 1
                                                width: 1
                                                height: parent.parent.height - parent.y - 16
                                                color: cardBorder
                                                anchors.top: parent.bottom
                                                anchors.horizontalCenter: parent.horizontalCenter
                                            }
                                        }

                                        Rectangle {
                                            Layout.fillWidth: true
                                            height: 64
                                            color: index === 0 ? subtleBg : "transparent"
                                            border.color: index === 0 ? cardBorder : "transparent"
                                            border.width: 1
                                            radius: 8
                                            ColumnLayout {
                                                anchors.fill: parent
                                                anchors.margins: 8
                                                spacing: 2
                                                Label {
                                                    text: modelData.title
                                                    color: index === 0 ? theme.textPrimary : theme.textSecondary
                                                    font.family: theme.headlineFont
                                                    font.pixelSize: 14
                                                    font.weight: Font.Medium
                                                    elide: Text.ElideRight
                                                }
                                                Label {
                                                    text: modelData.description
                                                    color: theme.textMuted
                                                    font.family: theme.bodyFont
                                                    font.pixelSize: 12
                                                    elide: Text.ElideRight
                                                }
                                            }
                                        }
                                    }
                                }

                                Label {
                                    text: "Nothing on the calendar yet."
                                    visible: dashboardPage.upcomingEvents.length === 0
                                    color: theme.textMuted
                                    font.family: theme.bodyFont
                                    font.pixelSize: 13
                                }
                            }
                        }
                    }
                }

                // Scratchpad
                Rectangle {
                    Layout.columnSpan: 5
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    Layout.minimumHeight: 280
                    color: cardBg
                    border.color: cardBorder
                    border.width: 1
                    radius: 16

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 16
                        spacing: 12

                        RowLayout {
                            Layout.fillWidth: true
                            RowLayout {
                                spacing: 8
                                Label {
                                    text: "📝"
                                    color: theme.textPrimary
                                    font.pixelSize: 18
                                }
                                Label {
                                    text: "Scratchpad"
                                    color: theme.textPrimary
                                    font.family: theme.headlineFont
                                    font.pixelSize: 18
                                    font.weight: Font.Medium
                                }
                            }
                            Item { Layout.fillWidth: true }
                            Button {
                                text: "↗"
                                flat: true
                                font.pixelSize: 16
                                onClicked: {
                                    console.log("Open scratchpad full view")
                                }
                            }
                        }

                        TextArea {
                            id: scratchpadArea
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            placeholderText: "Capture thoughts quickly..."
                            placeholderTextColor: theme.textMuted
                            color: theme.textPrimary
                            font.family: theme.bodyFont
                            font.pixelSize: 14
                            background: Rectangle {
                                color: subtleBg
                                border.color: cardBorder
                                border.width: 1
                                radius: 8
                            }
                            wrapMode: Text.WordWrap
                            selectByMouse: true
                            padding: 12

                            property string noteId: ""
                            property bool loading: true

                            function load() {
                                loading = true
                                var all = noteController.entries()
                                for (var i = 0; i < all.length; i++) {
                                    if (all[i].title === "Scratchpad") {
                                        noteId = all[i].id
                                        text = all[i].content
                                        loading = false
                                        return
                                    }
                                }
                                noteId = ""
                                text = ""
                                loading = false
                            }

                            Component.onCompleted: load()

                            onTextChanged: {
                                if (loading) return
                                saveTimer.restart()
                            }

                            Timer {
                                id: saveTimer
                                interval: 800
                                onTriggered: {
                                    if (scratchpadArea.noteId.length > 0) {
                                        noteController.updateEntry(scratchpadArea.noteId, "Scratchpad", scratchpadArea.text)
                                    } else {
                                        noteController.addEntry("Scratchpad", scratchpadArea.text)
                                        scratchpadArea.load()
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}