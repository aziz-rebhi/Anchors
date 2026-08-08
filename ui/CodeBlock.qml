import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Rectangle {
    id: root
    property string blockId: ""
    property string text: ""
    property string language: ""

    signal contentChanged(string newText)

    width: parent ? parent.width : 0
    height: Math.max(180, header.height + codeArea.implicitHeight + 40)
    color: "#1e1e2e"
    radius: 8

    function focusInput() {
        codeArea.forceActiveFocus()
        codeArea.cursorPosition = codeArea.text.length
    }

    // Helper: check if cursor is on the last line and at end
    function isAtEnd() {
        var pos = codeArea.cursorPosition
        var txt = codeArea.text
        // Find last newline before cursor
        var lastNewline = txt.lastIndexOf("\n", pos - 1)
        var lineAfterCursor = txt.indexOf("\n", pos)
        // If no newline after cursor, we're on the last line
        return (lineAfterCursor < 0) && (pos >= txt.length - 1)
    }

    // Top bar: language selector + copy + actions
    RowLayout {
        id: header
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.leftMargin: 10
        anchors.rightMargin: 10
        anchors.topMargin: 8
        spacing: 6
        height: 30

        // Language dropdown
        ComboBox {
            id: langCombo
            flat: true
            model: ["Plain Text", "JavaScript", "Python", "C++", "Java", "C#", "Go", "Rust",
                     "HTML", "CSS", "SQL", "JSON", "Markdown", "Bash", "YAML", "XML"]
            font.pixelSize: 11
            font.family: "monospace"
            displayText: root.language || "Plain Text"
            implicitWidth: 120
            implicitHeight: 24

            background: Rectangle {
                color: "#313244"
                radius: 4
                border.color: langCombo.hovered ? "#585b70" : "transparent"
                border.width: 1
            }
            contentItem: Text {
                text: langCombo.displayText
                font: langCombo.font
                color: "#cdd6f4"
                verticalAlignment: Text.AlignVCenter
                leftPadding: 6
            }

            onActivated: function(index) {
                var lang = langCombo.textAt(index)
                if (lang === "Plain Text") lang = ""
                if (noteEditor) {
                    noteEditor.updateBlockCodeLanguage(root.blockId, lang)
                }
            }

            Component.onCompleted: {
                var idx = -1
                for (var i = 0; i < model.length; i++) {
                    if (model[i] === (root.language || "Plain Text")) {
                        idx = i
                        break
                    }
                }
                if (idx >= 0) currentIndex = idx
            }
        }

        Item { Layout.fillWidth: true }

        // Exit hint
        Text {
            text: "Shift+Enter to exit"
            font.pixelSize: 10
            color: "#6c7086"
            opacity: codeArea.activeFocus ? 0.7 : 0.0
            Behavior on opacity { NumberAnimation { duration: 200 } }
        }

        // Copy button
        Text {
            text: "Copy"
            font.pixelSize: 11
            color: "#a6adc8"
            opacity: copyArea.containsMouse ? 1.0 : 0.5

            MouseArea {
                id: copyArea
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor
                onClicked: {
                    codeArea.selectAll()
                    codeArea.copy()
                    codeArea.deselect()
                }
            }
        }

        // Delete button
        Text {
            text: "\u2715"
            font.pixelSize: 13
            color: "#a6adc8"
            opacity: deleteArea.containsMouse ? 1.0 : 0.4

            MouseArea {
                id: deleteArea
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor
                onClicked: {
                    if (noteEditor) noteEditor.deleteBlock(root.blockId)
                }
            }
        }
    }

    // Separator
    Rectangle {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: header.bottom
        anchors.topMargin: 2
        height: 1
        color: "#313244"
    }

    // Code editor area
    Flickable {
        id: flickable
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: header.bottom
        anchors.bottom: parent.bottom
        anchors.margins: 8
        anchors.topMargin: 12
        clip: true
        contentWidth: codeArea.implicitWidth
        contentHeight: codeArea.implicitHeight

        TextArea {
            id: codeArea
            width: flickable.width
            text: root.text
            placeholderText: "// Write code here..."
            wrapMode: TextArea.Wrap
            font.pixelSize: 13
            font.family: "monospace"
            color: "#cdd6f4"
            background: Rectangle { color: "transparent" }

            onTextChanged: {
                if (text !== root.text) {
                    root.contentChanged(text)
                }
            }

            onActiveFocusChanged: {
                if (activeFocus && noteEditor) {
                    noteEditor.setFocusedBlock(root.blockId)
                }
            }

            Keys.onPressed: function(event) {
                if ((event.modifiers & Qt.ControlModifier) && event.key === Qt.Key_Z) {
                    if (event.modifiers & Qt.ShiftModifier)
                        noteEditor.redo()
                    else
                        noteEditor.undo()
                    event.accepted = true
                    return
                }

                // Tab → 4 spaces
                if (event.key === Qt.Key_Tab) {
                    codeArea.insert(codeArea.cursorPosition, "    ")
                    event.accepted = true
                    return
                }

                // Shift+Enter or Ctrl+Enter → exit code block, insert paragraph after
                if ((event.key === Qt.Key_Enter || event.key === Qt.Key_Return) &&
                    (event.modifiers & (Qt.ShiftModifier | Qt.ControlModifier))) {
                    var pos = codeArea.cursorPosition
                    var after = codeArea.text.substring(pos)
                    codeArea.text = codeArea.text.substring(0, pos)
                    root.contentChanged(codeArea.text)
                    noteEditor.insertBlockAfter(root.blockId, 0, after)
                    event.accepted = true
                    return
                }

                // Plain Enter → stay inside (newline). Do not accept; let TextArea handle it.
                if (event.key === Qt.Key_Enter || event.key === Qt.Key_Return) {
                    event.accepted = false
                    return
                }
            }
        }

        ScrollBar.vertical: ScrollBar { policy: ScrollBar.AsNeeded }
        ScrollBar.horizontal: ScrollBar { policy: ScrollBar.AsNeeded }
    }

    Keys.onPressed: function(event) {
        if (event.key === Qt.Key_Backspace && codeArea.text.length === 0 && !codeArea.activeFocus) {
            noteEditor.deleteBlock(root.blockId)
            event.accepted = true
        }
    }
}
