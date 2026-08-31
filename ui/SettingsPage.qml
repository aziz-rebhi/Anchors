import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Dialogs

Page {
    id: root
    Theme { id: theme }

    signal lockRequested()

    property var accentChoices: [
        "#89b4fa", "#f38ba8", "#a6e3a1", "#fab387",
        "#cba6f7", "#94e2d5", "#f9e2af", "#6c7086"
    ]

    readonly property real groupRadius: theme.radiusMedium
    readonly property real pageMargin: 40
    readonly property real maxContentWidth: 720

    background: Rectangle { color: theme.background }

    function showMsg(text, isError) {
        toast.text = text
        toast.isError = isError
        toast.visible = true
        toastTimer.restart()
    }

    Connections {
        target: settingsController
        function onOperationFailed(reason) { root.showMsg(reason, true) }
        function onOperationSucceeded(message) { root.showMsg(message, false) }
        function onLockRequested() { root.lockRequested() }
    }

    ScrollView {
        id: scroll
        anchors.fill: parent
        clip: true
        contentWidth: availableWidth
        ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
        ScrollBar.vertical.policy: ScrollBar.AsNeeded

        // Content must report a real height so the page can scroll
        ColumnLayout {
            id: col
            width: Math.min(root.maxContentWidth, scroll.availableWidth - pageMargin * 2)
            x: Math.max(pageMargin, (scroll.availableWidth - width) / 2)
            spacing: 24

            // top padding
            Item { Layout.preferredHeight: 12 }

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 4
                Label {
                    text: "Settings"
                    color: theme.textPrimary
                    font.pixelSize: 28
                    font.weight: Font.DemiBold
                    font.family: theme.headlineFont
                }
                Label {
                    text: "Preferences, security, and data"
                    color: theme.textMuted
                    font.pixelSize: 13
                }
            }

            GroupCard {
                title: "General"
                Layout.fillWidth: true

                GroupRow {
                    title: "Start page"
                    subtitle: "Page shown after unlock"
                    trailing: TahoeCombo {
                        model: [
                            { t: "Dashboard", v: "dashboard" },
                            { t: "Notes", v: "notes" },
                            { t: "Vault", v: "vault" },
                            { t: "Calendar", v: "calendar" },
                            { t: "To-Do", v: "todo" }
                        ]
                        value: settingsController.startPage
                        onValuePicked: settingsController.startPage = v
                    }
                }
                GroupSeparator {}
                GroupRow {
                    title: "Calendar default view"
                    subtitle: "Month or week when opening Calendar"
                    trailing: TahoeCombo {
                        model: [
                            { t: "Month", v: "month" },
                            { t: "Week", v: "week" }
                        ]
                        value: settingsController.calendarDefaultView
                        onValuePicked: settingsController.calendarDefaultView = v
                    }
                }
            }

            GroupCard {
                title: "Security"
                Layout.fillWidth: true

                GroupRow {
                    title: "Auto-lock"
                    subtitle: "Lock after inactivity"
                    trailing: TahoeCombo {
                        model: [
                            { t: "1 minute", v: 1 },
                            { t: "5 minutes", v: 5 },
                            { t: "15 minutes", v: 15 },
                            { t: "30 minutes", v: 30 },
                            { t: "Never", v: 0 }
                        ]
                        value: settingsController.autoLockMinutes
                        onValuePicked: settingsController.autoLockMinutes = v
                    }
                }
                GroupSeparator {}
                GroupRow {
                    title: "Clear clipboard after copy"
                    subtitle: "Wipe vault secrets from the clipboard"
                    trailing: RoundedSwitch {
                        checked: settingsController.clearClipboard
                        onToggled: settingsController.clearClipboard = checked
                    }
                }
                GroupSeparator {}
                GroupRow {
                    title: "Lock when minimized"
                    subtitle: "Lock when the window is minimized"
                    trailing: RoundedSwitch {
                        checked: settingsController.lockOnMinimize
                        onToggled: settingsController.lockOnMinimize = checked
                    }
                }
                GroupSeparator {}
                GroupRow {
                    title: "Lock now"
                    subtitle: "Lock the session immediately"
                    trailing: TahoeIconButton {
                        symbol: "🔒"
                        onClicked: settingsController.lockNow()
                    }
                }
                GroupSeparator {}
                GroupRow {
                    title: "Change master password"
                    subtitle: "Re-encrypt local data with a new password"
                    trailing: TahoeIconButton {
                        symbol: "🔑"
                        onClicked: passwordDialog.open()
                    }
                }
            }

            GroupCard {
                title: "Appearance"
                Layout.fillWidth: true

                GroupRow {
                    title: "Theme"
                    subtitle: settingsController.themeId === "light"
                              ? "Light theme is active" : "Dark theme is active"
                    trailing: ThemeSlider {
                        valueIsLight: settingsController.themeId === "light"
                        onToggled: function (light) {
                            settingsController.themeId = light ? "light" : "dark"
                        }
                    }
                }
                GroupSeparator {}
                GroupRow {
                    title: "Accent color"
                    subtitle: "Highlights and active states"
                    trailing: Row {
                        spacing: 8
                        Repeater {
                            model: root.accentChoices
                            delegate: Rectangle {
                                width: 18; height: 18; radius: 9
                                color: modelData
                                border.width: modelData === settingsController.accentColor ? 2 : 0
                                border.color: theme.textPrimary
                                scale: modelData === settingsController.accentColor ? 1.15 : 1
                                Behavior on scale { NumberAnimation { duration: 80 } }
                                MouseArea {
                                    anchors.fill: parent
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: settingsController.accentColor = modelData
                                }
                            }
                        }
                    }
                }
            }

            GroupCard {
                title: "Data"
                Layout.fillWidth: true

                GroupRow {
                    title: "Data location"
                    subtitle: settingsController.dataPath
                    trailing: TahoeIconButton {
                        symbol: "📂"
                        onClicked: settingsController.openDataLocation()
                    }
                }
                GroupSeparator {}
                GroupRow {
                    title: "Export backup"
                    subtitle: "Copy encrypted data files to a folder"
                    trailing: TahoeIconButton {
                        symbol: "📤"
                        onClicked: exportDialog.open()
                    }
                }
                GroupSeparator {}
                GroupRow {
                    title: "Import backup"
                    subtitle: "Restore from a previous export"
                    trailing: TahoeIconButton {
                        symbol: "📥"
                        onClicked: importDialog.open()
                    }
                }
                GroupSeparator {}
                GroupRow {
                    title: "Wipe all data"
                    subtitle: "Delete local encrypted files (cannot undo)"
                    trailing: TahoeIconButton {
                        symbol: "🗑"
                        danger: true
                        onClicked: wipeDialog.open()
                    }
                }
            }

            GroupCard {
                title: "About"
                Layout.fillWidth: true
                GroupRow {
                    title: "Anchors"
                    subtitle: "Private notes, vault, tasks & calendar"
                    trailing: Label {
                        text: "v" + settingsController.appVersion
                        color: theme.textMuted
                        font.pixelSize: 13
                    }
                }
            }

            Item { Layout.preferredHeight: 48 }
        }
    }

    Rectangle {
        id: toast
        property alias text: toastLabel.text
        property bool isError: false
        visible: false
        z: 10
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 28
        width: toastLabel.implicitWidth + 32
        height: 40
        radius: 20
        color: isError ? Qt.rgba(theme.danger.r, theme.danger.g, theme.danger.b, 0.2)
                       : Qt.rgba(theme.success.r, theme.success.g, theme.success.b, 0.2)
        border.color: isError ? theme.danger : theme.success
        border.width: 1
        Label {
            id: toastLabel
            anchors.centerIn: parent
            color: theme.textPrimary
            font.pixelSize: 12
        }
        Timer {
            id: toastTimer
            interval: 3200
            onTriggered: toast.visible = false
        }
    }

    FolderDialog {
        id: exportDialog
        title: "Choose folder for backup"
        onAccepted: {
            var path = selectedFolder.toString()
            settingsController.exportBackup(path)
        }
    }

    FolderDialog {
        id: importDialog
        title: "Choose backup folder"
        onAccepted: {
            var path = selectedFolder.toString()
            settingsController.importBackup(path)
        }
    }

    Dialog {
        id: wipeDialog
        title: "Wipe all data?"
        modal: true
        anchors.centerIn: parent
        standardButtons: Dialog.NoButton
        background: Rectangle {
            color: theme.surfaceAlt
            radius: groupRadius
            border.color: theme.border
        }
        ColumnLayout {
            anchors.margins: 20
            spacing: 16
            Label {
                text: "This deletes local encrypted files and locks the session."
                wrapMode: Text.WordWrap
                color: theme.textSecondary
                Layout.preferredWidth: 280
            }
            RowLayout {
                Layout.alignment: Qt.AlignRight
                spacing: 8
                TahoeIconButton { symbol: "✕"; onClicked: wipeDialog.close() }
                TahoeIconButton {
                    symbol: "✓"
                    danger: true
                    onClicked: {
                        settingsController.wipeAllData()
                        wipeDialog.close()
                    }
                }
            }
        }
    }

    Dialog {
        id: passwordDialog
        title: "Change master password"
        modal: true
        anchors.centerIn: parent
        width: 360
        background: Rectangle {
            color: theme.surfaceAlt
            radius: groupRadius
            border.color: theme.border
        }
        ColumnLayout {
            anchors.margins: 20
            anchors.fill: parent
            spacing: 12
            StyledTextField {
                id: curPass
                Layout.fillWidth: true
                placeholderText: "Current password"
                echoMode: TextInput.Password
            }
            StyledTextField {
                id: newPass
                Layout.fillWidth: true
                placeholderText: "New password"
                echoMode: TextInput.Password
            }
            StyledTextField {
                id: newPass2
                Layout.fillWidth: true
                placeholderText: "Confirm new password"
                echoMode: TextInput.Password
            }
            Label {
                id: passError
                color: theme.danger
                font.pixelSize: 12
                visible: text.length > 0
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
            }
            RowLayout {
                Layout.alignment: Qt.AlignRight
                spacing: 8
                TahoeIconButton { symbol: "✕"; onClicked: passwordDialog.close() }
                TahoeIconButton {
                    symbol: "✓"
                    onClicked: {
                        passError.text = ""
                        if (newPass.text.length < 8) {
                            passError.text = "New password must be at least 8 characters."
                            return
                        }
                        if (newPass.text !== newPass2.text) {
                            passError.text = "New passwords do not match."
                            return
                        }
                        var err = settingsController.changeMasterPassword(curPass.text, newPass.text)
                        if (err && err.length) {
                            passError.text = err
                            return
                        }
                        passwordDialog.close()
                        root.showMsg("Password changed.", false)
                    }
                }
            }
        }
        onOpened: {
            curPass.text = ""; newPass.text = ""; newPass2.text = ""; passError.text = ""
            curPass.forceActiveFocus()
        }
    }

    component GroupCard: ColumnLayout {
        id: card
        property string title: ""
        default property alias content: body.data
        spacing: 8

        Label {
            text: card.title
            color: theme.textMuted
            font.pixelSize: 12
            font.weight: Font.DemiBold
            leftPadding: 4
        }

        Rectangle {
            Layout.fillWidth: true
            implicitHeight: body.implicitHeight
            radius: groupRadius
            color: theme.surfaceAlt
            border.color: theme.border
            border.width: 1
            clip: true

            ColumnLayout {
                id: body
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: parent.top
                spacing: 0
            }
        }
    }

    component GroupSeparator: Rectangle {
        Layout.fillWidth: true
        Layout.leftMargin: 14
        height: 1
        color: theme.border
    }

    component GroupRow: Item {
        id: row
        property string title: ""
        property string subtitle: ""
        default property alias trailing: trail.data

        Layout.fillWidth: true
        implicitHeight: Math.max(52, textCol.implicitHeight + 22)

        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 14
            anchors.rightMargin: 12
            spacing: 12

            ColumnLayout {
                id: textCol
                Layout.fillWidth: true
                spacing: 2
                Label {
                    text: row.title
                    color: theme.textPrimary
                    font.pixelSize: 14
                }
                Label {
                    text: row.subtitle
                    color: theme.textMuted
                    font.pixelSize: 11
                    visible: row.subtitle.length > 0
                    wrapMode: Text.WordWrap
                    Layout.fillWidth: true
                }
            }

            Item {
                id: trail
                Layout.alignment: Qt.AlignVCenter
                implicitWidth: children.length ? children[0].implicitWidth : 0
                implicitHeight: children.length ? children[0].implicitHeight : 0
            }
        }
    }

    component TahoeCombo: ComboBox {
        id: combo
        property var value
        signal valuePicked(var v)

        textRole: "t"
        implicitWidth: 140
        implicitHeight: 28

        background: Rectangle {
            radius: height / 2
            color: theme.isDark ? Qt.rgba(1, 1, 1, 0.06) : Qt.rgba(0, 0, 0, 0.05)
            border.color: theme.border
        }
        contentItem: Text {
            text: combo.displayText
            color: theme.textPrimary
            font.pixelSize: 12
            verticalAlignment: Text.AlignVCenter
            leftPadding: 10
            rightPadding: 24
        }

        Component.onCompleted: syncIndex()
        onValueChanged: syncIndex()
        function syncIndex() {
            for (var i = 0; i < model.length; i++) {
                if (model[i].v === value) { currentIndex = i; return }
            }
        }
        onActivated: valuePicked(model[currentIndex].v)
    }

    component ThemeSlider: Rectangle {
        id: slider
        property bool valueIsLight: false
        signal toggled(bool light)

        implicitWidth: 112
        implicitHeight: 28
        radius: height / 2
        color: theme.isDark ? Qt.rgba(1, 1, 1, 0.08) : Qt.rgba(0, 0, 0, 0.06)
        border.color: theme.border
        border.width: 1

        Rectangle {
            id: thumb
            width: parent.width / 2 - 3
            height: parent.height - 6
            radius: height / 2
            anchors.verticalCenter: parent.verticalCenter
            x: slider.valueIsLight ? parent.width / 2 + 1 : 3
            color: theme.surface
            border.color: theme.border
            border.width: 1
            Behavior on x { NumberAnimation { duration: 160; easing.type: Easing.OutCubic } }
        }

        Row {
            anchors.fill: parent
            Text {
                width: parent.width / 2
                height: parent.height
                text: "Dark"
                font.pixelSize: 11
                font.weight: !slider.valueIsLight ? Font.DemiBold : Font.Normal
                color: !slider.valueIsLight ? theme.textPrimary : theme.textMuted
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }
            Text {
                width: parent.width / 2
                height: parent.height
                text: "Light"
                font.pixelSize: 11
                font.weight: slider.valueIsLight ? Font.DemiBold : Font.Normal
                color: slider.valueIsLight ? theme.textPrimary : theme.textMuted
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }
        }

        MouseArea {
            anchors.fill: parent
            cursorShape: Qt.PointingHandCursor
            onClicked: {
                var light = mouseX > width / 2
                if (light !== slider.valueIsLight)
                    slider.toggled(light)
            }
        }
    }

    component RoundedSwitch: Switch {
        id: sw
        implicitWidth: 42
        implicitHeight: 24

        indicator: Rectangle {
            implicitWidth: 42
            implicitHeight: 24
            radius: 12
            color: sw.checked
                   ? theme.tertiary
                   : (theme.isDark ? Qt.rgba(1, 1, 1, 0.12) : Qt.rgba(0, 0, 0, 0.12))
            border.color: sw.checked ? theme.tertiary : theme.border
            border.width: 1
            Behavior on color { ColorAnimation { duration: 120 } }

            Rectangle {
                width: 18
                height: 18
                radius: 9
                anchors.verticalCenter: parent.verticalCenter
                x: sw.checked ? parent.width - width - 3 : 3
                color: theme.isDark ? "#f5f5f7" : "#ffffff"
                border.color: theme.isDark ? Qt.rgba(0, 0, 0, 0.1) : Qt.rgba(0, 0, 0, 0.08)
                Behavior on x { NumberAnimation { duration: 140; easing.type: Easing.OutCubic } }
            }
        }
        contentItem: Item {}
    }

    component TahoeIconButton: Rectangle {
        property string symbol: ""
        property bool danger: false
        signal clicked()

        implicitWidth: 32
        implicitHeight: 32
        radius: 8
        color: ma.containsMouse
               ? (danger ? Qt.rgba(theme.danger.r, theme.danger.g, theme.danger.b, 0.2)
                         : (theme.isDark ? Qt.rgba(1, 1, 1, 0.1) : Qt.rgba(0, 0, 0, 0.08)))
               : (theme.isDark ? Qt.rgba(1, 1, 1, 0.05) : Qt.rgba(0, 0, 0, 0.04))

        Text {
            anchors.centerIn: parent
            text: parent.symbol
            font.pixelSize: 14
            // Color emoji (Flatpak + Linux)
            font.family: "Noto Color Emoji"
        }
        MouseArea {
            id: ma
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: Qt.PointingHandCursor
            onClicked: parent.clicked()
        }
    }
}