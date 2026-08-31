import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Page {
    id: root

    Theme { id: theme }

    property bool vaultUnlocked: false
    property string filterCategory: ""
    property var allEntries: []
    property string searchText: ""
    property string sortMode: "recent"

    readonly property int colService: 200
    readonly property int colUser: 220
    readonly property int colPassword: 160
    readonly property int colActions: 120

    readonly property var categoryDefs: [
        { key: "Social",        glyph: "👥" },
        { key: "Work",          glyph: "💼" },
        { key: "Learning",      glyph: "📚" },
        { key: "Finance",       glyph: "💳" },
        { key: "Gaming",        glyph: "🎮" },
        { key: "Shopping",      glyph: "🛒" },
        { key: "Entertainment", glyph: "🎬" },
        { key: "Email",         glyph: "✉️" },
        { key: "Development",   glyph: "💻" },
        { key: "Cloud",         glyph: "☁️" }
    ]

    function refresh() {
        allEntries = vaultController.entries()
    }

    function categoryCount(key) {
        return allEntries.filter(function (e) { return e.category === key }).length
    }

    function copyPassword(pw) {
        if (!pw || !pw.length)
            return
        clipHelper.text = pw
        clipHelper.selectAll()
        clipHelper.copy()
        // Optional toast if you add one later
    }

    readonly property var visibleEntries: {
        var list = allEntries.filter(function (e) {
            if (filterCategory.length > 0 && e.category !== filterCategory)
                return false
            if (searchText.length === 0)
                return true
            var q = searchText.toLowerCase()
            return (e.title || "").toLowerCase().indexOf(q) >= 0
                || (e.username || "").toLowerCase().indexOf(q) >= 0
        })
        var sorted = list.slice()
        if (sortMode === "az") {
            sorted.sort(function (a, b) {
                return (a.title || "").localeCompare(b.title || "")
            })
        } else {
            sorted.sort(function (a, b) {
                return (b.updatedAt || 0) - (a.updatedAt || 0)
            })
        }
        return sorted
    }

    background: Rectangle { color: theme.background }

    // Hidden helper for clipboard
    TextEdit {
        id: clipHelper
        visible: false
        width: 1
        height: 1
    }

    Loader {
        id: mainLoader
        anchors.fill: parent
        sourceComponent: root.vaultUnlocked ? vaultContentComponent : pinGateComponent
    }

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
                    text: authController.hashVaultPin()
                          ? "Your vault is locked. Enter your PIN to unlock."
                          : "Create a PIN to protect your vault."
                    color: theme.textSecondary
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

    Component {
        id: vaultContentComponent
        Item {
            anchors.fill: parent

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 24
                spacing: 16

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 12

                    ColumnLayout {
                        spacing: 4
                        Layout.fillWidth: true
                        Label {
                            text: "Vault"
                            color: theme.textPrimary
                            font.family: theme.headlineFont
                            font.pixelSize: 28
                            font.bold: true
                        }
                        Label {
                            text: "Securely manage your credentials across categories."
                            color: theme.textSecondary
                            font.pixelSize: 13
                            wrapMode: Text.WordWrap
                            Layout.fillWidth: true
                        }
                    }

                    StyledTextField {
                        Layout.preferredWidth: 240
                        Layout.alignment: Qt.AlignVCenter
                        placeholderText: "Search entries..."
                        text: root.searchText
                        onTextChanged: root.searchText = text
                    }
                }

                GridLayout {
                    Layout.fillWidth: true
                    columns: 5
                    columnSpacing: 12
                    rowSpacing: 12

                    Repeater {
                        model: root.categoryDefs
                        delegate: Rectangle {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 80
                            radius: theme.radiusMedium
                            color: root.filterCategory === modelData.key
                                   ? Qt.rgba(theme.tertiary.r, theme.tertiary.g, theme.tertiary.b, 0.18)
                                   : theme.surfaceAlt
                            border.width: 1
                            border.color: root.filterCategory === modelData.key
                                          ? theme.tertiary : theme.border

                            ColumnLayout {
                                anchors.fill: parent
                                anchors.margins: 12
                                spacing: 4
                                RowLayout {
                                    Layout.fillWidth: true
                                    Label {
                                        text: modelData.glyph
                                        font.pixelSize: 22
                                        font.family: "Noto Color Emoji"
                                    }
                                    Item { Layout.fillWidth: true }
                                    Label {
                                        text: root.categoryCount(modelData.key)
                                        color: theme.textPrimary
                                        font.bold: true
                                        font.pixelSize: 16
                                    }
                                }
                                Item { Layout.fillHeight: true }
                                Label {
                                    text: modelData.key
                                    color: theme.textPrimary
                                    font.pixelSize: 13
                                }
                            }
                            MouseArea {
                                anchors.fill: parent
                                cursorShape: Qt.PointingHandCursor
                                onClicked: {
                                    root.filterCategory =
                                        (root.filterCategory === modelData.key) ? "" : modelData.key
                                }
                            }
                        }
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    Label {
                        text: root.filterCategory.length > 0
                              ? root.filterCategory + " Entries" : "All Entries"
                        color: theme.textPrimary
                        font.pixelSize: 18
                        font.bold: true
                    }
                    Item { Layout.fillWidth: true }
                    Label {
                        text: "Clear filter"
                        color: theme.tertiary
                        font.pixelSize: 11
                        visible: root.filterCategory.length > 0
                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            onClicked: root.filterCategory = ""
                        }
                    }
                    Label {
                        text: "A-Z"
                        color: root.sortMode === "az" ? theme.tertiary : theme.textMuted
                        font.pixelSize: 12
                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            onClicked: root.sortMode = "az"
                        }
                    }
                    Label {
                        text: "Recent"
                        color: root.sortMode === "recent" ? theme.tertiary : theme.textMuted
                        font.pixelSize: 12
                        Layout.leftMargin: 12
                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            onClicked: root.sortMode = "recent"
                        }
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 32
                    color: "transparent"

                    RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: 12
                        anchors.rightMargin: 12
                        spacing: 8

                        Label {
                            text: "SERVICE"
                            Layout.preferredWidth: root.colService
                            Layout.maximumWidth: root.colService
                            color: theme.textMuted
                            font.pixelSize: 10
                            font.letterSpacing: 1
                        }
                        Label {
                            text: "USERNAME"
                            Layout.preferredWidth: root.colUser
                            Layout.maximumWidth: root.colUser
                            color: theme.textMuted
                            font.pixelSize: 10
                            font.letterSpacing: 1
                        }
                        Label {
                            text: "PASSWORD"
                            Layout.fillWidth: true
                            Layout.minimumWidth: root.colPassword
                            color: theme.textMuted
                            font.pixelSize: 10
                            font.letterSpacing: 1
                        }
                        Label {
                            text: "ACTIONS"
                            Layout.preferredWidth: root.colActions
                            Layout.maximumWidth: root.colActions
                            color: theme.textMuted
                            font.pixelSize: 10
                            font.letterSpacing: 1
                            horizontalAlignment: Text.AlignRight
                        }
                    }
                }

                ListView {
                    id: entryList
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    clip: true
                    spacing: 2
                    model: root.visibleEntries

                    delegate: Rectangle {
                        id: entryRow
                        width: ListView.view ? ListView.view.width : 400
                        height: 52
                        radius: theme.radiusSmall
                        color: hoverArea.containsMouse ? theme.surfaceAlt : "transparent"
                        property bool revealed: false

                        RowLayout {
                            anchors.fill: parent
                            anchors.leftMargin: 12
                            anchors.rightMargin: 12
                            spacing: 8

                            Label {
                                text: modelData.title || ""
                                Layout.preferredWidth: root.colService
                                Layout.maximumWidth: root.colService
                                color: theme.textPrimary
                                font.pixelSize: 14
                                elide: Text.ElideRight
                            }
                            Label {
                                text: modelData.username || ""
                                Layout.preferredWidth: root.colUser
                                Layout.maximumWidth: root.colUser
                                color: theme.textSecondary
                                font.pixelSize: 13
                                elide: Text.ElideRight
                            }

                            RowLayout {
                                Layout.fillWidth: true
                                Layout.minimumWidth: root.colPassword
                                spacing: 6

                                Label {
                                    Layout.fillWidth: true
                                    text: entryRow.revealed
                                          ? (modelData.password || "")
                                          : "•".repeat(Math.min((modelData.password || "").length, 12))
                                    color: theme.textSecondary
                                    font.pixelSize: 13
                                    elide: Text.ElideRight
                                }
                                IconButton {
                                    iconName: entryRow.revealed ? "visibility_off" : "visibility"
                                    iconColor: theme.textMuted
                                    tooltip: entryRow.revealed ? "Hide password" : "Show password"
                                    onClicked: entryRow.revealed = !entryRow.revealed
                                }
                            }

                            RowLayout {
                                Layout.preferredWidth: root.colActions
                                Layout.maximumWidth: root.colActions
                                spacing: 2
                                layoutDirection: Qt.RightToLeft

                                IconButton {
                                    iconName: "delete"
                                    iconColor: theme.danger
                                    tooltip: "Delete entry"
                                    onClicked: vaultController.deleteEntry(modelData.id)
                                }
                                IconButton {
                                    iconName: "edit"
                                    iconColor: theme.textSecondary
                                    tooltip: "Edit entry"
                                    onClicked: editDialog.openForEdit(modelData)
                                }
                                IconButton {
                                    iconName: "copy"
                                    iconColor: theme.textSecondary
                                    tooltip: "Copy password"
                                    onClicked: root.copyPassword(modelData.password || "")
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

                    Label {
                        anchors.horizontalCenter: parent.horizontalCenter
                        anchors.top: parent.top
                        anchors.topMargin: 32
                        visible: entryList.count === 0
                        text: root.filterCategory.length > 0
                              ? "No entries in this category."
                              : "No entries yet."
                        color: theme.textMuted
                        font.pixelSize: 13
                    }
                }
            }

            Rectangle {
                id: addBtn
                width: 56
                height: 56
                radius: 28
                color: theme.tertiary
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                anchors.margins: 24

                Label {
                    anchors.centerIn: parent
                    text: "+"
                    color: theme.onAccent
                    font.pixelSize: 28
                    font.bold: true
                }

                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: editDialog.openForCreate()
                }
            }

            VaultEntryDialog { id: editDialog }
        }
    }

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
        if (visible && vaultUnlocked)
            refresh()
    }

    Component.onCompleted: {
        root.vaultUnlocked = false
        root.filterCategory = ""
    }

    component IconButton: Item {
        id: iconBtn
        property string iconName: "visibility"
        property color iconColor: theme.textSecondary
        property string tooltip: ""
        signal clicked()

        width: 28
        height: 28

        Label {
            anchors.centerIn: parent
            text: {
                switch (iconBtn.iconName) {
                case "visibility": return "👁"
                case "visibility_off": return "👁‍🗨"
                case "edit": return "✏️"
                case "delete": return "🗑️"
                case "copy": return "📋"
                default: return "?"
                }
            }
            color: iconBtn.iconColor
            font.pixelSize: 15
            font.family: "Noto Color Emoji"
        }

        MouseArea {
            id: iconMouseArea
            anchors.fill: parent
            cursorShape: Qt.PointingHandCursor
            hoverEnabled: true
            onEntered: parent.scale = 1.15
            onExited: parent.scale = 1.0
            onClicked: iconBtn.clicked()
        }
        Behavior on scale { NumberAnimation { duration: 80 } }

        ToolTip {
            visible: iconBtn.tooltip.length > 0 && iconMouseArea.containsMouse
            text: iconBtn.tooltip
            delay: 400
        }
    }
}