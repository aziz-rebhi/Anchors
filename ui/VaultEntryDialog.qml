import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Popup {
    id: root

    Theme { id: theme }

    property string editingId: ""
    readonly property bool isEditing: editingId.length > 0
    property var knownCategories: []

    modal: true
    focus: true
    x: (parent ? parent.width - width : 0) / 2
    y: (parent ? parent.height - height : 0) / 2
    width: 400
    padding: 20
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

    background: Rectangle {
        color: theme.surface
        radius: theme.radiusMedium
        border.width: 1
        border.color: theme.border
    }

    readonly property var servicePresets: [
        { name: "Facebook",  glyph: "📘", url: "https://www.facebook.com" },
        { name: "Instagram", glyph: "📸", url: "https://www.instagram.com" },
        { name: "Google",    glyph: "🔵", url: "https://accounts.google.com" },
        { name: "Gmail",     glyph: "📧", url: "https://mail.google.com" },
        { name: "X / Twitter", glyph: "🐦", url: "https://x.com" },
        { name: "GitHub",    glyph: "🐙", url: "https://github.com" },
        { name: "LinkedIn",  glyph: "💼", url: "https://www.linkedin.com" },
        { name: "Amazon",    glyph: "📦", url: "https://www.amazon.com" },
        { name: "Netflix",   glyph: "🎬", url: "https://www.netflix.com" },
        { name: "Discord",   glyph: "💬", url: "https://discord.com" },
        { name: "Steam",     glyph: "🎮", url: "https://store.steampowered.com" },
        { name: "Apple",     glyph: "🍎", url: "https://appleid.apple.com" },
        { name: "Microsoft", glyph: "🪟", url: "https://account.microsoft.com" },
        { name: "PayPal",    glyph: "💳", url: "https://www.paypal.com" },
        { name: "Spotify",   glyph: "🎵", url: "https://accounts.spotify.com" },
        { name: "Custom",    glyph: "🔗", url: "" }
    ]

    function applyPreset(p) {
        if (!p) return
        if (p.name !== "Custom")
            titleField.text = p.name
        if (p.url && p.url.length)
            urlField.text = p.url
    }

    function openForCreate() {
        editingId = ""
        titleField.text = ""
        usernameField.text = ""
        passwordField.text = ""
        urlField.text = ""
        categoryField.text = knownCategories.length ? knownCategories[0] : ""
        errorLabel.text = ""
        open()
    }

    function openForEdit(entry) {
        editingId = entry.id
        titleField.text = entry.title || ""
        usernameField.text = entry.username || ""
        passwordField.text = entry.password || ""
        urlField.text = entry.url || ""
        categoryField.text = entry.category || ""
        errorLabel.text = ""
        open()
    }

    ColumnLayout {
        width: parent.width
        spacing: 12

        Label {
            text: root.isEditing ? "Edit entry" : "Add password"
            color: theme.textPrimary
            font.family: theme.headlineFont
            font.pixelSize: 18
            font.bold: true
        }

        Label {
            text: "Quick pick"
            color: theme.textMuted
            font.pixelSize: 11
            visible: !root.isEditing
        }

        Flow {
            Layout.fillWidth: true
            spacing: 6
            visible: !root.isEditing
            Repeater {
                model: root.servicePresets
                delegate: Rectangle {
                    width: 36
                    height: 36
                    radius: 8
                    color: presetMa.containsMouse ? theme.hoverFill : theme.surfaceAlt
                    border.color: theme.border
                    border.width: 1
                    Text {
                        anchors.centerIn: parent
                        text: modelData.glyph
                        font.pixelSize: 16
                        font.family: "Noto Color Emoji"
                    }
                    MouseArea {
                        id: presetMa
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: root.applyPreset(modelData)
                    }
                    ToolTip.visible: presetMa.containsMouse
                    ToolTip.text: modelData.name
                    ToolTip.delay: 300
                }
            }
        }

        StyledTextField {
            id: titleField
            placeholderText: "Service name"
            Layout.fillWidth: true
        }
        StyledTextField {
            id: usernameField
            placeholderText: "Username / email"
            Layout.fillWidth: true
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 8
            StyledTextField {
                id: passwordField
                placeholderText: "Password"
                echoMode: TextInput.Password
                Layout.fillWidth: true
            }
            Rectangle {
                width: 40
                height: 40
                radius: 10
                color: genMa.containsMouse ? theme.hoverFill : theme.surfaceAlt
                border.color: theme.border
                border.width: 1
                Text {
                    anchors.centerIn: parent
                    text: "🔑"
                    font.pixelSize: 16
                    font.family: "Noto Color Emoji"
                }
                MouseArea {
                    id: genMa
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: passwordField.text = vaultController.generatePassword(16, true, true)
                }
                ToolTip.visible: genMa.containsMouse
                ToolTip.text: "Generate password"
                ToolTip.delay: 300
            }
        }

        StyledTextField {
            id: urlField
            placeholderText: "URL (optional)"
            Layout.fillWidth: true
        }

        ColumnLayout {
            Layout.fillWidth: true
            spacing: 6
            Label {
                text: "Category"
                color: theme.textMuted
                font.pixelSize: 11
            }
            StyledTextField {
                id: categoryField
                Layout.fillWidth: true
                placeholderText: "Type a category or pick below"
            }
            Flow {
                Layout.fillWidth: true
                spacing: 6
                Repeater {
                    model: root.knownCategories
                    delegate: Rectangle {
                        height: 26
                        radius: 13
                        color: categoryField.text === modelData
                               ? Qt.rgba(theme.tertiary.r, theme.tertiary.g, theme.tertiary.b, 0.25)
                               : theme.surfaceAlt
                        border.color: theme.border
                        border.width: 1
                        implicitWidth: chipLab.implicitWidth + 16
                        Label {
                            id: chipLab
                            anchors.centerIn: parent
                            text: modelData
                            font.pixelSize: 11
                            color: theme.textSecondary
                        }
                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            onClicked: categoryField.text = modelData
                        }
                    }
                }
            }
        }

        Label {
            id: errorLabel
            color: theme.danger
            font.pixelSize: 12
            visible: text.length > 0
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.topMargin: 4
            spacing: 8
            Item { Layout.fillWidth: true }

            Rectangle {
                width: 40
                height: 40
                radius: 10
                color: cancelMa.containsMouse ? theme.hoverFill : theme.surfaceAlt
                border.color: theme.border
                border.width: 1
                Text {
                    anchors.centerIn: parent
                    text: "✕"
                    color: theme.textSecondary
                    font.pixelSize: 16
                }
                MouseArea {
                    id: cancelMa
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: root.close()
                }
                ToolTip.visible: cancelMa.containsMouse
                ToolTip.text: "Cancel"
                ToolTip.delay: 300
            }

            Rectangle {
                width: 40
                height: 40
                radius: 10
                color: theme.tertiary
                Text {
                    anchors.centerIn: parent
                    text: "✓"
                    color: theme.onAccent
                    font.pixelSize: 18
                    font.bold: true
                }
                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        if (titleField.text.trim().length === 0) {
                            errorLabel.text = "Service name is required."
                            return
                        }
                        var category = categoryField.text.trim()
                        if (!category.length) {
                            errorLabel.text = "Category is required."
                            return
                        }
                        var ok
                        if (root.isEditing) {
                            ok = vaultController.updateEntry(
                                root.editingId, titleField.text, usernameField.text,
                                passwordField.text, urlField.text, category)
                        } else {
                            ok = vaultController.addEntry(
                                titleField.text, usernameField.text,
                                passwordField.text, urlField.text, category)
                        }
                        if (ok) {
                            if (typeof settingsController !== "undefined" && settingsController)
                                settingsController.addVaultCategory(category)
                            root.close()
                        } else {
                            errorLabel.text = "Could not save. Is the vault unlocked?"
                        }
                    }
                }
                ToolTip.text: root.isEditing ? "Save" : "Add"
                ToolTip.delay: 300
            }
        }
    }
}