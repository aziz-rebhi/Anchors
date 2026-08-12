import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Item {
    id: root
    property string blockId: ""
    property string text: ""
    property int indent: 0
    signal contentChanged(string newText)

    Theme { id: theme }

    width: parent ? parent.width : 0
    height: Math.max(28, input.implicitHeight + 8)

    function focusInput(atStart) {
        input.forceActiveFocus()
        input.cursorPosition = atStart ? 0 : input.text.length
    }
    function isOnFirstLine() {
        return input.text.lastIndexOf("\n", input.cursorPosition - 1) < 0
    }
    function isOnLastLine() {
        return input.text.indexOf("\n", input.cursorPosition) < 0
    }

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 4 + root.indent * 24
        spacing: 8

        Text {
            text: "•"
            color: theme.textSecondary
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
            font.family: theme.bodyFont
            color: theme.textPrimary
            background: Item {}

            onTextChanged: if (text !== root.text) root.contentChanged(text)
            onActiveFocusChanged: if (activeFocus && noteEditor) noteEditor.setFocusedBlock(root.blockId)

            Keys.onPressed: function (e) {
                if (e.key === Qt.Key_Tab) {
                    noteEditor.indentBlock(root.blockId)
                    e.accepted = true
                    return
                }
                if (e.key === Qt.Key_Backtab || (e.key === Qt.Key_Tab && (e.modifiers & Qt.ShiftModifier))) {
                    noteEditor.outdentBlock(root.blockId)
                    e.accepted = true
                    return
                }
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
                if (e.key === Qt.Key_Up && isOnFirstLine()) {
                    noteEditor.focusAdjacent(root.blockId, false)
                    e.accepted = true
                    return
                }
                if (e.key === Qt.Key_Down && isOnLastLine()) {
                    noteEditor.focusAdjacent(root.blockId, true)
                    e.accepted = true
                    return
                }
            }
        }
    }
}