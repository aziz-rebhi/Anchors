import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Popup {
    id: menu

    // Set by the parent block when '/' is typed
    property string blockId: ""
    property string filterText: ""
    property var onSelectBlock: null  // function(blockId, typeCode) called when user picks a type

    // Position relative to the TextArea cursor
    property real cursorX: 0
    property real cursorY: 0

    x: cursorX - 10
    y: cursorY + 24  // below the cursor line
    width: 280
    height: Math.min(listView.contentHeight + 16, 320)
    padding: 8

    modal: false
    focus: false
    closePolicy: Popup.NoAutoClose

    background: Rectangle {
        color: "#2a2a3a"
        border.color: "#555555"
        border.width: 1
        radius: 8
    }

    // Block type definitions for the menu
    property var blockTypes: [
        { type: 0,  name: "Text",        desc: "Plain text block",           icon: "T"  },
        { type: 1,  name: "Heading 1",   desc: "Large section heading",       icon: "H1" },
        { type: 2,  name: "Heading 2",   desc: "Medium section heading",     icon: "H2" },
        { type: 3,  name: "Heading 3",   desc: "Small section heading",      icon: "H3" },
        { type: 4,  name: "To-do",       desc: "Checkbox with text",         icon: "\u2611" },
        { type: 5,  name: "Code",        desc: "Code snippet block",         icon: "</>" },
        { type: 8,  name: "Divider",     desc: "Horizontal / Vertical divider", icon: "---" },
        { type: 9,  name: "Quote",       desc: "Block quote",                icon: "\u201C" },
        { type: 6,  name: "Image",       desc: "Embed an image",             icon: "\u25A3" },
        { type: 7,  name: "Table",       desc: "Simple table",               icon: "\u2637" }
    ]

    // Filtered list
    property var filteredTypes: {
        if (!filterText || filterText.length === 0) {
            return blockTypes
        }
        var q = filterText.toLowerCase()
        return blockTypes.filter(function(item) {
            return item.name.toLowerCase().indexOf(q) >= 0 ||
                   item.desc.toLowerCase().indexOf(q) >= 0
        })
    }

    // --- Public API called from ParagraphBlock's Keys.onPressed ---
    function moveUp() {
        listView.currentIndex = Math.max(0, listView.currentIndex - 1)
        listView.positionViewAtIndex(listView.currentIndex, ListView.Contain)
    }

    function moveDown() {
        listView.currentIndex = Math.min(listView.count - 1, listView.currentIndex + 1)
        listView.positionViewAtIndex(listView.currentIndex, ListView.Contain)
    }

    function selectCurrent() {
        if (listView.currentIndex >= 0 && listView.currentIndex < filteredTypes.length) {
            var selected = filteredTypes[listView.currentIndex]
            if (menu.onSelectBlock) {
                menu.onSelectBlock(menu.blockId, selected.type)
            }
        }
        menu.close()
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 4

        // Search hint
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 20
            color: "transparent"

            Text {
                anchors.left: parent.left
                anchors.verticalCenter: parent.verticalCenter
                text: "Filter..."
                font.pixelSize: 11
                color: "#888888"
            }
        }

        // Separator
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 1
            color: "#444444"
        }

        // Block type list
        ListView {
            id: listView
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            model: menu.filteredTypes
            currentIndex: 0
            highlightMoveDuration: 100

            highlight: Rectangle {
                color: "#3a3a4a"
                radius: 4
            }

            delegate: ItemDelegate {
                id: delegate
                width: listView.width
                height: 44

                background: Rectangle {
                    color: delegate.hovered || listView.currentIndex === index ? "#3a3a4a" : "transparent"
                    radius: 4
                }

                contentItem: RowLayout {
                    spacing: 12

                    // Icon area
                    Rectangle {
                        Layout.preferredWidth: 40
                        Layout.preferredHeight: 32
                        color: "#333344"
                        radius: 4

                        Text {
                            anchors.centerIn: parent
                            text: modelData.icon
                            font.pixelSize: 12
                            font.bold: true
                            color: "#aaaaaa"
                        }
                    }

                    // Name + description
                    Column {
                        spacing: 2
                        Layout.fillWidth: true

                        Text {
                            text: modelData.name
                            font.pixelSize: 13
                            font.weight: Font.Medium
                            color: "#ffffff"
                        }

                        Text {
                            text: modelData.desc
                            font.pixelSize: 11
                            color: "#888888"
                        }
                    }
                }

                onClicked: {
                    if (menu.onSelectBlock) {
                        menu.onSelectBlock(menu.blockId, modelData.type)
                    }
                    menu.close()
                }
            }
        }

        // Bottom hint
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 20
            color: "transparent"

            Row {
                anchors.left: parent.left
                anchors.verticalCenter: parent.verticalCenter
                spacing: 12

                Text {
                    text: "\u2191\u2193 navigate"
                    font.pixelSize: 10
                    color: "#666666"
                }
                Text {
                    text: "Enter select"
                    font.pixelSize: 10
                    color: "#666666"
                }
                Text {
                    text: "Esc dismiss"
                    font.pixelSize: 10
                    color: "#666666"
                }
            }
        }
    }
}