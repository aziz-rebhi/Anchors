import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Item {
    id: root
    property string blockId: ""
    property int orientation: 0  // 0 = horizontal, 1 = vertical

    width: parent ? parent.width : 0
    height: root.orientation === 0 ? 28 : 200

    function focusInput() {
        // Non-text block — insert a paragraph after it
        noteEditor.insertBlockAfter(root.blockId, 0, "")
    }

    // === Horizontal divider ===
    Rectangle {
        visible: root.orientation === 0
        anchors.verticalCenter: parent.verticalCenter
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: 8
        height: 2
        color: "#555555"
        radius: 1
    }

    // === Vertical divider (Notion-style column split) ===
    Row {
        visible: root.orientation === 1
        anchors.fill: parent
        anchors.margins: 4
        spacing: 0

        // Left column placeholder
        Rectangle {
            width: (parent.width - 6) / 2
            height: parent.height
            color: "#3a3a4a"
            radius: 6
            border.color: "#555555"
            border.width: 1

            Text {
                anchors.centerIn: parent
                text: "Column 1"
                font.pixelSize: 13
                color: "#888888"
            }
        }

        // Vertical line
        Rectangle {
            width: 6
            height: parent.height
            color: "#555555"
            radius: 3

            // Drag handle indicator
            Rectangle {
                anchors.centerIn: parent
                width: 2
                height: 30
                color: "#777777"
                radius: 1
            }
        }

        // Right column placeholder
        Rectangle {
            width: (parent.width - 6) / 2
            height: parent.height
            color: "#3a3a4a"
            radius: 6
            border.color: "#555555"
            border.width: 1

            Text {
                anchors.centerIn: parent
                text: "Column 2"
                font.pixelSize: 13
                color: "#888888"
            }
        }
    }

    // Toggle button on hover
    Rectangle {
        id: toggleBtn
        anchors.right: parent.right
        anchors.verticalCenter: root.orientation === 0 ? parent.verticalCenter : undefined
        anchors.top: root.orientation === 1 ? parent.top : undefined
        anchors.margins: 2
        width: 26
        height: 26
        radius: 6
        color: toggleArea.containsMouse ? "#4a4a5a" : "transparent"
        opacity: toggleArea.containsMouse ? 1.0 : 0.0
        Behavior on opacity { NumberAnimation { duration: 150 } }

        Text {
            anchors.centerIn: parent
            text: root.orientation === 0 ? "\u21C4" : "\u21C5"
            font.pixelSize: 14
            color: "#aaaaaa"
        }

        MouseArea {
            id: toggleArea
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: Qt.PointingHandCursor
            onClicked: {
                var newOrientation = root.orientation === 0 ? 1 : 0
                root.orientation = newOrientation
                if (noteEditor) {
                    noteEditor.updateBlockDividerOrientation(root.blockId, newOrientation)
                }
            }
        }

        ToolTip.visible: toggleArea.containsMouse
        ToolTip.text: root.orientation === 0 ? "Switch to vertical divider" : "Switch to horizontal divider"
        ToolTip.delay: 500
    }

    // Key handling at the Item level
    Keys.onPressed: function(event) {
        if ((event.modifiers & Qt.ControlModifier) && event.key === Qt.Key_Z) {
            if (event.modifiers & Qt.ShiftModifier)
                noteEditor.redo()
            else
                noteEditor.undo()
            event.accepted = true
        } else if (event.key === Qt.Key_Enter || event.key === Qt.Key_Return) {
            noteEditor.insertBlockAfter(root.blockId, 0, "")
            event.accepted = true
        } else if (event.key === Qt.Key_Backspace || event.key === Qt.Key_Delete) {
            noteEditor.deleteBlock(root.blockId)
            event.accepted = true
        }
    }
}