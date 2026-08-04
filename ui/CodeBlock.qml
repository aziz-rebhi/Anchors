import QtQuick 2.15
import QtQuick.Controls 2.15

Rectangle {
    id: root
    property string blockId: ""
    property string text: ""
    property string language: ""
    property alias textArea: textArea

    signal contentChanged(string newText)

    width: parent ? parent.width : 0
    height: Math.max(60, langRow.height + textArea.implicitHeight + 16)
    color: "#f7f7f8"
    radius: 4

    Column {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.margins: 8
        spacing: 4

        Row {
            id: langRow
            width: parent.width
            height: 16

            Text {
                text: root.language || "code"
                font.pixelSize: 11
                font.family: "monospace"
                color: "#999"
            }
        }

        TextArea {
            id: textArea
            width: parent.width
            text: root.text
            placeholderText: "Write code..."
            wrapMode: Text.NoWrap
            font.pixelSize: 13
            font.family: "monospace"
            color: "#e06c75"
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
                    noteEditor.insertBlockAfter(blockId, 5, after)
                    event.accepted = true
                } else if (event.key === Qt.Key_Backspace && textArea.cursorPosition === 0) {
                    noteEditor.mergeWithPrevious(blockId)
                    event.accepted = true
                }
            }
        }
    }
}