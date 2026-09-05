import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Page {
    id: root

    Theme { id: theme }

    signal unlocked(string name)

    property bool passwordVisible: false
    property int lockoutSeconds: authController.lockoutSecondsRemaining()

    function formatRemaining(sec) {
        return Math.floor(sec / 60) + "m " + (sec % 60) + "s"
    }

    Timer {
        interval: 500
        repeat: true
        running: true
        onTriggered: root.lockoutSeconds = authController.lockoutSecondsRemaining()
    }

    background: Rectangle { color: theme.background }

    ColumnLayout {
        anchors.centerIn: parent
        spacing: 20
        width: Math.min(parent.width * 0.8, 340)

        Item {
            Layout.alignment: Qt.AlignHCenter
            width: 96
            height: 96

            Repeater {
                model: 3
                Rectangle {
                    readonly property int step: index
                    anchors.centerIn: parent
                    width: 96 + step * 26
                    height: width
                    radius: width / 2
                    color: theme.secondary
                    opacity: 0.10 - step * 0.03
                }
            }

            Rectangle {
                anchors.centerIn: parent
                width: 72
                height: 72
                radius: 36
                color: theme.surfaceAlt
                border.width: 1
                border.color: theme.border

                Item {
                    anchors.centerIn: parent
                    width: 32
                    height: 32

                    Rectangle {
                        width: 20
                        height: 20
                        radius: 10
                        color: "transparent"
                        border.color: theme.tertiary
                        border.width: 3
                        anchors.horizontalCenter: parent.horizontalCenter
                        y: 0
                    }
                    Rectangle {
                        width: 24
                        height: 10
                        color: parent.parent.color
                        anchors.horizontalCenter: parent.horizontalCenter
                        y: 10
                    }
                    Rectangle {
                        width: 28
                        height: 18
                        radius: 4
                        color: theme.tertiary
                        anchors.horizontalCenter: parent.horizontalCenter
                        y: 14
                    }
                }
            }
        }

        Label {
            text: "System Locked"
            color: theme.textPrimary
            font.family: theme.headlineFont
            font.pixelSize: 26
            font.bold: true
            Layout.alignment: Qt.AlignHCenter
        }

        Label {
            text: "Authentication required to proceed."
            color: theme.textSecondary
            font.family: theme.bodyFont
            font.pixelSize: 13
            Layout.alignment: Qt.AlignHCenter
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.topMargin: 8
            spacing: 8

            StyledTextField {
                id: passwordField
                placeholderText: "Enter passphrase"
                echoMode: root.passwordVisible ? TextInput.Normal : TextInput.Password
                enabled: root.lockoutSeconds === 0
                Layout.fillWidth: true
                onAccepted: unlockButton.clicked()
            }

            Rectangle {
                width: 40
                height: 40
                radius: 10
                color: eyeMa.containsMouse ? theme.hoverFill : theme.surfaceAlt
                border.color: theme.border
                border.width: 1
                enabled: root.lockoutSeconds === 0

                Text {
                    anchors.centerIn: parent
                    text: root.passwordVisible ? "🙈" : "👁"
                    font.pixelSize: 16
                    font.family: "Noto Color Emoji"
                }
                MouseArea {
                    id: eyeMa
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: root.passwordVisible = !root.passwordVisible
                }
                ToolTip.visible: eyeMa.containsMouse
                ToolTip.text: root.passwordVisible ? "Hide password" : "Show password"
                ToolTip.delay: 300
            }
        }

        Label {
            id: errorLabel
            text: "Incorrect password."
            color: theme.danger
            font.family: theme.bodyFont
            font.pixelSize: 12
            visible: false
            Layout.alignment: Qt.AlignHCenter
        }

        Label {
            id: lockoutLabel
            text: "Too many failed attempts. Try again in "
                  + root.formatRemaining(root.lockoutSeconds)
            color: theme.danger
            font.family: theme.bodyFont
            font.pixelSize: 12
            visible: root.lockoutSeconds > 0
            Layout.alignment: Qt.AlignHCenter
        }

        PrimaryButton {
            id: unlockButton
            text: root.lockoutSeconds > 0 ? "LOCKED" : "UNLOCK  \u2192"
            enabled: root.lockoutSeconds === 0
            Layout.fillWidth: true
            onClicked: {
                errorLabel.visible = false
                if (authController.tryUnlock(passwordField.text)) {
                    var name = authController.currentUserName()
                    passwordField.text = ""
                    root.passwordVisible = false
                    root.unlocked(name)
                }
            }
        }
    }

    Connections {
        target: authController
        function onUnlockFailed() {
            errorLabel.visible = true
            passwordField.text = ""
            root.passwordVisible = false
            passwordField.forceActiveFocus()
        }
    }
}