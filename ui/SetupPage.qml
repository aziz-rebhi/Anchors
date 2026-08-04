import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Page {
    id: root

    Theme { id: theme }

    signal accountReady(string name)

    background: Rectangle { color: theme.background }

    ColumnLayout {
        anchors.centerIn: parent
        spacing: 16
        width: Math.min(parent.width * 0.8, 360)

        Image {
            source: "logo.png"
            Layout.alignment: Qt.AlignHCenter
            Layout.preferredWidth: 88
            Layout.preferredHeight: 88 * (sourceSize.height / Math.max(sourceSize.width, 1))
            fillMode: Image.PreserveAspectFit
            Layout.bottomMargin: 8
        }

        Label {
            text: "Create your account"
            color: theme.textPrimary
            font.family: theme.headlineFont
            font.pixelSize: 24
            font.bold: true
            Layout.alignment: Qt.AlignHCenter
        }

        Label {
            text: "Everything you enter stays encrypted, on this device."
            color: theme.textSecondary
            font.family: theme.bodyFont
            font.pixelSize: 13
            Layout.alignment: Qt.AlignHCenter
        }

        StyledTextField {
            id: nameField
            placeholderText: "Your name"
            Layout.fillWidth: true
            Layout.topMargin: 8
        }

        StyledTextField {
            id: passwordField
            placeholderText: "Master password"
            echoMode: TextInput.Password
            Layout.fillWidth: true
        }

        StyledTextField {
            id: confirmField
            placeholderText: "Confirm password"
            echoMode: TextInput.Password
            Layout.fillWidth: true
            onAccepted: createButton.clicked()
        }

        Label {
            id: errorLabel
            color: theme.danger
            font.family: theme.bodyFont
            font.pixelSize: 12
            visible: text.length > 0
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignHCenter
            horizontalAlignment: Text.AlignHCenter
        }

        PrimaryButton {
            id: createButton
            text: "CREATE ACCOUNT"
            Layout.fillWidth: true
            onClicked: {
                errorLabel.text = ""

                if (passwordField.text.length === 0) {
                    errorLabel.text = "Please enter a password."
                    return
                }
                if (passwordField.text !== confirmField.text) {
                    errorLabel.text = "Passwords don't match."
                    return
                }

                if (authController.createAccount(nameField.text, passwordField.text)) {
                    root.accountReady(nameField.text)
                }
            }
        }
    }

    Connections {
        target: authController
        function onAccountCreationFailed(reason) {
            errorLabel.text = reason
        }
    }
}
