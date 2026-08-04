import QtQuick 2.15
import QtQuick.Controls 2.15

Rectangle {
    id: root
    property string blockId: ""
    property string text: ""

    signal contentChanged(string newText)

    width: parent ? parent.width : 0
    height: Math.max(30, textArea.implicitHeight + 8)
    color: "transparent"

    function focusInput() {
        textArea.forceActiveFocus()
        textArea.cursorPosition = textArea.text.length
    }

    Rectangle {
        id: accentBar
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        width: 3
        color: "#666666"
        radius: 1
    }

    TextArea {
        id: textArea
        anchors.left: accentBar.right
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.leftMargin: 10
        anchors.rightMargin: 8
        anchors.topMargin: 4
        anchors.bottomMargin: 4
        text: root.text
        placeholderText: "Quote..."
        wrapMode: Text.Wrap
        font.pixelSize: 14
        font.italic: true
        color: "#dddddd"
        background: Rectangle { color: "transparent" }

        onTextChanged: {
            if (text !== root.text) {
                root.contentChanged(text)
            }
        }

        onActiveFocusChanged: {
            if (activeFocus && noteEditor) {
                noteEditor.setFocusedBlock(root.blockId)
            }
        }

        Keys.onPressed: function(event) {
            if ((event.modifiers & Qt.ControlModifier) && event.key === Qt.Key_Z) {
                if (event.modifiers & Qt.ShiftModifier)
                    noteEditor.redo()
                else
                    noteEditor.undo()
                event.accepted = true
            } else if (event.key === Qt.Key_Tab) {
                textArea.insert(textArea.cursorPosition, "    ")
                event.accepted = true
            } else if (event.key === Qt.Key_Enter || event.key === Qt.Key_Return) {
                var pos = textArea.cursorPosition
                var after = text.substring(pos)
                textArea.text = text.substring(0, pos)
                root.contentChanged(textArea.text)
                noteEditor.insertBlockAfter(root.blockId, 0, after)
                event.accepted = true
            } else if (event.key === Qt.Key_Backspace && textArea.cursorPosition === 0) {
                noteEditor.mergeWithPrevious(root.blockId)
                event.accepted = true
            }
        }
    }
}
