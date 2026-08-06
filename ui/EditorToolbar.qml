import QtQuick 2.15
import QtQuick.Controls 2.15

Rectangle {
    id: root
    height: 40
    color: "transparent"
    property string currentBlockId: ""

    signal insertType(int typeCode)
    signal changeType(int typeCode)

    Row {
        anchors.verticalCenter: parent.verticalCenter
        anchors.left: parent.left
        anchors.leftMargin: 4
        spacing: 2

        // Text
        ToolbarButton {
            label: "T"
            tooltip: "Text (Paragraph)"
            onClicked: root.insertType(0)
        }

        // H1
        ToolbarButton {
            label: "H1"
            tooltip: "Heading 1"
            onClicked: root.insertType(1)
        }

        // H2
        ToolbarButton {
            label: "H2"
            tooltip: "Heading 2"
            onClicked: root.insertType(2)
        }

        // H3
        ToolbarButton {
            label: "H3"
            tooltip: "Heading 3"
            onClicked: root.insertType(3)
        }

        // Separator
        Rectangle {
            width: 1
            height: 20
            color: "#555555"
            anchors.verticalCenter: parent.verticalCenter
        }

        // To-do
        ToolbarButton {
            label: "\u2611"
            tooltip: "To-do"
            onClicked: root.insertType(4)
        }

        // Code
        ToolbarButton {
            label: "</>"
            tooltip: "Code"
            font.pixelSize: 11
            onClicked: root.insertType(5)
        }

        // Quote
        ToolbarButton {
            label: "\u201C"
            tooltip: "Quote"
            onClicked: root.insertType(9)
        }

        // Divider
        ToolbarButton {
            label: "---"
            tooltip: "Divider"
            font.pixelSize: 10
            onClicked: root.insertType(8)
        }

        // Image
        ToolbarButton {
            label: "\u25A3"
            tooltip: "Image"
            onClicked: root.insertType(6)
        }

        // Table
        ToolbarButton {
            label: "\u2637"
            tooltip: "Table"
            onClicked: root.insertType(7)
        }
        ToolbarButton {
            label: "↶"
            tooltip: "Undo (Ctrl+Z)"
            enabled: noteEditor && noteEditor.canUndo
            opacity: enabled ? 1 : 0.35
            onClicked: if (noteEditor) noteEditor.undo()
        }
        ToolbarButton {
            label: "↷"
            tooltip: "Redo (Ctrl+Shift+Z)"
            enabled: noteEditor && noteEditor.canRedo
            opacity: enabled ? 1 : 0.35
            onClicked: if (noteEditor) noteEditor.redo()
        }
        Rectangle {
            width: 1; height: 20; color: "#555555"
            anchors.verticalCenter: parent.verticalCenter
        }
    }

    component ToolbarButton : Rectangle {
        id: btn
        property string label: ""
        property string tooltip: ""
        property alias font: labelItem.font
        signal clicked()

        width: 32
        height: 28
        radius: 4
        color: btnArea.containsMouse ? Qt.rgba(1, 1, 1, 0.1) : "transparent"
        Behavior on color { ColorAnimation { duration: 100 } }

        Text {
            id: labelItem
            anchors.centerIn: parent
            text: btn.label
            font.pixelSize: 13
            font.bold: true
            color: btnArea.containsMouse ? "#ffffff" : "#999999"
        }

        MouseArea {
            id: btnArea
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: Qt.PointingHandCursor
            onClicked: btn.clicked()
        }

        ToolTip.visible: btnArea.containsMouse && btn.tooltip.length > 0
        ToolTip.text: btn.tooltip
        ToolTip.delay: 500
    }
}
