import QtQuick 2.15
import QtQuick.Controls 2.15

Rectangle {
    id: root
    property string blockId: ""
    property string text: ""
    property bool checked: false

    signal contentChanged(string newText)

    width: parent ? parent.width : 0
    height: Math.max(30, textArea.implicitHeight + 8)
    color: "transparent"

    function focusInput() {
        textArea.forceActiveFocus()
        textArea.cursorPosition = textArea.text.length
    }

    Row {
        anchors.fill: parent
        anchors.margins: 4
        spacing: 8

        Rectangle {
            id: checkbox
            width: 20
            height: 20
            anchors.verticalCenter: parent.verticalCenter
            radius: 4
            color: "transparent"
            border.color: root.checked ? "#88c0d0" : "#888888"
            border.width: 2

            Text {
                anchors.centerIn: parent
                text: "\u2713"
                font.pixelSize: 14
                font.bold: true
                color: root.checked ? "#88c0d0" : "transparent"
                visible: root.checked
            }

            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor
                onClicked: {
                    if (noteEditor) noteEditor.toggleBlockChecked(root.blockId)
                }
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
            color: root.checked ? "#888888" : "#dddddd"
            background: Rectangle { color: "transparent" }

            onTextChanged: {
                if (text !== root.text)
                    root.contentChanged(text)
            }

            onActiveFocusChanged: {
                if (activeFocus && noteEditor)
                    noteEditor.setFocusedBlock(root.blockId)
            }

            Keys.onPressed: function (event) {
                if ((event.modifiers & Qt.ControlModifier) && event.key === Qt.Key_Z) {
                    if (event.modifiers & Qt.ShiftModifier)
                        noteEditor.redo()
                    else
                        noteEditor.undo()
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

                    if (event.modifiers & Qt.ControlModifier) {
                        // Ctrl+Enter → leave the list (paragraph)
                        noteEditor.insertBlockAfter(root.blockId, 0, after)
                    } else {
                        // Enter → next to-do
                        noteEditor.insertBlockAfter(root.blockId, 4, after)
                    }
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
            }
        }
    }
}