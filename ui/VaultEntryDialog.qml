import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Popup {
    id: root

    Theme { id: theme }

    property string editingId: ""
    readonly property bool isEditing: editingId.length > 0

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

    // Preset apps: glyph + default URL (title filled when picked)
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

    readonly property var categoryOptions: [
        "Social", "Work", "Learning", "Finance", "Gaming",
        "Shopping", "Entertainment", "Email", "Development", "Cloud"
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
        categoryCombo.currentIndex = 0
        errorLabel.text = ""
        open()
    }

    function openForEdit(entry) {
        editingId = entry.id
        titleField.text = entry.title || ""
        usernameField.text = entry.username || ""
        passwordField.text = entry.password || ""
        urlField.text = entry.url || ""
        var idx = categoryOptions.indexOf(entry.category)
        categoryCombo.currentIndex = idx < 0 ? 0 : idx
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
            // Generate — icon
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

        ComboBox {
            id: categoryCombo
            Layout.fillWidth: true
            model: root.categoryOptions

            contentItem: Text {
                leftPadding: 12
                rightPadding: 28
                text: categoryCombo.displayText
                color: theme.textPrimary
                font.pixelSize: 14
                verticalAlignment: Text.AlignVCenter
            }
            background: Rectangle {
                color: theme.surfaceAlt
                border.color: theme.border
                border.width: 1
                radius: theme.radiusMedium
                implicitHeight: 44
            }
            delegate: ItemDelegate {
                width: categoryCombo.width
                contentItem: Text {
                    text: modelData
                    color: theme.textPrimary
                    font.pixelSize: 14
                }
                background: Rectangle {
                    color: highlighted ? theme.surfaceAlt : theme.surface
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

            // Cancel
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

            // Save
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
                        var category = categoryCombo.displayText
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
                        if (ok) root.close()
                        else errorLabel.text = "Could not save. Is the vault unlocked?"
                    }
                }
                ToolTip.text: root.isEditing ? "Save" : "Add"
                ToolTip.delay: 300
            }
        }
    }
}