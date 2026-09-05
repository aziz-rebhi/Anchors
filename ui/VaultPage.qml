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
    property var revealedId: null

    readonly property int colService: 200
    readonly property int colUser: 180
    readonly property int colUrl: 200
    readonly property int colPassword: 160
    readonly property int colActions: 140

    readonly property var displayCategories: {
        var set = {}
        var list = []
        var saved = []
        if (typeof settingsController !== "undefined" && settingsController
                && settingsController.vaultCategories)
            saved = settingsController.vaultCategories

        for (var i = 0; i < saved.length; i++) {
            var s = ("" + saved[i]).trim()
            if (s.length && !set[s.toLowerCase()]) {
                set[s.toLowerCase()] = true
                list.push(s)
            }
        }
        for (var j = 0; j < allEntries.length; j++) {
            var c = (allEntries[j].category || "").trim()
            if (c.length && !set[c.toLowerCase()]) {
                set[c.toLowerCase()] = true
                list.push(c)
            }
        }
        list.sort(function (a, b) { return a.localeCompare(b) })
        return list
    }

    function refresh() {
        if (typeof vaultController === "undefined" || !vaultController) {
            allEntries = []
            return
        }
        allEntries = vaultController.entries() || []
        revealedId = null
    }

    function categoryCount(key) {
        return allEntries.filter(function (e) {
            return (e.category || "") === key
        }).length
    }

    function copyPassword(pw) {
        if (!pw || !pw.length)
            return
        if (typeof clipboardGuard === "undefined" || !clipboardGuard)
            return
        var secs = 0
        if (typeof settingsController !== "undefined" && settingsController)
            secs = settingsController.clearClipboard
                   ? settingsController.clipboardClearSeconds : 0
        clipboardGuard.copyWithAutoClear(pw, secs)
    }

    function openUrl(url) {
        if (!url || !url.length)
            return
        // Only ever hand http(s) URLs to the OS. Anything without a scheme
        // gets https:// assumed; javascript:, file:, etc. are rejected.
        var u = url
        if (u.indexOf("://") < 0)
            u = "https://" + u
        if (u.indexOf("http://") === 0 || u.indexOf("https://") === 0)
            Qt.openUrlExternally(u)
    }

    function openCreate() {
        editDialog.knownCategories = root.displayCategories
        editDialog.openForCreate()
    }

    function openEdit(entry) {
        editDialog.knownCategories = root.displayCategories
        editDialog.openForEdit(entry)
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
                || (e.url || "").toLowerCase().indexOf(q) >= 0
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

    VaultEntryDialog {
        id: editDialog
        parent: Overlay.overlay
    }

    Dialog {
        id: newCategoryDialog
        modal: true
        parent: Overlay.overlay
        anchors.centerIn: parent
        width: 320
        padding: 0
        standardButtons: Dialog.NoButton
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

        background: Rectangle {
            color: theme.surface
            radius: theme.radiusMedium
            border.color: theme.border
            border.width: 1
        }

        ColumnLayout {
            width: parent.width
            spacing: 0

            Label {
                Layout.fillWidth: true
                Layout.leftMargin: 20
                Layout.rightMargin: 20
                Layout.topMargin: 18
                text: "New category"
                color: theme.textPrimary
                font.family: theme.headlineFont
                font.pixelSize: 16
                font.bold: true
            }

            Label {
                Layout.fillWidth: true
                Layout.leftMargin: 20
                Layout.rightMargin: 20
                Layout.topMargin: 4
                text: "Name a group for your passwords."
                color: theme.textMuted
                font.pixelSize: 12
                wrapMode: Text.WordWrap
            }

            StyledTextField {
                id: newCatField
                Layout.fillWidth: true
                Layout.leftMargin: 20
                Layout.rightMargin: 20
                Layout.topMargin: 16
                placeholderText: "Category name"
                onAccepted: addCatBtn.clicked()
            }

            Label {
                id: newCatError
                Layout.fillWidth: true
                Layout.leftMargin: 20
                Layout.rightMargin: 20
                Layout.topMargin: 6
                text: ""
                color: theme.danger
                font.pixelSize: 11
                visible: text.length > 0
            }

            RowLayout {
                Layout.fillWidth: true
                Layout.leftMargin: 20
                Layout.rightMargin: 20
                Layout.topMargin: 18
                Layout.bottomMargin: 18
                spacing: 10

                Item { Layout.fillWidth: true }

                Rectangle {
                    width: 88
                    height: 34
                    radius: 10
                    color: cancelMa.containsMouse ? theme.hoverFill : theme.surfaceAlt
                    border.color: theme.border
                    border.width: 1
                    Label {
                        anchors.centerIn: parent
                        text: "Cancel"
                        color: theme.textSecondary
                        font.pixelSize: 13
                    }
                    MouseArea {
                        id: cancelMa
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: newCategoryDialog.close()
                    }
                }

                Rectangle {
                    id: addCatBtn
                    width: 88
                    height: 34
                    radius: 10
                    color: theme.tertiary
                    Label {
                        anchors.centerIn: parent
                        text: "Add"
                        color: theme.onAccent
                        font.pixelSize: 13
                        font.bold: true
                    }
                    function clicked() {
                        newCatError.text = ""
                        var n = newCatField.text.trim()
                        if (!n.length) {
                            newCatError.text = "Enter a name."
                            return
                        }
                        var existing = root.displayCategories
                        for (var i = 0; i < existing.length; i++) {
                            if (("" + existing[i]).toLowerCase() === n.toLowerCase()) {
                                newCatError.text = "That category already exists."
                                return
                            }
                        }
                        if (typeof settingsController !== "undefined" && settingsController)
                            settingsController.addVaultCategory(n)
                        root.filterCategory = n
                        newCatField.text = ""
                        newCategoryDialog.close()
                    }
                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: addCatBtn.clicked()
                    }
                }
            }
        }

        onOpened: {
            newCatError.text = ""
            newCatField.text = ""
            newCatField.forceActiveFocus()
        }
    }

    Loader {
        id: mainLoader
        anchors.fill: parent
        sourceComponent: root.vaultUnlocked ? vaultContentComponent : pinGateComponent
    }

    Component {
        id: pinGateComponent
        Item {
            id: pinGate
            anchors.fill: parent

            property int lockoutSeconds: authController.pinLockoutSecondsRemaining()

            function formatRemaining(sec) {
                return Math.floor(sec / 60) + "m " + (sec % 60) + "s"
            }

            Timer {
                interval: 500
                repeat: true
                running: true
                onTriggered: pinGate.lockoutSeconds = authController.pinLockoutSecondsRemaining()
            }

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
                          ? "The vault uses a PIN as an extra view lock. Enter your PIN to unlock this view."
                          : "Create a PIN to add a view lock for the vault."
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
                    enabled: pinGate.lockoutSeconds === 0
                    Layout.fillWidth: true
                    onAccepted: unlockButton.clicked()
                }
                StyledTextField {
                    id: confirmField
                    placeholderText: "Confirm PIN"
                    echoMode: TextInput.Password
                    inputMethodHints: Qt.ImhDigitsOnly
                    maximumLength: 8
                    enabled: pinGate.lockoutSeconds === 0
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
                Label {
                    id: pinLockoutLabel
                    text: "Too many failed attempts. Try again in "
                          + pinGate.formatRemaining(pinGate.lockoutSeconds)
                    color: theme.danger
                    font.pixelSize: 12
                    visible: pinGate.lockoutSeconds > 0
                    wrapMode: Text.WordWrap
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignHCenter
                }
                PrimaryButton {
                    id: unlockButton
                    text: pinGate.lockoutSeconds > 0
                          ? "LOCKED"
                          : (authController.hashVaultPin() ? "Unlock" : "Set PIN")
                    enabled: pinGate.lockoutSeconds === 0
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
                            text: "Create categories and store credentials your way."
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
                        model: root.displayCategories
                        delegate: Rectangle {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 72
                            radius: theme.radiusMedium
                            color: root.filterCategory === modelData
                                   ? Qt.rgba(theme.tertiary.r, theme.tertiary.g, theme.tertiary.b, 0.18)
                                   : theme.surfaceAlt
                            border.width: 1
                            border.color: root.filterCategory === modelData
                                          ? theme.tertiary : theme.border

                            ColumnLayout {
                                anchors.fill: parent
                                anchors.margins: 12
                                spacing: 4

                                RowLayout {
                                    Layout.fillWidth: true
                                    spacing: 6

                                    Label {
                                        text: modelData
                                        color: theme.textPrimary
                                        font.pixelSize: 13
                                        font.bold: true
                                        elide: Text.ElideRight
                                        Layout.fillWidth: true
                                    }

                                    // Count when category has entries
                                    Label {
                                        text: "" + root.categoryCount(modelData)
                                        color: theme.textMuted
                                        font.pixelSize: 14
                                        font.bold: true
                                        visible: root.categoryCount(modelData) > 0
                                    }

                                    // Delete only when empty (no overlap with count)
                                    Rectangle {
                                        visible: root.categoryCount(modelData) === 0
                                        width: 22
                                        height: 22
                                        radius: 11
                                        color: delMa.containsMouse
                                               ? Qt.rgba(theme.danger.r, theme.danger.g, theme.danger.b, 0.25)
                                               : "transparent"
                                        Text {
                                            anchors.centerIn: parent
                                            text: "×"
                                            color: theme.textMuted
                                            font.pixelSize: 14
                                        }
                                        MouseArea {
                                            id: delMa
                                            anchors.fill: parent
                                            hoverEnabled: true
                                            cursorShape: Qt.PointingHandCursor
                                            onClicked: {
                                                if (typeof settingsController !== "undefined")
                                                    settingsController.removeVaultCategory(modelData)
                                                if (root.filterCategory === modelData)
                                                    root.filterCategory = ""
                                            }
                                        }
                                        ToolTip.visible: delMa.containsMouse
                                        ToolTip.text: "Delete category"
                                        ToolTip.delay: 300
                                    }
                                }

                                Item { Layout.fillHeight: true }
                            }

                            MouseArea {
                                anchors.fill: parent
                                z: -1
                                cursorShape: Qt.PointingHandCursor
                                onClicked: {
                                    root.filterCategory =
                                        (root.filterCategory === modelData) ? "" : modelData
                                }
                            }
                        }
                    }

                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 72
                        radius: theme.radiusMedium
                        color: addCatMa.containsMouse ? theme.hoverFill : theme.surfaceAlt
                        border.width: 1
                        border.color: theme.border
                        ColumnLayout {
                            anchors.centerIn: parent
                            spacing: 2
                            Label {
                                text: "+"
                                color: theme.tertiary
                                font.pixelSize: 22
                                Layout.alignment: Qt.AlignHCenter
                            }
                            Label {
                                text: "New category"
                                color: theme.textMuted
                                font.pixelSize: 11
                                Layout.alignment: Qt.AlignHCenter
                            }
                        }
                        MouseArea {
                            id: addCatMa
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: newCategoryDialog.open()
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
                        spacing: 16
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
                            text: "URL"
                            Layout.preferredWidth: root.colUrl
                            Layout.maximumWidth: root.colUrl
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
                        // One row revealed at a time; keyed by model id so the
                        // state is correct even when delegate instances recycle.
                        readonly property bool revealed: root.revealedId === modelData.id

                        RowLayout {
                            anchors.fill: parent
                            anchors.leftMargin: 12
                            anchors.rightMargin: 12
                            spacing: 16

                            // SERVICE — text only, no icon
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

                            Label {
                                text: (modelData.url && modelData.url.length) ? modelData.url : "—"
                                Layout.preferredWidth: root.colUrl
                                Layout.maximumWidth: root.colUrl
                                color: (modelData.url && modelData.url.length)
                                       ? theme.tertiary : theme.textMuted
                                font.pixelSize: 12
                                elide: Text.ElideMiddle
                                MouseArea {
                                    anchors.fill: parent
                                    enabled: !!(modelData.url && modelData.url.length)
                                    cursorShape: enabled ? Qt.PointingHandCursor : Qt.ArrowCursor
                                    onClicked: root.openUrl(modelData.url)
                                }
                            }

                            RowLayout {
                                Layout.fillWidth: true
                                Layout.minimumWidth: root.colPassword
                                spacing: 6
                                Label {
                                    Layout.fillWidth: true
                                    // Constant-length mask: reveals nothing about
                                    // the real password length, and the "•" in
                                    // an 8-digit PIN can't leak either.
                                    text: entryRow.revealed
                                          ? (modelData.password || "")
                                          : "••••••••••"
                                    color: theme.textSecondary
                                    font.pixelSize: 13
                                    elide: Text.ElideRight
                                }
                                IconButton {
                                    iconName: entryRow.revealed ? "visibility_off" : "visibility"
                                    iconColor: theme.textMuted
                                    tooltip: entryRow.revealed ? "Hide password" : "Show password"
                                    onClicked: root.revealedId =
                                        (root.revealedId === modelData.id) ? null : modelData.id
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
                                    onClicked: root.openEdit(modelData)
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
                    onClicked: root.openCreate()
                }
            }
        }
    }

    Connections {
        target: typeof vaultController !== "undefined" ? vaultController : null
        function onEntriesChanged() { root.refresh() }
    }
    Connections {
        target: typeof settingsController !== "undefined" ? settingsController : null
        function onVaultCategoriesChanged() {
            root.filterCategory = root.filterCategory
        }
    }
    Connections {
        target: typeof session !== "undefined" ? session : null
        function onLocked() {
            root.vaultUnlocked = false
            root.filterCategory = ""
            root.revealedId = null
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