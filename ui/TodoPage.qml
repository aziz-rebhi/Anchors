import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Page {
    id: root
    Theme { id: theme }

    property var allTasks: []
    property var allProjects: []
    property string currentProjectId: ""

    readonly property var colorPalette: [
        "#f38ba8", "#fab387", "#f9e2af", "#a6e3a1",
        "#89b4fa", "#cba6f7", "#94e2d5", "#6c7086"
    ]
    readonly property var emojiChoices: [
        "📁", "💼", "🏠", "📚", "🎯", "💪", "🛒", "💡",
        "🎮", "✈️", "❤️", "🔧", "📝", "⭐", "🎵", "🧠"
    ]

    function refresh() {
        allTasks = taskController.entries()
        allProjects = taskController.projects()
    }

    Component.onCompleted: refresh()

    Connections {
        target: taskController
        function onEntriesChanged() { root.refresh() }
        function onProjectsChanged() { root.refresh() }
    }

    function tasksForProject(pid) {
        return allTasks.filter(function (t) {
            var p = t.projectId || ""
            return p === (pid || "")
        })
    }

    readonly property var currentTasks: tasksForProject(currentProjectId)
    readonly property var openTasks: currentTasks.filter(function (t) { return !t.done })
    readonly property var completedTasks: currentTasks.filter(function (t) { return t.done })

    readonly property var inboxCount: allTasks.filter(function (t) {
        return !t.done && (!t.projectId || t.projectId === "")
    }).length

    function projectOpenCount(pid) {
        return allTasks.filter(function (t) {
            return !t.done && (t.projectId || "") === (pid || "")
        }).length
    }

    function currentProjectMeta() {
        if (!currentProjectId || currentProjectId === "")
            return { name: "Inbox", emoji: "📥", color: theme.tertiary }
        for (var i = 0; i < allProjects.length; i++) {
            if (allProjects[i].id === currentProjectId)
                return allProjects[i]
        }
        return { name: "Inbox", emoji: "📥", color: theme.tertiary }
    }

    background: Rectangle { color: theme.background }

    RowLayout {
        anchors.fill: parent
        spacing: 0

        Rectangle {
            Layout.preferredWidth: 240
            Layout.fillHeight: true
            color: theme.surface
            border.width: 1
            border.color: theme.border

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 12
                spacing: 8

                Label {
                    text: "To-Do"
                    color: theme.textPrimary
                    font.family: theme.headlineFont
                    font.pixelSize: 20
                    font.bold: true
                }

                ProjectRow {
                    Layout.fillWidth: true
                    emoji: "📥"
                    title: "Inbox"
                    accent: theme.tertiary
                    count: root.inboxCount
                    selected: root.currentProjectId === ""
                    onClicked: root.currentProjectId = ""
                }

                RowLayout {
                    Layout.fillWidth: true
                    Layout.topMargin: 8
                    spacing: 4

                    Label {
                        text: "PROJECTS"
                        color: theme.textMuted
                        font.pixelSize: 10
                        Layout.fillWidth: true
                    }

                    Rectangle {
                        width: 26
                        height: 26
                        radius: 6
                        color: newProjMa.containsMouse ? theme.hoverFill : "transparent"

                        Text {
                            anchors.centerIn: parent
                            text: "+"
                            color: theme.textSecondary
                            font.pixelSize: 16
                            font.bold: true
                        }
                        MouseArea {
                            id: newProjMa
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                projectDialog.editId = ""
                                projectDialog.editName = ""
                                projectDialog.editEmoji = "📁"
                                projectDialog.editColor = root.colorPalette[4]
                                projectDialog.open()
                            }
                        }
                        ToolTip.visible: newProjMa.containsMouse
                        ToolTip.text: "New project"
                        ToolTip.delay: 400
                    }
                }

                ListView {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    clip: true
                    model: root.allProjects
                    spacing: 2
                    delegate: ProjectRow {
                        width: ListView.view ? ListView.view.width : 200
                        emoji: modelData.emoji || "📁"
                        title: modelData.name
                        accent: modelData.color || theme.tertiary
                        count: root.projectOpenCount(modelData.id)
                        selected: root.currentProjectId === modelData.id
                        onClicked: root.currentProjectId = modelData.id
                        onPressAndHold: {
                            projectDialog.editId = modelData.id
                            projectDialog.editName = modelData.name
                            projectDialog.editEmoji = modelData.emoji
                            projectDialog.editColor = modelData.color
                            projectDialog.open()
                        }
                    }
                }
            }
        }

        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 0

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 64
                color: theme.background

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 24
                    anchors.rightMargin: 24
                    spacing: 12

                    Rectangle {
                        width: 4
                        height: 28
                        radius: 2
                        color: root.currentProjectMeta().color || theme.tertiary
                    }
                    Text {
                        text: (root.currentProjectMeta().emoji || "") + " "
                        font.pixelSize: 22
                        font.family: "Noto Color Emoji"
                    }
                    Label {
                        text: root.currentProjectMeta().name || "Inbox"
                        color: theme.textPrimary
                        font.family: theme.headlineFont
                        font.pixelSize: 24
                        font.bold: true
                    }
                    Item { Layout.fillWidth: true }
                    Label {
                        text: root.openTasks.length + " active"
                        color: theme.textMuted
                        font.pixelSize: 12
                    }

                    Rectangle {
                        visible: root.currentProjectId.length > 0
                        width: 32
                        height: 32
                        radius: 8
                        color: editMa.containsMouse ? theme.hoverFill : "transparent"

                        Text {
                            anchors.centerIn: parent
                            text: "✎"
                            color: theme.textSecondary
                            font.pixelSize: 14
                        }
                        MouseArea {
                            id: editMa
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                var p = root.currentProjectMeta()
                                projectDialog.editId = root.currentProjectId
                                projectDialog.editName = p.name
                                projectDialog.editEmoji = p.emoji
                                projectDialog.editColor = p.color
                                projectDialog.open()
                            }
                        }
                        ToolTip.visible: editMa.containsMouse
                        ToolTip.text: "Edit project"
                        ToolTip.delay: 400
                    }
                }
            }

            RowLayout {
                Layout.fillWidth: true
                Layout.leftMargin: 24
                Layout.rightMargin: 24
                Layout.topMargin: 8
                spacing: 8

                StyledTextField {
                    id: newTaskField
                    Layout.fillWidth: true
                    placeholderText: "Add a task in " + (root.currentProjectMeta().name || "Inbox") + "..."
                    onAccepted: addBtnMa.clicked()
                }

                Rectangle {
                    Layout.preferredWidth: 40
                    Layout.preferredHeight: 40
                    radius: 20
                    color: theme.tertiary

                    Text {
                        anchors.centerIn: parent
                        text: "+"
                        color: theme.onAccent
                        font.pixelSize: 20
                        font.bold: true
                    }
                    MouseArea {
                        id: addBtnMa
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            var title = newTaskField.text.trim()
                            if (!title.length) return
                            taskController.addEntry(title, 0, root.currentProjectId)
                            newTaskField.text = ""
                        }
                    }
                    ToolTip.visible: addBtnMa.containsMouse
                    ToolTip.text: "Add task"
                    ToolTip.delay: 400
                }
            }

            ScrollView {
                id: taskScroll
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.margins: 16
                clip: true
                contentWidth: availableWidth

                ColumnLayout {
                    id: taskColumn
                    width: taskScroll.availableWidth > 0 ? taskScroll.availableWidth : 400
                    spacing: 6

                    Label {
                        text: "Active"
                        color: theme.textMuted
                        font.pixelSize: 11
                        visible: root.openTasks.length > 0
                    }

                    Repeater {
                        model: root.openTasks
                        delegate: TaskRow {
                            Layout.fillWidth: true
                            width: taskColumn.width
                            taskId: modelData.id
                            title: modelData.title || ""
                            done: false
                            accent: root.currentProjectMeta().color || theme.tertiary
                            onToggle: taskController.setDone(taskId, true)
                            onRemove: taskController.deleteEntry(taskId)
                        }
                    }

                    Label {
                        text: "No active tasks"
                        color: theme.textMuted
                        font.pixelSize: 13
                        visible: root.openTasks.length === 0
                        Layout.topMargin: 12
                    }

                    Label {
                        text: "Completed"
                        color: theme.textMuted
                        font.pixelSize: 11
                        Layout.topMargin: 16
                        visible: root.completedTasks.length > 0
                    }

                    Repeater {
                        model: root.completedTasks
                        delegate: TaskRow {
                            Layout.fillWidth: true
                            width: taskColumn.width
                            taskId: modelData.id
                            title: modelData.title || ""
                            done: true
                            accent: root.currentProjectMeta().color || theme.tertiary
                            onToggle: taskController.setDone(taskId, false)
                            onRemove: taskController.deleteEntry(taskId)
                        }
                    }

                    Item { Layout.preferredHeight: 24 }
                }
            }
        }
    }

    Dialog {
        id: projectDialog
        property string editId: ""
        property string editName: ""
        property string editEmoji: "📁"
        property string editColor: "#89b4fa"
        property bool isEdit: editId.length > 0

        title: isEdit ? "Edit project" : "New project"
        modal: true
        anchors.centerIn: parent
        width: 340

        background: Rectangle {
            color: theme.surfaceAlt
            radius: 12
            border.color: theme.border
        }

        ColumnLayout {
            anchors.margins: 16
            anchors.fill: parent
            spacing: 12

            StyledTextField {
                id: nameField
                Layout.fillWidth: true
                placeholderText: "Project name"
                text: projectDialog.editName
            }

            Label { text: "Emoji"; color: theme.textSecondary; font.pixelSize: 11 }
            Flow {
                Layout.fillWidth: true
                spacing: 6
                Repeater {
                    model: root.emojiChoices
                    delegate: Rectangle {
                        width: 32; height: 32; radius: 6
                        color: projectDialog.editEmoji === modelData ? theme.surface : "transparent"
                        border.color: projectDialog.editEmoji === modelData ? theme.tertiary : "transparent"
                        border.width: 1
                        Text {
                            anchors.centerIn: parent
                            text: modelData
                            font.pixelSize: 16
                            font.family: "Noto Color Emoji"
                        }
                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            onClicked: projectDialog.editEmoji = modelData
                        }
                    }
                }
            }

            Label { text: "Color"; color: theme.textSecondary; font.pixelSize: 11 }
            Flow {
                Layout.fillWidth: true
                spacing: 8
                Repeater {
                    model: root.colorPalette
                    delegate: Rectangle {
                        width: 28; height: 28; radius: 14
                        color: modelData
                        border.color: projectDialog.editColor === modelData ? theme.textPrimary : "transparent"
                        border.width: 2
                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            onClicked: projectDialog.editColor = modelData
                        }
                    }
                }
            }

            RowLayout {
                Layout.fillWidth: true
                Layout.topMargin: 4
                spacing: 8

                Rectangle {
                    visible: projectDialog.isEdit
                    width: 36
                    height: 36
                    radius: 8
                    color: delMa.containsMouse ? Qt.rgba(0.9, 0.2, 0.2, 0.2) : "transparent"

                    Text {
                        anchors.centerIn: parent
                        text: "🗑"
                        font.pixelSize: 14
                        font.family: "Noto Color Emoji"
                    }
                    MouseArea {
                        id: delMa
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            taskController.deleteProject(projectDialog.editId)
                            if (root.currentProjectId === projectDialog.editId)
                                root.currentProjectId = ""
                            projectDialog.close()
                        }
                    }
                    ToolTip.visible: delMa.containsMouse
                    ToolTip.text: "Delete project"
                    ToolTip.delay: 400
                }

                Item { Layout.fillWidth: true }

                Rectangle {
                    width: 36
                    height: 36
                    radius: 8
                    color: cancelMa.containsMouse ? theme.hoverFill : "transparent"

                    Text {
                        anchors.centerIn: parent
                        text: "✕"
                        color: theme.textSecondary
                        font.pixelSize: 14
                    }
                    MouseArea {
                        id: cancelMa
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: projectDialog.close()
                    }
                }

                Rectangle {
                    width: 36
                    height: 36
                    radius: 8
                    color: theme.tertiary

                    Text {
                        anchors.centerIn: parent
                        text: "✓"
                        color: theme.onAccent
                        font.pixelSize: 16
                        font.bold: true
                    }
                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            var n = nameField.text.trim()
                            if (!n.length) return
                            if (projectDialog.isEdit)
                                taskController.updateProject(projectDialog.editId, n,
                                                             projectDialog.editEmoji, projectDialog.editColor)
                            else
                                taskController.addProject(n, projectDialog.editEmoji, projectDialog.editColor)
                            projectDialog.close()
                        }
                    }
                }
            }
        }

        onOpened: {
            nameField.text = editName
            nameField.forceActiveFocus()
        }
    }

    component ProjectRow: Rectangle {
        property string emoji: ""
        property string title: ""
        property string accent: "#89b4fa"
        property int count: 0
        property bool selected: false
        signal clicked()
        signal pressAndHold()

        height: 36
        implicitHeight: 36
        radius: 8
        color: selected ? theme.hoverFill
                        : (ma.containsMouse ? Qt.rgba(1, 1, 1, 0.04) : "transparent")

        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 8
            anchors.rightMargin: 8
            spacing: 8
            Rectangle { width: 4; height: 18; radius: 2; color: accent }
            Text {
                text: emoji
                font.pixelSize: 14
                font.family: "Noto Color Emoji"
            }
            Label {
                text: title
                color: selected ? theme.textPrimary : theme.textSecondary
                font.pixelSize: 13
                Layout.fillWidth: true
                elide: Text.ElideRight
            }
            Label {
                text: count > 0 ? ("" + count) : ""
                color: theme.textMuted
                font.pixelSize: 11
            }
        }
        MouseArea {
            id: ma
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: Qt.PointingHandCursor
            onClicked: parent.clicked()
            onPressAndHold: parent.pressAndHold()
        }
    }

    component TaskRow: Item {
        id: taskRow
        property string taskId: ""
        property string title: ""
        property bool done: false
        property string accent: "#89b4fa"
        signal toggle()
        signal remove()

        width: parent ? parent.width : 400
        implicitWidth: width
        height: 44
        implicitHeight: 44

        Rectangle {
            anchors.fill: parent
            radius: 8
            color: rowMa.containsMouse ? theme.hoverFill : "transparent"
        }

        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 10
            anchors.rightMargin: 10
            spacing: 12

            Rectangle {
                Layout.preferredWidth: 22
                Layout.preferredHeight: 22
                Layout.alignment: Qt.AlignVCenter
                radius: 6
                border.width: taskRow.done ? 0 : 1
                border.color: taskRow.accent
                color: taskRow.done ? taskRow.accent : "transparent"

                Text {
                    anchors.centerIn: parent
                    text: taskRow.done ? "✓" : ""
                    color: theme.onAccent
                    font.pixelSize: 12
                    font.bold: true
                }
                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: taskRow.toggle()
                }
            }

            Label {
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignVCenter
                text: taskRow.title
                color: taskRow.done ? theme.textMuted : theme.textPrimary
                font.strikeout: taskRow.done
                font.pixelSize: 14
                elide: Text.ElideRight
            }

            Text {
                Layout.alignment: Qt.AlignVCenter
                text: "✕"
                color: theme.textMuted
                font.pixelSize: 12
                opacity: rowMa.containsMouse ? 0.9 : 0
                Behavior on opacity { NumberAnimation { duration: 80 } }
                MouseArea {
                    anchors.fill: parent
                    anchors.margins: -8
                    cursorShape: Qt.PointingHandCursor
                    onClicked: taskRow.remove()
                }
            }
        }

        MouseArea {
            id: rowMa
            anchors.fill: parent
            hoverEnabled: true
            acceptedButtons: Qt.NoButton
            z: -1
        }
    }
}