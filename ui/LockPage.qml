import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Page {
    id: root

    Theme { id: theme }

    // Emitted once the password is verified and the Session is unlocked.
    signal unlocked(string name)
    // Not wired to anything yet - the mockup shows it, but the README
    // states there's deliberately no recovery mechanism if you lose your
    // master password. Flagging that conflict rather than guessing at
    // what this should do; happy to wire it up once you tell me the
    // intent (e.g. a "quit app" action, or something else entirely).
    signal emergencyOverrideRequested()

    background: Rectangle { color: theme.background }

    ColumnLayout {
        anchors.centerIn: parent
        spacing: 20
        width: Math.min(parent.width * 0.8, 340)

        // --- glow + padlock icon ---
        Item {
            Layout.alignment: Qt.AlignHCenter
            width: 96
            height: 96

            // Soft radial glow, faked with stacked translucent circles
            // (no GraphicalEffects dependency needed).
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
                        // masks the bottom half of the ring above so only
                        // the shackle's top arc is visible
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

        StyledTextField {
            id: passwordField
            placeholderText: "Enter passphrase"
            echoMode: TextInput.Password
            Layout.fillWidth: true
            Layout.topMargin: 8
            onAccepted: unlockButton.clicked()
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

        PrimaryButton {
            id: unlockButton
            text: "UNLOCK  \u2192"
            Layout.fillWidth: true
            onClicked: {
                errorLabel.visible = false
                if (authController.tryUnlock(passwordField.text)) {
                    var name = authController.currentUserName()
                    passwordField.text = ""
                    root.unlocked(name)
                }
            }
        }

        Label {
            text: "EMERGENCY OVERRIDE"
            color: theme.textMuted
            font.family: theme.labelFont
            font.pixelSize: 10
            font.letterSpacing: 1.5
            Layout.alignment: Qt.AlignHCenter
            Layout.topMargin: 8

            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor
                onClicked: root.emergencyOverrideRequested()
            }
        }
    }

    Connections {
        target: authController
        function onUnlockFailed() {
            errorLabel.visible = true
            passwordField.text = ""
            passwordField.forceActiveFocus()
        }
    }
}
