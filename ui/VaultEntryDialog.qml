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
    width: 380
    padding: 20
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

    background: Rectangle {
        color: theme.surface
        radius: theme.radiusMedium
        border.width: 1
        border.color: theme.border
    }

    // ---- Category list (static) ----
    readonly property var categoryOptions: ["Social", "Work", "Learning", "Finance", "Other"]

    function openForCreate() {
        editingId = ""
        titleField.text = ""
        usernameField.text = ""
        passwordField.text = ""
        urlField.text = ""
        categoryCombo.currentIndex = 0 // default to first category
        errorLabel.text = ""
        open()
    }

    function openForEdit(entry) {
        editingId = entry.id
        titleField.text = entry.title
        usernameField.text = entry.username
        passwordField.text = entry.password
        urlField.text = entry.url
        // Set the combo to the category if it matches, else "Other"
        var idx = categoryOptions.indexOf(entry.category)
        categoryCombo.currentIndex = (idx >= 0) ? idx : 4
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
            PrimaryButton {
                text: "Generate"
                outlined: true
                Layout.preferredHeight: 44
                onClicked: passwordField.text = vaultController.generatePassword(16, true, true)
            }
        }
        StyledTextField {
            id: urlField
            placeholderText: "URL (optional)"
            Layout.fillWidth: true
        }

        // ---- Category dropdown ----
        ComboBox {
            id: categoryCombo
            Layout.fillWidth: true
            model: root.categoryOptions
            // Use a custom delegate to match the StyledTextField style
            delegate: ItemDelegate {
                width: categoryCombo.width
                text: modelData
                font.family: theme.bodyFont
                font.pixelSize: 14
                highlighted: categoryCombo.highlightedIndex === index
                background: Rectangle {
                    color: highlighted ? theme.surfaceAlt : "transparent"
                }
            }
            // Style the content to match dark theme
            contentItem: Text {
                text: categoryCombo.displayText
                color: theme.textPrimary
                font.family: theme.bodyFont
                font.pixelSize: 14
                horizontalAlignment: Text.AlignLeft
                verticalAlignment: Text.AlignVCenter
                elide: Text.ElideRight
            }
            background: Rectangle {
                color: theme.surfaceAlt
                border.color: theme.border
                border.width: 1
                radius: theme.radiusMedium
                implicitHeight: 44
                // Arrow indicator
                Label {
                    anchors.right: parent.right
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.rightMargin: 12
                    text: "▼"
                    color: theme.textMuted
                    font.pixelSize: 12
                }
            }
            popup: Popup {
                y: categoryCombo.height + 2
                width: categoryCombo.width
                padding: 4
                background: Rectangle {
                    color: theme.surfaceAlt
                    border.color: theme.border
                    border.width: 1
                    radius: theme.radiusMedium
                }
                contentItem: ListView {
                    clip: true
                    implicitHeight: contentHeight
                    model: categoryCombo.popup.visible ? categoryCombo.delegateModel : null
                    currentIndex: categoryCombo.highlightedIndex
                    ScrollIndicator.vertical: ScrollIndicator { }
                }
            }
        }

        Label {
            id: errorLabel
            color: theme.danger
            font.family: theme.bodyFont
            font.pixelSize: 12
            visible: text.length > 0
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.topMargin: 4
            spacing: 8

            PrimaryButton {
                text: "Cancel"
                outlined: true
                Layout.fillWidth: true
                onClicked: root.close()
            }
            PrimaryButton {
                text: root.isEditing ? "Save" : "Add"
                Layout.fillWidth: true
                onClicked: {
                    if (titleField.text.trim().length === 0) {
                        errorLabel.text = "Service name is required."
                        return
                    }

                    var category = categoryCombo.displayText

                    var ok
                    if (root.isEditing) {
                        ok = vaultController.updateEntry(root.editingId, titleField.text, usernameField.text,
                                                          passwordField.text, urlField.text, category)
                    } else {
                        ok = vaultController.addEntry(titleField.text, usernameField.text,
                                                       passwordField.text, urlField.text, category)
                    }

                    if (ok) {
                        root.close()
                    } else {
                        errorLabel.text = "Could not save. Is the vault unlocked?"
                    }
                }
            }
        }
    }
}