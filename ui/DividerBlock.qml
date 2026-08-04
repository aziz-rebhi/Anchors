import QtQuick 2.15
import QtQuick.Controls 2.15

Item {
    id: root
    property string blockId: ""

    width: parent ? parent.width : 0
    height: 36
    visible: true

    Rectangle {
        width: parent.width - 48
        height: 1
        anchors.centerIn: parent
        color: "#d0d0d0"
    }

    // Allow clicking to select/delete
    MouseArea {
        anchors.fill: parent
        onClicked: textArea.forceActiveFocus()
    }

    // Hidden focus receiver for keyboard events
    TextArea {
        id: textArea
        visible: false
        focus: false
        Keys.onPressed: function(event) {
            if (event.key === Qt.Key_Z && (event.modifiers & Qt.ControlModifier)) {
                event.modifiers & Qt.ShiftModifier ? noteEditor.redo() : noteEditor.undo()
                event.accepted = true
            } else if (event.key === Qt.Key_Y && (event.modifiers & Qt.ControlModifier)) {
                noteEditor.redo()
                event.accepted = true
            } else if (event.key === Qt.Key_Backspace) {
                noteEditor.deleteBlock(root.blockId)
                event.accepted = true
            } else if (event.key === Qt.Key_Enter || event.key === Qt.Key_Return) {
                noteEditor.insertBlockAfter(root.blockId, 0, "")
                event.accepted = true
            }
        }
    }
}