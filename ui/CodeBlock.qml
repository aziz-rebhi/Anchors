import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Anchors 1.0

Rectangle {
    id: root
    property string blockId: ""
    property string text: ""
    property string language: "auto"

    signal contentChanged(string newText)

    width: parent ? parent.width : 0
    height: Math.max(180, header.height + Math.max(120, codeArea.contentHeight) + 40)
    color: "#1e1e2e"
    radius: 8

    readonly property var langOptions: [
        { label: "Auto",         value: "auto" },
        { label: "Plain Text",   value: "text" },
        { label: "JavaScript",   value: "js" },
        { label: "Python",       value: "python" },
        { label: "c / C++",          value: "cpp" },
        { label: "Bash / Linux", value: "bash" },
        { label: "QML",          value: "qml" },
        { label: "HTML",         value: "html" },
        { label: "CSS",          value: "css" },
        { label: "JSON",         value: "json" },
        { label: "SQL",          value: "text" },
        { label: "Markdown",     value: "text" },
        { label: "YAML",         value: "text" },
        { label: "XML",          value: "text" },
        { label: "Java",         value: "text" },
        { label: "C#",           value: "text" },
        { label: "Go",           value: "text" },
        { label: "Rust",         value: "text" }
    ]

    property string detectedLabel: ""

    function focusInput() {
        codeArea.forceActiveFocus()
        codeArea.cursorPosition = codeArea.text.length
    }

    function labelForValue(v) {
        var val = v && v.length ? v : "auto"
        if (val === "Plain Text" || val === "") return "Auto"
        if (val === "C++" || val === "c++") return "C++"
        if (val === "JavaScript") return "JavaScript"
        if (val === "Bash" || val === "shell" || val === "sh") return "Bash / Linux"
        for (var i = 0; i < langOptions.length; i++) {
            if (langOptions[i].value === val || langOptions[i].label === val)
                return langOptions[i].label
        }
        return "Auto"
    }

    function valueForLabel(label) {
        for (var i = 0; i < langOptions.length; i++) {
            if (langOptions[i].label === label)
                return langOptions[i].value
        }
        return "auto"
    }

    function persistLanguage(value) {
        root.language = value
        bridge.language = value
        if (noteEditor)
            noteEditor.updateBlockCodeLanguage(root.blockId, value)
        refreshDetected()
    }

    function refreshDetected() {
        if (!bridge || !codeArea)
            return
        var id = bridge.detectCode(codeArea.text)
        root.detectedLabel = (root.language === "auto" || root.language === "")
                           ? labelForValue(id)
                           : ""
        bridge.redetect()
    }

    CodeHighlightBridge {
        id: bridge
    }

    Component.onCompleted: {
        bridge.language = root.language && root.language.length ? root.language : "auto"
    }

    // ── Header ──────────────────────────────────────────────────
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

        ComboBox {
            id: langCombo
            flat: true
            model: {
                var labels = []
                for (var i = 0; i < root.langOptions.length; i++)
                    labels.push(root.langOptions[i].label)
                return labels
            }
            font.pixelSize: 11
            font.family: "monospace"
            displayText: root.labelForValue(root.language)
            implicitWidth: 130
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

            onActivated: function (index) {
                var label = langCombo.model[index]
                root.persistLanguage(root.valueForLabel(label))
            }

            Component.onCompleted: {
                var label = root.labelForValue(root.language)
                var idx = model.indexOf(label)
                if (idx >= 0)
                    currentIndex = idx
            }
        }

        Text {
            visible: root.detectedLabel.length > 0
                     && (root.language === "auto" || root.language === "")
            text: "· " + root.detectedLabel
            font.pixelSize: 10
            color: "#6c7086"
        }

        Item { Layout.fillWidth: true }

        Text {
            text: "Shift+Enter to exit"
            font.pixelSize: 10
            color: "#6c7086"
            opacity: codeArea.activeFocus ? 0.7 : 0.0
            Behavior on opacity { NumberAnimation { duration: 200 } }
        }

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
                onClicked: if (noteEditor) noteEditor.deleteBlock(root.blockId)
            }
        }
    }

    Rectangle {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: header.bottom
        anchors.topMargin: 2
        height: 1
        color: "#313244"
    }

    // ── Editor (TextEdit so QSyntaxHighlighter formats apply) ───
    Flickable {
        id: flickable
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: header.bottom
        anchors.bottom: parent.bottom
        anchors.margins: 8
        anchors.topMargin: 12
        clip: true
        contentWidth: Math.max(width, codeArea.contentWidth)
        contentHeight: Math.max(height, codeArea.contentHeight)
        boundsBehavior: Flickable.StopAtBounds
        interactive: true

        TextEdit {
            id: codeArea
            width: Math.max(flickable.width, contentWidth + 4)
            height: Math.max(flickable.height, contentHeight + 4)
            text: root.text
            color: "#cdd6f4"
            selectedTextColor: "#1e1e2e"
            selectionColor: "#89b4fa"
            font.pixelSize: 13
            font.family: "JetBrains Mono, Cascadia Code, Fira Code, monospace"
            wrapMode: TextEdit.NoWrap
            selectByMouse: true
            persistentSelection: true
            activeFocusOnPress: true
            cursorVisible: activeFocus

            Component.onCompleted: {
                Qt.callLater(function () {
                    if (!codeArea || !bridge)
                        return
                    bridge.attach(codeArea.textDocument)
                    bridge.language = root.language && root.language.length ? root.language : "auto"
                    root.refreshDetected()
                })
            }

            onTextChanged: {
                if (text !== root.text)
                    root.contentChanged(text)
                if (bridge && (bridge.language === "auto" || bridge.language === ""))
                    detectTimer.restart()
            }

            onActiveFocusChanged: {
                if (activeFocus && noteEditor)
                    noteEditor.setFocusedBlock(root.blockId)
            }

            Keys.onPressed: function (event) {
                if ((event.modifiers & Qt.ControlModifier) && event.key === Qt.Key_V) {
                    if (noteEditor && noteEditor.pasteImageFromClipboard()) {
                        event.accepted = true
                        return
                    }
                }

                if ((event.modifiers & Qt.ControlModifier) && event.key === Qt.Key_Z) {
                    if (event.modifiers & Qt.ShiftModifier)
                        noteEditor.redo()
                    else
                        noteEditor.undo()
                    event.accepted = true
                    return
                }

                if (event.key === Qt.Key_Tab) {
                    codeArea.insert(codeArea.cursorPosition, "    ")
                    event.accepted = true
                    return
                }

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

                if (event.key === Qt.Key_Enter || event.key === Qt.Key_Return) {
                    event.accepted = false
                    return
                }
            }
        }

        ScrollBar.vertical: ScrollBar { policy: ScrollBar.AsNeeded }
        ScrollBar.horizontal: ScrollBar { policy: ScrollBar.AsNeeded }
    }

    // Empty placeholder
    Text {
        anchors.left: flickable.left
        anchors.top: flickable.top
        anchors.leftMargin: 4
        anchors.topMargin: 4
        text: "// Write code here..."
        color: "#585b70"
        font.pixelSize: 13
        font.family: codeArea.font.family
        visible: codeArea.text.length === 0 && !codeArea.activeFocus
    }

    Timer {
        id: detectTimer
        interval: 250
        onTriggered: {
            if (!root || !bridge || !codeArea)
                return
            root.refreshDetected()
        }
    }

    Keys.onPressed: function (event) {
        if (event.key === Qt.Key_Backspace && codeArea.text.length === 0 && !codeArea.activeFocus) {
            noteEditor.deleteBlock(root.blockId)
            event.accepted = true
        }
    }
}