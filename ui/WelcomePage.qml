import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Page {
    id: root

    Theme { id: theme }

    property string userName: ""

    signal continueRequested()

    background: Rectangle { color: theme.background }

    ColumnLayout {
        anchors.centerIn: parent
        spacing: 14

        Item {
            Layout.alignment: Qt.AlignHCenter
            width: 140
            height: 140

            Repeater {
                model: 3
                Rectangle {
                    readonly property int step: index
                    anchors.centerIn: parent
                    width: 140 + step * 30
                    height: width
                    radius: width / 2
                    color: theme.tertiary
                    opacity: 0.08 - step * 0.025
                }
            }

            Image {
                id: logo
                anchors.centerIn: parent
                width: 120
                height: 120 * (sourceSize.height / Math.max(sourceSize.width, 1))
                source: "logo.png"
                fillMode: Image.PreserveAspectFit
            }
        }

        Label {
            text: userName.length > 0 ? "Welcome, " + userName : "Welcome to Anchor"
            color: theme.textPrimary
            font.family: theme.headlineFont
            font.pixelSize: 26
            font.bold: true
            Layout.alignment: Qt.AlignHCenter
        }

        Label {
            text: "Your vault is unlocked and ready."
            color: theme.textSecondary
            font.family: theme.bodyFont
            font.pixelSize: 13
            Layout.alignment: Qt.AlignHCenter
        }

        Label {
            text: "Tap anywhere to continue"
            color: theme.textMuted
            font.family: theme.labelFont
            font.pixelSize: 11
            font.letterSpacing: 1
            Layout.alignment: Qt.AlignHCenter
            Layout.topMargin: 24
        }
    }

    MouseArea {
        anchors.fill: parent
        onClicked: root.continueRequested()
    }

    Timer {
        interval: 1800
        running: true
        onTriggered: root.continueRequested()
    }
}
