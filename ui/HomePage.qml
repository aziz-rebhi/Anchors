import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Page {
    id: root

    Theme { id: theme }

    property string activeKey: {
        if (typeof settingsController !== "undefined" && settingsController.startPage)
            return settingsController.startPage
        return "dashboard"
    }

    property bool sidebarCollapsed: true

    readonly property int sidebarExpandedWidth: 220
    readonly property int sidebarCollapsedWidth: 72

    readonly property string iconSuffix: theme.isDark ? "white" : "black"

    readonly property var navItems: [
        { label: "Dashboard", iconSource: "qrc:/icons/icons/dashboard-" + iconSuffix + ".svg",   key: "dashboard" },
        { label: "Vault",     iconSource: "qrc:/icons/icons/shield-lock-" + iconSuffix + ".svg", key: "vault" },
        { label: "Notes",     iconSource: "qrc:/icons/icons/note-" + iconSuffix + ".svg",        key: "notes" },
        { label: "Calendar",  iconSource: "qrc:/icons/icons/calendar-" + iconSuffix + ".svg",    key: "calendar" },
        { label: "To-Do",     iconSource: "qrc:/icons/icons/todo-" + iconSuffix + ".svg",        key: "todo" }
    ]

    background: Rectangle { color: theme.background }

    RowLayout {
        anchors.fill: parent
        spacing: 0

        Rectangle {
            id: sidebar
            Layout.preferredWidth: root.sidebarCollapsed
                                   ? root.sidebarCollapsedWidth
                                   : root.sidebarExpandedWidth
            Layout.fillHeight: true
            color: theme.surface
            clip: true

            MouseArea {
                anchors.fill: parent
                z: -1
                onClicked: root.sidebarCollapsed = !root.sidebarCollapsed
            }

            Rectangle {
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                anchors.right: parent.right
                width: 1
                color: theme.border
                opacity: 0.55
            }

            Behavior on Layout.preferredWidth {
                NumberAnimation { duration: 220; easing.type: Easing.OutCubic }
            }

            ColumnLayout {
                anchors.fill: parent
                anchors.topMargin: 16
                anchors.bottomMargin: 12
                anchors.leftMargin: 10
                anchors.rightMargin: 10
                spacing: 2

                Item {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 44
                    Layout.bottomMargin: 14

                    MouseArea {
                        anchors.fill: parent
                        onClicked: {}
                    }

                    Image {
                        id: logo
                        source: "qrc:/qml/logo.png"
                        width: 28
                        height: 28
                        fillMode: Image.PreserveAspectFit
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.left: root.sidebarCollapsed ? undefined : parent.left
                        anchors.leftMargin: 6
                        anchors.horizontalCenter: root.sidebarCollapsed ? parent.horizontalCenter : undefined
                    }

                    Column {
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.left: logo.right
                        anchors.leftMargin: 10
                        spacing: 1
                        visible: !root.sidebarCollapsed

                        Label {
                            text: "Anchor"
                            color: theme.textPrimary
                            font.family: theme.headlineFont
                            font.bold: true
                            font.pixelSize: 14
                        }
                        Label {
                            text: "Productivity"
                            color: theme.textMuted
                            font.family: theme.bodyFont
                            font.pixelSize: 10
                        }
                    }

                    Item {
                        id: collapseTop
                        width: 28
                        height: 28
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.right: parent.right
                        visible: !root.sidebarCollapsed
                        property bool hovered: false

                        Rectangle {
                            anchors.fill: parent
                            radius: 8
                            color: collapseTop.hovered
                                   ? (theme.isDark ? Qt.rgba(1,1,1,0.08) : Qt.rgba(0,0,0,0.06))
                                   : "transparent"
                        }

                        Image {
                            source: "qrc:/icons/icons/double-arrow-left-" + root.iconSuffix + ".svg"
                            width: 16
                            height: 16
                            anchors.centerIn: parent
                            fillMode: Image.PreserveAspectFit
                            sourceSize: Qt.size(32, 32)
                        }

                        MouseArea {
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onEntered: collapseTop.hovered = true
                            onExited: collapseTop.hovered = false
                            onClicked: root.sidebarCollapsed = true
                        }
                    }
                }

                Repeater {
                    model: root.navItems

                    delegate: Item {
                        id: navItem
                        Layout.fillWidth: true
                        Layout.preferredHeight: 42
                        property bool hovered: false
                        property bool active: modelData.key === root.activeKey

                        Rectangle {
                            anchors.fill: parent
                            radius: 10
                            color: navItem.active
                                   ? Qt.rgba(theme.tertiary.r, theme.tertiary.g, theme.tertiary.b, 0.16)
                                   : navItem.hovered
                                     ? (theme.isDark ? Qt.rgba(1,1,1,0.05) : Qt.rgba(0,0,0,0.04))
                                     : "transparent"
                            Behavior on color { ColorAnimation { duration: 100 } }

                            Rectangle {
                                visible: navItem.active
                                width: 3
                                height: 16
                                radius: 1.5
                                color: theme.tertiary
                                anchors.left: parent.left
                                anchors.leftMargin: 3
                                anchors.verticalCenter: parent.verticalCenter
                            }
                        }

                        Image {
                            id: navIcon
                            source: modelData.iconSource
                            width: 22
                            height: 22
                            fillMode: Image.PreserveAspectFit
                            sourceSize.width: 44
                            sourceSize.height: 44
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.left: root.sidebarCollapsed ? undefined : parent.left
                            anchors.leftMargin: root.sidebarCollapsed ? 0 : 14
                            anchors.horizontalCenter: root.sidebarCollapsed ? parent.horizontalCenter : undefined
                            opacity: navItem.active ? 1.0 : 0.75
                        }

                        Label {
                            text: modelData.label
                            color: navItem.active ? theme.tertiary : theme.textSecondary
                            font.family: theme.bodyFont
                            font.pixelSize: 13
                            font.weight: navItem.active ? Font.DemiBold : Font.Normal
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.left: navIcon.right
                            anchors.leftMargin: 12
                            visible: opacity > 0
                            opacity: root.sidebarCollapsed ? 0 : 1
                            Behavior on opacity { NumberAnimation { duration: 100 } }
                        }

                        ToolTip {
                            visible: root.sidebarCollapsed && navItem.hovered
                            text: modelData.label
                            delay: 300
                        }

                        MouseArea {
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onEntered: navItem.hovered = true
                            onExited: navItem.hovered = false
                            onClicked: root.activeKey = modelData.key
                        }
                    }
                }

                Item { Layout.fillHeight: true }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.bottomMargin: 4
                    height: 1
                    color: theme.border
                    opacity: 0.45
                    visible: !root.sidebarCollapsed
                }

                SidebarBtn {
                    iconSource: "qrc:/icons/icons/settings-" + root.iconSuffix + ".svg"
                    label: "Settings"
                    collapsed: root.sidebarCollapsed
                    onClicked: root.activeKey = "settings"
                }

                SidebarBtn {
                    iconSource: "qrc:/icons/icons/lock-" + root.iconSuffix + ".svg"
                    label: "Lock now"
                    collapsed: root.sidebarCollapsed
                    onClicked: session.lock()
                }
            }
        }

        Loader {
            Layout.fillWidth: true
            Layout.fillHeight: true
            sourceComponent: {
                switch (root.activeKey) {
                case "dashboard": return dashboardComponent
                case "vault":     return vaultComponent
                case "notes":     return notesComponent
                case "calendar":  return calendarComponent
                case "todo":      return todoComponent
                case "settings":  return settingsComponent
                default:          return dashboardComponent
                }
            }
        }
    }

    component SidebarBtn : Item {
        id: btn
        property string iconSource: ""
        property string label: ""
        property bool collapsed: false
        signal clicked()

        Layout.fillWidth: true
        Layout.preferredHeight: 38
        property bool hovered: false

        Rectangle {
            anchors.fill: parent
            radius: 10
            color: btn.hovered
                   ? (theme.isDark ? Qt.rgba(1,1,1,0.05) : Qt.rgba(0,0,0,0.04))
                   : "transparent"
        }

        Image {
            id: btnIcon
            source: btn.iconSource
            width: 18
            height: 18
            fillMode: Image.PreserveAspectFit
            sourceSize.width: 36
            sourceSize.height: 36
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: btn.collapsed ? undefined : parent.left
            anchors.leftMargin: btn.collapsed ? 0 : 14
            anchors.horizontalCenter: btn.collapsed ? parent.horizontalCenter : undefined
            opacity: btn.hovered ? 1.0 : 0.75
        }

        Label {
            text: btn.label
            color: btn.hovered ? theme.textPrimary : theme.textSecondary
            font.family: theme.bodyFont
            font.pixelSize: 12
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: btnIcon.right
            anchors.leftMargin: 12
            visible: opacity > 0
            opacity: btn.collapsed ? 0 : 1
            Behavior on opacity { NumberAnimation { duration: 100 } }
        }

        ToolTip {
            visible: btn.collapsed && btn.hovered
            text: btn.label
            delay: 300
        }

        MouseArea {
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: Qt.PointingHandCursor
            onEntered: btn.hovered = true
            onExited: btn.hovered = false
            onClicked: btn.clicked()
        }
    }

    Component {
        id: dashboardComponent
        DashboardPage {
            onNavigateRequested: function (pageName) { root.activeKey = pageName }
        }
    }
    Component { id: vaultComponent;    VaultPage {} }
    Component { id: notesComponent;    NotesPage {} }
    Component { id: calendarComponent; CalendarPage {} }
    Component { id: todoComponent;     TodoPage {} }
    Component {
        id: settingsComponent
        SettingsPage {
            onLockRequested: session.lock()
        }
    }
    Component.onCompleted: {
        if (typeof settingsController !== "undefined" && settingsController.startPage)
            activeKey = settingsController.startPage
    }
}