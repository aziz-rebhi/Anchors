import QtQuick 2.15
import QtQuick.Controls 2.15

Rectangle {
    id: root
    property string blockId: ""
    property string text: ""
    property bool checked: false
    property alias textArea: textArea

    signal contentChanged(string newText)
    signal todoCheckedChanged(bool checked)

    width: parent ? parent.width : 0
    height: Math.max(30, row.implicitHeight + 8)
    color: "transparent"

    Row {
        id: row
        anchors.fill: parent
        anchors.margins: 4
        spacing: 8

        CheckBox {
            id: checkBox
            checked: root.checked
            anchors.verticalCenter: parent.verticalCenter
            onToggled: root.todoCheckedChanged(checked)
        }

        TextArea {
            id: textArea
            width: parent.width - checkBox.width - parent.spacing
            text: root.text
            placeholderText: "To-do..."
            wrapMode: Text.Wrap
            font.pixelSize: 14
            color: root.checked ? "#999" : "#222"
            font.strikeout: root.checked
            background: Rectangle { color: "transparent"; border.width: 0 }
            onTextChanged: {
                if (text !== root.text) {
                    root.contentChanged(text)
                }
            }
        }
    }
}
