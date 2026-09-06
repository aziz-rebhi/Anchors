import QtQuick 2.15
import QtQuick.Controls 2.15

Rectangle {
    id: root
    property string blockId: ""
    property string text: ""
    signal contentChanged(string newText)

    Theme { id: theme }

    width: parent ? parent.width : 0
    height: Math.max(30, textArea.implicitHeight + 8)
    color: "transparent"

    function focusInput(atStart) {
        textArea.forceActiveFocus()
        textArea.cursorPosition = atStart ? 0 : textArea.text.length
    }
    function plainText() { return textArea.getText(0, textArea.length) }
    function plainLength() { return plainText().length }
    function isOnFirstLine() {
        return plainText().lastIndexOf("\n", textArea.cursorPosition - 1) < 0
    }
    function isOnLastLine() {
        return plainText().indexOf("\n", textArea.cursorPosition) < 0
    }
    function registerFocus() {
        var item = root.parent
        while (item) {
            if (typeof item.claimFocus === "function") {
                item.claimFocus(textArea)
                break
            }
            item = item.parent
        }
        if (noteEditor)
            noteEditor.setFocusedBlock(root.blockId)
    }

    Rectangle {
        id: accentBar
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        width: 3
        color: theme.secondary
        radius: 1
    }

    TextArea {
        id: textArea
        anchors.left: accentBar.right
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.leftMargin: 10
        anchors.rightMargin: 8
        anchors.topMargin: 4
        anchors.bottomMargin: 4
        text: root.text
        placeholderText: "Quote..."
        wrapMode: Text.Wrap
        font.pixelSize: 14
        font.family: theme.bodyFont
        font.italic: true
        color: theme.textPrimary
        placeholderTextColor: theme.textMuted
        background: Rectangle { color: "transparent" }
        textFormat: TextEdit.RichText
        persistentSelection: true
        selectByMouse: true

        onTextChanged: if (text !== root.text) root.contentChanged(text)
        onActiveFocusChanged: if (activeFocus) root.registerFocus()

        Keys.onPressed: function (event) {
            if (event.modifiers & Qt.ControlModifier) {
                if (typeof richTextHelper !== "undefined") {
                    if (event.key === Qt.Key_B) { richTextHelper.toggleBold(textArea); event.accepted = true; return }
                    if (event.key === Qt.Key_I) { richTextHelper.toggleItalic(textArea); event.accepted = true; return }
                    if (event.key === Qt.Key_U) { richTextHelper.toggleUnderline(textArea); event.accepted = true; return }
                    if (event.key === Qt.Key_S) { richTextHelper.toggleStrike(textArea); event.accepted = true; return }
                }
            }
            if ((event.modifiers & Qt.ControlModifier) && event.key === Qt.Key_Z) {
                if (event.modifiers & Qt.ShiftModifier) noteEditor.redo()
                else noteEditor.undo()
                event.accepted = true
                return
            }
            if (event.key === Qt.Key_Enter || event.key === Qt.Key_Return) {
                var after = ""
                if (typeof richTextHelper !== "undefined") {
                    var parts = richTextHelper.splitAtCursor(textArea)
                    after = parts.after || ""
                    root.contentChanged(textArea.text)
                } else {
                    var plain = root.plainText()
                    var pos = Math.min(textArea.cursorPosition, plain.length)
                    after = plain.substring(pos)
                    textArea.text = plain.substring(0, pos)
                    root.contentChanged(textArea.text)
                }
                if (event.modifiers & (Qt.ControlModifier | Qt.ShiftModifier))
                    noteEditor.exitContainer(root.blockId, 0, after)
                else
                    noteEditor.insertBlockAfter(root.blockId, 0, after)
                event.accepted = true
                return
            }
            if (event.key === Qt.Key_Backspace && textArea.cursorPosition === 0) {
                if (root.plainLength() === 0)
                    noteEditor.deleteBlock(root.blockId)
                else
                    noteEditor.mergeWithPrevious(root.blockId)
                event.accepted = true
            }
            if (event.key === Qt.Key_Up && isOnFirstLine()) {
                noteEditor.focusAdjacent(root.blockId, false)
                event.accepted = true
                return
            }
            if (event.key === Qt.Key_Down && isOnLastLine()) {
                noteEditor.focusAdjacent(root.blockId, true)
                event.accepted = true
                return
            }
        }
    }
}