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
        spacing: 6

        Text {
            text: "∑ Equation"
            color: "#888"
            font.pixelSize: 11
        }

        // Simple preview (raw LaTeX until KaTeX is wired)
        Text {
            Layout.fillWidth: true
            text: root.latex.length ? root.latex : "…"
            color: "#c4b5fd"
            font.pixelSize: 16
            font.family: "monospace"
            wrapMode: Text.Wrap
        }

        TextArea {
            id: input
            Layout.fillWidth: true
            Layout.preferredHeight: Math.max(36, implicitHeight)
            text: root.latex
            placeholderText: "LaTeX, e.g. E = mc^2 or \\int_0^1 x^2 dx"
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
                }
            }
        }
    }
}