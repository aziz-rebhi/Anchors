import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

ListView {
    id: root
    model: (noteEditor && noteEditor.model) ? noteEditor.model : null
    delegate: BlockDelegate {}
    spacing: 2
    clip: true

    onCountChanged: {
        if (count > 0) {
            positionViewAtEnd()
        }
    }

    // --- Auto-focus mechanism ---
    // When the controller sets pendingFocusId, find that delegate and focus it
    Connections {
        target: noteEditor
        function onPendingFocusIdChanged(id) {
            if (id.length > 0) {
                focusTimer.targetId = id
                focusTimer.start()
            }
        }
    }

    Timer {
        id: focusTimer
        interval: 30
        property string targetId: ""
        onTriggered: {
            for (var i = 0; i < root.count; i++) {
                var delegate = root.itemAtIndex(i)
                if (delegate && delegate.blockId === targetId) {
                    delegate.focusInput()
                    root.positionViewAtIndex(i, ListView.Contain)
                    break
                }
            }
        }
    }
}
