import QtQuick 2.15
import QtQuick.Controls 2.15

Item {
    id: root
    property string blockId: ""
    property int rows: 0
    property int cols: 0
    property var cells: []

    width: parent ? parent.width : 0
    height: rows > 0 && cols > 0 ? grid.implicitHeight + 16 : 60

    // Empty table placeholder
    Rectangle {
        width: parent.width - 16
        height: 48
        anchors.centerIn: parent
        color: "#f7f7f8"
        radius: 4
        border.width: 1
        border.color: "#e0e0e0"
        visible: root.rows === 0

        Text {
            anchors.centerIn: parent
            text: "Empty table"
            color: "#bbb"
            font.pixelSize: 13
        }
    }

    // Basic grid display
    Grid {
        id: grid
        anchors.fill: parent
        anchors.margins: 8
        columns: root.cols
        columnSpacing: 1
        rowSpacing: 1
        visible: root.rows > 0 && root.cols > 0

        Repeater {
            model: root.rows * root.cols
            Rectangle {
                width: (root.width - 16) / root.cols
                height: 28
                color: "#f7f7f8"

                Text {
                    anchors.fill: parent
                    anchors.margins: 4
                    text: {
                        var r = Math.floor(index / root.cols)
                        var c = index % root.cols
                        if (root.cells && root.cells[r] && root.cells[r][c] !== undefined)
                            return root.cells[r][c]
                        return ""
                    }
                    font.pixelSize: 12
                    color: "#444"
                    verticalAlignment: Text.AlignVCenter
                }
            }
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