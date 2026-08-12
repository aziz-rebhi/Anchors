import QtQuick 2.15
import QtQuick.Controls 2.15

Rectangle {
    id: root
    property string blockId: ""
    property string text: ""
    signal contentChanged(string newText)

    Theme { id: theme }

    width: parent ? parent.width : 0
    height: Math.max(30, textArea.implicitHeight + 8)
    color: "transparent"

    function focusInput(atStart) {
        textArea.forceActiveFocus()
        textArea.cursorPosition = atStart ? 0 : textArea.text.length
    }
    function isOnFirstLine() {
        return textArea.text.lastIndexOf("\n", textArea.cursorPosition - 1) < 0
    }
    function isOnLastLine() {
        return textArea.text.indexOf("\n", textArea.cursorPosition) < 0
    }

    Rectangle {
        id: accentBar
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        width: 3
        color: theme.secondary
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
        font.family: theme.bodyFont
        font.italic: true
        color: theme.textPrimary
        placeholderTextColor: theme.textMuted
        background: Rectangle { color: "transparent" }

        onTextChanged: if (text !== root.text) root.contentChanged(text)
        onActiveFocusChanged: if (activeFocus && noteEditor) noteEditor.setFocusedBlock(root.blockId)

        Keys.onPressed: function (event) {
            if ((event.modifiers & Qt.ControlModifier) && event.key === Qt.Key_Z) {
                if (event.modifiers & Qt.ShiftModifier) noteEditor.redo()
                else noteEditor.undo()
                event.accepted = true
                return
            }
            if (event.key === Qt.Key_Enter || event.key === Qt.Key_Return) {
                var pos = textArea.cursorPosition
                var after = text.substring(pos)
                textArea.text = text.substring(0, pos)
                root.contentChanged(textArea.text)
                if (event.modifiers & (Qt.ControlModifier | Qt.ShiftModifier))
                    noteEditor.exitContainer(root.blockId, 0, after)
                else
                    noteEditor.insertBlockAfter(root.blockId, 0, after)
                event.accepted = true
                return
            }
            if (event.key === Qt.Key_Backspace && textArea.cursorPosition === 0) {
                if (textArea.text.length === 0)
                    noteEditor.deleteBlock(root.blockId)
                else
                    noteEditor.mergeWithPrevious(root.blockId)
                event.accepted = true
            }
            if (event.key === Qt.Key_Up && isOnFirstLine()) {
                noteEditor.focusAdjacent(root.blockId, false)
                event.accepted = true
                return
            }
            if (event.key === Qt.Key_Down && isOnLastLine()) {
                noteEditor.focusAdjacent(root.blockId, true)
                event.accepted = true
                return
            }
        }
    }
}