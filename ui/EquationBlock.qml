import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Rectangle {
    id: root
    property string blockId: ""
    property string latex: ""
    property bool displayMode: true
    signal contentChanged(string newLatex)

    width: parent ? parent.width : 0
    height: col.implicitHeight + 16
    radius: 8
    color: "#1a1a24"
    border.color: "#3a3a4a"
    border.width: 1

    function focusInput() {
        input.forceActiveFocus()
        input.cursorPosition = input.text.length
    }

    ColumnLayout {
        id: col
        anchors.fill: parent
        anchors.margins: 10
        spacing: 8

        Text {
            text: "∑ Equation"
            color: "#888"
            font.pixelSize: 11
        }

        // Lightweight preview (raw LaTeX styled) — no Chromium
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: Math.max(40, previewText.implicitHeight + 16)
            radius: 6
            color: "#12121a"
            border.color: "#333"
            border.width: 1

            Text {
                id: previewText
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                anchors.margins: 10
                text: root.latex.length ? root.latex : "…"
                color: "#c4b5fd"
                font.pixelSize: 18
                font.family: "serif"
                wrapMode: Text.Wrap
                horizontalAlignment: Text.AlignHCenter
            }
        }

        TextArea {
            id: input
            Layout.fillWidth: true
            Layout.preferredHeight: Math.max(36, implicitHeight)
            text: root.latex
            placeholderText: "LaTeX, e.g. E = mc^2 or \\frac{a}{b}"
            wrapMode: Text.Wrap
            font.pixelSize: 13
            font.family: "monospace"
            color: "#ddd"
            background: Rectangle {
                color: "#12121a"
                radius: 4
                border.color: "#333"
            }
            onTextChanged: if (text !== root.latex) root.contentChanged(text)
            onActiveFocusChanged: if (activeFocus && noteEditor) noteEditor.setFocusedBlock(root.blockId)

            Keys.onPressed: function (e) {
                if ((e.modifiers & Qt.ControlModifier) &&
                    (e.key === Qt.Key_Return || e.key === Qt.Key_Enter)) {
                    noteEditor.insertBlockAfter(root.blockId, 0, "")
                    e.accepted = true
                    return
                }
                if (e.key === Qt.Key_Up) {
                    noteEditor.focusAdjacent(root.blockId, false)
                    e.accepted = true
                    return
                }
                if (e.key === Qt.Key_Down) {
                    noteEditor.focusAdjacent(root.blockId, true)
                    e.accepted = true
                    return
                }
            }
        }
    }
}