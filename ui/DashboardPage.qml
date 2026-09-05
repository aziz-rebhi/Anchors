import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Page {
    id: dashboardPage

    Theme { id: theme }

    property var openTasks: []
    property var upcomingEvents: []
    property var recentNotes: []
    property var weekDays: []
    property int overdueCount: 0
    property int vaultCount: 0
    property int notesCount: 0
    property string profileName: ""
    property string todoFocusFilter: ""

    signal navigateRequested(string pageName, string filter, var payload)

    property string greeting: "Good morning"
    property string todayLabel: ""

    function refreshClock() {
        var h = new Date().getHours()
        greeting = h < 12 ? "Good morning" : (h < 18 ? "Good afternoon" : "Good evening")
        todayLabel = Qt.formatDate(new Date(), "dddd, MMMM d")
    }

    function tDue(t) {
        if (!t) return 0
        return t.dueAt || 0
    }

    function startOfDay(d) {
        var x = new Date(d)
        x.setHours(0, 0, 0, 0)
        return x
    }

    function priorityScore(t) {
        var due = tDue(t)
        if (due <= 0) return 5000000000
        var now = Date.now() / 1000
        var day = 86400
        if (due < now) return due
        var todayEnd = startOfDay(new Date()).getTime() / 1000 + day
        if (due < todayEnd) return 1000000000 + due
        if (due < now + 7 * day) return 2000000000 + due
        return 3000000000 + due
    }

    function projectMeta(projectId) {
        var fallback = { name: "Inbox", emoji: "📥", color: theme.tertiary }
        if (!projectId) return fallback
        if (typeof taskController === "undefined" || !taskController)
            return fallback
        try {
            var projects = taskController.projects()
            if (!projects) return fallback
            for (var i = 0; i < projects.length; i++) {
                if (projects[i].id === projectId)
                    return projects[i]
            }
        } catch (e) {}
        return fallback
    }

    function refreshTasks() {
        if (typeof taskController === "undefined" || !taskController)
            return
        var all = taskController.entries() || []
        var open = all.filter(function (t) { return t && !t.done })
        open.sort(function (a, b) {
            return priorityScore(a) - priorityScore(b)
        })
        openTasks = open.slice(0, 5)

        var nowSecs = Date.now() / 1000
        overdueCount = open.filter(function (t) {
            return tDue(t) > 0 && tDue(t) < nowSecs
        }).length
    }

    function refreshEvents() {
        if (typeof calendarController === "undefined" || !calendarController) {
            upcomingEvents = []
            buildWeekStrip([])
            return
        }
        var all = calendarController.entries() || []
        var nowSecs = Date.now() / 1000
        var upcoming = all.filter(function (e) {
            if (!e || e.start === undefined || e.start === null)
                return false
            return new Date(e.start).getTime() / 1000 >= nowSecs
        })
        upcoming.sort(function (a, b) {
            return new Date(a.start) - new Date(b.start)
        })
        upcomingEvents = upcoming.slice(0, 4)
        buildWeekStrip(all)
    }

    function buildWeekStrip(allEvents) {
        var events = allEvents || []
        var days = []
        var base = startOfDay(new Date())
        for (var i = 0; i < 7; i++) {
            var d = new Date(base)
            d.setDate(base.getDate() + i)
            var dayStart = d.getTime()
            var dayEnd = dayStart + 86400000
            var count = 0
            for (var j = 0; j < events.length; j++) {
                if (!events[j] || events[j].start === undefined)
                    continue
                var t = new Date(events[j].start).getTime()
                if (t >= dayStart && t < dayEnd)
                    count++
            }
            days.push({
                label: Qt.formatDate(d, "ddd"),
                dayNum: d.getDate(),
                isToday: i === 0,
                eventCount: count,
                firstDate: d
            })
        }
        weekDays = days
    }

    function refreshNotes() {
        if (typeof noteController === "undefined" || !noteController) {
            notesCount = 0
            recentNotes = []
            return
        }
        var all = noteController.entries() || []
        notesCount = all.length
        var list = all.filter(function (n) {
            return n && (n.title || "") !== "Scratchpad"
        })
        list.sort(function (a, b) {
            return (b.updatedAt || b.createdAt || 0) - (a.updatedAt || a.createdAt || 0)
        })
        recentNotes = list.slice(0, 3)
    }

    function refreshVault() {
        vaultCount = 0
        try {
            if (typeof vaultController !== "undefined" && vaultController)
                vaultCount = (vaultController.entries() || []).length
        } catch (e) {}
    }

    function refreshProfile() {
        profileName = ""
        try {
            if (typeof settingsController !== "undefined" && settingsController
                    && settingsController.userName)
                profileName = settingsController.userName
        } catch (e) {}
    }

    function refreshAll() {
        refreshClock()
        refreshTasks()
        refreshEvents()
        refreshNotes()
        refreshVault()
        refreshProfile()
    }

    function goTodo(filter) {
        todoFocusFilter = filter || ""
        navigateRequested("todo", todoFocusFilter, null)
    }
    function goNotes()    { navigateRequested("notes", "", null) }
    function goCalendar() { navigateRequested("calendar", "", null) }
    function goCalendarDay(d) { navigateRequested("calendar", "", d) }
    function goVault()    { navigateRequested("vault", "", null) }

    function quickAddTask() {
        if (typeof taskController === "undefined" || !taskController)
            return
        var t = quickField.text.trim()
        if (!t.length) return
        taskController.addEntry(t, 0, "")
        quickField.text = ""
        refreshTasks()
    }

    function nextEventMinutes() {
        if (!upcomingEvents || upcomingEvents.length === 0)
            return -1
        var ev = upcomingEvents[0]
        if (!ev || ev.start === undefined)
            return -1
        return Math.max(0, Math.round((new Date(ev.start) - new Date()) / 60000))
    }

    Component.onCompleted: refreshAll()

    onVisibleChanged: {
        if (visible)
            refreshAll()
    }

    Connections {
        target: typeof taskController !== "undefined" ? taskController : null
        function onEntriesChanged() { dashboardPage.refreshTasks() }
        function onProjectsChanged() { dashboardPage.refreshTasks() }
    }
    Connections {
        target: typeof calendarController !== "undefined" ? calendarController : null
        function onEntriesChanged() { dashboardPage.refreshEvents() }
    }
    Connections {
        target: typeof noteController !== "undefined" ? noteController : null
        function onEntriesChanged() { dashboardPage.refreshNotes() }
    }
    Connections {
        target: typeof vaultController !== "undefined" ? vaultController : null
        function onEntriesChanged() { dashboardPage.refreshVault() }
    }
    Connections {
        target: typeof session !== "undefined" ? session : null
        function onLocked() {
            scratchpadArea.persist()   // best-effort; meaningful when the
                                        // HomePage outlives the key drop briefly
            dashboardPage.refreshAll()
        }
    }

    // Keep time-sensitive numbers honest while the page sits open: the
    // overdue count flips the moment a task crosses its due time, "Next
    // event" rolls forward, and the greeting/date refresh at midnight.
    Timer {
        interval: 60000
        repeat: true
        running: true
        onTriggered: {
            dashboardPage.refreshAll()
        }
    }

    background: Rectangle { color: theme.background }

    ScrollView {
        id: scroll
        anchors.fill: parent
        clip: true
        contentWidth: availableWidth
        ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

        // Equal left/right margins
        Item {
            width: scroll.availableWidth
            height: col.implicitHeight + 48

            ColumnLayout {
                id: col
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.top: parent.top
                anchors.topMargin: 8
                width: Math.min(2200, parent.width - 32)
                spacing: 18

                // Header
                RowLayout {
                    Layout.fillWidth: true
                    ColumnLayout {
                        spacing: 2
                        Label {
                            text: dashboardPage.greeting.toUpperCase()
                                  + (dashboardPage.profileName.length
                                     ? (", " + dashboardPage.profileName).toUpperCase()
                                     : "")
                            color: theme.textMuted
                            font.pixelSize: 11
                            font.letterSpacing: 1.1
                            opacity: 0.8
                        }
                        Label {
                            text: "Today"
                            color: theme.textPrimary
                            font.family: theme.headlineFont
                            font.pixelSize: 30
                            font.weight: Font.Bold
                        }
                        Label {
                            text: dashboardPage.todayLabel
                            color: theme.textSecondary
                            font.pixelSize: 13
                        }
                    }
                    Item { Layout.fillWidth: true }
                }

                // Week strip
                Rectangle {
                    Layout.fillWidth: true
                    height: 72
                    radius: theme.radiusMedium
                    color: theme.surfaceAlt
                    border.color: theme.border
                    border.width: 1

                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 10
                        spacing: 6
                        Repeater {
                            model: dashboardPage.weekDays
                            delegate: Rectangle {
                                Layout.fillWidth: true
                                Layout.fillHeight: true
                                radius: 10
                                color: modelData.isToday
                                       ? Qt.rgba(theme.tertiary.r, theme.tertiary.g, theme.tertiary.b, 0.15)
                                       : "transparent"
                                border.color: modelData.isToday ? theme.tertiary : "transparent"
                                border.width: 1

                                ColumnLayout {
                                    anchors.centerIn: parent
                                    spacing: 2
                                    Label {
                                        text: modelData.label
                                        color: theme.textMuted
                                        font.pixelSize: 10
                                        Layout.alignment: Qt.AlignHCenter
                                    }
                                    Label {
                                        text: "" + modelData.dayNum
                                        color: theme.textPrimary
                                        font.pixelSize: 15
                                        font.weight: Font.DemiBold
                                        Layout.alignment: Qt.AlignHCenter
                                    }
                                    Rectangle {
                                        width: 5; height: 5; radius: 2.5
                                        color: theme.tertiary
                                        visible: modelData.eventCount > 0
                                        Layout.alignment: Qt.AlignHCenter
                                    }
                                }
                                MouseArea {
                                    anchors.fill: parent
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: dashboardPage.goCalendarDay(modelData.firstDate)
                                }
                            }
                        }
                    }
                }

                // Summary chips
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 10
                    SummaryChip {
                        Layout.fillWidth: true
                        title: "Vault"
                        value: "" + dashboardPage.vaultCount
                        subtitle: "entries"
                        onClicked: dashboardPage.goVault()
                    }
                    SummaryChip {
                        Layout.fillWidth: true
                        title: "Notes"
                        value: "" + dashboardPage.notesCount
                        subtitle: "total"
                        onClicked: dashboardPage.goNotes()
                    }
                    SummaryChip {
                        Layout.fillWidth: true
                        title: "Overdue"
                        value: "" + dashboardPage.overdueCount
                        subtitle: "tasks"
                        accent: dashboardPage.overdueCount > 0 ? theme.danger : theme.tertiary
                        onClicked: dashboardPage.goTodo("overdue")
                    }
                }

                // Quick add
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 8
                    StyledTextField {
                        id: quickField
                        Layout.fillWidth: true
                        placeholderText: "Quick add task to Inbox…"
                        onAccepted: dashboardPage.quickAddTask()
                    }
                    Rectangle {
                        width: 40; height: 40; radius: 20
                        color: theme.tertiary
                        Label {
                            anchors.centerIn: parent
                            text: "+"
                            color: theme.onAccent
                            font.pixelSize: 20
                            font.bold: true
                        }
                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            onClicked: dashboardPage.quickAddTask()
                        }
                    }
                }

                // Priority + Action
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 16

                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredWidth: 2
                        Layout.minimumHeight: 200
                        implicitHeight: priorityCol.implicitHeight + 32
                        color: theme.surfaceAlt
                        border.color: theme.border
                        border.width: 1
                        radius: theme.radiusMedium

                        ColumnLayout {
                            id: priorityCol
                            anchors.left: parent.left
                            anchors.right: parent.right
                            anchors.top: parent.top
                            anchors.margins: 16
                            spacing: 10

                            RowLayout {
                                Layout.fillWidth: true
                                Label {
                                    text: "✓"
                                    color: theme.tertiary
                                    font.pixelSize: 16
                                    font.bold: true
                                }
                                Label {
                                    text: "Priority Tasks"
                                    color: theme.textPrimary
                                    font.family: theme.headlineFont
                                    font.pixelSize: 17
                                    font.weight: Font.DemiBold
                                }
                                Item { Layout.fillWidth: true }
                                Rectangle {
                                    radius: 999
                                    implicitHeight: 30
                                    implicitWidth: addLbl.implicitWidth + 20
                                    color: Qt.rgba(theme.tertiary.r, theme.tertiary.g, theme.tertiary.b, 0.18)
                                    Label {
                                        id: addLbl
                                        anchors.centerIn: parent
                                        text: "Open To-Do"
                                        color: theme.tertiary
                                        font.pixelSize: 12
                                    }
                                    MouseArea {
                                        anchors.fill: parent
                                        cursorShape: Qt.PointingHandCursor
                                        onClicked: dashboardPage.goTodo("")
                                    }
                                }
                            }

                            Repeater {
                                model: dashboardPage.openTasks
                                delegate: Rectangle {
                                    Layout.fillWidth: true
                                    height: 52
                                    radius: 10
                                    color: rowMa.containsMouse ? theme.hoverFill : "transparent"

                                    readonly property var proj: dashboardPage.projectMeta(
                                        modelData ? modelData.projectId : "")

                                    RowLayout {
                                        anchors.fill: parent
                                        anchors.leftMargin: 4
                                        anchors.rightMargin: 8
                                        spacing: 10

                                        Rectangle {
                                            width: 20; height: 20; radius: 6
                                            border.color: theme.tertiary
                                            border.width: 1.5
                                            color: "transparent"
                                            Layout.alignment: Qt.AlignVCenter
                                            MouseArea {
                                                anchors.fill: parent
                                                cursorShape: Qt.PointingHandCursor
                                                onClicked: {
                                                    if (modelData && taskController)
                                                        taskController.setDone(modelData.id, true)
                                                }
                                            }
                                        }

                                        ColumnLayout {
                                            Layout.fillWidth: true
                                            spacing: 2
                                            Label {
                                                text: modelData ? (modelData.title || "") : ""
                                                color: theme.textPrimary
                                                font.pixelSize: 14
                                                elide: Text.ElideRight
                                                Layout.fillWidth: true
                                            }
                                            RowLayout {
                                                spacing: 6
                                                Text {
                                                    text: (proj && proj.emoji) ? proj.emoji : "📥"
                                                    font.pixelSize: 11
                                                    font.family: "Noto Color Emoji"
                                                }
                                                Label {
                                                    text: (proj && proj.name) ? proj.name : "Inbox"
                                                    color: theme.textMuted
                                                    font.pixelSize: 11
                                                }
                                                Label {
                                                    visible: modelData && (modelData.dueAt || 0) > 0
                                                    text: {
                                                        if (!modelData) return ""
                                                        var d = modelData.dueAt
                                                        var now = Date.now() / 1000
                                                        if (d < now) return "· Overdue"
                                                        return "· Due " + Qt.formatDateTime(
                                                            new Date(d * 1000), "MMM d, h:mm AP")
                                                    }
                                                    color: modelData && (modelData.dueAt || 0) < Date.now() / 1000
                                                           ? theme.danger : theme.textMuted
                                                    font.pixelSize: 11
                                                }
                                            }
                                        }
                                    }

                                    MouseArea {
                                        id: rowMa
                                        anchors.fill: parent
                                        hoverEnabled: true
                                        z: -1
                                        cursorShape: Qt.PointingHandCursor
                                        onClicked: dashboardPage.goTodo("")
                                    }
                                }
                            }

                            Label {
                                visible: dashboardPage.openTasks.length === 0
                                text: "No open tasks. You're clear."
                                color: theme.textMuted
                                font.pixelSize: 13
                            }
                        }
                    }

                    Rectangle {
                        Layout.preferredWidth: 1
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        Layout.minimumWidth: 200
                        Layout.maximumWidth: 280
                        Layout.minimumHeight: 200
                        color: theme.surfaceAlt
                        border.color: theme.border
                        border.width: 1
                        radius: theme.radiusMedium

                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 16
                            spacing: 10

                            Rectangle {
                                Layout.fillWidth: true
                                height: 3
                                radius: 2
                                color: dashboardPage.overdueCount > 0 ? theme.danger : theme.tertiary
                            }

                            Label {
                                text: dashboardPage.overdueCount > 0 ? "Action Required" : "All clear"
                                color: theme.textPrimary
                                font.family: theme.headlineFont
                                font.pixelSize: 16
                                font.weight: Font.DemiBold
                            }

                            Item { Layout.fillHeight: true }

                            Label {
                                text: "" + dashboardPage.overdueCount
                                color: theme.textPrimary
                                font.pixelSize: 48
                                font.weight: Font.Bold
                                Layout.alignment: Qt.AlignHCenter
                                opacity: dashboardPage.overdueCount > 0 ? 1 : 0.4
                            }
                            Label {
                                text: dashboardPage.overdueCount === 0
                                      ? "No overdue tasks."
                                      : (dashboardPage.overdueCount === 1
                                         ? "task is overdue." : "tasks are overdue.")
                                color: theme.textSecondary
                                font.pixelSize: 13
                                horizontalAlignment: Text.AlignHCenter
                                Layout.alignment: Qt.AlignHCenter
                                Layout.fillWidth: true
                            }

                            Item { Layout.fillHeight: true }

                            Rectangle {
                                Layout.fillWidth: true
                                height: 36
                                radius: 10
                                border.color: theme.border
                                color: dashboardPage.overdueCount > 0
                                       ? Qt.rgba(theme.danger.r, theme.danger.g, theme.danger.b, 0.12)
                                       : "transparent"
                                Label {
                                    anchors.centerIn: parent
                                    text: "Review Actions"
                                    color: theme.textPrimary
                                    font.pixelSize: 12
                                    font.weight: Font.Medium
                                }
                                MouseArea {
                                    anchors.fill: parent
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: dashboardPage.goTodo(
                                        dashboardPage.overdueCount > 0 ? "overdue" : "")
                                }
                            }
                        }
                    }
                }

                // Schedule + Scratchpad (fixed equal height)
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 16

                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredWidth: 7
                        Layout.preferredHeight: 260
                        Layout.minimumHeight: 260
                        color: theme.surfaceAlt
                        border.color: theme.border
                        border.width: 1
                        radius: theme.radiusMedium

                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 16
                            spacing: 10

                            RowLayout {
                                Layout.fillWidth: true
                                Label {
                                    text: "📅"
                                    font.family: "Noto Color Emoji"
                                    font.pixelSize: 16
                                }
                                Label {
                                    text: "Schedule"
                                    color: theme.textPrimary
                                    font.family: theme.headlineFont
                                    font.pixelSize: 17
                                    font.weight: Font.DemiBold
                                }
                                Item { Layout.fillWidth: true }
                                Label {
                                    visible: dashboardPage.nextEventMinutes() >= 0
                                    text: "Next: " + dashboardPage.nextEventMinutes() + "m"
                                    color: theme.textMuted
                                    font.pixelSize: 11
                                }
                            }

                            Flickable {
                                Layout.fillWidth: true
                                Layout.fillHeight: true
                                clip: true
                                contentHeight: scheduleInner.implicitHeight
                                boundsBehavior: Flickable.StopAtBounds

                                ColumnLayout {
                                    id: scheduleInner
                                    width: parent.width
                                    spacing: 8

                                    Repeater {
                                        model: dashboardPage.upcomingEvents
                                        delegate: Rectangle {
                                            Layout.fillWidth: true
                                            height: 56
                                            radius: 10
                                            color: index === 0
                                                   ? Qt.rgba(theme.tertiary.r, theme.tertiary.g, theme.tertiary.b, 0.08)
                                                   : "transparent"
                                            border.color: index === 0 ? theme.border : "transparent"
                                            border.width: 1

                                            RowLayout {
                                                anchors.fill: parent
                                                anchors.margins: 10
                                                spacing: 12
                                                Label {
                                                    text: modelData && modelData.start !== undefined
                                                          ? Qt.formatDateTime(new Date(modelData.start), "h:mm AP")
                                                          : ""
                                                    color: theme.textSecondary
                                                    font.pixelSize: 12
                                                    Layout.preferredWidth: 72
                                                }
                                                Rectangle {
                                                    width: 8; height: 8; radius: 4
                                                    color: index === 0 ? theme.tertiary : theme.textMuted
                                                }
                                                ColumnLayout {
                                                    Layout.fillWidth: true
                                                    spacing: 2
                                                    Label {
                                                        text: modelData ? (modelData.title || "") : ""
                                                        color: theme.textPrimary
                                                        font.pixelSize: 14
                                                        elide: Text.ElideRight
                                                        Layout.fillWidth: true
                                                    }
                                                    Label {
                                                        text: modelData ? (modelData.description || "") : ""
                                                        color: theme.textMuted
                                                        font.pixelSize: 12
                                                        visible: modelData && (modelData.description || "").length > 0
                                                        elide: Text.ElideRight
                                                        Layout.fillWidth: true
                                                    }
                                                }
                                            }
                                            MouseArea {
                                                anchors.fill: parent
                                                cursorShape: Qt.PointingHandCursor
                                                onClicked: dashboardPage.goCalendar()
                                            }
                                        }
                                    }

                                    Label {
                                        visible: dashboardPage.upcomingEvents.length === 0
                                        text: "Nothing on the calendar yet."
                                        color: theme.textMuted
                                        font.pixelSize: 13
                                        Layout.topMargin: 8
                                    }
                                }
                            }
                        }
                    }

                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredWidth: 5
                        Layout.preferredHeight: 260
                        Layout.minimumHeight: 260
                        color: theme.surfaceAlt
                        border.color: theme.border
                        border.width: 1
                        radius: theme.radiusMedium

                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 16
                            spacing: 8

                            RowLayout {
                                Layout.fillWidth: true
                                Label {
                                    text: "📝"
                                    font.family: "Noto Color Emoji"
                                    font.pixelSize: 16
                                }
                                Label {
                                    text: "Scratchpad"
                                    color: theme.textPrimary
                                    font.family: theme.headlineFont
                                    font.pixelSize: 17
                                    font.weight: Font.DemiBold
                                }
                                Item { Layout.fillWidth: true }
                                Label {
                                    text: "↗"
                                    color: theme.textMuted
                                    MouseArea {
                                        anchors.fill: parent
                                        anchors.margins: -6
                                        cursorShape: Qt.PointingHandCursor
                                        onClicked: dashboardPage.goNotes()
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
                                font.pixelSize: 13
                                wrapMode: Text.WordWrap
                                selectByMouse: true
                                padding: 10
                                background: Rectangle {
                                    color: theme.surface
                                    border.color: theme.border
                                    radius: 10
                                }
                                property string noteId: ""
                                property bool loading: true
                                property string lastSaved: ""
                                function load() {
                                    loading = true
                                    if (typeof noteController === "undefined" || !noteController) {
                                        noteId = ""
                                        text = ""
                                        lastSaved = ""
                                        loading = false
                                        return
                                    }
                                    var all = noteController.entries() || []
                                    for (var i = 0; i < all.length; i++) {
                                        if (all[i].title === "Scratchpad") {
                                            noteId = all[i].id
                                            text = all[i].content || ""
                                            lastSaved = text
                                            loading = false
                                            return
                                        }
                                    }
                                    noteId = ""
                                    text = ""
                                    lastSaved = ""
                                    loading = false
                                }
                                // Flush immediately instead of waiting for the
                                // debounce, so edits are never lost to a quick
                                // navigation, focus click-away or app exit.
                                function persist() {
                                    if (typeof noteController === "undefined" || !noteController)
                                        return
                                    var t = scratchpadArea.text
                                    if (t === scratchpadArea.lastSaved)
                                        return
                                    if (scratchpadArea.noteId.length > 0) {
                                        if (noteController.updateEntry(
                                                scratchpadArea.noteId, "Scratchpad", t))
                                            scratchpadArea.lastSaved = t
                                    } else if (t.trim().length > 0) {
                                        noteController.addEntry("Scratchpad", t)
                                        scratchpadArea.load()
                                    }
                                }
                                Component.onCompleted: load()
                                onActiveFocusChanged: if (!activeFocus && !loading) persist()
                                onTextChanged: if (!loading) saveTimer.restart()
                                Component.onDestruction: persist()
                                Timer {
                                    id: saveTimer
                                    interval: 800
                                    onTriggered: scratchpadArea.persist()
                                }
                            }

                            Label {
                                text: "Recent notes"
                                color: theme.textMuted
                                font.pixelSize: 11
                                visible: dashboardPage.recentNotes.length > 0
                            }
                            Repeater {
                                model: dashboardPage.recentNotes
                                delegate: Rectangle {
                                    Layout.fillWidth: true
                                    height: 28
                                    radius: 6
                                    color: nMa.containsMouse ? theme.hoverFill : "transparent"
                                    Label {
                                        anchors.verticalCenter: parent.verticalCenter
                                        anchors.left: parent.left
                                        anchors.leftMargin: 6
                                        anchors.right: parent.right
                                        anchors.rightMargin: 6
                                        text: modelData ? (modelData.title || "Untitled") : ""
                                        color: theme.textSecondary
                                        font.pixelSize: 12
                                        elide: Text.ElideRight
                                    }
                                    MouseArea {
                                        id: nMa
                                        anchors.fill: parent
                                        hoverEnabled: true
                                        cursorShape: Qt.PointingHandCursor
                                        onClicked: dashboardPage.goNotes()
                                    }
                                }
                            }
                        }
                    }
                }

                Item { Layout.preferredHeight: 24 }
            }
        }
    }

    component SummaryChip: Rectangle {
        property string title: ""
        property string value: "0"
        property string subtitle: ""
        property color accent: theme.tertiary
        signal clicked()

        height: 64
        radius: theme.radiusMedium
        color: theme.surfaceAlt
        border.color: theme.border
        border.width: 1

        RowLayout {
            anchors.fill: parent
            anchors.margins: 12
            spacing: 10
            ColumnLayout {
                spacing: 2
                Layout.fillWidth: true
                Label {
                    text: title
                    color: theme.textMuted
                    font.pixelSize: 11
                }
                Label {
                    text: value
                    color: theme.textPrimary
                    font.pixelSize: 20
                    font.weight: Font.DemiBold
                }
            }
            Label {
                text: subtitle
                color: theme.textMuted
                font.pixelSize: 11
            }
        }
        MouseArea {
            anchors.fill: parent
            cursorShape: Qt.PointingHandCursor
            onClicked: parent.clicked()
        }
    }
}