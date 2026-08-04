import QtQuick 2.15
import QtQuick.Controls 2.15

Rectangle {
    id: root
    property string blockId: ""
    property string text: ""
    property alias textArea: textArea

    signal contentChanged(string newText)

    width: parent ? parent.width : 0
    height: Math.max(30, textArea.implicitHeight + 8)
    color: "transparent"

    function focusInput() {
        textArea.forceActiveFocus()
        textArea.cursorPosition = textArea.text.length
    }

    // Slash command menu instance
    SlashCommandMenu {
        id: slashMenu
        blockId: root.blockId

        onSelectBlock: function(menuBlockId, typeCode) {
            // Find the "/" and grab clean text before it
            var slashPos = textArea.text.indexOf("/")
            var cleanText = ""
            if (slashPos >= 0) {
                cleanText = textArea.text.substring(0, slashPos)
            } else {
                cleanText = textArea.text
            }

            root.slashActive = false
            slashMenu.close()

            // Clear the slash text in the model FIRST
            noteEditor.updateBlockContent(root.blockId, cleanText)

            if (typeCode !== 0) {
                // Change to the selected type — pendingFocusId will handle focus
                noteEditor.changeBlockType(root.blockId, typeCode)
            }
            // For type 0 (Text), the block stays as paragraph, focus stays
        }
    }

    property bool slashActive: false
    property int slashStartPos: -1

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
                if (!root.slashActive) {
                    var trimmed = text.trim()
                    if (trimmed === "/" || trimmed === "/ ") {
                        root.slashActive = true
                        root.slashStartPos = text.indexOf("/")
                        slashMenu.filterText = ""
                        slashMenu.cursorX = textArea.cursorRectangle.x
                        slashMenu.cursorY = textArea.cursorRectangle.y
                        slashMenu.open()
                    }
                } else {
                    var slashIdx = text.indexOf("/")
                    if (slashIdx >= 0) {
                        var filter = text.substring(slashIdx + 1)
                        if (filter.indexOf(" ") >= 0) {
                            root.slashActive = false
                            slashMenu.close()
                        } else {
                            slashMenu.filterText = filter
                        }
                    } else {
                        root.slashActive = false
                        slashMenu.close()
                    }
                }
                root.contentChanged(text)
            }
        }

        onActiveFocusChanged: {
            if (activeFocus && noteEditor) {
                noteEditor.setFocusedBlock(root.blockId)
            }
            if (!activeFocus && root.slashActive) {
                root.slashActive = false
                slashMenu.close()
            }
        }

        Keys.onPressed: function(event) {
            // Undo/Redo
            if ((event.modifiers & Qt.ControlModifier) && event.key === Qt.Key_Z) {
                if (event.modifiers & Qt.ShiftModifier)
                    noteEditor.redo()
                else
                    noteEditor.undo()
                event.accepted = true
                return
            }

            // Tab: insert 4 spaces (NOT change focus)
            if (event.key === Qt.Key_Tab) {
                textArea.insert(textArea.cursorPosition, "    ")
                event.accepted = true
                return
            }

            // Slash menu open — route navigation keys
            if (root.slashActive && slashMenu.visible) {
                if (event.key === Qt.Key_Escape) {
                    root.slashActive = false
                    slashMenu.close()
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
                    var slashIdx = text.indexOf("/")
                    if (slashIdx >= 0 && textArea.cursorPosition <= slashIdx + 1) {
                        root.slashActive = false
                        slashMenu.close()
                    }
                    event.accepted = false
                    return
                }
                event.accepted = false
                return
            }

            // Enter: split block at cursor
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

            // Backspace at position 0: merge with previous
            if (event.key === Qt.Key_Backspace && textArea.cursorPosition === 0) {
                noteEditor.mergeWithPrevious(root.blockId)
                event.accepted = true
                return
            }
        }
    }
}
