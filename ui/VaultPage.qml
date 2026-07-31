import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Page {
    id: root

    Theme { id: theme }

    property var allEntries: []
    property string searchText: ""
    property string sortMode: "recent" // "recent" | "az"

    readonly property var categoryDefs: [
        { key: "Social", glyph: "\u25CB" },
        { key: "Work", glyph: "\u25A0" },
        { key: "Learning", glyph: "\u25B3" },
        { key: "Finance", glyph: "\u25C6" }
    ]

    function refresh() {
        allEntries = vaultController.entries()
    }

    Component.onCompleted: refresh()

    Connections {
        target: vaultController
        function onEntriesChanged() { root.refresh() }
    }

    readonly property var visibleEntries: {
        var list = allEntries.filter(function (e) {
            if (searchText.length === 0) return true
            var q = searchText.toLowerCase()
            return e.title.toLowerCase().indexOf(q) >= 0 || e.username.toLowerCase().indexOf(q) >= 0
        })
        var sorted = list.slice()
        if (sortMode === "az") {
            sorted.sort(function (a, b) { return a.title.localeCompare(b.title) })
        } else {
            sorted.sort(function (a, b) { return b.updatedAt - a.updatedAt })
        }
        return sorted
    }

    function categoryCount(key) {
        return allEntries.filter(function (e) { return e.category === key }).length
    }

    background: Rectangle { color: theme.background }

    ScrollView {
        anchors.fill: parent
        contentWidth: availableWidth
        clip: true

        ColumnLayout {
            width: parent.width
            spacing: theme.spacingLarge
            anchors.margins: 24

            RowLayout {
                Layout.fillWidth: true
                Layout.margins: 24

                ColumnLayout {
                    spacing: 4
                    Label {
                        text: "Vault"
                        color: theme.textPrimary
                        font.family: theme.headlineFont
                        font.pixelSize: 30
                        font.bold: true
                    }
                    Label {
                        text: "Securely manage your credentials across categories."
                        color: theme.textSecondary
                        font.family: theme.bodyFont
                        font.pixelSize: 13
                    }
                }

                Item { Layout.fillWidth: true }

                StyledTextField {
                    Layout.preferredWidth: 260
                    placeholderText: "Search entries..."
                    text: root.searchText
                    onTextChanged: root.searchText = text
                }
            }

            // --- Category tiles ---
            GridLayout {
                Layout.fillWidth: true
                Layout.leftMargin: 24
                Layout.rightMargin: 24
                columns: 4
                columnSpacing: 16
                rowSpacing: 16

                Repeater {
                    model: root.categoryDefs
                    delegate: Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 96
                        radius: theme.radiusMedium
                        color: theme.surfaceAlt
                        border.width: 1
                        border.color: theme.border

                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 14
                            spacing: 4

                            RowLayout {
                                Layout.fillWidth: true
                                Label {
                                    text: modelData.glyph
                                    color: theme.tertiary
                                    font.pixelSize: 16
                                }
                                Item { Layout.fillWidth: true }
                                Label {
                                    text: root.categoryCount(modelData.key)
                                    color: theme.textPrimary
                                    font.family: theme.bodyFont
                                    font.pixelSize: 16
                                    font.bold: true
                                }
                            }
                            Item { Layout.fillHeight: true }
                            Label {
                                text: modelData.key
                                color: theme.textPrimary
                                font.family: theme.headlineFont
                                font.pixelSize: 15
                            }
                        }
                    }
                }
            }

            // --- Entries table ---
            ColumnLayout {
                Layout.fillWidth: true
                Layout.leftMargin: 24
                Layout.rightMargin: 24
                spacing: 10

                RowLayout {
                    Layout.fillWidth: true
                    Label {
                        text: "All Entries"
                        color: theme.textPrimary
                        font.family: theme.headlineFont
                        font.pixelSize: 18
                        font.bold: true
                    }
                    Item { Layout.fillWidth: true }
                    Label {
                        text: "A-Z"
                        color: root.sortMode === "az" ? theme.tertiary : theme.textMuted
                        font.family: theme.labelFont
                        font.pixelSize: 12
                        MouseArea { anchors.fill: parent; onClicked: root.sortMode = "az" }
                    }
                    Label {
                        text: "Recent"
                        color: root.sortMode === "recent" ? theme.tertiary : theme.textMuted
                        font.family: theme.labelFont
                        font.pixelSize: 12
                        Layout.leftMargin: 12
                        MouseArea { anchors.fill: parent; onClicked: root.sortMode = "recent" }
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 36
                    color: "transparent"
                    RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: 12
                        anchors.rightMargin: 12
                        Label { text: "SERVICE"; Layout.preferredWidth: 220; color: theme.textMuted; font.pixelSize: 10; font.letterSpacing: 1 }
                        Label { text: "USERNAME"; Layout.fillWidth: true; color: theme.textMuted; font.pixelSize: 10; font.letterSpacing: 1 }
                        Label { text: "PASSWORD"; Layout.preferredWidth: 160; color: theme.textMuted; font.pixelSize: 10; font.letterSpacing: 1 }
                        Label { text: "ACTIONS"; Layout.preferredWidth: 90; color: theme.textMuted; font.pixelSize: 10; font.letterSpacing: 1; horizontalAlignment: Text.AlignRight }
                    }
                }

                Repeater {
                    model: root.visibleEntries
                    delegate: Rectangle {
                        id: entryRow
                        Layout.fillWidth: true
                        Layout.preferredHeight: 52
                        radius: theme.radiusSmall
                        color: hoverArea.containsMouse ? theme.surfaceAlt : "transparent"

                        property bool revealed: false

                        RowLayout {
                            anchors.fill: parent
                            anchors.leftMargin: 12
                            anchors.rightMargin: 12
                            spacing: 8

                            Label {
                                text: modelData.title
                                Layout.preferredWidth: 220
                                color: theme.textPrimary
                                font.family: theme.bodyFont
                                font.pixelSize: 14
                                elide: Text.ElideRight
                            }
                            Label {
                                text: modelData.username
                                Layout.fillWidth: true
                                color: theme.textSecondary
                                font.family: theme.bodyFont
                                font.pixelSize: 13
                                elide: Text.ElideRight
                            }
                            RowLayout {
                                Layout.preferredWidth: 160
                                spacing: 6
                                Label {
                                    text: entryRow.revealed ? modelData.password : "\u2022".repeat(Math.min(modelData.password.length, 12))
                                    color: theme.textSecondary
                                    font.family: theme.bodyFont
                                    font.pixelSize: 13
                                    Layout.fillWidth: true
                                    elide: Text.ElideRight
                                }
                                Label {
                                    text: entryRow.revealed ? "hide" : "show"
                                    color: theme.tertiary
                                    font.pixelSize: 11
                                    MouseArea {
                                        anchors.fill: parent
                                        onClicked: entryRow.revealed = !entryRow.revealed
                                    }
                                }
                            }
                            RowLayout {
                                Layout.preferredWidth: 90
                                spacing: 10
                                Label {
                                    text: "Edit"
                                    color: theme.textSecondary
                                    font.pixelSize: 12
                                    MouseArea {
                                        anchors.fill: parent
                                        onClicked: editDialog.openForEdit(modelData)
                                    }
                                }
                                Label {
                                    text: "Del"
                                    color: theme.danger
                                    font.pixelSize: 12
                                    MouseArea {
                                        anchors.fill: parent
                                        onClicked: vaultController.deleteEntry(modelData.id)
                                    }
                                }
                            }
                        }

                        MouseArea {
                            id: hoverArea
                            anchors.fill: parent
                            hoverEnabled: true
                            z: -1
                            acceptedButtons: Qt.NoButton
                        }
                    }
                }

                Label {
                    text: "No entries yet."
                    visible: root.visibleEntries.length === 0
                    color: theme.textMuted
                    font.family: theme.bodyFont
                    font.pixelSize: 13
                    Layout.topMargin: 8
                }
            }

            Item { Layout.preferredHeight: 24 }
        }
    }

    PrimaryButton {
        text: "+  Add Password"
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: 24
        onClicked: editDialog.openForCreate()
    }

    VaultEntryDialog {
        id: editDialog
    }
}
