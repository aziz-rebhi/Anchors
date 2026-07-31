import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Page {
    id: root

    Theme { id: theme }

    // ---- Vault state ----
    property bool vaultUnlocked: false
    property string filterCategory: ""

    // ---- Data properties ----
    property var allEntries: []
    property string searchText: ""
    property string sortMode: "recent"

    readonly property var categoryDefs: [
        { key: "Social", glyph: "\u25CB" },
        { key: "Work", glyph: "\u25A0" },
        { key: "Learning", glyph: "\u25B3" },
        { key: "Finance", glyph: "\u25C6" }
    ]

    // ---- Data functions ----
    function refresh() {
        allEntries = vaultController.entries()
    }

    function categoryCount(key) {
        return allEntries.filter(function (e) { return e.category === key }).length
    }

    readonly property var visibleEntries: {
        var list = allEntries.filter(function (e) {
            if (filterCategory.length > 0 && e.category !== filterCategory) return false
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

    background: Rectangle { color: theme.background }

    Loader {
        id: mainLoader
        anchors.fill: parent
        sourceComponent: root.vaultUnlocked ? vaultContentComponent : pinGateComponent
    }

    // ---- PIN Gate Component (unchanged) ----
    Component {
        id: pinGateComponent
        Item {
            anchors.fill: parent
            ColumnLayout {
                anchors.centerIn: parent
                spacing: 16
                width: Math.min(parent.width * 0.8, 320)

                Label {
                    text: authController.hashVaultPin() ? "Enter Vault PIN" : "Set Vault PIN"
                    color: theme.textPrimary
                    font.family: theme.headlineFont
                    font.pixelSize: 22
                    font.bold: true
                    Layout.alignment: Qt.AlignHCenter
                }
                Label {
                    text: authController.hashVaultPin() ? "Your vault is locked. Enter your PIN to unlock." : "Create a PIN to protect your vault."
                    color: theme.textSecondary
                    font.family: theme.bodyFont
                    font.pixelSize: 13
                    wrapMode: Text.WordWrap
                    horizontalAlignment: Text.AlignHCenter
                    Layout.alignment: Qt.AlignHCenter
                    Layout.fillWidth: true
                }
                StyledTextField {
                    id: pinField
                    placeholderText: authController.hashVaultPin() ? "Enter PIN" : "Create a PIN"
                    echoMode: TextInput.Password
                    inputMethodHints: Qt.ImhDigitsOnly
                    maximumLength: 8
                    Layout.fillWidth: true
                    onAccepted: unlockButton.clicked()
                }
                StyledTextField {
                    id: confirmField
                    placeholderText: "Confirm PIN"
                    echoMode: TextInput.Password
                    inputMethodHints: Qt.ImhDigitsOnly
                    maximumLength: 8
                    Layout.fillWidth: true
                    visible: !authController.hashVaultPin()
                    onAccepted: unlockButton.clicked()
                }
                Label {
                    id: pinErrorLabel
                    color: theme.danger
                    font.family: theme.bodyFont
                    font.pixelSize: 12
                    visible: text.length > 0
                    wrapMode: Text.WordWrap
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignHCenter
                }
                PrimaryButton {
                    id: unlockButton
                    text: authController.hashVaultPin() ? "Unlock" : "Set PIN"
                    Layout.fillWidth: true
                    onClicked: {
                        pinErrorLabel.text = ""
                        if (authController.hashVaultPin()) {
                            if (authController.verifyVaultPin(pinField.text)) {
                                pinField.text = ""
                                root.vaultUnlocked = true
                                mainLoader.sourceComponent = vaultContentComponent
                                root.refresh()
                            } else {
                                pinErrorLabel.text = "Incorrect PIN."
                                pinField.text = ""
                                pinField.forceActiveFocus()
                            }
                        } else {
                            if (pinField.text.length < 4) {
                                pinErrorLabel.text = "PIN must be at least 4 digits."
                                return
                            }
                            if (pinField.text !== confirmField.text) {
                                pinErrorLabel.text = "PINs do not match."
                                return
                            }
                            if (authController.setVaultPin(pinField.text)) {
                                pinField.text = ""
                                confirmField.text = ""
                                root.vaultUnlocked = true
                                mainLoader.sourceComponent = vaultContentComponent
                                root.refresh()
                            } else {
                                pinErrorLabel.text = "Failed to set PIN. Please try again."
                            }
                        }
                    }
                }
            }
        }
    }

    // ---- Vault Content Component (with icons) ----
    Component {
        id: vaultContentComponent
        Item {
            anchors.fill: parent

            ScrollView {
                anchors.fill: parent
                contentWidth: availableWidth
                clip: true

                ColumnLayout {
                    width: parent.width
                    spacing: theme.spacingLarge
                    anchors.margins: 24

                    // Header
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

                    // Category tiles (clickable)
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
                                color: root.filterCategory === modelData.key ? Qt.rgba(theme.tertiary.r, theme.tertiary.g, theme.tertiary.b, 0.2) : theme.surfaceAlt
                                border.width: 1
                                border.color: root.filterCategory === modelData.key ? theme.tertiary : theme.border

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

                                MouseArea {
                                    anchors.fill: parent
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: {
                                        if (root.filterCategory === modelData.key)
                                            root.filterCategory = ""
                                        else
                                            root.filterCategory = modelData.key
                                    }
                                }
                            }
                        }
                    }

                    // Entries table
                    ColumnLayout {
                        Layout.fillWidth: true
                        Layout.leftMargin: 24
                        Layout.rightMargin: 24
                        spacing: 10

                        RowLayout {
                            Layout.fillWidth: true
                            Label {
                                text: root.filterCategory.length > 0 ? root.filterCategory + " Entries" : "All Entries"
                                color: theme.textPrimary
                                font.family: theme.headlineFont
                                font.pixelSize: 18
                                font.bold: true
                            }
                            Item { Layout.fillWidth: true }

                            Label {
                                text: "Clear filter"
                                color: theme.tertiary
                                font.family: theme.labelFont
                                font.pixelSize: 11
                                visible: root.filterCategory.length > 0
                                MouseArea {
                                    anchors.fill: parent
                                    onClicked: root.filterCategory = ""
                                }
                            }

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

                                    // Password field with reveal icon
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

                                        // Eye icon toggle
                                        IconButton {
                                            iconName: entryRow.revealed ? "visibility_off" : "visibility"
                                            iconColor: theme.textMuted
                                            tooltip: entryRow.revealed ? "Hide password" : "Show password"
                                            onClicked: entryRow.revealed = !entryRow.revealed
                                        }
                                    }

                                    // Action buttons: Edit, Delete
                                    RowLayout {
                                        Layout.preferredWidth: 90
                                        spacing: 4

                                        IconButton {
                                            iconName: "edit"
                                            iconColor: theme.textSecondary
                                            tooltip: "Edit entry"
                                            onClicked: editDialog.openForEdit(modelData)
                                        }

                                        IconButton {
                                            iconName: "delete"
                                            iconColor: theme.danger
                                            tooltip: "Delete entry"
                                            onClicked: vaultController.deleteEntry(modelData.id)
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
                            text: root.filterCategory.length > 0 ? "No entries in this category." : "No entries yet."
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

            // ---- Floating Add Button (key icon) ----
            Rectangle {
                id: addBtn
                width: 56
                height: 56
                radius: 28
                color: theme.tertiary
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                anchors.margins: 24
                layer.enabled: true
                layer.effect: null

                // Drop shadow
                Rectangle {
                    anchors.fill: parent
                    radius: parent.radius
                    color: "transparent"
                    border.width: 0
                    z: -1
                    Rectangle {
                        anchors.fill: parent
                        radius: parent.radius
                        color: "black"
                        opacity: 0.2
                        anchors.margins: 2
                    }
                }

                Label {
                    anchors.centerIn: parent
                    text: "🔑"  // key icon
                    color: "#0A140A"
                    font.pixelSize: 28
                    font.weight: Font.Light
                }

                MouseArea {
                    id: addBtnMouseArea
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    hoverEnabled: true
                    onEntered: {
                        addBtn.scale = 1.1
                        addBtn.color = Qt.darker(theme.tertiary, 1.1)
                    }
                    onExited: {
                        addBtn.scale = 1.0
                        addBtn.color = theme.tertiary
                    }
                    onClicked: editDialog.openForCreate()
                }

                Behavior on scale {
                    NumberAnimation { duration: 100 }
                }
                Behavior on color {
                    ColorAnimation { duration: 100 }
                }

                ToolTip {
                    visible: addBtnMouseArea.containsMouse
                    text: "Add password"
                    delay: 500
                }
            }

            VaultEntryDialog {
                id: editDialog
            }
        }
    }

    // ---- Connections ----
    Connections {
        target: vaultController
        function onEntriesChanged() { root.refresh() }
    }

    Connections {
        target: session
        function onLocked() {
            root.vaultUnlocked = false
            root.filterCategory = ""
            mainLoader.sourceComponent = pinGateComponent
        }
    }

    onVisibleChanged: {
        if (visible && vaultUnlocked) {
            refresh()
        }
    }

    Component.onCompleted: {
        root.vaultUnlocked = false
        root.filterCategory = ""
    }

    // ---- Helper IconButton component (inline) ----
    component IconButton : Item {
        id: iconBtn
        property string iconName: "visibility"
        property color iconColor: theme.textSecondary
        property string tooltip: ""
        signal clicked()

        width: 28
        height: 28
        opacity: 1.0

        Label {
            id: iconLabel
            anchors.centerIn: parent
            text: {
                switch(iconBtn.iconName) {
                    case "visibility": return "👁"
                    case "visibility_off": return "👁‍🗨"
                    case "edit": return "✏️"
                    case "delete": return "🗑️"
                    default: return "?"
                }
            }
            color: iconBtn.iconColor
            font.pixelSize: 18
            font.family: theme.bodyFont
        }

        MouseArea {
            id: iconMouseArea
            anchors.fill: parent
            cursorShape: Qt.PointingHandCursor
            hoverEnabled: true
            onEntered: {
                parent.scale = 1.2
                iconLabel.color = Qt.darker(iconBtn.iconColor, 1.3)
            }
            onExited: {
                parent.scale = 1.0
                iconLabel.color = iconBtn.iconColor
            }
            onClicked: iconBtn.clicked()
        }

        Behavior on scale { NumberAnimation { duration: 100 } }

        ToolTip {
            visible: iconBtn.tooltip.length > 0 && iconMouseArea.containsMouse
            text: iconBtn.tooltip
            delay: 500
        }
    }
}