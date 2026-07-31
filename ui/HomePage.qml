import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Page {
    id: root

    Theme { id: theme }

    property string activeKey: "dashboard"
    readonly property var navItems: [
        { label: "Dashboard", glyph: "\u25A6", key: "dashboard" },
        { label: "Vault",     glyph: "\u25C6", key: "vault" },
        { label: "Notes",     glyph: "\u2261", key: "notes" },
        { label: "Calendar",  glyph: "\u25A1", key: "calendar" },
        { label: "To-Do",     glyph: "\u2713", key: "todo" }
    ]

    background: Rectangle { color: theme.background }

    RowLayout {
        anchors.fill: parent
        spacing: 0

        // --- Sidebar ---
        Rectangle {
            Layout.preferredWidth: 208
            Layout.fillHeight: true
            color: theme.surface

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 16
                spacing: 4

                RowLayout {
                    spacing: 10
                    Layout.bottomMargin: 20

                    Image {
                        source: "logo.png"
                        Layout.preferredWidth: 32
                        Layout.preferredHeight: 32 * (sourceSize.height / Math.max(sourceSize.width, 1))
                        fillMode: Image.PreserveAspectFit
                    }

                    ColumnLayout {
                        spacing: 0
                        Label {
                            text: "Anchor"
                            color: theme.textPrimary
                            font.family: theme.headlineFont
                            font.bold: true
                            font.pixelSize: 15
                        }
                        Label {
                            text: "Productivity Suite"
                            color: theme.textMuted
                            font.family: theme.bodyFont
                            font.pixelSize: 10
                        }
                    }
                }

                Repeater {
                    model: root.navItems
                    delegate: Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 36
                        radius: theme.radiusSmall
                        color: modelData.key === root.activeKey
                               ? Qt.rgba(theme.tertiary.r, theme.tertiary.g, theme.tertiary.b, 0.16)
                               : "transparent"

                        Rectangle {
                            visible: modelData.key === root.activeKey
                            width: 3
                            height: parent.height - 12
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.left: parent.left
                            radius: 2
                            color: theme.tertiary
                        }

                        RowLayout {
                            anchors.fill: parent
                            anchors.leftMargin: 14
                            spacing: 10

                            Label {
                                text: modelData.glyph
                                color: modelData.key === root.activeKey ? theme.tertiary : theme.textSecondary
                                font.pixelSize: 14
                            }
                            Label {
                                text: modelData.label
                                color: modelData.key === root.activeKey ? theme.tertiary : theme.textSecondary
                                font.family: theme.bodyFont
                                font.pixelSize: 13
                            }
                        }

                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            onClicked: root.activeKey = modelData.key
                        }
                    }
                }

                Item { Layout.fillHeight: true }

                Rectangle {
                    Layout.fillWidth: true
                    height: 1
                    color: theme.border
                    Layout.bottomMargin: 8
                }

                Label {
                    text: "Settings"
                    color: theme.textSecondary
                    font.family: theme.bodyFont
                    font.pixelSize: 12
                    Layout.bottomMargin: 4
                }
                Label {
                    text: "Lock now"
                    color: theme.textSecondary
                    font.family: theme.bodyFont
                    font.pixelSize: 12

                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: session.lock()
                    }
                }
            }
        }

        // --- Content ---
        Loader {
            id: contentLoader
            Layout.fillWidth: true
            Layout.fillHeight: true

            sourceComponent: {
                switch (root.activeKey) {
                case "dashboard": return dashboardComponent
                case "vault": return vaultComponent
                case "notes": return notesComponent
                case "calendar": return calendarComponent
                case "todo": return todoComponent
                default: return dashboardComponent
                }
            }
        }
    }

    Component {
        id: dashboardComponent
        DashboardPage {
            onNavigateRequested: function (pageName) { root.activeKey = pageName }
        }
    }
    Component { id: vaultComponent; VaultPage {} }
    Component { id: notesComponent; NotesPage {} }
    Component { id: calendarComponent; CalendarPage {} }
    Component { id: todoComponent; TodoPage {} }
}
