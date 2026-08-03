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

    TextField {
        id: textField
        anchors.fill: parent
        anchors.margins: 4
        text: root.text
        placeholderText: "Heading..."
        font.pixelSize: root.level === 1 ? 28 : (root.level === 2 ? 22 : 18)
        font.bold: true
        color: "#222"
        background: Rectangle { color: "transparent" }
        onTextChanged: {
            if (text !== root.text) {
                root.contentChanged(text)
            }
        }
        Keys.onPressed: (event) => {
            if (event.key === Qt.Key_Enter || event.key === Qt.Key_Return) {
                // Insert a paragraph after this heading
                noteEditor.insertBlock("", 0, 0, "")
                event.accepted = true
            }
        }
    }
}