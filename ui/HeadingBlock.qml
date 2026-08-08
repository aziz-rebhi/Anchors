import QtQuick 2.15
import QtQuick.Controls 2.15

Rectangle {
    id: root
    property string blockId: ""
    property int level: 1
    property string text: ""
    property alias textField: textField

    signal contentChanged(string newText)

    width: parent ? parent.width : 0
    height: Math.max(30, textField.implicitHeight + 8)
    color: "transparent"

    function focusInput() {
        textField.forceActiveFocus()
        textField.cursorPosition = textField.text.length
    }

    TextField {
        id: textField
        anchors.fill: parent
        anchors.margins: 4
        text: root.text
        placeholderText: "Heading..."
        font.pixelSize: level === 1 ? 28 : (level === 2 ? 22 : (level === 3 ? 18 : 16))
        font.bold: true
        color: "#e8e8e8"
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
                if (event.modifiers & Qt.ShiftModifier) noteEditor.redo()
                else noteEditor.undo()
                event.accepted = true
                return
            }
            if (event.key === Qt.Key_Tab) {
                textField.insert(textField.cursorPosition, "    ")
                event.accepted = true
                return
            }
            if (event.key === Qt.Key_Enter || event.key === Qt.Key_Return) {
                var pos = textField.cursorPosition
                var after = textField.text.substring(pos)
                textField.text = textField.text.substring(0, pos)
                root.contentChanged(textField.text)
                if (event.modifiers & (Qt.ControlModifier | Qt.ShiftModifier))
                    noteEditor.exitContainer(root.blockId, 0, after)
                else
                    noteEditor.insertBlockAfter(root.blockId, 0, after)
                event.accepted = true
                return
            }
            if (event.key === Qt.Key_Backspace && textField.cursorPosition === 0) {
                if (textField.text.length === 0)
                    noteEditor.deleteBlock(root.blockId)
                else
                    noteEditor.mergeWithPrevious(root.blockId)
                event.accepted = true
            }
        }
    }
}