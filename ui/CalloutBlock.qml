import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Rectangle {
    id: root
    property string blockId: ""
    property string text: ""
    property string emoji: "💡"
    signal contentChanged(string newText)

    width: parent ? parent.width : 0
    height: Math.max(48, row.implicitHeight + 16)
    radius: 8
    color: "#2a2a18"
    border.color: "#5a5a20"
    border.width: 1

    function focusInput() {
        input.forceActiveFocus()
        input.cursorPosition = input.text.length
    }

    RowLayout {
        id: row
        anchors.fill: parent
        anchors.margins: 10
        spacing: 10

        Text {
            text: root.emoji
            font.pixelSize: 18
            Layout.alignment: Qt.AlignTop
        }

        TextArea {
            id: input
            Layout.fillWidth: true
            text: root.text
            wrapMode: Text.Wrap
            font.pixelSize: 14
            color: "#eeeeee"
            background: Item {}

            onTextChanged: if (text !== root.text) root.contentChanged(text)

            onActiveFocusChanged: {
                if (activeFocus && noteEditor)
                    noteEditor.setFocusedBlock(root.blockId)
            }

            Keys.onPressed: function (event) {
                // Ctrl+Enter or plain Enter → new paragraph after callout
                if (event.key === Qt.Key_Return || event.key === Qt.Key_Enter) {
                    if ((event.modifiers & Qt.ControlModifier) || !(event.modifiers & Qt.ShiftModifier)) {
                        noteEditor.insertBlockAfter(root.blockId, 0, "")
                        event.accepted = true
                        return
                    }
                    // Shift+Enter → newline inside callout (default)
                }

                if (event.key === Qt.Key_Backspace && text.length === 0) {
                    noteEditor.deleteBlock(root.blockId)
                    event.accepted = true
                    return
                }

                if ((event.modifiers & Qt.ControlModifier) && event.key === Qt.Key_Z) {
                    if (event.modifiers & Qt.ShiftModifier)
                        noteEditor.redo()
                    else
                        noteEditor.undo()
                    event.accepted = true
                }
            }
        }
    }
}