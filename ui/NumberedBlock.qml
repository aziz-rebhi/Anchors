import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Item {
    id: root
    property string blockId: ""
    property string text: ""
    property int number: 1
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
            text: root.number + "."
            color: "#aaa"
            font.pixelSize: 14
            Layout.alignment: Qt.AlignTop
            Layout.topMargin: 6
            Layout.preferredWidth: 28
            horizontalAlignment: Text.AlignRight
        }
        TextArea {
            id: input
            Layout.fillWidth: true
            text: root.text
            wrapMode: Text.Wrap
            font.pixelSize: 14
            color: "#ddd"
            background: Item {}
            onTextChanged: if (text !== root.text) root.contentChanged(text)
            onActiveFocusChanged: if (activeFocus && noteEditor) noteEditor.setFocusedBlock(root.blockId)
            Keys.onPressed: function (e) {
                if (e.key === Qt.Key_Return && !(e.modifiers & Qt.ShiftModifier)) {
                    noteEditor.insertBlockAfter(root.blockId, 13, "")
                    e.accepted = true
                } else if (e.key === Qt.Key_Backspace && text.length === 0) {
                    noteEditor.changeBlockType(root.blockId, 0)
                    e.accepted = true
                }
            }
        }
    }
}