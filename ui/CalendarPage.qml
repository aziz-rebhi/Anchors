import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Page {
    id: root

    Theme { id: theme }

    property var allEvents: []
    property date viewDate: new Date()
    property date selectedDate: new Date()

    function refresh() {
        allEvents = calendarController.entries()
    }

    Component.onCompleted: refresh()

    Connections {
        target: calendarController
        function onEntriesChanged() { root.refresh() }
    }

    function isSameDay(a, b) {
        return a.getFullYear() === b.getFullYear() &&
               a.getMonth() === b.getMonth() &&
               a.getDate() === b.getDate()
    }

    function eventsOn(dateObj) {
        return allEvents.filter(function (e) {
            return root.isSameDay(new Date(e.start), dateObj)
        }).sort(function (a, b) { return new Date(a.start) - new Date(b.start) })
    }

    readonly property var selectedDayEvents: eventsOn(selectedDate)

    // ----- Manual month grid helpers -----
    function daysInMonth(month, year) {
        return new Date(year, month + 1, 0).getDate()
    }

    function firstDayOfMonth(month, year) {
        return new Date(year, month, 1).getDay() // 0 = Sunday
    }

    function buildMonthGrid() {
        var grid = []
        var year = viewDate.getFullYear()
        var month = viewDate.getMonth()
        var days = daysInMonth(month, year)
        var firstDay = firstDayOfMonth(month, year)
        // We'll build a 6x7 grid (max 42 cells)
        var cells = []
        var today = new Date()
        for (var i = 0; i < 42; i++) {
            var day = i - firstDay + 1
            var dateObj = new Date(year, month, day)
            var isInMonth = (day >= 1 && day <= days)
            var isToday = isInMonth && isSameDay(dateObj, today)
            var hasEvent = isInMonth && eventsOn(dateObj).length > 0
            cells.push({
                day: day,
                isInMonth: isInMonth,
                isToday: isToday,
                hasEvent: hasEvent,
                date: dateObj
            })
        }
        return cells
    }

    property var monthCells: buildMonthGrid()

    // Update when viewDate changes
    onViewDateChanged: monthCells = buildMonthGrid()

    background: Rectangle { color: theme.background }

    RowLayout {
        anchors.fill: parent
        spacing: 0

        // ---- Month grid ----
        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.margins: 24
            spacing: 16

            RowLayout {
                Layout.fillWidth: true

                Label {
                    text: Qt.formatDate(root.viewDate, "MMMM yyyy")
                    color: theme.textPrimary
                    font.family: theme.headlineFont
                    font.pixelSize: 26
                    font.bold: true
                }

                Item { Layout.fillWidth: true }

                Row {
                    spacing: 4
                    Rectangle {
                        width: 70; height: 32; radius: theme.radiusPill
                        color: theme.surfaceAlt
                        Label { anchors.centerIn: parent; text: "Today"; color: theme.textSecondary; font.pixelSize: 12 }
                        MouseArea {
                            anchors.fill: parent
                            onClicked: { root.viewDate = new Date(); root.selectedDate = new Date() }
                        }
                    }
                    Rectangle {
                        width: 32; height: 32; radius: theme.radiusPill
                        color: theme.surfaceAlt
                        Label { anchors.centerIn: parent; text: "\u2039"; color: theme.textSecondary }
                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                var d = new Date(root.viewDate)
                                d.setMonth(d.getMonth() - 1)
                                root.viewDate = d
                            }
                        }
                    }
                    Rectangle {
                        width: 32; height: 32; radius: theme.radiusPill
                        color: theme.surfaceAlt
                        Label { anchors.centerIn: parent; text: "\u203A"; color: theme.textSecondary }
                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                var d = new Date(root.viewDate)
                                d.setMonth(d.getMonth() + 1)
                                root.viewDate = d
                            }
                        }
                    }
                }
            }

            // Day-of-week headers
            RowLayout {
                Layout.fillWidth: true
                Repeater {
                    model: ["S", "M", "T", "W", "T", "F", "S"]
                    delegate: Label {
                        text: modelData
                        color: theme.textMuted
                        font.family: theme.labelFont
                        font.pixelSize: 11
                        horizontalAlignment: Text.AlignHCenter
                        Layout.fillWidth: true
                    }
                }
            }

            // Month grid (6 rows, 7 columns)
            GridLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                columns: 7
                rowSpacing: 4
                columnSpacing: 4

                Repeater {
                    model: root.monthCells
                    delegate: Rectangle {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        Layout.minimumWidth: 30
                        Layout.minimumHeight: 30
                        color: {
                            if (!modelData.isInMonth) return "transparent"
                            if (root.isSameDay(modelData.date, root.selectedDate)) return theme.surfaceAlt
                            if (modelData.isToday) return Qt.rgba(theme.tertiary.r, theme.tertiary.g, theme.tertiary.b, 0.2)
                            return "transparent"
                        }
                        border.color: root.isSameDay(modelData.date, root.selectedDate) ? theme.tertiary : "transparent"
                        border.width: 1
                        radius: theme.radiusSmall

                        Column {
                            anchors.centerIn: parent
                            spacing: 2

                            Label {
                                text: modelData.isInMonth ? modelData.day : ""
                                color: {
                                    if (!modelData.isInMonth) return theme.textMuted
                                    if (root.isSameDay(modelData.date, root.selectedDate)) return theme.tertiary
                                    if (modelData.isToday) return theme.tertiary
                                    return theme.textPrimary
                                }
                                font.family: theme.bodyFont
                                font.pixelSize: 13
                                anchors.horizontalCenter: parent.horizontalCenter
                            }

                            Row {
                                spacing: 2
                                anchors.horizontalCenter: parent.horizontalCenter
                                Repeater {
                                    model: modelData.hasEvent ? 1 : 0 // we show a dot if any event
                                    delegate: Rectangle {
                                        width: 4; height: 4; radius: 2
                                        color: theme.tertiary
                                    }
                                }
                            }
                        }

                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                if (modelData.isInMonth) {
                                    root.selectedDate = modelData.date
                                }
                            }
                        }
                    }
                }
            }
        }

        // ---- Agenda sidebar ----
        Rectangle {
            Layout.preferredWidth: 280
            Layout.fillHeight: true
            color: theme.surface

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 16
                spacing: 12

                RowLayout {
                    Layout.fillWidth: true
                    ColumnLayout {
                        spacing: 0
                        Label {
                            text: Qt.formatDate(root.selectedDate, "dddd")
                            color: theme.textPrimary
                            font.family: theme.headlineFont
                            font.pixelSize: 18
                            font.bold: true
                        }
                        Label {
                            text: Qt.formatDate(root.selectedDate, "MMMM d, yyyy").toUpperCase()
                            color: theme.textMuted
                            font.pixelSize: 10
                            font.letterSpacing: 1
                        }
                    }
                    Item { Layout.fillWidth: true }
                    Rectangle {
                        width: 28; height: 28; radius: 14
                        color: theme.tertiary
                        Label { anchors.centerIn: parent; text: "+"; color: "#0A140A"; font.pixelSize: 16 }
                        MouseArea {
                            anchors.fill: parent
                            onClicked: eventDialog.openForCreate(root.selectedDate)
                        }
                    }
                }

                ScrollView {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    clip: true

                    ColumnLayout {
                        width: parent.width
                        spacing: 10

                        Repeater {
                            model: root.selectedDayEvents
                            delegate: Rectangle {
                                Layout.fillWidth: true
                                radius: theme.radiusSmall
                                color: theme.surfaceAlt
                                border.width: 1
                                border.color: Qt.rgba(0,0,0,0)
                                Layout.preferredHeight: contentCol.implicitHeight + 20

                                Rectangle {
                                    width: 3
                                    anchors.top: parent.top
                                    anchors.bottom: parent.bottom
                                    anchors.left: parent.left
                                    color: modelData.color
                                }

                                ColumnLayout {
                                    id: contentCol
                                    anchors.fill: parent
                                    anchors.margins: 10
                                    anchors.leftMargin: 14
                                    spacing: 2

                                    RowLayout {
                                        Layout.fillWidth: true
                                        Label {
                                            text: modelData.title
                                            color: theme.textPrimary
                                            font.family: theme.headlineFont
                                            font.pixelSize: 14
                                            font.bold: true
                                            Layout.fillWidth: true
                                            elide: Text.ElideRight
                                        }
                                        Label {
                                            text: modelData.allDay ? "All day" : Qt.formatTime(new Date(modelData.start), "h:mm AP")
                                            color: theme.textMuted
                                            font.pixelSize: 10
                                        }
                                    }
                                    Label {
                                        text: modelData.description
                                        visible: text.length > 0
                                        color: theme.textSecondary
                                        font.pixelSize: 12
                                        wrapMode: Text.WordWrap
                                        Layout.fillWidth: true
                                    }
                                    RowLayout {
                                        Layout.topMargin: 4
                                        Label {
                                            text: "Edit"
                                            color: theme.tertiary
                                            font.pixelSize: 11
                                            MouseArea { anchors.fill: parent; onClicked: eventDialog.openForEdit(modelData) }
                                        }
                                        Label {
                                            text: "Delete"
                                            color: theme.danger
                                            font.pixelSize: 11
                                            Layout.leftMargin: 12
                                            MouseArea { anchors.fill: parent; onClicked: calendarController.deleteEntry(modelData.id) }
                                        }
                                    }
                                }
                            }
                        }

                        Label {
                            text: "No events this day."
                            visible: root.selectedDayEvents.length === 0
                            color: theme.textMuted
                            font.pixelSize: 13
                        }
                    }
                }
            }
        }
    }

    CalendarEventDialog {
        id: eventDialog
    }
}