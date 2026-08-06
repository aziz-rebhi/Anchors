import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Window 2.15

Rectangle {
    id: root
    property string blockId: ""
    property string text: ""
    property alias textArea: textArea
    property bool slashActive: false

    signal contentChanged(string newText)

    width: parent ? parent.width : 0
    height: Math.max(30, textArea.implicitHeight + 8)
    color: "transparent"

    function focusInput() {
        textArea.forceActiveFocus()
        textArea.cursorPosition = textArea.text.length
    }

    function openSlashMenu() {
        root.slashActive = true
        slashMenu.filterText = ""
        // Position near the text area
        var pos = textArea.mapToItem(Overlay.overlay, 0, textArea.height)
        slashMenu.cursorX = Math.max(8, pos.x)
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
            var slashPos = textArea.text.lastIndexOf("/")
            var cleanText = slashPos >= 0 ? textArea.text.substring(0, slashPos) : textArea.text

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
        color: "#dddddd"
        background: Rectangle { color: "transparent"; border.width: 0 }

        onTextChanged: {
            if (text === root.text)
                return

            if (!root.slashActive) {
                // Open when the block is just "/" or ends with a lone "/"
                if (text === "/" || (text.length > 0 && text.charAt(text.length - 1) === "/"
                                     && text.lastIndexOf("/") === text.length - 1
                                     && text.indexOf("\n") < 0)) {
                    openSlashMenu()
                }
            } else {
                var slashIdx = text.lastIndexOf("/")
                if (slashIdx < 0) {
                    closeSlashMenu()
                } else {
                    var filter = text.substring(slashIdx + 1)
                    if (filter.indexOf("\n") >= 0 || filter.indexOf(" ") >= 0) {
                        closeSlashMenu()
                    } else {
                        slashMenu.filterText = filter
                    }
                }
            }

            root.contentChanged(text)
        }

        onActiveFocusChanged: {
            if (activeFocus && noteEditor)
                noteEditor.setFocusedBlock(root.blockId)
            // Do not close slash menu on focus changes
        }

        Keys.onPressed: function (event) {
            if ((event.modifiers & Qt.ControlModifier) && event.key === Qt.Key_Z) {
                if (event.modifiers & Qt.ShiftModifier)
                    noteEditor.redo()
                else
                    noteEditor.undo()
                event.accepted = true
                return
            }

            if (event.key === Qt.Key_Tab) {
                textArea.insert(textArea.cursorPosition, "    ")
                event.accepted = true
                return
            }

            if (root.slashActive && slashMenu.visible) {
                if (event.key === Qt.Key_Escape) {
                    closeSlashMenu()
                    event.accepted = true
                    return
                }
                if (event.key === Qt.Key_Enter || event.key === Qt.Key_Return) {
                    slashMenu.selectCurrent()
                    event.accepted = true
                    return
                }
                if (event.key === Qt.Key_Up) {
                    slashMenu.moveUp()
                    event.accepted = true
                    return
                }
                if (event.key === Qt.Key_Down) {
                    slashMenu.moveDown()
                    event.accepted = true
                    return
                }
                if (event.key === Qt.Key_Backspace) {
                    var slashIdx = text.lastIndexOf("/")
                    if (slashIdx >= 0 && textArea.cursorPosition <= slashIdx + 1)
                        closeSlashMenu()
                    event.accepted = false
                    return
                }
                event.accepted = false
                return
            }

            if (event.key === Qt.Key_Enter || event.key === Qt.Key_Return) {
                var cursorPos = textArea.cursorPosition
                var before = text.substring(0, cursorPos)
                var after = text.substring(cursorPos)
                textArea.text = before
                root.contentChanged(textArea.text)
                noteEditor.insertBlockAfter(root.blockId, 0, after)
                event.accepted = true
                return
            }

            if (event.key === Qt.Key_Backspace && textArea.cursorPosition === 0) {
                noteEditor.mergeWithPrevious(root.blockId)
                event.accepted = true
                return
            }
        }
    }
}