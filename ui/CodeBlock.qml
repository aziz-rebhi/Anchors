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

    Theme { id: theme }

    width: parent ? parent.width : 0
    height: Math.max(180, header.height + Math.max(120, codeArea.contentHeight) + 40)
    color: theme.codeBg
    radius: 8
    border.color: theme.border
    border.width: 1

    readonly property var langOptions: [
        { label: "Auto", value: "auto" },
        { label: "Plain Text", value: "text" },
        { label: "JavaScript", value: "js" },
        { label: "Python", value: "python" },
        { label: "c / C++", value: "cpp" },
        { label: "Bash / Linux", value: "bash" },
        { label: "QML", value: "qml" },
        { label: "HTML", value: "html" },
        { label: "CSS", value: "css" },
        { label: "JSON", value: "json" },
        { label: "SQL", value: "text" },
        { label: "Markdown", value: "text" },
        { label: "YAML", value: "text" },
        { label: "XML", value: "text" },
        { label: "Java", value: "text" },
        { label: "C#", value: "text" },
        { label: "Go", value: "text" },
        { label: "Rust", value: "text" }
    ]

    property string detectedLabel: ""

    function focusInput() {
        codeArea.forceActiveFocus()
        codeArea.cursorPosition = codeArea.text.length
    }

    function labelForValue(v) {
        var val = v && v.length ? v : "auto"
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
        if (!bridge || !codeArea) return
        var id = bridge.detectCode ? bridge.detectCode(codeArea.text) : ""
        root.detectedLabel = (root.language === "auto" || root.language === "")
                           ? labelForValue(id) : ""
        if (bridge.redetect) bridge.redetect()
    }

    CodeHighlightBridge { id: bridge }

    Component.onCompleted: {
        bridge.language = root.language && root.language.length ? root.language : "auto"
    }

    RowLayout {
        id: header
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.margins: 10
        anchors.bottomMargin: 0
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
                color: theme.codeHeader
                radius: 4
                border.color: langCombo.hovered ? theme.border : "transparent"
            }
            contentItem: Text {
                text: langCombo.displayText
                font: langCombo.font
                color: theme.codeText
                verticalAlignment: Text.AlignVCenter
                leftPadding: 6
            }
            onActivated: function (index) {
                root.persistLanguage(root.valueForLabel(langCombo.model[index]))
            }
            Component.onCompleted: {
                var idx = model.indexOf(root.labelForValue(root.language))
                if (idx >= 0) currentIndex = idx
            }
        }

        Text {
            visible: root.detectedLabel.length > 0
                     && (root.language === "auto" || root.language === "")
            text: "· " + root.detectedLabel
            font.pixelSize: 10
            color: theme.codeMuted
        }

        Item { Layout.fillWidth: true }

        Text {
            text: "Shift+Enter to exit"
            font.pixelSize: 10
            color: theme.codeMuted
            opacity: codeArea.activeFocus ? 0.7 : 0
        }

        Text {
            text: "Copy"
            font.pixelSize: 11
            color: theme.textSecondary
            opacity: copyArea.containsMouse ? 1 : 0.55
            MouseArea {
                id: copyArea
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor
                onClicked: {
                    codeArea.selectAll(); codeArea.copy(); codeArea.deselect()
                }
            }
        }

        Text {
            text: "\u2715"
            font.pixelSize: 13
            color: theme.textSecondary
            opacity: deleteArea.containsMouse ? 1 : 0.4
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
        height: 1
        color: theme.border
    }

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

        TextEdit {
            id: codeArea
            width: Math.max(flickable.width, contentWidth + 4)
            height: Math.max(flickable.height, contentHeight + 4)
            text: root.text
            color: theme.codeText
            selectedTextColor: theme.codeBg
            selectionColor: theme.tertiary
            font.pixelSize: 13
            font.family: "JetBrains Mono, Cascadia Code, Fira Code, monospace"
            wrapMode: TextEdit.NoWrap
            selectByMouse: true
            persistentSelection: true
            activeFocusOnPress: true
            cursorVisible: activeFocus

            Component.onCompleted: {
                Qt.callLater(function () {
                    if (!codeArea || !bridge) return
                    bridge.attach(codeArea.textDocument)
                    bridge.language = root.language && root.language.length ? root.language : "auto"
                    root.refreshDetected()
                })
            }

            onTextChanged: {
                if (text !== root.text) root.contentChanged(text)
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
                    if (event.modifiers & Qt.ShiftModifier) noteEditor.redo()
                    else noteEditor.undo()
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

    Text {
        anchors.left: flickable.left
        anchors.top: flickable.top
        anchors.margins: 4
        text: "// Write code here..."
        color: theme.codeMuted
        font.pixelSize: 13
        font.family: codeArea.font.family
        visible: codeArea.text.length === 0 && !codeArea.activeFocus
    }

    Timer {
        id: detectTimer
        interval: 250
        onTriggered: if (root && bridge && codeArea) root.refreshDetected()
    }
}