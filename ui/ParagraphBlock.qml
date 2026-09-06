import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Window 2.15

Rectangle {
    id: root
    property string blockId: ""
    property string text: ""
    property alias textArea: textArea
    property bool slashActive: false

    Theme { id: theme }
    signal contentChanged(string newText)

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
    function reportSelection() {
        var item = root.parent
        while (item) {
            if (typeof item.rememberSelection === "function") {
                item.rememberSelection(textArea, textArea.selectionStart, textArea.selectionEnd)
                break
            }
            item = item.parent
        }
    }
    function handleFormatKeys(event) {
        if (!(event.modifiers & Qt.ControlModifier))
            return false
        if (typeof richTextHelper === "undefined")
            return false
        if (event.key === Qt.Key_B) { richTextHelper.toggleBold(textArea); event.accepted = true; return true }
        if (event.key === Qt.Key_I) { richTextHelper.toggleItalic(textArea); event.accepted = true; return true }
        if (event.key === Qt.Key_U) { richTextHelper.toggleUnderline(textArea); event.accepted = true; return true }
        if (event.key === Qt.Key_S) { richTextHelper.toggleStrike(textArea); event.accepted = true; return true }
        return false
    }
    function splitAndContinue(modifiers) {
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
        if (modifiers & (Qt.ControlModifier | Qt.ShiftModifier))
            noteEditor.exitContainer(root.blockId, 0, after)
        else
            noteEditor.insertBlockAfter(root.blockId, 0, after)
    }
    function openSlashMenu() {
        root.slashActive = true
        slashMenu.filterText = ""
        var pos = textArea.mapToItem(Overlay.overlay, 0, textArea.height)
        var menuH = Math.min(360, slashMenu.height > 0 ? slashMenu.height : 360)
        var overlayH = Overlay.overlay ? Overlay.overlay.height : 800
        slashMenu.cursorX = Math.max(8, pos.x)
        if (pos.y + menuH > overlayH - 16)
            slashMenu.cursorY = Math.max(8, pos.y - textArea.height - menuH - 8)
        else
            slashMenu.cursorY = pos.y + 4
        slashMenu.open()
    }
    function closeSlashMenu() {
        root.slashActive = false
        if (slashMenu.visible)
            slashMenu.close()
    }

    SlashCommandMenu {
        id: slashMenu
        blockId: root.blockId
        parent: Overlay.overlay
        onBlockSelected: function (menuBlockId, typeCode) {
            var plain = root.plainText()
            var slashPos = plain.lastIndexOf("/")
            var cleanText = slashPos >= 0 ? plain.substring(0, slashPos) : plain
            root.closeSlashMenu()
            noteEditor.updateBlockContent(root.blockId, cleanText)
            if (typeCode !== 0)
                noteEditor.changeBlockType(root.blockId, typeCode)
        }
    }

    TextArea {
        id: textArea
        anchors.fill: parent
        anchors.margins: 4
        text: root.text
        placeholderText: "Type '/' for commands..."
        wrapMode: Text.Wrap
        font.pixelSize: 14
        font.family: theme.bodyFont
        color: theme.textPrimary
        placeholderTextColor: theme.textMuted
        background: Rectangle { color: "transparent"; border.width: 0 }
        textFormat: TextEdit.RichText
        persistentSelection: true
        selectByMouse: true

        onTextChanged: {
            if (text === root.text)
                return
            var plain = root.plainText()
            if (!root.slashActive) {
                if (plain.length > 0 && plain.charAt(plain.length - 1) === "/")
                    openSlashMenu()
            } else {
                var slashIdx = plain.lastIndexOf("/")
                if (slashIdx < 0) {
                    closeSlashMenu()
                } else {
                    var filter = plain.substring(slashIdx + 1)
                    if (filter.indexOf("\n") >= 0 || filter.indexOf(" ") >= 0)
                        closeSlashMenu()
                    else
                        slashMenu.filterText = filter
                }
            }
            root.contentChanged(textArea.text)
        }

        onActiveFocusChanged: if (activeFocus) root.registerFocus()
        onCursorPositionChanged: root.reportSelection()
        onSelectedTextChanged: root.reportSelection()

        Keys.onPressed: function (event) {
            if (root.handleFormatKeys(event))
                return
            if ((event.modifiers & Qt.ControlModifier) && event.key === Qt.Key_V) {
                if (noteEditor && noteEditor.pasteImageFromClipboard()) {
                    event.accepted = true
                    return
                }
                event.accepted = false
                return
            }
            if ((event.modifiers & Qt.ControlModifier) && event.key === Qt.Key_Z) {
                if (event.modifiers & Qt.ShiftModifier) noteEditor.redo()
                else noteEditor.undo()
                event.accepted = true
                return
            }
            if (event.key === Qt.Key_Tab) {
                textArea.insert(textArea.cursorPosition, "    ")
                event.accepted = true
                return
            }
            if (root.slashActive && slashMenu.visible) {
                if (event.key === Qt.Key_Escape) { closeSlashMenu(); event.accepted = true; return }
                if (event.key === Qt.Key_Enter || event.key === Qt.Key_Return) {
                    slashMenu.selectCurrent(); event.accepted = true; return
                }
                if (event.key === Qt.Key_Up) { slashMenu.moveUp(); event.accepted = true; return }
                if (event.key === Qt.Key_Down) { slashMenu.moveDown(); event.accepted = true; return }
                event.accepted = false
                return
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
            if (event.key === Qt.Key_Enter || event.key === Qt.Key_Return) {
                root.splitAndContinue(event.modifiers)
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
        }
    }
}