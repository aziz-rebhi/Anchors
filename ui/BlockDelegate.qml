import QtQuick 2.15
import QtQuick.Controls 2.15

Item {
    id: root
    property var blockData: (model && model.blockData) ? model.blockData : null
    property string blockId: (model && model.id) ? model.id : ""

    // Semantic types from BlockModel::TypeRole:
    // 0=Paragraph, 1=H1, 2=H2, 3=H3, 4=Todo, 5=Code, 6=Image, 7=Table
    width: ListView.view ? ListView.view.width : 0
    height: blockLoader.item ? blockLoader.item.height : 30

    Loader {
        id: blockLoader
        anchors.left: parent.left
        anchors.right: parent.right
        sourceComponent: {
            if (!model) return null
            var type = model.type
            switch (type) {
            case "0": return paragraphComponent
            case "1": case "2": case "3": return headingComponent
            case "4": return todoComponent
            case "5": return codeComponent
            case "6": return imageComponent
            case "7": return tableComponent
            case "8": return quoteComponent
            case "9": return dividerComponent
            default: return paragraphComponent
            }
        }
    }

    Component {
        id: paragraphComponent
        ParagraphBlock {
            blockId: root.blockId
            text: root.blockData ? root.blockData.text : ""
            onContentChanged: function(newText) {
                if (noteEditor) noteEditor.updateBlockContent(blockId, newText)
            }
        }
    }

    Component {
        id: headingComponent
        HeadingBlock {
            blockId: root.blockId
            level: parseInt(model.type)
            text: root.blockData ? root.blockData.text : ""
            onContentChanged: function(newText) {
                if (noteEditor) noteEditor.updateBlockContent(blockId, newText)
            }
        }
    }

    Component {
        id: todoComponent
        TodoBlock {
            blockId: root.blockId
            text: root.blockData ? root.blockData.text : ""
            checked: root.blockData ? root.blockData.checked : false
            onContentChanged: function(newText) {
                if (noteEditor) noteEditor.updateBlockContent(blockId, newText)
            }
            onTodoCheckedChanged: function(isChecked) {
                if (noteEditor) noteEditor.toggleBlockChecked(blockId, isChecked)
            }
        }
    }

    Component {
        id: quoteComponent
        QuoteBlock {
            blockId: root.blockId
            text: root.blockData ? root.blockData.text : ""
            onContentChanged: function(newText) {
                if (noteEditor) noteEditor.updateBlockContent(blockId, newText)
            }
        }
    }

    Component {
        id: dividerComponent
        DividerBlock {
            blockId: root.blockId
        }
    }

    Component {
        id: codeComponent
        CodeBlock {
            blockId: root.blockId
            text: root.blockData ? root.blockData.code : ""
            language: root.blockData ? root.blockData.language : ""
            onContentChanged: function(newText) {
                if (noteEditor) noteEditor.updateBlockContent(blockId, newText)
            }
        }
    }

    Component {
        id: imageComponent
        ImageBlock {
            blockId: root.blockId
            source: root.blockData ? root.blockData.filePath : ""
            caption: root.blockData ? root.blockData.caption : ""
        }
    }

    Component {
        id: tableComponent
        TableBlock {
            blockId: root.blockId
            rows: root.blockData ? root.blockData.rows : 0
            cols: root.blockData ? root.blockData.cols : 0
            cells: root.blockData ? root.blockData.cells : []
        }
    }
}