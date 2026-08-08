import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

ListView {
    id: root
    model: (noteEditor && noteEditor.model) ? noteEditor.model : null
    delegate: BlockDelegate {}
    spacing: 2
    clip: true

    // Remember scroll while the model is reset (insert/delete/rebuild)
    property real savedContentY: 0
    property bool restoreScroll: false

    Connections {
        target: root.model
        function onModelAboutToBeReset() {
            root.savedContentY = root.contentY
            root.restoreScroll = true
        }
        function onModelReset() {
            if (!root.restoreScroll)
                return
            // Defer until delegates are laid out
            Qt.callLater(function () {
                if (!root.restoreScroll)
                    return
                root.restoreScroll = false
                var maxY = Math.max(0, root.contentHeight - root.height)
                root.contentY = Math.min(root.savedContentY, maxY)
            })
        }
    }

    // Focus the block the controller asked for (and only then scroll to it)
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
                    // Prefer Contain so we don't jump to the very top/bottom
                    root.positionViewAtIndex(i, ListView.Contain)
                    delegate.focusInput()
                    break
                }
            }
        }
    }
}