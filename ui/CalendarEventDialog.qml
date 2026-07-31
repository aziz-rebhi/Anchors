import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Popup {
    id: root

    Theme { id: theme }

    property string editingId: ""
    readonly property bool isEditing: editingId.length > 0
    property date targetDate: new Date()

    readonly property var colorSwatches: ["#2E8B57", "#4F9F4F", "#E0A15C", "#E0574C", "#5C8FE0"]
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
        descriptionField.text = entry.description
        startTimeField.text = Qt.formatTime(start, "hh:mm")
        endTimeField.text = Qt.formatTime(new Date(entry.end), "hh:mm")
        allDaySwitch.checked = entry.allDay
        selectedColor = entry.color
        errorLabel.text = ""
        open()
    }

    function combine(dateObj, hhmm) {
        var parts = hhmm.split(":")
        var h = parseInt(parts[0], 10) || 0
        var m = parseInt(parts[1], 10) || 0
        var d = new Date(dateObj)
        d.setHours(h, m, 0, 0)
        return d
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
            font.family: theme.bodyFont
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
            Switch {
                id: allDaySwitch
            }
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

            PrimaryButton {
                text: "Cancel"
                outlined: true
                Layout.fillWidth: true
                onClicked: root.close()
            }
            PrimaryButton {
                text: root.isEditing ? "Save" : "Create"
                Layout.fillWidth: true
                onClicked: {
                    if (titleField.text.trim().length === 0) {
                        errorLabel.text = "Title is required."
                        return
                    }

                    var startD = allDaySwitch.checked ? root.targetDate : root.combine(root.targetDate, startTimeField.text)
                    var endD = allDaySwitch.checked ? root.targetDate : root.combine(root.targetDate, endTimeField.text)
                    var startIso = Qt.formatDateTime(startD, "yyyy-MM-ddThh:mm:ss")
                    var endIso = Qt.formatDateTime(endD, "yyyy-MM-ddThh:mm:ss")

                    var ok
                    if (root.isEditing) {
                        ok = calendarController.updateEntry(root.editingId, titleField.text, descriptionField.text,
                                                              startIso, endIso, allDaySwitch.checked, root.selectedColor)
                    } else {
                        ok = calendarController.addEntry(titleField.text, descriptionField.text,
                                                           startIso, endIso, allDaySwitch.checked, root.selectedColor)
                    }

                    if (ok) {
                        root.close()
                    } else {
                        errorLabel.text = "Could not save. Is the calendar unlocked?"
                    }
                }
            }
        }
    }
}
