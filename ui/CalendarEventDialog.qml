import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Popup {
    id: root

    Theme { id: theme }

    property string editingId: ""
    readonly property bool isEditing: editingId.length > 0
    property date targetDate: new Date()

    readonly property var colorSwatches: [
        "#2E8B57", "#4F9F4F", "#E0A15C", "#E0574C", "#5C8FE0",
        "#cba6f7", "#f38ba8", "#94e2d5"
    ]
    property string selectedColor: colorSwatches[0]

    modal: true
    focus: true
    x: (parent ? parent.width - width : 0) / 2
    y: (parent ? parent.height - height : 0) / 2
    width: 360
    padding: 20
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

    background: Rectangle {
        color: theme.surface
        radius: theme.radiusMedium
        border.width: 1
        border.color: theme.border
    }

    function openForCreate(onDate) {
        editingId = ""
        targetDate = onDate
        titleField.text = ""
        descriptionField.text = ""
        startTimeField.text = "09:00"
        endTimeField.text = "10:00"
        allDaySwitch.checked = false
        selectedColor = colorSwatches[0]
        errorLabel.text = ""
        open()
    }

    function openForEdit(entry) {
        editingId = entry.id
        var start = new Date(entry.start)
        targetDate = start
        titleField.text = entry.title
        descriptionField.text = entry.description || ""
        startTimeField.text = Qt.formatTime(start, "hh:mm")
        endTimeField.text = Qt.formatTime(new Date(entry.end), "hh:mm")
        allDaySwitch.checked = entry.allDay
        selectedColor = entry.color || colorSwatches[0]
        errorLabel.text = ""
        open()
    }

    function combine(dateObj, hhmm) {
        var parts = (hhmm || "00:00").split(":")
        var h = parseInt(parts[0], 10)
        var m = parseInt(parts[1], 10)
        if (isNaN(h)) h = 0
        if (isNaN(m)) m = 0
        var d = new Date(dateObj)
        d.setHours(h, m, 0, 0)
        return d
    }

    function isValidTime(hhmm) {
        var re = /^([01]?\d|2[0-3]):([0-5]\d)$/
        return re.test((hhmm || "").trim())
    }

    ColumnLayout {
        width: parent.width
        spacing: 12

        Label {
            text: root.isEditing ? "Edit event" : "New event"
            color: theme.textPrimary
            font.family: theme.headlineFont
            font.pixelSize: 18
            font.bold: true
        }

        Label {
            text: Qt.formatDate(root.targetDate, "dddd, MMMM d, yyyy")
            color: theme.textSecondary
            font.pixelSize: 12
        }

        StyledTextField {
            id: titleField
            placeholderText: "Event title"
            Layout.fillWidth: true
        }
        StyledTextField {
            id: descriptionField
            placeholderText: "Description / location (optional)"
            Layout.fillWidth: true
        }

        RowLayout {
            spacing: 8
            Switch { id: allDaySwitch }
            Label { text: "All day"; color: theme.textSecondary; font.pixelSize: 13 }
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 8
            visible: !allDaySwitch.checked
            StyledTextField {
                id: startTimeField
                placeholderText: "Start (hh:mm)"
                Layout.fillWidth: true
            }
            StyledTextField {
                id: endTimeField
                placeholderText: "End (hh:mm)"
                Layout.fillWidth: true
            }
        }

        RowLayout {
            spacing: 8
            Repeater {
                model: root.colorSwatches
                delegate: Rectangle {
                    width: 22
                    height: 22
                    radius: 11
                    color: modelData
                    border.width: modelData === root.selectedColor ? 2 : 0
                    border.color: theme.textPrimary
                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: root.selectedColor = modelData
                    }
                }
            }
        }

        Label {
            id: errorLabel
            color: theme.danger
            font.pixelSize: 12
            visible: text.length > 0
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.topMargin: 4
            spacing: 8

            Item { Layout.fillWidth: true }

            // Cancel
            Rectangle {
                width: 36; height: 36; radius: 8
                color: cancelMa.containsMouse ? Qt.rgba(1,1,1,0.08) : "transparent"
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
                    onClicked: root.close()
                }
                ToolTip.visible: cancelMa.containsMouse
                ToolTip.text: "Cancel"
                ToolTip.delay: 400
            }

            // Save / Create
            Rectangle {
                width: 36; height: 36; radius: 8
                color: saveMa.containsMouse ? theme.tertiary
                                            : Qt.rgba(theme.tertiary.r, theme.tertiary.g, theme.tertiary.b, 0.85)
                Text {
                    anchors.centerIn: parent
                    text: "✓"
                    color: "#0A140A"
                    font.pixelSize: 16
                    font.bold: true
                }
                MouseArea {
                    id: saveMa
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        if (titleField.text.trim().length === 0) {
                            errorLabel.text = "Title is required."
                            return
                        }
                        if (!allDaySwitch.checked) {
                            if (!root.isValidTime(startTimeField.text) || !root.isValidTime(endTimeField.text)) {
                                errorLabel.text = "Use time format HH:mm (e.g. 09:30)."
                                return
                            }
                            var startD = root.combine(root.targetDate, startTimeField.text)
                            var endD = root.combine(root.targetDate, endTimeField.text)
                            if (endD.getTime() <= startD.getTime()) {
                                errorLabel.text = "End time must be after start time."
                                return
                            }
                        }

                        var startDate = allDaySwitch.checked
                                      ? root.targetDate
                                      : root.combine(root.targetDate, startTimeField.text)
                        var endDate = allDaySwitch.checked
                                    ? root.targetDate
                                    : root.combine(root.targetDate, endTimeField.text)
                        var startIso = Qt.formatDateTime(startDate, "yyyy-MM-ddThh:mm:ss")
                        var endIso = Qt.formatDateTime(endDate, "yyyy-MM-ddThh:mm:ss")

                        var ok
                        if (root.isEditing) {
                            ok = calendarController.updateEntry(
                                root.editingId, titleField.text, descriptionField.text,
                                startIso, endIso, allDaySwitch.checked, root.selectedColor)
                        } else {
                            ok = calendarController.addEntry(
                                titleField.text, descriptionField.text,
                                startIso, endIso, allDaySwitch.checked, root.selectedColor)
                        }

                        if (ok) root.close()
                        else errorLabel.text = "Could not save. Is the calendar unlocked?"
                    }
                }
                ToolTip.visible: saveMa.containsMouse
                ToolTip.text: root.isEditing ? "Save" : "Create"
                ToolTip.delay: 400
            }
        }
    }
}