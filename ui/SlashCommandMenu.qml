import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Popup {
    id: menu

    property string blockId: ""
    property string filterText: ""
    property real cursorX: 0
    property real cursorY: 0

    signal blockSelected(string blockId, int typeCode)

    x: cursorX
    y: cursorY
    width: 300
    height: Math.min(listColumn.implicitHeight + 16, 360)
    padding: 8
    modal: false
    focus: false
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

    background: Rectangle {
        color: "#1e1e28"
        border.color: "#3a3a4a"
        border.width: 1
        radius: 10
    }

    readonly property var allItems: [
        { section: "Basic blocks", type: 0,  name: "Text",          shortcut: "",     icon: "T",   keywords: "text paragraph plain" },
        { section: "Basic blocks", type: 1,  name: "Heading 1",     shortcut: "#",    icon: "H1",  keywords: "heading h1 title" },
        { section: "Basic blocks", type: 2,  name: "Heading 2",     shortcut: "##",   icon: "H2",  keywords: "heading h2" },
        { section: "Basic blocks", type: 3,  name: "Heading 3",     shortcut: "###",  icon: "H3",  keywords: "heading h3" },
        { section: "Basic blocks", type: 10, name: "Heading 4",     shortcut: "####", icon: "H4",  keywords: "heading h4" },
        { section: "Basic blocks", type: 11, name: "Bulleted list", shortcut: "-",    icon: "•",   keywords: "bullet list unordered" },
        { section: "Basic blocks", type: 4,  name: "To-do list",    shortcut: "[]",   icon: "☑",   keywords: "todo task check" },
        { section: "Basic blocks", type: 9,  name: "Quote",         shortcut: "\"",   icon: "“",   keywords: "quote cite" },
        { section: "Basic blocks", type: 12, name: "Callout",       shortcut: "",     icon: "💡",  keywords: "callout note tip" },
        { section: "Basic blocks", type: 8,  name: "Divider",       shortcut: "---",  icon: "—",   keywords: "divider line hr" },
        { section: "Basic blocks", type: 13, name: "Numbered list", shortcut: "1.",   icon: "1.",  keywords: "numbered ordered list" },
        { section: "Basic blocks", type: 15, name: "Toggle list",   shortcut: ">",    icon: "▶",   keywords: "toggle collapse" },
        { section: "Media",        type: 6,  name: "Image",         shortcut: "",     icon: "🖼",  keywords: "image photo picture" },
        { section: "Media",        type: 5,  name: "Code",          shortcut: "```",  icon: "</>", keywords: "code snippet" },
        { section: "Media",        type: 7,  name: "Table",         shortcut: "",     icon: "▦",  keywords: "table grid" },
        { section: "Advanced",     type: 14, name: "Equation",      shortcut: "$$",   icon: "∑",   keywords: "math latex equation formula" },
        { section: "Layout",       type: 16, name: "Columns",       shortcut: "",     icon: "║║",  keywords: "split columns" }
    ]

    property var filteredItems: {
        var q = (filterText || "").trim().toLowerCase()
        if (!q.length)
            return allItems
        return allItems.filter(function (it) {
            return it.name.toLowerCase().indexOf(q) >= 0
                || it.keywords.indexOf(q) >= 0
                || it.shortcut.indexOf(q) >= 0
        })
    }

    property var displayModel: {
        var rows = []
        var lastSection = ""
        var items = filteredItems
        for (var i = 0; i < items.length; i++) {
            var it = items[i]
            if (it.section !== lastSection) {
                rows.push({ kind: "header", title: it.section })
                lastSection = it.section
            }
            rows.push({
                kind: "item",
                type: it.type,
                name: it.name,
                shortcut: it.shortcut,
                icon: it.icon,
                itemIndex: i
            })
        }
        return rows
    }

    property int selectedItemIndex: 0

    function moveUp() {
        if (filteredItems.length === 0) return
        selectedItemIndex = (selectedItemIndex - 1 + filteredItems.length) % filteredItems.length
        listView.positionViewAtIndex(indexOfSelectedRow(), ListView.Contain)
    }

    function moveDown() {
        if (filteredItems.length === 0) return
        selectedItemIndex = (selectedItemIndex + 1) % filteredItems.length
        listView.positionViewAtIndex(indexOfSelectedRow(), ListView.Contain)
    }

    function indexOfSelectedRow() {
        for (var i = 0; i < displayModel.length; i++) {
            if (displayModel[i].kind === "item" && displayModel[i].itemIndex === selectedItemIndex)
                return i
        }
        return 0
    }

    function selectCurrent() {
        if (selectedItemIndex < 0 || selectedItemIndex >= filteredItems.length)
            return
        var it = filteredItems[selectedItemIndex]
        menu.blockSelected(menu.blockId, it.type)
        menu.close()
    }

    onFilterTextChanged: selectedItemIndex = 0
    onOpened: selectedItemIndex = 0

    ColumnLayout {
        id: listColumn
        anchors.fill: parent
        spacing: 4

        Text {
            Layout.fillWidth: true
            Layout.leftMargin: 6
            text: filterText.length ? ("/" + filterText) : "Type to filter..."
            color: "#777777"
            font.pixelSize: 11
        }

        Rectangle {
            Layout.fillWidth: true
            height: 1
            color: "#333333"
        }

        ListView {
            id: listView
            Layout.fillWidth: true
            Layout.preferredHeight: Math.min(contentHeight, 280)
            clip: true
            model: menu.displayModel
            spacing: 1
            currentIndex: menu.indexOfSelectedRow()

            delegate: Item {
                width: listView.width
                height: modelData.kind === "header" ? 22 : 36

                Text {
                    visible: modelData.kind === "header"
                    anchors.left: parent.left
                    anchors.leftMargin: 8
                    anchors.verticalCenter: parent.verticalCenter
                    text: modelData.title || ""
                    color: "#666666"
                    font.pixelSize: 10
                    font.bold: true
                }

                Rectangle {
                    visible: modelData.kind === "item"
                    anchors.fill: parent
                    radius: 6
                    color: (modelData.itemIndex === menu.selectedItemIndex) ? "#2f2f3d" : "transparent"

                    RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: 8
                        anchors.rightMargin: 8
                        spacing: 10

                        Rectangle {
                            Layout.preferredWidth: 28
                            Layout.preferredHeight: 28
                            radius: 6
                            color: "#2a2a36"
                            Text {
                                anchors.centerIn: parent
                                text: modelData.icon || ""
                                color: "#cccccc"
                                font.pixelSize: 12
                            }
                        }

                        Text {
                            Layout.fillWidth: true
                            text: modelData.name || ""
                            color: "#eeeeee"
                            font.pixelSize: 13
                            elide: Text.ElideRight
                        }

                        Text {
                            text: modelData.shortcut || ""
                            color: "#666666"
                            font.pixelSize: 11
                            visible: (modelData.shortcut || "").length > 0
                        }
                    }

                    MouseArea {
                        anchors.fill: parent
                        hoverEnabled: true
                        onEntered: menu.selectedItemIndex = modelData.itemIndex
                        onClicked: {
                            menu.blockSelected(menu.blockId, modelData.type)
                            menu.close()
                        }
                    }
                }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            height: 1
            color: "#333333"
        }

        Text {
            Layout.fillWidth: true
            Layout.leftMargin: 6
            text: "Close menu  ·  esc"
            color: "#555555"
            font.pixelSize: 10
        }
    }
}