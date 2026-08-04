import QtQuick 2.15
import QtQuick.Controls 2.15

Item {
    id: root
    property string blockId: ""
    property string source: ""
    property string caption: ""

    width: parent ? parent.width : 0
    height: Math.max(60, column.implicitHeight + 16)

    Column {
        id: column
        anchors.fill: parent
        anchors.margins: 8
        spacing: 4

        // Placeholder or image
        Rectangle {
            width: parent.width
            height: root.source ? img.height : 120
            color: "#f0f0f0"
            radius: 4
            border.width: 1
            border.color: "#ddd"
            visible: !root.source || img.status === Image.Error

            Text {
                anchors.centerIn: parent
                text: "No image"
                color: "#bbb"
                font.pixelSize: 13
            }
        }

        Image {
            id: img
            width: parent.width
            fillMode: Image.PreserveAspectFit
            source: root.source
            visible: root.source && status === Image.Ready
        }

        // Caption
        TextField {
            width: parent.width
            text: root.caption
            placeholderText: "Add a caption..."
            font.pixelSize: 12
            color: "#888"
            background: Rectangle { color: "transparent" }
            visible: root.source || text.length > 0
        }
    }

    // Keyboard handling
    Keys.onPressed: function(event) {
        if (event.key === Qt.Key_Z && (event.modifiers & Qt.ControlModifier)) {
            event.modifiers & Qt.ShiftModifier ? noteEditor.redo() : noteEditor.undo()
            event.accepted = true
        } else if (event.key === Qt.Key_Y && (event.modifiers & Qt.ControlModifier)) {
            noteEditor.redo()
            event.accepted = true
        } else if (event.key === Qt.Key_Backspace) {
            noteEditor.deleteBlock(root.blockId)
            event.accepted = true
        } else if (event.key === Qt.Key_Enter || event.key === Qt.Key_Return) {
            noteEditor.insertBlockAfter(root.blockId, 0, "")
            event.accepted = true
        }
    }
}