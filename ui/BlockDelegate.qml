import QtQuick 2.15
import QtQuick.Controls 2.15

Item {
    id: root
    Theme { id: theme }

    property var blockData: (model && model.blockData) ? model.blockData : null
    property string blockId: (model && model.id) ? model.id : ""
    property int blockType: (model && model.type !== undefined) ? parseInt(model.type) : 0
    property int depth: (model && model.depth !== undefined) ? parseInt(model.depth) : 0
    property bool dragging: false
    property bool rowHovered: false

    width: ListView.view ? ListView.view.width : 0
    height: blockLoader.item ? blockLoader.item.height : 30
    property alias innerLoader: blockLoader

    function focusInput() {
        if (blockLoader && blockLoader.item && blockLoader.item.focusInput)
            blockLoader.item.focusInput()
    }

    function extractText() {
        if (!root.blockData) return ""
        if (root.blockData.text !== undefined) return root.blockData.text || ""
        if (root.blockData.code !== undefined) return root.blockData.code || ""
        if (root.blockData.latex !== undefined) return root.blockData.latex || ""
        return ""
    }

    HoverHandler {
        onHoveredChanged: root.rowHovered = hovered
    }

    Rectangle {
        id: grip
        width: 18
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        color: gripMa.containsMouse || root.dragging ? theme.surfaceAlt : "transparent"
        radius: 4
        visible: root.depth === 0
        z: 10
        opacity: root.rowHovered || root.dragging || menuBtn.containsMouse ? 1 : 0.35

        Text {
            anchors.centerIn: parent
            text: "⋮⋮"
            font.pixelSize: 10
            color: theme.textMuted
        }

        MouseArea {
            id: gripMa
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: Qt.SizeAllCursor
            preventStealing: true

            onPressed: function (mouse) {
                root.dragging = true
                if (!root.ListView.view) return
                root.ListView.view.dragBlockId = root.blockId
                root.ListView.view.dragFromIndex = index
                root.ListView.view.dragHoverY = mapToItem(root.ListView.view, 0, mouse.y).y
            }
            onPositionChanged: function (mouse) {
                if (!pressed || !root.ListView.view) return
                root.ListView.view.dragHoverY = mapToItem(root.ListView.view, 0, mouse.y).y
            }
            onReleased: {
                root.dragging = false
                if (root.ListView.view) root.ListView.view.finishDrag()
            }
            onCanceled: {
                root.dragging = false
                if (root.ListView.view) root.ListView.view.cancelDrag()
            }
        }
    }

    Rectangle {
        id: menuBtn
        width: 22
        height: 22
        anchors.left: grip.right
        anchors.leftMargin: 2
        anchors.verticalCenter: parent.verticalCenter
        radius: 4
        visible: root.depth === 0
        color: menuBtnMa.containsMouse ? theme.surfaceAlt : "transparent"
        opacity: root.rowHovered || menuBtnMa.containsMouse || blockMenu.visible ? 1 : 0
        z: 10
        property bool containsMouse: menuBtnMa.containsMouse

        Text {
            anchors.centerIn: parent
            text: "⋯"
            font.pixelSize: 14
            color: theme.textMuted
        }

        MouseArea {
            id: menuBtnMa
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: Qt.PointingHandCursor
            onClicked: blockMenu.open()
        }
    }

    Menu {
        id: blockMenu
        x: menuBtn.x
        y: menuBtn.y + menuBtn.height

        MenuItem {
            text: "Duplicate"
            onTriggered: {
                if (!noteEditor) return
                if (noteEditor.duplicateBlock)
                    noteEditor.duplicateBlock(root.blockId)
                else
                    noteEditor.insertBlockAfter(root.blockId, root.blockType, root.extractText())
            }
        }
        MenuItem {
            text: "Delete"
            onTriggered: if (noteEditor) noteEditor.deleteBlock(root.blockId)
        }
        MenuSeparator {}
        Menu {
            title: "Turn into"
            MenuItem { text: "Text";          onTriggered: noteEditor.changeBlockType(root.blockId, 0) }
            MenuItem { text: "Heading 1";     onTriggered: noteEditor.changeBlockType(root.blockId, 1) }
            MenuItem { text: "Heading 2";     onTriggered: noteEditor.changeBlockType(root.blockId, 2) }
            MenuItem { text: "Heading 3";     onTriggered: noteEditor.changeBlockType(root.blockId, 3) }
            MenuItem { text: "Heading 4";     onTriggered: noteEditor.changeBlockType(root.blockId, 10) }
            MenuSeparator {}
            MenuItem { text: "Bulleted list"; onTriggered: noteEditor.changeBlockType(root.blockId, 11) }
            MenuItem { text: "Numbered list"; onTriggered: noteEditor.changeBlockType(root.blockId, 13) }
            MenuItem { text: "To-do";         onTriggered: noteEditor.changeBlockType(root.blockId, 4) }
            MenuItem { text: "Toggle";        onTriggered: noteEditor.changeBlockType(root.blockId, 15) }
            MenuSeparator {}
            MenuItem { text: "Quote";         onTriggered: noteEditor.changeBlockType(root.blockId, 9) }
            MenuItem { text: "Callout";       onTriggered: noteEditor.changeBlockType(root.blockId, 12) }
            MenuItem { text: "Code";          onTriggered: noteEditor.changeBlockType(root.blockId, 5) }
            MenuItem { text: "Equation";      onTriggered: noteEditor.changeBlockType(root.blockId, 14) }
            MenuSeparator {}
            MenuItem { text: "Divider";       onTriggered: noteEditor.changeBlockType(root.blockId, 8) }
            MenuItem { text: "Columns";       onTriggered: noteEditor.changeBlockType(root.blockId, 16) }
        }
    }

    Loader {
        id: blockLoader
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.leftMargin: (root.depth > 0 ? 28 : 0) + 42

        sourceComponent: {
            if (!model) return null
            switch (root.blockType) {
            case 0:  return paragraphComponent
            case 1: case 2: case 3: case 10: return headingComponent
            case 4:  return todoComponent
            case 5:  return codeComponent
            case 6:  return imageComponent
            case 7:  return tableComponent
            case 8:  return dividerComponent
            case 9:  return quoteComponent
            case 11: return bulletComponent
            case 12: return calloutComponent
            case 13: return numberedComponent
            case 14: return equationComponent
            case 15: return toggleComponent
            case 16: return columnsComponent
            default: return paragraphComponent
            }
        }
    }

    Component {
        id: paragraphComponent
        ParagraphBlock {
            blockId: root.blockId
            text: root.blockData ? root.blockData.text || "" : ""
            onContentChanged: function (newText) {
                if (noteEditor) noteEditor.updateBlockContent(blockId, newText)
            }
        }
    }
    Component {
        id: headingComponent
        HeadingBlock {
            blockId: root.blockId
            level: root.blockData && root.blockData.level !== undefined
                   ? root.blockData.level
                   : (root.blockType <= 3 ? root.blockType : 4)
            text: root.blockData ? root.blockData.text || "" : ""
            onContentChanged: function (newText) {
                if (noteEditor) noteEditor.updateBlockContent(blockId, newText)
            }
        }
    }
    Component {
        id: todoComponent
        TodoBlock {
            blockId: root.blockId
            text: root.blockData ? root.blockData.text || "" : ""
            checked: root.blockData ? root.blockData.checked || false : false
            onContentChanged: function (newText) {
                if (noteEditor) noteEditor.updateBlockContent(blockId, newText)
            }
        }
    }
    Component {
        id: codeComponent
        CodeBlock {
            blockId: root.blockId
            text: root.blockData ? root.blockData.code || "" : ""
            language: root.blockData ? root.blockData.language || "" : ""
            onContentChanged: function (newText) {
                if (noteEditor) noteEditor.updateBlockContent(blockId, newText)
            }
        }
    }
    Component {
        id: imageComponent
        ImageBlock {
            blockId: root.blockId
            source: root.blockData ? root.blockData.source || "" : ""
            caption: root.blockData ? root.blockData.caption || "" : ""
        }
    }
    Component {
        id: tableComponent
        TableBlock {
            blockId: root.blockId
            rows: root.blockData ? root.blockData.rows || 1 : 1
            cols: root.blockData ? root.blockData.cols || 1 : 1
            cells: root.blockData ? root.blockData.cells || [] : []
        }
    }
    Component {
        id: dividerComponent
        DividerBlock {
            blockId: root.blockId
            orientation: root.blockData ? (root.blockData.orientation || 0) : 0
        }
    }
    Component {
        id: quoteComponent
        QuoteBlock {
            blockId: root.blockId
            text: root.blockData ? root.blockData.text || "" : ""
            onContentChanged: function (newText) {
                if (noteEditor) noteEditor.updateBlockContent(blockId, newText)
            }
        }
    }
    Component {
        id: calloutComponent
        CalloutBlock {
            blockId: root.blockId
            text: root.blockData ? root.blockData.text || "" : ""
            emoji: root.blockData ? root.blockData.emoji || "💡" : "💡"
            onContentChanged: function (newText) {
                if (noteEditor) noteEditor.updateBlockContent(blockId, newText)
            }
        }
    }
    Component {
        id: bulletComponent
        BulletBlock {
            blockId: root.blockId
            text: root.blockData ? root.blockData.text || "" : ""
            indent: root.blockData ? (root.blockData.indent || 0) : 0
            onContentChanged: function (t) {
                if (noteEditor) noteEditor.updateBlockContent(blockId, t)
            }
        }
    }
    Component {
        id: numberedComponent
        NumberedBlock {
            blockId: root.blockId
            text: root.blockData ? root.blockData.text || "" : ""
            indent: root.blockData ? (root.blockData.indent || 0) : 0
            number: noteEditor ? noteEditor.numberedIndex(root.blockId) : 1
            onContentChanged: function (t) {
                if (noteEditor) noteEditor.updateBlockContent(blockId, t)
            }
        }
    }
    Component {
        id: equationComponent
        EquationBlock {
            blockId: root.blockId
            latex: root.blockData ? root.blockData.latex || "" : ""
            displayMode: root.blockData ? (root.blockData.displayMode !== false) : true
            onContentChanged: function (t) {
                if (noteEditor) noteEditor.updateBlockContent(blockId, t)
            }
        }
    }
    Component {
        id: toggleComponent
        ToggleBlock {
            blockId: root.blockId
            text: root.blockData ? root.blockData.text || "" : ""
            collapsed: root.blockData ? root.blockData.collapsed === true : false
            onContentChanged: function (t) {
                if (noteEditor) noteEditor.updateBlockContent(blockId, t)
            }
        }
    }
    Component {
        id: columnsComponent
        ColumnsBlock {
            blockId: root.blockId
            columnCount: root.blockData ? (root.blockData.count || 2) : 2
        }
    }
}