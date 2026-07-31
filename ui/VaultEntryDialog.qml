import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

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

    function openForCreate() {
        editingId = ""
        titleField.text = ""
        usernameField.text = ""
        passwordField.text = ""
        urlField.text = ""
        categoryField.text = ""
        errorLabel.text = ""
        open()
    }

    function openForEdit(entry) {
        editingId = entry.id
        titleField.text = entry.title
        usernameField.text = entry.username
        passwordField.text = entry.password
        urlField.text = entry.url
        categoryField.text = entry.category
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
        StyledTextField {
            id: categoryField
            placeholderText: "Category (e.g. Social, Work, Learning, Finance)"
            Layout.fillWidth: true
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

                    var ok
                    if (root.isEditing) {
                        ok = vaultController.updateEntry(root.editingId, titleField.text, usernameField.text,
                                                          passwordField.text, urlField.text, categoryField.text)
                    } else {
                        ok = vaultController.addEntry(titleField.text, usernameField.text,
                                                       passwordField.text, urlField.text, categoryField.text)
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
