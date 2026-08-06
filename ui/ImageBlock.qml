import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Dialogs

Item {
    id: root
    property string blockId: ""
    property string source: ""
    property string caption: ""

    width: parent ? parent.width : 0
    height: source ? imageDisplay.height + captionCol.height + 16
                   : placeholderBox.height + captionCol.height + 16

    function focusInput() {
        if (source !== "") {
            captionInput.forceActiveFocus()
        } else {
            fileDialog.open()
        }
    }

    // Auto-open file picker when a brand-new empty image block appears
    Component.onCompleted: {
        if (source === "") {
            Qt.callLater(function() { fileDialog.open() })
        }
    }

    // ---- Image display ----
    Rectangle {
        id: imageDisplay
        visible: root.source !== ""
        width: parent.width - 16
        height: source ? Math.min(400, image.implicitHeight > 0 ? image.implicitHeight : 200) : 0
        color: "#2a2a3a"
        radius: 8
        border.color: "#555555"
        border.width: 1
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        anchors.topMargin: 8
        clip: true

        Image {
            id: image
            anchors.fill: parent
            anchors.margins: 4
            source: root.source
            fillMode: Image.PreserveAspectFit
            cache: false
            asynchronous: true
        }

        Rectangle {
            anchors.fill: parent
            color: "#000000"
            opacity: changeArea.containsMouse ? 0.4 : 0
            Behavior on opacity { NumberAnimation { duration: 150 } }

            Text {
                anchors.centerIn: parent
                text: "Click to change image"
                font.pixelSize: 13
                color: "#ffffff"
                visible: changeArea.containsMouse
            }

            MouseArea {
                id: changeArea
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor
                onClicked: fileDialog.open()
            }
        }
    }

    // ---- Placeholder ----
    Rectangle {
        id: placeholderBox
        visible: root.source === ""
        width: parent.width - 16
        height: 140
        color: "#2a2a3a"
        radius: 8
        border.color: "#555555"
        border.width: 2
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        anchors.topMargin: 8

        Column {
            anchors.centerIn: parent
            spacing: 8
            Text {
                text: "\u25A3"
                font.pixelSize: 36
                color: "#888888"
                anchors.horizontalCenter: parent.horizontalCenter
            }
            Text {
                text: "Click to add image"
                font.pixelSize: 13
                color: "#aaaaaa"
                anchors.horizontalCenter: parent.horizontalCenter
            }
        }

        MouseArea {
            anchors.fill: parent
            cursorShape: Qt.PointingHandCursor
            onClicked: fileDialog.open()
        }
    }

    // ---- Caption ----
    Column {
        id: captionCol
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: (root.source !== "" ? imageDisplay.bottom : placeholderBox.bottom)
        anchors.topMargin: 6
        anchors.leftMargin: 24
        anchors.rightMargin: 24
        spacing: 4

        TextField {
            id: captionInput
            width: parent.width
            text: root.caption
            placeholderText: "Add a caption..."
            font.pixelSize: 12
            color: "#cccccc"
            placeholderTextColor: "#666666"
            background: Rectangle { color: "transparent" }
            onEditingFinished: {
                if (noteEditor && text !== root.caption)
                    noteEditor.updateBlockImageCaption(root.blockId, text)
            }
            onActiveFocusChanged: {
                if (activeFocus && noteEditor)
                    noteEditor.setFocusedBlock(root.blockId)
            }
        }
    }

    // ---- File dialog (Qt6) ----
    FileDialog {
        id: fileDialog
        title: "Choose an image"
        fileMode: FileDialog.OpenFile
        nameFilters: ["Image files (*.png *.jpg *.jpeg *.gif *.bmp *.svg *.webp)", "All files (*)"]
        onAccepted: {
            if (noteEditor && selectedFile) {
                // selectedFile is a url; keep file:// prefix for QML Image
                noteEditor.updateBlockImageSource(root.blockId, selectedFile.toString())
            }
        }
    }

    Keys.onPressed: function(event) {
        if ((event.modifiers & Qt.ControlModifier) && event.key === Qt.Key_V) {
                // Clipboard image: user can still use file dialog; full clipboard image
                // needs C++ helper — file dialog remains primary
            }
        if (event.key === Qt.Key_Backspace && root.source === "") {
            noteEditor.deleteBlock(root.blockId)
            event.accepted = true
        } else if (event.key === Qt.Key_Enter || event.key === Qt.Key_Return) {
            noteEditor.insertBlockAfter(root.blockId, 0, "")
            event.accepted = true
        }
    }
}