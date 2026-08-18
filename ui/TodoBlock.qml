import QtQuick 2.15
import QtQuick.Controls 2.15

Rectangle {
    id: root
    property string blockId: ""
    property string text: ""
    property bool checked: false
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

    Row {
        anchors.fill: parent
        anchors.margins: 4
        spacing: 8

        Rectangle {
            width: 20
            height: 20
            anchors.verticalCenter: parent.verticalCenter
            radius: 4
            color: "transparent"
            border.color: root.checked ? theme.secondary : theme.textMuted
            border.width: 2
            Text {
                anchors.centerIn: parent
                text: "\u2713"
                font.pixelSize: 14
                font.bold: true
                color: theme.secondary
                visible: root.checked
            }
            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor
                onClicked: if (noteEditor) noteEditor.toggleBlockChecked(root.blockId)
            }
        }

        TextArea {
            id: textArea
            width: parent.width - 28
            anchors.verticalCenter: parent.verticalCenter
            text: root.text
            placeholderText: "To-do..."
            wrapMode: Text.Wrap
            font.pixelSize: 14
            font.family: theme.bodyFont
            color: root.checked ? theme.textMuted : theme.textPrimary
            placeholderTextColor: theme.textMuted
            background: Rectangle { color: "transparent" }

            onTextChanged: if (text !== root.text) root.contentChanged(text)
            onActiveFocusChanged: if (activeFocus && noteEditor) noteEditor.setFocusedBlock(root.blockId)

            Keys.onPressed: function (event) {
                if ((event.modifiers & Qt.ControlModifier) && event.key === Qt.Key_V) {
                    if (noteEditor && noteEditor.pasteImageFromClipboard()) {
                        event.accepted = true
                        return
                    }
                    event.accepted = false
                    return
                }
                if ((event.modifiers & Qt.ControlModifier) && event.key === Qt.Key_Z) {
                    if (event.modifiers & Qt.ShiftModifier) noteEditor.redo()
                    else noteEditor.undo()
                    event.accepted = true
                    return
                }
                if (event.key === Qt.Key_Tab) {
                    textArea.insert(textArea.cursorPosition, "    ")
                    event.accepted = true
                    return
                }
                if (event.key === Qt.Key_Enter || event.key === Qt.Key_Return) {
                    var pos = textArea.cursorPosition
                    var after = text.substring(pos)
                    textArea.text = text.substring(0, pos)
                    root.contentChanged(textArea.text)
                    if (event.modifiers & Qt.ControlModifier)
                        noteEditor.insertBlockAfter(root.blockId, 0, after)
                    else
                        noteEditor.insertBlockAfter(root.blockId, 4, after)
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
}