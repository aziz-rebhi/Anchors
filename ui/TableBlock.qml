import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Item {
    id: root
    property string blockId: ""
    property int rows: 1
    property int cols: 1
    property var cells: []
    property bool tableFocused: false

    width: parent ? parent.width : 0
    height: tableGrid.height + (tableFocused ? 44 : 12)

    function getCellText(r, c) {
        if (root.cells && root.cells.length > r && root.cells[r]) {
            var rowArr = root.cells[r]
            if (rowArr.length !== undefined && rowArr.length > c)
                return rowArr[c] || ""
        }
        return ""
    }

    function collectCells() {
        var newCells = []
        for (var r = 0; r < root.rows; r++) {
            var rowArr = []
            for (var c = 0; c < root.cols; c++) {
                var flatIdx = r * root.cols + c
                var cellItem = tableRepeater.itemAt(flatIdx)
                if (cellItem && cellItem.cellTextInput)
                    rowArr.push(cellItem.cellTextInput.text)
                else
                    rowArr.push(root.getCellText(r, c))
            }
            newCells.push(rowArr)
        }
        return newCells
    }

    function syncCellsToModel() {
        if (noteEditor)
            noteEditor.updateBlockTableData(root.blockId, collectCells())
    }

    function focusInput() {
        var firstCell = tableRepeater.itemAt(0)
        if (firstCell && firstCell.cellTextInput) {
            firstCell.cellTextInput.forceActiveFocus()
            firstCell.cellTextInput.cursorPosition = 0
        }
    }

    function focusCellAt(flatIdx) {
        var cell = tableRepeater.itemAt(flatIdx)
        if (cell && cell.cellTextInput)
            cell.cellTextInput.forceActiveFocus()
    }

    Grid {
        id: tableGrid
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.margins: 8
        rows: root.rows
        columns: root.cols
        rowSpacing: 0
        columnSpacing: 0

        Repeater {
            id: tableRepeater
            model: root.rows * root.cols

            Rectangle {
                width: tableGrid.width / Math.max(root.cols, 1)
                height: 36
                color: "transparent"
                border.color: "#555555"
                border.width: 1

                property alias cellTextInput: cellInput
                property int cellRow: Math.floor(index / root.cols)
                property int cellCol: index % root.cols

                TextInput {
                    id: cellInput
                    anchors.fill: parent
                    anchors.margins: 6
                    verticalAlignment: Text.AlignVCenter
                    font.pixelSize: 13
                    color: "#ffffff"
                    clip: true
                    selectByMouse: true
                    text: root.getCellText(Math.floor(index / root.cols), index % root.cols)

                    onActiveFocusChanged: {
                        if (activeFocus && noteEditor) {
                            noteEditor.setFocusedBlock(root.blockId)
                            root.tableFocused = true
                        }
                    }

                    Keys.onPressed: function(event) {
                        if ((event.modifiers & Qt.ControlModifier) && event.key === Qt.Key_Z) {
                            if (event.modifiers & Qt.ShiftModifier)
                                noteEditor.redo()
                            else
                                noteEditor.undo()
                            event.accepted = true
                            return
                        }
                        if (event.key === Qt.Key_Tab) {
                            var nextIdx = index + 1
                            if (nextIdx < root.rows * root.cols) {
                                root.focusCellAt(nextIdx)
                            } else {
                                root.syncCellsToModel()
                                noteEditor.insertBlockAfter(root.blockId, 0, "")
                            }
                            event.accepted = true
                            return
                        }
                        if (event.key === Qt.Key_Backtab) {
                            var prevIdx = index - 1
                            if (prevIdx >= 0)
                                root.focusCellAt(prevIdx)
                            event.accepted = true
                            return
                        }
                        if (event.key === Qt.Key_Enter || event.key === Qt.Key_Return) {
                            root.syncCellsToModel()
                            noteEditor.insertBlockAfter(root.blockId, 0, "")
                            event.accepted = true
                            return
                        }
                    }
                }
            }
        }
    }

    // Action buttons — only visible while the table has focus
    Row {
        visible: root.tableFocused
        anchors.top: tableGrid.bottom
        anchors.topMargin: 4
        anchors.left: parent.left
        anchors.leftMargin: 8
        spacing: 6

        Button {
            text: "+ Row"
            font.pixelSize: 11
            flat: true
            contentItem: Text {
                text: parent.text
                color: "#cccccc"
                font.pixelSize: 11
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter
            }
            onClicked: {
                root.syncCellsToModel()
                root.rows++
                var cells = root.collectCells()
                while (cells.length < root.rows) {
                    var emptyRow = []
                    for (var i = 0; i < root.cols; i++) emptyRow.push("")
                    cells.push(emptyRow)
                }
                noteEditor.updateBlockTableData(root.blockId, cells)
            }
        }
        Button {
            text: "+ Col"
            font.pixelSize: 11
            flat: true
            contentItem: Text {
                text: parent.text
                color: "#cccccc"
                font.pixelSize: 11
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter
            }
            onClicked: {
                root.syncCellsToModel()
                root.cols++
                var cells = root.collectCells()
                for (var r = 0; r < cells.length; r++) {
                    while (cells[r].length < root.cols)
                        cells[r].push("")
                }
                noteEditor.updateBlockTableData(root.blockId, cells)
            }
        }
        Button {
            text: "Delete table"
            font.pixelSize: 11
            flat: true
            contentItem: Text {
                text: parent.text
                color: "#f38ba8"
                font.pixelSize: 11
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter
            }
            onClicked: noteEditor.deleteBlock(root.blockId)
        }
    }

    // Clear the focused flag when focus leaves the whole table
    Connections {
        target: noteEditor
        function onFocusedBlockIdChanged(id) {
            if (id !== root.blockId)
                root.tableFocused = false
        }
    }

    Keys.onPressed: function(event) {
        if (event.key === Qt.Key_Backspace) {
            noteEditor.deleteBlock(root.blockId)
            event.accepted = true
        } else if (event.key === Qt.Key_Enter || event.key === Qt.Key_Return) {
            root.syncCellsToModel()
            noteEditor.insertBlockAfter(root.blockId, 0, "")
            event.accepted = true
        }
    }
}