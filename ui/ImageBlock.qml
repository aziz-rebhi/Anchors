import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Dialogs

Rectangle {
    id: root
    Theme { id: theme }

    property string blockId: ""
    property string source: ""
    property string caption: ""

    signal sourceChangedByUser(string path)
    signal captionChangedByUser(string text)

    width: parent ? parent.width : 0
    height: col.implicitHeight + 16
    radius: 8
    color: theme.surfaceAlt
    border.color: theme.border
    border.width: 1

    function focusInput() {
        captionField.forceActiveFocus()
    }

    ColumnLayout {
        id: col
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: 10
        spacing: 8

        Text {
            text: "Image"
            color: theme.textMuted
            font.pixelSize: 11
        }

        // Preview
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: root.source.length > 0 ? Math.min(280, img.implicitHeight + 16) : 120
            radius: 6
            color: theme.surface
            border.color: theme.border
            border.width: 1
            clip: true

            Image {
                id: img
                anchors.fill: parent
                anchors.margins: 8
                source: root.source
                fillMode: Image.PreserveAspectFit
                asynchronous: true
                visible: root.source.length > 0
            }

            Label {
                anchors.centerIn: parent
                visible: root.source.length === 0
                text: "No image — paste (Ctrl+V) or choose a file"
                color: theme.textMuted
                font.pixelSize: 13
            }

            MouseArea {
                anchors.fill: parent
                onClicked: fileDialog.open()
            }
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            Button {
                text: root.source.length ? "Replace…" : "Choose file…"
                onClicked: fileDialog.open()
            }

            TextField {
                id: captionField
                Layout.fillWidth: true
                text: root.caption
                placeholderText: "Caption (optional)"
                color: theme.textPrimary
                placeholderTextColor: theme.textMuted
                background: Rectangle {
                    color: theme.surface
                    radius: 4
                    border.color: theme.border
                }
                onTextChanged: {
                    if (text !== root.caption)
                        root.captionChangedByUser(text)
                }
                onActiveFocusChanged: {
                    if (activeFocus && noteEditor)
                        noteEditor.setFocusedBlock(root.blockId)
                }
            }
        }
    }

    FileDialog {
        id: fileDialog
        title: "Choose image"
        fileMode: FileDialog.OpenFile
        nameFilters: [ "Images (*.png *.jpg *.jpeg *.gif *.webp *.bmp)", "All files (*)" ]
        onAccepted: {
            var path = selectedFile.toString()
            if (noteEditor)
                noteEditor.updateBlockImageSource(root.blockId, path)
        }
    }

    // Keep controller in sync when caption changes
    Connections {
        target: root
        function onCaptionChangedByUser(text) {
            if (noteEditor)
                noteEditor.updateBlockImageCaption(root.blockId, text)
        }
        function onSourceChangedByUser(path) {
            if (noteEditor)
                noteEditor.updateBlockImageSource(root.blockId, path)
        }
    }

    Keys.onPressed: function (e) {
        if ((e.modifiers & Qt.ControlModifier) && e.key === Qt.Key_V) {
            if (noteEditor && noteEditor.pasteImageFromClipboard()) {
                e.accepted = true
                return
            }
        }
        if ((e.modifiers & Qt.ControlModifier) &&
            (e.key === Qt.Key_Return || e.key === Qt.Key_Enter)) {
            if (noteEditor)
                noteEditor.insertBlockAfter(root.blockId, 0, "")
            e.accepted = true
        }
    }
}