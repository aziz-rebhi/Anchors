import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

ListView {
    id: root
    model: (noteEditor && noteEditor.model) ? noteEditor.model : null
    delegate: BlockDelegate {}
    spacing: 2
    clip: true

    // ── Scroll restore on model reset ───────────────────────────
    property real savedContentY: 0
    property bool restoreScroll: false

    // ── Drag-and-drop state ─────────────────────────────────────
    property string dragBlockId: ""
    property int dragFromIndex: -1
    property real dragHoverY: -1
    property int dropIndex: -1

    onDragHoverYChanged: {
        if (dragFromIndex < 0)
            return
        var y = dragHoverY + contentY
        var idx = indexAt(10, y)
        if (idx < 0) {
            if (y < contentY + 8)
                idx = 0
            else
                idx = count
        }
        dropIndex = idx
    }

    function finishDrag() {
        if (dragFromIndex < 0 || !dragBlockId.length) {
            cancelDrag()
            return
        }
        var to = dropIndex
        if (to < 0)
            to = count
        // When moving downward, account for the slot the item leaves
        if (to > dragFromIndex)
            to -= 1
        if (to !== dragFromIndex && noteEditor)
            noteEditor.moveBlock(dragBlockId, to)
        cancelDrag()
    }

    function cancelDrag() {
        dragBlockId = ""
        dragFromIndex = -1
        dragHoverY = -1
        dropIndex = -1
    }

    // Drop indicator line
    Rectangle {
        visible: root.dragFromIndex >= 0 && root.dropIndex >= 0
        width: parent.width - 24
        height: 2
        x: 12
        z: 100
        radius: 1
        color: "#4F9F4F"
        y: {
            if (root.dropIndex <= 0)
                return 0
            if (root.dropIndex >= root.count) {
                var last = root.itemAtIndex(root.count - 1)
                return last ? (last.y + last.height - root.contentY) : 0
            }
            var it = root.itemAtIndex(root.dropIndex)
            return it ? (it.y - root.contentY) : 0
        }
    }

    Connections {
        target: root.model
        function onModelAboutToBeReset() {
            root.savedContentY = root.contentY
            root.restoreScroll = true
        }
        function onModelReset() {
            if (!root.restoreScroll)
                return
            Qt.callLater(function () {
                if (!root.restoreScroll)
                    return
                root.restoreScroll = false
                var maxY = Math.max(0, root.contentHeight - root.height)
                root.contentY = Math.min(root.savedContentY, maxY)
            })
        }
    }

    Connections {
        target: noteEditor
        function onPendingFocusIdChanged(id) {
            if (id && id.length > 0) {
                focusTimer.targetId = id
                focusTimer.start()
            }
        }
    }

    Timer {
        id: focusTimer
        interval: 40
        property string targetId: ""
        onTriggered: {
            for (var i = 0; i < root.count; i++) {
                var delegate = root.itemAtIndex(i)
                if (delegate && delegate.blockId === targetId) {
                    root.positionViewAtIndex(i, ListView.Contain)
                    delegate.focusInput()
                    break
                }
            }
        }
    }
}