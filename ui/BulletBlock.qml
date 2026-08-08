import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Item {
    id: root
    property string blockId: ""
    property string text: ""
    signal contentChanged(string newText)

    width: parent ? parent.width : 0
    height: Math.max(28, input.implicitHeight + 8)

    function focusInput() {
        input.forceActiveFocus()
        input.cursorPosition = input.text.length
    }

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 4
        spacing: 8

        Text {
            text: "•"
            color: "#a6adc8"
            font.pixelSize: 16
            Layout.alignment: Qt.AlignTop
            Layout.topMargin: 4
        }

        TextArea {
            id: input
            Layout.fillWidth: true
            text: root.text
            wrapMode: Text.Wrap
            font.pixelSize: 14
            color: "#dddddd"
            background: Item {}

            onTextChanged: if (text !== root.text) root.contentChanged(text)
            onActiveFocusChanged: if (activeFocus && noteEditor) noteEditor.setFocusedBlock(root.blockId)

            Keys.onPressed: function (e) {
                if ((e.modifiers & Qt.ControlModifier) && e.key === Qt.Key_Z) {
                    if (e.modifiers & Qt.ShiftModifier) noteEditor.redo()
                    else noteEditor.undo()
                    e.accepted = true
                    return
                }
                if (e.key === Qt.Key_Return || e.key === Qt.Key_Enter) {
                    var pos = input.cursorPosition
                    var after = text.substring(pos)
                    input.text = text.substring(0, pos)
                    root.contentChanged(input.text)
                    if (e.modifiers & (Qt.ControlModifier | Qt.ShiftModifier))
                        noteEditor.exitContainer(root.blockId, 0, after)
                    else
                        noteEditor.insertBlockAfter(root.blockId, 11, after)
                    e.accepted = true
                    return
                }
                if (e.key === Qt.Key_Backspace && input.cursorPosition === 0) {
                    if (text.length === 0)
                        noteEditor.changeBlockType(root.blockId, 0)
                    else
                        noteEditor.mergeWithPrevious(root.blockId)
                    e.accepted = true
                }
            }
        }
    }
}