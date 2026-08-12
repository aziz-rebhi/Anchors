import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Item {
    id: root
    property string blockId: ""
    property string text: ""
    property bool collapsed: false
    signal contentChanged(string newText)

    Theme { id: theme }

    width: parent ? parent.width : 0
    height: headerRow.implicitHeight + 8

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
        id: headerRow
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        spacing: 6

        Text {
            text: root.collapsed ? "▶" : "▼"
            color: theme.textSecondary
            font.pixelSize: 12
            Layout.preferredWidth: 18
            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor
                onClicked: if (noteEditor) noteEditor.toggleCollapsed(root.blockId)
            }
        }

        TextArea {
            id: input
            Layout.fillWidth: true
            text: root.text
            placeholderText: "Toggle heading..."
            wrapMode: Text.Wrap
            font.pixelSize: 14
            font.bold: true
            font.family: theme.bodyFont
            color: theme.textPrimary
            placeholderTextColor: theme.textMuted
            background: Item {}

            onTextChanged: if (text !== root.text) root.contentChanged(text)
            onActiveFocusChanged: if (activeFocus && noteEditor) noteEditor.setFocusedBlock(root.blockId)

            Keys.onPressed: function (e) {
                if (e.key === Qt.Key_Return || e.key === Qt.Key_Enter) {
                    if (e.modifiers & Qt.ControlModifier)
                        noteEditor.insertBlockAfter(root.blockId, 0, "")
                    else
                        noteEditor.insertInside(root.blockId, 0, "")
                    e.accepted = true
                    return
                }
                if (e.key === Qt.Key_Backspace && text.length === 0) {
                    noteEditor.deleteBlock(root.blockId)
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