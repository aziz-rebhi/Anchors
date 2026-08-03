import QtQuick 2.15
import QtQuick.Controls 2.15

Rectangle {
    id: root
    property string blockId: ""
    property string text: ""
    property alias textArea: textArea

    // Renamed to avoid conflict with property 'text'
    signal contentChanged(string newText)

    width: parent ? parent.width : 0
    height: Math.max(30, textArea.implicitHeight + 8)
    color: "transparent"

    TextArea {
        id: textArea
        anchors.fill: parent
        anchors.margins: 4
        text: root.text
        placeholderText: "Type '/' for commands..."
        wrapMode: Text.Wrap
        font.pixelSize: 14
        color: "#222"
        background: Rectangle { color: "transparent"; border.width: 0 }
        onTextChanged: {
            if (text !== root.text) {
                root.contentChanged(text)
            }
        }
    }
}