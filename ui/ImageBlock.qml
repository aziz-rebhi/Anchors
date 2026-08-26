import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Rectangle {
    id: root
    Theme { id: theme }

    property string blockId: ""
    property string latex: ""
    property string source: ""
    property string caption: ""
    property bool displayMode: true
    signal contentChanged(string newLatex)

    width: parent ? parent.width : 0
    height: col.implicitHeight + 16
    radius: 8
    color: theme.surfaceAlt
    border.color: theme.border
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
            color: theme.textMuted
            font.pixelSize: 11
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: Math.max(40, previewText.implicitHeight + 16)
            radius: 6
            color: theme.surface
            border.color: theme.border
            border.width: 1

            Text {
                id: previewText
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                anchors.margins: 10
                text: root.latex.length ? root.latex : "…"
                color: theme.tertiary
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
            color: theme.textPrimary
            placeholderTextColor: theme.textMuted
            background: Rectangle {
                color: theme.surface
                radius: 4
                border.color: theme.border
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