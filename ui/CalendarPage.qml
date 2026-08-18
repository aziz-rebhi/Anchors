import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Page {
    id: root
    Theme { id: theme }

    property var allEvents: []
    property date viewDate: new Date()
    property date selectedDate: new Date()
    // "month" | "week"
    property string viewMode: "month"

    function refresh() {
        allEvents = calendarController.entries()
        monthCells = buildMonthGrid()
    }

    Component.onCompleted: refresh()

    Connections {
        target: calendarController
        function onEntriesChanged() { root.refresh() }
    }

    function startOfDay(d) {
        var x = new Date(d)
        x.setHours(0, 0, 0, 0)
        return x
    }

    function endOfDay(d) {
        var x = new Date(d)
        x.setHours(23, 59, 59, 999)
        return x
    }

    function isSameDay(a, b) {
        return a.getFullYear() === b.getFullYear()
            && a.getMonth() === b.getMonth()
            && a.getDate() === b.getDate()
    }

    // Includes multi-day events (start..end overlaps the day)
    function eventsOn(dateObj) {
        var dayStart = startOfDay(dateObj).getTime()
        var dayEnd = endOfDay(dateObj).getTime()
        return allEvents.filter(function (e) {
            var s = new Date(e.start).getTime()
            var en = new Date(e.end).getTime()
            if (e.allDay) {
                // treat as full calendar day of start (and end if different)
                var es = startOfDay(new Date(e.start)).getTime()
                var ee = endOfDay(new Date(e.end)).getTime()
                return es <= dayEnd && ee >= dayStart
            }
            return s <= dayEnd && en >= dayStart
        }).sort(function (a, b) { return new Date(a.start) - new Date(b.start) })
    }

    function eventColorsOn(dateObj) {
        var list = eventsOn(dateObj)
        var colors = []
        for (var i = 0; i < list.length && colors.length < 3; i++) {
            var c = list[i].color || theme.tertiary
            if (colors.indexOf(c) < 0)
                colors.push(c)
        }
        return colors
    }

    readonly property var selectedDayEvents: eventsOn(selectedDate)

    function daysInMonth(month, year) {
        return new Date(year, month + 1, 0).getDate()
    }

    function firstDayOfMonth(month, year) {
        return new Date(year, month, 1).getDay()
    }

    function buildMonthGrid() {
        var year = viewDate.getFullYear()
        var month = viewDate.getMonth()
        var days = daysInMonth(month, year)
        var firstDay = firstDayOfMonth(month, year)
        var cells = []
        var today = new Date()
        for (var i = 0; i < 42; i++) {
            var day = i - firstDay + 1
            var dateObj = new Date(year, month, day)
            var isInMonth = (day >= 1 && day <= days)
            cells.push({
                day: day,
                isInMonth: isInMonth,
                isToday: isInMonth && isSameDay(dateObj, today),
                colors: isInMonth ? eventColorsOn(dateObj) : [],
                date: dateObj
            })
        }
        return cells
    }

    property var monthCells: buildMonthGrid()
    onViewDateChanged: monthCells = buildMonthGrid()

    function weekDates() {
        // Week containing selectedDate, Sunday start
        var d = startOfDay(selectedDate)
        var day = d.getDay()
        var start = new Date(d)
        start.setDate(d.getDate() - day)
        var days = []
        for (var i = 0; i < 7; i++) {
            var x = new Date(start)
            x.setDate(start.getDate() + i)
            days.push(x)
        }
        return days
    }

    background: Rectangle { color: theme.background }

    RowLayout {
        anchors.fill: parent
        spacing: 0

        // ---- Main calendar ----
        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.margins: 24
            spacing: 16

            RowLayout {
                Layout.fillWidth: true

                Label {
                    text: root.viewMode === "week"
                          ? ("Week of " + Qt.formatDate(root.weekDates()[0], "MMM d"))
                          : Qt.formatDate(root.viewDate, "MMMM yyyy")
                    color: theme.textPrimary
                    font.family: theme.headlineFont
                    font.pixelSize: 26
                    font.bold: true
                }

                Item { Layout.fillWidth: true }

                Row {
                    spacing: 6

                    // Month / Week toggle
                    Rectangle {
                        width: 70; height: 32; radius: theme.radiusPill
                        color: root.viewMode === "month" ? Qt.rgba(theme.tertiary.r, theme.tertiary.g, theme.tertiary.b, 0.25) : theme.surfaceAlt
                        Label { anchors.centerIn: parent; text: "Month"; color: theme.textSecondary; font.pixelSize: 12 }
                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            onClicked: root.viewMode = "month"
                        }
                    }
                    Rectangle {
                        width: 70; height: 32; radius: theme.radiusPill
                        color: root.viewMode === "week" ? Qt.rgba(theme.tertiary.r, theme.tertiary.g, theme.tertiary.b, 0.25) : theme.surfaceAlt
                        Label { anchors.centerIn: parent; text: "Week"; color: theme.textSecondary; font.pixelSize: 12 }
                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            onClicked: root.viewMode = "week"
                        }
                    }

                    Rectangle {
                        width: 70; height: 32; radius: theme.radiusPill
                        color: theme.surfaceAlt
                        Label { anchors.centerIn: parent; text: "Today"; color: theme.textSecondary; font.pixelSize: 12 }
                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                var n = new Date()
                                root.viewDate = n
                                root.selectedDate = n
                            }
                        }
                    }

                    Rectangle {
                        width: 32; height: 32; radius: theme.radiusPill
                        color: theme.surfaceAlt
                        Label { anchors.centerIn: parent; text: "‹"; color: theme.textSecondary; font.pixelSize: 16 }
                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                if (root.viewMode === "week") {
                                    var d = new Date(root.selectedDate)
                                    d.setDate(d.getDate() - 7)
                                    root.selectedDate = d
                                    root.viewDate = d
                                } else {
                                    var m = new Date(root.viewDate)
                                    m.setMonth(m.getMonth() - 1)
                                    root.viewDate = m
                                }
                            }
                        }
                    }
                    Rectangle {
                        width: 32; height: 32; radius: theme.radiusPill
                        color: theme.surfaceAlt
                        Label { anchors.centerIn: parent; text: "›"; color: theme.textSecondary; font.pixelSize: 16 }
                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                if (root.viewMode === "week") {
                                    var d = new Date(root.selectedDate)
                                    d.setDate(d.getDate() + 7)
                                    root.selectedDate = d
                                    root.viewDate = d
                                } else {
                                    var m = new Date(root.viewDate)
                                    m.setMonth(m.getMonth() + 1)
                                    root.viewDate = m
                                }
                            }
                        }
                    }
                }
            }

            // ===== MONTH VIEW =====
            ColumnLayout {
                visible: root.viewMode === "month"
                Layout.fillWidth: true
                Layout.fillHeight: true
                spacing: 8

                RowLayout {
                    Layout.fillWidth: true
                    Repeater {
                        model: ["S", "M", "T", "W", "T", "F", "S"]
                        delegate: Label {
                            text: modelData
                            color: theme.textMuted
                            font.pixelSize: 11
                            horizontalAlignment: Text.AlignHCenter
                            Layout.fillWidth: true
                        }
                    }
                }

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
                            Layout.minimumHeight: 36
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
                                spacing: 3

                                Label {
                                    text: modelData.isInMonth ? modelData.day : ""
                                    color: {
                                        if (!modelData.isInMonth) return theme.textMuted
                                        if (root.isSameDay(modelData.date, root.selectedDate) || modelData.isToday)
                                            return theme.tertiary
                                        return theme.textPrimary
                                    }
                                    font.pixelSize: 13
                                    anchors.horizontalCenter: parent.horizontalCenter
                                }

                                Row {
                                    spacing: 3
                                    anchors.horizontalCenter: parent.horizontalCenter
                                    Repeater {
                                        model: modelData.colors || []
                                        delegate: Rectangle {
                                            width: 5; height: 5; radius: 2.5
                                            color: modelData
                                        }
                                    }
                                }
                            }

                            MouseArea {
                                anchors.fill: parent
                                onClicked: {
                                    if (modelData.isInMonth)
                                        root.selectedDate = modelData.date
                                }
                                onDoubleClicked: {
                                    if (modelData.isInMonth) {
                                        root.selectedDate = modelData.date
                                        eventDialog.openForCreate(modelData.date)
                                    }
                                }
                            }
                        }
                    }
                }
            }

            // ===== WEEK VIEW =====
            RowLayout {
                visible: root.viewMode === "week"
                Layout.fillWidth: true
                Layout.fillHeight: true
                spacing: 8

                Repeater {
                    model: root.weekDates()
                    delegate: Rectangle {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        radius: theme.radiusSmall
                        color: root.isSameDay(modelData, root.selectedDate) ? theme.surfaceAlt : theme.surface
                        border.color: root.isSameDay(modelData, root.selectedDate) ? theme.tertiary : theme.border
                        border.width: 1

                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 8
                            spacing: 6

                            Label {
                                text: Qt.formatDate(modelData, "ddd")
                                color: theme.textMuted
                                font.pixelSize: 11
                            }
                            Label {
                                text: Qt.formatDate(modelData, "d")
                                color: root.isSameDay(modelData, new Date()) ? theme.tertiary : theme.textPrimary
                                font.pixelSize: 18
                                font.bold: true
                            }

                            Repeater {
                                model: root.eventsOn(modelData).slice(0, 4)
                                delegate: Rectangle {
                                    Layout.fillWidth: true
                                    height: 22
                                    radius: 4
                                    color: Qt.rgba(0, 0, 0, 0.2)
                                    Rectangle {
                                        width: 3
                                        anchors.left: parent.left
                                        anchors.top: parent.top
                                        anchors.bottom: parent.bottom
                                        color: modelData.color || theme.tertiary
                                    }
                                    Label {
                                        anchors.left: parent.left
                                        anchors.leftMargin: 8
                                        anchors.verticalCenter: parent.verticalCenter
                                        anchors.right: parent.right
                                        anchors.rightMargin: 4
                                        text: modelData.title
                                        color: theme.textPrimary
                                        font.pixelSize: 11
                                        elide: Text.ElideRight
                                    }
                                    MouseArea {
                                        anchors.fill: parent
                                        onClicked: eventDialog.openForEdit(modelData)
                                    }
                                }
                            }

                            Item { Layout.fillHeight: true }
                        }

                        MouseArea {
                            anchors.fill: parent
                            z: -1
                            onClicked: root.selectedDate = modelData
                            onDoubleClicked: {
                                root.selectedDate = modelData
                                eventDialog.openForCreate(modelData)
                            }
                        }
                    }
                }
            }
        }

        // ---- Agenda sidebar ----
        Rectangle {
            Layout.preferredWidth: 300
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
                        width: 32; height: 32; radius: 16
                        color: theme.tertiary
                        Text {
                            anchors.centerIn: parent
                            text: "+"
                            color: "#0A140A"
                            font.pixelSize: 18
                            font.bold: true
                        }
                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            onClicked: eventDialog.openForCreate(root.selectedDate)
                        }
                        ToolTip.visible: hovered
                        ToolTip.text: "New event"
                        readonly property alias hovered: addEventMa.containsMouse
                        MouseArea {
                            id: addEventMa
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: eventDialog.openForCreate(root.selectedDate)
                        }
                    }
                }

                ScrollView {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    clip: true
                    contentWidth: availableWidth

                    ColumnLayout {
                        width: parent.width
                        spacing: 10

                        Repeater {
                            model: root.selectedDayEvents
                            delegate: Rectangle {
                                Layout.fillWidth: true
                                radius: theme.radiusSmall
                                color: theme.surfaceAlt
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
                                            text: modelData.allDay ? "All day"
                                                  : Qt.formatTime(new Date(modelData.start), "h:mm AP")
                                            color: theme.textMuted
                                            font.pixelSize: 10
                                        }
                                    }
                                    Label {
                                        text: modelData.description || ""
                                        visible: text.length > 0
                                        color: theme.textSecondary
                                        font.pixelSize: 12
                                        wrapMode: Text.WordWrap
                                        Layout.fillWidth: true
                                    }

                                    RowLayout {
                                        Layout.topMargin: 6
                                        spacing: 8

                                        Rectangle {
                                            width: 28; height: 28; radius: 6
                                            color: editMa.containsMouse ? Qt.rgba(1,1,1,0.08) : "transparent"
                                            Text { anchors.centerIn: parent; text: "✎"; color: theme.tertiary; font.pixelSize: 13 }
                                            MouseArea {
                                                id: editMa
                                                anchors.fill: parent
                                                hoverEnabled: true
                                                cursorShape: Qt.PointingHandCursor
                                                onClicked: eventDialog.openForEdit(modelData)
                                            }
                                            ToolTip.visible: editMa.containsMouse
                                            ToolTip.text: "Edit"
                                            ToolTip.delay: 400
                                        }
                                        Rectangle {
                                            width: 28; height: 28; radius: 6
                                            color: delMa.containsMouse ? Qt.rgba(0.9,0.2,0.2,0.15) : "transparent"
                                            Text { anchors.centerIn: parent; text: "🗑"; font.pixelSize: 13 }
                                            MouseArea {
                                                id: delMa
                                                anchors.fill: parent
                                                hoverEnabled: true
                                                cursorShape: Qt.PointingHandCursor
                                                onClicked: calendarController.deleteEntry(modelData.id)
                                            }
                                            ToolTip.visible: delMa.containsMouse
                                            ToolTip.text: "Delete"
                                            ToolTip.delay: 400
                                        }
                                    }
                                }

                                MouseArea {
                                    anchors.fill: parent
                                    z: -1
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: eventDialog.openForEdit(modelData)
                                }
                            }
                        }

                        ColumnLayout {
                            visible: root.selectedDayEvents.length === 0
                            spacing: 8
                            Layout.topMargin: 12
                            Label {
                                text: "No events this day."
                                color: theme.textMuted
                                font.pixelSize: 13
                            }
                            Label {
                                text: "Press + or double-click a day to add one."
                                color: theme.textMuted
                                font.pixelSize: 12
                            }
                        }
                    }
                }
            }
        }
    }

    CalendarEventDialog {
        id: eventDialog
        parent: Overlay.overlay
        anchors.centerIn: parent
    }
}