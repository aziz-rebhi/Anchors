import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Page {
    id: root

    Theme { id: theme }

    property var allNotes: []
    property string searchText: ""
    property string selectedId: ""
    property bool loadingEditor: false

    readonly property var visibleNotes: {
        var list = allNotes.filter(function (n) {
            if (searchText.length === 0) return true
            var q = searchText.toLowerCase()
            return n.title.toLowerCase().indexOf(q) >= 0 || n.content.toLowerCase().indexOf(q) >= 0
        })
        var sorted = list.slice()
        sorted.sort(function (a, b) { return b.updatedAt - a.updatedAt })
        return sorted
    }

    function refresh(preserveSelection) {
        allNotes = noteController.entries()
        if (!preserveSelection || !allNotes.some(function (n) { return n.id === selectedId })) {
            if (allNotes.length > 0) {
                selectNote(visibleNotes[0])
            } else {
                selectedId = ""
                loadingEditor = true
                titleField.text = ""
                bodyArea.text = ""
                loadingEditor = false
            }
        }
    }

    function selectNote(note) {
        if (!note) return
        loadingEditor = true
        selectedId = note.id
        titleField.text = note.title
        bodyArea.text = note.content
        loadingEditor = false
    }

    Component.onCompleted: refresh(false)

    Connections {
        target: noteController
        function onEntriesChanged() { root.refresh(true) }
    }

    Timer {
        id: saveTimer
        interval: 600
        onTriggered: {
            if (root.selectedId.length === 0) return
            noteController.updateEntry(root.selectedId, titleField.text, bodyArea.text)
        }
    }

    background: Rectangle { color: theme.background }

    RowLayout {
        anchors.fill: parent
        spacing: 0

        // --- Notes list ---
        Rectangle {
            Layout.preferredWidth: 260
            Layout.fillHeight: true
            color: theme.surface

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 14
                spacing: 10

                StyledTextField {
                    Layout.fillWidth: true
                    placeholderText: "Search notes..."
                    text: root.searchText
                    onTextChanged: root.searchText = text
                }

                PrimaryButton {
                    text: "+  New Note"
                    Layout.fillWidth: true
                    onClicked: {
                        if (noteController.addEntry("Untitled note", "")) {
                            // refresh() runs via entriesChanged and selects
                            // the most-recently-updated note automatically.
                        }
                    }
                }

                ScrollView {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    clip: true

                    ColumnLayout {
                        width: parent.width
                        spacing: 2

                        Repeater {
                            model: root.visibleNotes
                            delegate: Rectangle {
                                Layout.fillWidth: true
                                Layout.preferredHeight: 58
                                radius: theme.radiusSmall
                                color: modelData.id === root.selectedId ? theme.surfaceAlt : "transparent"

                                ColumnLayout {
                                    anchors.fill: parent
                                    anchors.margins: 8
                                    spacing: 2
                                    Label {
                                        text: modelData.title.length > 0 ? modelData.title : "Untitled note"
                                        color: theme.textPrimary
                                        font.family: theme.bodyFont
                                        font.pixelSize: 13
                                        font.bold: modelData.id === root.selectedId
                                        elide: Text.ElideRight
                                        Layout.fillWidth: true
                                    }
                                    Label {
                                        text: modelData.content.substring(0, 60)
                                        color: theme.textMuted
                                        font.pixelSize: 11
                                        elide: Text.ElideRight
                                        Layout.fillWidth: true
                                    }
                                }

                                MouseArea {
                                    anchors.fill: parent
                                    onClicked: root.selectNote(modelData)
                                }
                            }
                        }
                    }
                }
            }
        }

        // --- Editor ---
        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            visible: root.selectedId.length > 0

            RowLayout {
                Layout.fillWidth: true
                Layout.margins: 20
                spacing: 12

                // Decorative for now - full rich-text formatting needs a
                // QQuickTextDocument bridge on the C++ side, which isn't
                // wired up yet. Plain text saves/loads correctly either way.
                Label { text: "B"; font.bold: true; color: theme.textMuted; font.pixelSize: 14 }
                Label { text: "I"; font.italic: true; color: theme.textMuted; font.pixelSize: 14 }
                Label { text: "U"; font.underline: true; color: theme.textMuted; font.pixelSize: 14 }

                Item { Layout.fillWidth: true }

                Label {
                    text: "Saved"
                    color: theme.textMuted
                    font.pixelSize: 11
                }

                Label {
                    text: "Delete"
                    color: theme.danger
                    font.pixelSize: 12
                    MouseArea {
                        anchors.fill: parent
                        onClicked: noteController.deleteEntry(root.selectedId)
                    }
                }
            }

            ColumnLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.leftMargin: 24
                Layout.rightMargin: 24
                Layout.bottomMargin: 20
                spacing: 12

                TextField {
                    id: titleField
                    Layout.fillWidth: true
                    placeholderText: "Title"
                    color: theme.textPrimary
                    font.family: theme.headlineFont
                    font.pixelSize: 26
                    font.bold: true
                    background: null
                    onTextChanged: if (!root.loadingEditor) saveTimer.restart()
                }

                TextArea {
                    id: bodyArea
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    placeholderText: "Start writing..."
                    color: theme.textPrimary
                    font.family: theme.bodyFont
                    font.pixelSize: 15
                    wrapMode: Text.WordWrap
                    selectByMouse: true
                    background: null
                    onTextChanged: if (!root.loadingEditor) saveTimer.restart()
                }
            }
        }

        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            visible: root.selectedId.length === 0

            Label {
                anchors.centerIn: parent
                text: "Select or create a note to get started."
                color: theme.textMuted
                font.family: theme.bodyFont
                font.pixelSize: 13
            }
        }
    }
}
