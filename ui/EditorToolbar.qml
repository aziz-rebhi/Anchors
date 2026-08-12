import QtQuick 2.15
import QtQuick.Controls 2.15

Rectangle {
    id: root
    height: 40
    color: "transparent"
    property string currentBlockId: ""

    Theme { id: theme }

    signal insertType(int typeCode)
    signal changeType(int typeCode)

    function applyType(typeCode) {
        var id = root.currentBlockId
        if ((!id || !id.length) && noteEditor)
            id = noteEditor.focusedBlockId || ""
        if (id && id.length)
            root.changeType(typeCode)
        else
            root.insertType(typeCode)
    }

    Flickable {
        anchors.fill: parent
        anchors.leftMargin: 4
        contentWidth: row.width
        contentHeight: height
        clip: true
        flickableDirection: Flickable.HorizontalFlick
        interactive: contentWidth > width

        Row {
            id: row
            anchors.verticalCenter: parent.verticalCenter
            spacing: 2

            ToolbarButton { label: "T";   tooltip: "Text";          onClicked: root.applyType(0) }
            ToolbarButton { label: "H1";  tooltip: "Heading 1";     onClicked: root.applyType(1) }
            ToolbarButton { label: "H2";  tooltip: "Heading 2";     onClicked: root.applyType(2) }
            ToolbarButton { label: "H3";  tooltip: "Heading 3";     onClicked: root.applyType(3) }
            ToolbarButton { label: "H4";  tooltip: "Heading 4";     onClicked: root.applyType(10) }

            ToolSep {}

            ToolbarButton { label: "•";   tooltip: "Bulleted list"; onClicked: root.applyType(11) }
            ToolbarButton { label: "1.";  tooltip: "Numbered list"; onClicked: root.applyType(13) }
            ToolbarButton { label: "☑";   tooltip: "To-do";         onClicked: root.applyType(4) }
            ToolbarButton { label: "▶";   tooltip: "Toggle";        onClicked: root.applyType(15) }

            ToolSep {}

            ToolbarButton { label: "“";   tooltip: "Quote";         onClicked: root.applyType(9) }
            ToolbarButton { label: "💡";  tooltip: "Callout";       onClicked: root.applyType(12) }
            ToolbarButton { label: "∑";   tooltip: "Equation";      onClicked: root.applyType(14) }
            ToolbarButton { label: "</>"; tooltip: "Code"; font.pixelSize: 11; onClicked: root.applyType(5) }

            ToolSep {}

            ToolbarButton { label: "—";   tooltip: "Divider"; font.pixelSize: 10; onClicked: root.applyType(8) }
            ToolbarButton { label: "🖼";  tooltip: "Image";         onClicked: root.applyType(6) }
            ToolbarButton { label: "▦";   tooltip: "Table";         onClicked: root.applyType(7) }
            ToolbarButton { label: "║║";  tooltip: "2 columns";     onClicked: root.applyType(16) }

            ToolSep {}

            ToolbarButton {
                label: "↶"; tooltip: "Undo (Ctrl+Z)"
                enabled: noteEditor && noteEditor.canUndo
                opacity: enabled ? 1 : 0.35
                onClicked: if (noteEditor) noteEditor.undo()
            }
            ToolbarButton {
                label: "↷"; tooltip: "Redo (Ctrl+Shift+Z)"
                enabled: noteEditor && noteEditor.canRedo
                opacity: enabled ? 1 : 0.35
                onClicked: if (noteEditor) noteEditor.redo()
            }
        }
    }

    component ToolSep : Rectangle {
        width: 1; height: 20
        color: theme.border
        anchors.verticalCenter: parent ? parent.verticalCenter : undefined
    }

    component ToolbarButton : Rectangle {
        id: btn
        property string label: ""
        property string tooltip: ""
        property alias font: labelItem.font
        signal clicked()

        width: Math.max(32, labelItem.implicitWidth + 12)
        height: 28
        radius: 4
        color: btnArea.containsMouse ? Qt.rgba(1, 1, 1, 0.08) : "transparent"
        Behavior on color { ColorAnimation { duration: 100 } }

        Text {
            id: labelItem
            anchors.centerIn: parent
            text: btn.label
            font.pixelSize: 13
            font.bold: true
            font.family: theme.labelFont
            color: btnArea.containsMouse ? theme.textPrimary : theme.textSecondary
        }

        MouseArea {
            id: btnArea
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: Qt.PointingHandCursor
            onClicked: btn.clicked()
        }

        ToolTip.visible: btnArea.containsMouse && btn.tooltip.length > 0
        ToolTip.text: btn.tooltip
        ToolTip.delay: 400
    }
}