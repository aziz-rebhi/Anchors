import QtQuick 2.15
import QtQuick.Controls 2.15

Rectangle {
    id: root
    property string blockId: ""
    property string text: ""
    property alias textArea: textArea

    signal contentChanged(string newText)

    width: parent ? parent.width : 0
    height: Math.max(30, textArea.implicitHeight + 8)
    color: "transparent"

    Row {
        anchors.fill: parent
        anchors.margins: 4
        spacing: 10

        Rectangle {
            width: 3
            height: parent.height
            radius: 1.5
            color: "#888"
        }

        TextArea {
            id: textArea
            width: parent.width - 3 - parent.spacing
            text: root.text
            placeholderText: "Quote..."
            wrapMode: Text.Wrap
            font.pixelSize: 14
            font.italic: true
            color: "#555"
            background: Rectangle { color: "transparent"; border.width: 0 }
            onTextChanged: {
                if (text !== root.text) {
                    root.contentChanged(text)
                }
            }
            Keys.onPressed: function(event) {
                if (event.key === Qt.Key_Z && (event.modifiers & Qt.ControlModifier)) {
                    event.modifiers & Qt.ShiftModifier ? noteEditor.redo() : noteEditor.undo()
                    event.accepted = true
                } else if (event.key === Qt.Key_Y && (event.modifiers & Qt.ControlModifier)) {
                    noteEditor.redo()
                    event.accepted = true
                } else if (event.key === Qt.Key_Enter || event.key === Qt.Key_Return) {
                    var pos = textArea.cursorPosition
                    var before = textArea.text.substring(0, pos)
                    var after = textArea.text.substring(pos)
                    textArea.text = before
                    root.contentChanged(before)
                    noteEditor.insertBlockAfter(blockId, 0, after)
                    event.accepted = true
                } else if (event.key === Qt.Key_Backspace && textArea.cursorPosition === 0) {
                    noteEditor.mergeWithPrevious(blockId)
                    event.accepted = true
                }
            }
        }
    }
}