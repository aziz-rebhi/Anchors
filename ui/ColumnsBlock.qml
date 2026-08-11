import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Rectangle {
    id: root
    property string blockId: ""
    property int columnCount: 2
    property int activeColumn: 0
    property string pendingCellFocus: ""
    property string slashCellId: ""
    property string slashCellText: ""
    property bool chromeVisible: rootHovered || root.activeFocus
    property bool rootHovered: false

    width: parent ? parent.width : 0
    height: mainCol.implicitHeight + 12
    radius: 8
    color: "#1a1a22"
    border.color: "#3a3a4a"
    border.width: 1

    HoverHandler { onHoveredChanged: root.rootHovered = hovered }

    ListModel { id: childrenModel }
    property string lastStructure: ""

    function structureKey(list) {
        var s = ""
        for (var i = 0; i < list.length; i++)
            s += list[i].id + ":" + list[i].columnIndex + ":" + list[i].type + ":" + (list[i].checked ? "1" : "0") + ";"
        return s + "|c=" + root.columnCount
    }

    function refresh(force) {
        if (!noteEditor) return
        var list = noteEditor.columnChildren(root.blockId)
        var key = structureKey(list)
        if (!force && key === lastStructure) {
            for (var i = 0; i < list.length; i++) {
                for (var j = 0; j < childrenModel.count; j++) {
                    if (childrenModel.get(j).id === list[i].id) {
                        if (childrenModel.get(j).text !== list[i].text)
                            childrenModel.setProperty(j, "text", list[i].text)
                        if (childrenModel.get(j).type !== list[i].type)
                            childrenModel.setProperty(j, "type", list[i].type)
                        if (childrenModel.get(j).checked !== list[i].checked)
                            childrenModel.setProperty(j, "checked", list[i].checked)
                        break
                    }
                }
            }
            return
        }
        lastStructure = key
        childrenModel.clear()
        for (var k = 0; k < list.length; k++)
            childrenModel.append(list[k])
        if (root.pendingCellFocus.length)
            focusTimer.restart()
    }

    function focusCellById(id) {
        if (!id || !id.length) return false
        for (var c = 0; c < columnRepeater.count; c++) {
            var colItem = columnRepeater.itemAt(c)
            if (!colItem || !colItem.colContent) continue
            var content = colItem.colContent
            for (var i = 0; i < content.children.length; i++) {
                var wrap = content.children[i]
                if (!wrap || !wrap.visible) continue
                var ta = wrap.cell
                if (ta && ta.blockId === id) {
                    ta.forceActiveFocus()
                    ta.cursorPosition = ta.text.length
                    root.activeColumn = c
                    return true
                }
            }
        }
        return false
    }

    function focusColumn(col) {
        if (col < 0 || col >= root.columnCount) return
        root.activeColumn = col
        var lastId = ""
        for (var i = 0; i < childrenModel.count; i++) {
            if (childrenModel.get(i).columnIndex === col)
                lastId = childrenModel.get(i).id
        }
        if (lastId.length) {
            root.pendingCellFocus = lastId
            focusTimer.restart()
        } else if (noteEditor) {
            noteEditor.insertInColumn(root.blockId, col, 0, "")
        }
    }

    function openSlashForCell(cellItem) {
        root.slashCellId = cellItem.blockId
        root.slashCellText = cellItem.text
        slashMenu.filterText = ""
        var pos = cellItem.mapToItem(Overlay.overlay, 0, cellItem.height)
        var menuH = 320
        var overlayH = Overlay.overlay ? Overlay.overlay.height : 800
        slashMenu.cursorX = Math.max(8, pos.x)
        slashMenu.cursorY = (pos.y + menuH > overlayH - 16)
            ? Math.max(8, pos.y - cellItem.height - menuH - 8)
            : pos.y + 4
        slashMenu.open()
    }

    function numberedPrefix(cellId, colIndex) {
        var n = 0
        for (var i = 0; i < childrenModel.count; i++) {
            var it = childrenModel.get(i)
            if (it.columnIndex !== colIndex) continue
            if (it.type === 13) {
                n++
                if (it.id === cellId)
                    return n + ". "
            } else {
                n = 0
            }
        }
        return "1. "
    }

    function fontSizeForType(t) {
        if (t === 1) return 22
        if (t === 2) return 18
        if (t === 3) return 16
        if (t === 10) return 15
        return 14
    }

    function prefixForType(t, cellId, colIndex) {
        if (t === 11) return "• "
        if (t === 13) return numberedPrefix(cellId, colIndex)
        if (t === 9)  return "“ "
        if (t === 12) return "💡 "
        return ""
    }

    Timer {
        id: focusTimer
        interval: 0
        onTriggered: {
            if (root.pendingCellFocus.length) {
                root.focusCellById(root.pendingCellFocus)
                root.pendingCellFocus = ""
            }
        }
    }

    SlashCommandMenu {
        id: slashMenu
        blockId: root.slashCellId
        parent: Overlay.overlay

        onBlockSelected: function (menuBlockId, typeCode) {
            var cellId = root.slashCellId
            if (!cellId.length || !noteEditor) {
                slashMenu.close()
                return
            }
            if (typeCode === 16 || typeCode === 17 || typeCode === 18) {
                slashMenu.close()
                return
            }

            var colIndex = 0
            for (var j = 0; j < childrenModel.count; j++) {
                if (childrenModel.get(j).id === cellId) {
                    colIndex = childrenModel.get(j).columnIndex
                    break
                }
            }

            var full = root.slashCellText.length ? root.slashCellText : ""
            if (!full.length) {
                for (var k = 0; k < childrenModel.count; k++) {
                    if (childrenModel.get(k).id === cellId) {
                        full = childrenModel.get(k).text || ""
                        break
                    }
                }
            }

            var slashPos = full.lastIndexOf("/")
            var before = slashPos >= 0 ? full.substring(0, slashPos) : full
            if (before.endsWith("\n"))
                before = before.substring(0, before.length - 1)

            noteEditor.updateBlockContent(cellId, before)
            slashMenu.close()

            var prior = before.replace(/\s/g, "")
            if (prior.length > 0) {
                noteEditor.insertInColumn(root.blockId, colIndex, typeCode, "")
            } else {
                if (typeCode !== 0)
                    noteEditor.changeBlockType(cellId, typeCode)
                root.pendingCellFocus = cellId
                focusTimer.restart()
            }
        }
    }

    Component.onCompleted: refresh(true)
    Connections {
        target: noteEditor
        function onDocumentModified() { root.refresh(false) }
        function onPendingFocusIdChanged(id) {
            if (!id || !id.length) return
            root.pendingCellFocus = id
            focusTimer.restart()
        }
    }

    Column {
        id: mainCol
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.margins: 8
        spacing: 6

        RowLayout {
            width: parent.width
            spacing: 8

            Repeater {
                id: columnRepeater
                model: root.columnCount

                Rectangle {
                    id: colBox
                    property int colIndex: index
                    property alias colContent: colContent
                    Layout.fillWidth: true
                    Layout.minimumHeight: 56
                    implicitHeight: Math.max(56, colContent.implicitHeight + 12)
                    Layout.preferredHeight: implicitHeight
                    radius: 6
                    color: "#12121a"
                    border.color: root.activeColumn === colIndex ? "#5a5a8a" : "#333"
                    border.width: 1

                    Column {
                        id: colContent
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.top: parent.top
                        anchors.margins: 6
                        spacing: 4

                        Repeater {
                            model: childrenModel
                            delegate: Item {
                                width: colContent.width
                                height: visible ? cellRow.height : 0
                                visible: columnIndex === colBox.colIndex
                                property alias cell: cell

                                Row {
                                    id: cellRow
                                    width: parent.width
                                    spacing: 6

                                    Rectangle {
                                        visible: (model.type || 0) === 4
                                        width: 18
                                        height: 18
                                        anchors.top: parent.top
                                        anchors.topMargin: 4
                                        radius: 4
                                        color: "transparent"
                                        border.color: model.checked ? "#88c0d0" : "#888"
                                        border.width: 2
                                        Text {
                                            anchors.centerIn: parent
                                            text: "✓"
                                            color: "#88c0d0"
                                            visible: model.checked === true
                                            font.pixelSize: 12
                                        }
                                        MouseArea {
                                            anchors.fill: parent
                                            cursorShape: Qt.PointingHandCursor
                                            onClicked: if (noteEditor) noteEditor.toggleBlockChecked(model.id)
                                        }
                                    }

                                    Text {
                                        id: prefix
                                        text: root.prefixForType(model.type || 0, model.id, colBox.colIndex)
                                        color: "#a6adc8"
                                        font.pixelSize: root.fontSizeForType(model.type || 0)
                                        font.bold: (model.type >= 1 && model.type <= 3) || model.type === 10
                                        visible: text.length > 0 && (model.type || 0) !== 4
                                        anchors.top: parent.top
                                        anchors.topMargin: 4
                                    }

                                    TextArea {
                                        id: cell
                                        property string blockId: model.id
                                        property int blockType: model.type || 0
                                        width: parent.width
                                               - (prefix.visible ? prefix.width + 6 : 0)
                                               - ((model.type || 0) === 4 ? 24 : 0)
                                        text: model.text || ""
                                        wrapMode: Text.Wrap
                                        color: model.checked ? "#888" : "#ddd"
                                        font.pixelSize: root.fontSizeForType(blockType)
                                        font.bold: (blockType >= 1 && blockType <= 3) || blockType === 10
                                        font.italic: blockType === 9
                                        font.family: blockType === 5 ? "monospace" : font.family
                                        background: Item {}

                                        onTextChanged: {
                                            if (noteEditor && text !== model.text)
                                                noteEditor.updateBlockContent(model.id, text)

                                            if (text.length > 0 && text.charAt(text.length - 1) === "/") {
                                                root.openSlashForCell(cell)
                                                slashMenu.filterText = ""
                                            } else if (slashMenu.visible && root.slashCellId === cell.blockId) {
                                                root.slashCellText = text
                                                var si = text.lastIndexOf("/")
                                                if (si < 0) {
                                                    slashMenu.close()
                                                } else {
                                                    var filter = text.substring(si + 1)
                                                    if (filter.indexOf("\n") >= 0 || filter.indexOf(" ") >= 0)
                                                        slashMenu.close()
                                                    else
                                                        slashMenu.filterText = filter
                                                }
                                            }
                                        }

                                        onActiveFocusChanged: {
                                            if (activeFocus) {
                                                root.activeColumn = colBox.colIndex
                                                if (noteEditor)
                                                    noteEditor.setFocusedBlock(model.id)
                                            }
                                        }

                                        Keys.onPressed: function (e) {
                                            if (slashMenu.visible && root.slashCellId === cell.blockId) {
                                                if (e.key === Qt.Key_Escape) { slashMenu.close(); e.accepted = true; return }
                                                if (e.key === Qt.Key_Enter || e.key === Qt.Key_Return) { slashMenu.selectCurrent(); e.accepted = true; return }
                                                if (e.key === Qt.Key_Up) { slashMenu.moveUp(); e.accepted = true; return }
                                                if (e.key === Qt.Key_Down) { slashMenu.moveDown(); e.accepted = true; return }
                                            }

                                            if ((e.key === Qt.Key_Return || e.key === Qt.Key_Enter)
                                                && (e.modifiers & Qt.ControlModifier)) {
                                                noteEditor.insertBlockAfter(root.blockId, 0, "")
                                                e.accepted = true
                                                return
                                            }

                                            if ((e.key === Qt.Key_Return || e.key === Qt.Key_Enter)
                                                && (e.modifiers & Qt.ShiftModifier)) {
                                                cell.insert(cell.cursorPosition, "\n")
                                                e.accepted = true
                                                return
                                            }

                                            if (e.key === Qt.Key_Return || e.key === Qt.Key_Enter) {
                                                var pos = cell.cursorPosition
                                                var after = cell.text.substring(pos)
                                                cell.text = cell.text.substring(0, pos)
                                                noteEditor.updateBlockContent(model.id, cell.text)
                                                noteEditor.insertInColumn(root.blockId, colBox.colIndex, 0, after)
                                                e.accepted = true
                                                return
                                            }

                                            if (e.key === Qt.Key_Tab && (e.modifiers & Qt.ControlModifier)) {
                                                if (e.modifiers & Qt.ShiftModifier)
                                                    root.focusColumn(Math.max(0, colBox.colIndex - 1))
                                                else
                                                    root.focusColumn(Math.min(root.columnCount - 1, colBox.colIndex + 1))
                                                e.accepted = true
                                                return
                                            }

                                            if (e.key === Qt.Key_Backspace && cell.cursorPosition === 0
                                                && cell.text.length === 0) {
                                                if (childrenModel.count <= 1)
                                                    noteEditor.deleteBlock(root.blockId)
                                                else
                                                    noteEditor.deleteBlock(model.id)
                                                e.accepted = true
                                                return
                                            }
                                        }
                                    }
                                }
                            }
                        }

                        Text {
                            visible: {
                                var n = 0
                                for (var i = 0; i < childrenModel.count; i++)
                                    if (childrenModel.get(i).columnIndex === colBox.colIndex) n++
                                return n === 0
                            }
                            text: "Empty"
                            color: "#555"
                            font.pixelSize: 12
                            padding: 8
                            MouseArea {
                                anchors.fill: parent
                                onClicked: noteEditor.insertInColumn(root.blockId, colBox.colIndex, 0, "")
                            }
                        }
                    }
                }
            }
        }

        Row {
            spacing: 8
            visible: root.chromeVisible
            height: visible ? implicitHeight : 0

            Button {
                text: "+ Column"
                flat: true
                onClicked: if (noteEditor) noteEditor.addColumn(root.blockId)
            }
            Button {
                text: "− Column"
                flat: true
                enabled: root.columnCount > 1
                onClicked: if (noteEditor) noteEditor.removeColumn(root.blockId, root.columnCount - 1)
            }
            Button {
                text: "Delete"
                flat: true
                onClicked: if (noteEditor) noteEditor.deleteBlock(root.blockId)
            }
        }
    }
}