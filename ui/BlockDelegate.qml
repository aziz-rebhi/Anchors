import QtQuick 2.15
import QtQuick.Controls 2.15

Item {
    id: root
    property var blockData: (model && model.blockData) ? model.blockData : null
    property string blockId: (model && model.id) ? model.id : ""
    property int blockType: (model && model.type !== undefined) ? parseInt(model.type) : 0

    width: ListView.view ? ListView.view.width : 0
    height: blockLoader.item ? blockLoader.item.height : 30

    // Expose for the focus mechanism in BlockList
    property alias innerLoader: blockLoader

    // Called by BlockList's focus timer
    function focusInput() {
        if (blockLoader && blockLoader.item && blockLoader.item.focusInput) {
            blockLoader.item.focusInput()
        }
    }

    Loader {
        id: blockLoader
        anchors.left: parent.left
        anchors.right: parent.right
        sourceComponent: {
            if (!model) return null
            switch (root.blockType) {
            case 0:  return paragraphComponent
            case 1:  return headingComponent
            case 2:  return headingComponent
            case 3:  return headingComponent
            case 4:  return todoComponent
            case 5:  return codeComponent
            case 6:  return imageComponent
            case 7:  return tableComponent
            case 8:  return dividerComponent
            case 9:  return quoteComponent
            default: return paragraphComponent
            }
        }
    }

    Component {
        id: paragraphComponent
        ParagraphBlock {
            blockId: root.blockId
            text: root.blockData ? root.blockData.text || "" : ""
            onContentChanged: function(newText) {
                if (noteEditor) noteEditor.updateBlockContent(blockId, newText)
            }
        }
    }

    Component {
        id: headingComponent
        HeadingBlock {
            blockId: root.blockId
            level: root.blockType
            text: root.blockData ? root.blockData.text || "" : ""
            onContentChanged: function(newText) {
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
            onContentChanged: function(newText) {
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
            onContentChanged: function(newText) {
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
            onContentChanged: function(newText) {
                if (noteEditor) noteEditor.updateBlockContent(blockId, newText)
            }
        }
    }
}
