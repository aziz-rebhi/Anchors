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
}