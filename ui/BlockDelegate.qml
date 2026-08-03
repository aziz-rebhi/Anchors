import QtQuick 2.15
import QtQuick.Controls 2.15

Item {
    id: root
    property var blockData: (model && model.data) ? model.data : null
    property string blockId: (model && model.id) ? model.id : ""

    // FIX: give the delegate explicit dimensions
    // The Loader no longer uses anchors.fill (which forced 0 height
    // because the parent Item had no height). Instead the Item
    // sizes itself from the loaded component.
    width: ListView.view ? ListView.view.width : 0
    height: blockLoader.item ? blockLoader.item.height : 30

    Loader {
        id: blockLoader
        // Only anchor horizontally — let height come from the loaded item
        anchors.left: parent.left
        anchors.right: parent.right
        sourceComponent: {
            if (!model) return null
            var type = model.type
            switch (type) {
            case "0": return paragraphComponent
            case "1": return headingComponent
            case "2": return headingComponent
            default: return paragraphComponent
            }
        }
    }

    Component {
        id: paragraphComponent
        ParagraphBlock {
            blockId: root.blockId
            text: root.blockData ? root.blockData.text : ""
            onContentChanged: {
                if (noteEditor) noteEditor.updateBlockContent(blockId, contentChanged)
            }
        }
    }

    Component {
        id: headingComponent
        HeadingBlock {
            blockId: root.blockId
            level: model && model.type === "1" ? 1 : 2
            text: root.blockData ? root.blockData.text : ""
            onContentChanged: {
                if (noteEditor) noteEditor.updateBlockContent(blockId, contentChanged)
            }
        }
    }
}