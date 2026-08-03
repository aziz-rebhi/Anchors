import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Page {
    id: root

    Theme { id: theme }

    // ---- Data (same as before) ----
    property var allNotes: []
    property string searchText: ""
    property string selectedId: ""
    property bool loadingEditor: false

    // ---- Tree model (same as before) ----
    ListModel {
        id: treeModel
    }

    property var expandedPaths: ({})
    property string currentFolderPath: ""  // for note creation inside folder
    property bool sortByRecent: false

    // ---- Inline folder creation (same) ----
    property bool isCreatingFolder: false
    property string newFolderName: ""

    // ---- Computed visible notes (same) ----
    property var filteredNotes: {
        var list = allNotes.filter(function (n) {
            if (n.title && n.title.startsWith("_")) return false
            if (searchText.length > 0) {
                var q = searchText.toLowerCase()
                if (n.title.toLowerCase().indexOf(q) < 0 && n.content.toLowerCase().indexOf(q) < 0)
                    return false
            }
            return true
        })
        list.sort(function (a, b) { return b.updatedAt - a.updatedAt })
        return list
    }

    // ---- Build tree (same) ----
    function buildTree() {
        treeModel.clear()
        if (allNotes.length === 0) return

        var root = { children: {}, notes: [] }

        function getNode(pathParts) {
            var node = root
            for (var i = 0; i < pathParts.length; i++) {
                var part = pathParts[i]
                if (!node.children[part]) {
                    node.children[part] = { children: {}, notes: [] }
                }
                node = node.children[part]
            }
            return node
        }

        for (var i = 0; i < allNotes.length; i++) {
            var note = allNotes[i]
            if (note.title && note.title.startsWith("_")) continue
            var folder = note.folder || ""
            var parts = folder.length > 0 ? folder.split("/") : []
            var node = getNode(parts)
            node.notes.push(note)
        }

        function computeCounts(node) {
            var count = node.notes.length
            var childNames = Object.keys(node.children)
            for (var i = 0; i < childNames.length; i++) {
                count += computeCounts(node.children[childNames[i]])
            }
            node._count = count
            return count
        }
        computeCounts(root)

        function flatten(node, depth, pathParts) {
            var folderNames = Object.keys(node.children).sort()
            for (var i = 0; i < folderNames.length; i++) {
                var name = folderNames[i]
                var childNode = node.children[name]
                var fullPath = pathParts.concat(name).join("/")
                var isExpanded = (expandedPaths[fullPath] !== undefined) ? expandedPaths[fullPath] : false

                treeModel.append({
                    type: "folder",
                    name: name,
                    depth: depth,
                    expanded: isExpanded,
                    noteId: "",
                    folderPath: fullPath,
                    noteCount: childNode._count
                })

                if (isExpanded) {
                    var notesInFolder = childNode.notes.slice()
                    notesInFolder.sort(root.sortByRecent ? function (a, b) { return b.updatedAt - a.updatedAt } : function (a, b) { return a.title.localeCompare(b.title) })
                    for (var j = 0; j < notesInFolder.length; j++) {
                        var note = notesInFolder[j]
                        treeModel.append({
                            type: "note",
                            name: note.title || "Untitled",
                            depth: depth + 1,
                            expanded: false,
                            noteId: note.id,
                            folderPath: fullPath,
                            noteCount: 0
                        })
                    }
                    flatten(childNode, depth + 1, pathParts.concat(name))
                }
            }
        }

        flatten(root, 0, [])

        var rootNotes = root.notes.slice()
        rootNotes.sort(root.sortByRecent ? function (a, b) { return b.updatedAt - a.updatedAt } : function (a, b) { return a.title.localeCompare(b.title) })
        for (var k = 0; k < rootNotes.length; k++) {
            var rootNote = rootNotes[k]
            treeModel.append({
                type: "note",
                name: rootNote.title || "Untitled",
                depth: 0,
                expanded: false,
                noteId: rootNote.id,
                folderPath: "",
                noteCount: 0
            })
        }
    }

    // ---- Toggle folder (same) ----
    function toggleFolder(fullPath) {
        expandedPaths[fullPath] = !expandedPaths[fullPath]
        buildTree()
    }

    // ---- Create folder (same) ----
    function createFolder(name) {
        if (name.trim().length === 0) {
            isCreatingFolder = false
            newFolderName = ""
            return
        }
        var folderName = name.trim()
        var ok = noteController.addEntryInFolder("New note", "", folderName)
        if (ok) {
            currentFolderPath = folderName
            expandedPaths[folderName] = true
            refresh(false)
            var newNote = allNotes.find(function(n) { return n.folder === folderName && n.title === "New note" })
            if (newNote) {
                selectNote(newNote)
                scrollToNote(newNote.id)
            }
            isCreatingFolder = false
            newFolderName = ""
        } else {
            isCreatingFolder = false
            newFolderName = ""
        }
    }

    // ---- Refresh ----
    function refresh(preserveSelection) {
        allNotes = noteController.entries()
        buildTree()
        if (preserveSelection && selectedId.length > 0) {
            var found = false
            for (var i = 0; i < treeModel.count; i++) {
                if (treeModel.get(i).noteId === selectedId) { found = true; break }
            }
            if (!found) selectedId = ""
        }
        if (selectedId.length === 0 && allNotes.length > 0) {
            if (filteredNotes.length > 0)
                selectNote(filteredNotes[0])
            else if (allNotes.length > 0)
                selectNote(allNotes[0])
        }
        isCreatingFolder = false
        newFolderName = ""
    }

    // ---- Save current note back through noteController ----
    function saveCurrentNote() {
        if (selectedId.length === 0 || !noteEditor || !noteEditor.model) return

        var title = noteEditor.noteTitle
        var model  = noteEditor.model
        var lines  = []
        for (var i = 0; i < model.rowCount(); i++) {
            var idx  = model.index(i, 0)
            var blockData = model.data(idx, 0x0103) // BlockModel::DataRole
            if (blockData && blockData.text !== undefined) {
                lines.push(blockData.text)
            }
        }
        var content = lines.join("\n")

        noteController.updateEntry(selectedId, title, content)
        allNotes = noteController.entries()
            buildTree()
    }

    // ---- Select a note (loads it into the new editor) ----
    function selectNote(note) {
        if (!note) return
        // Save the previous note first
        if (selectedId.length > 0 && noteEditor && noteEditor.model) {
            saveCurrentNote()
        }
        selectedId = note.id
        titleField.loadingEditor = true
        var title = note.title
        var content = note.content || ""
        var paragraphs = content.split(/\n/)
        noteEditor.loadFromContent(title, paragraphs)
        titleField.loadingEditor = false
    }

    // ---- Scroll to a note in the sidebar (same) ----
    function scrollToNote(noteId) {
        Qt.callLater(function () {
            for (var i = 0; i < treeModel.count; i++) {
                var row = treeModel.get(i)
                if (row.type === "note" && row.noteId === noteId) {
                    folderListView.positionViewAtIndex(i, ListView.Contain)
                    return
                }
            }
        })
    }

    // ---- Pagination functions (old, no longer needed but kept to avoid errors) ----
    // We'll keep them as stubs or remove them since we replaced the editor.
    // Actually we can remove all pagination code because we are using BlockList now.
    // But to avoid breaking anything, we'll keep the properties and functions as no-ops.

    // ---- UI ----
    background: Rectangle { color: theme.background }

    RowLayout {
        anchors.fill: parent
        spacing: 0

        // ---- Sidebar (unchanged) ----
        Rectangle {
            Layout.preferredWidth: 280
            Layout.fillHeight: true
            color: theme.surfaceAlt
            border.width: 1
            border.color: theme.border
            Layout.leftMargin: 8
            Layout.rightMargin: 0
            Layout.topMargin: 8
            Layout.bottomMargin: 8
            radius: theme.radiusSmall

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 12
                spacing: 8

                StyledTextField {
                    Layout.fillWidth: true
                    placeholderText: "Search notes..."
                    text: root.searchText
                    onTextChanged: root.searchText = text
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 2

                    Label {
                        id: locationLabel
                        text: root.currentFolderPath.length > 0 ? ("\u2192 " + root.currentFolderPath) : "NOTES"
                        color: root.currentFolderPath.length > 0 ? theme.tertiary : theme.textMuted
                        font.family: theme.labelFont
                        font.pixelSize: 10
                        font.letterSpacing: root.currentFolderPath.length > 0 ? 0 : 1
                        elide: Text.ElideRight
                        Layout.fillWidth: true

                        ToolTip.visible: locationArea.containsMouse && root.currentFolderPath.length > 0
                        ToolTip.text: "New notes go here - click to reset to root"
                        ToolTip.delay: 500

                        MouseArea {
                            id: locationArea
                            anchors.fill: parent
                            hoverEnabled: true
                            visible: root.currentFolderPath.length > 0
                            cursorShape: Qt.PointingHandCursor
                            onClicked: root.currentFolderPath = ""
                        }
                    }

                    SidebarIconButton {
                        glyph: "\u2302" // reset to root
                        tooltipText: "New notes go to root"
                        visible: root.currentFolderPath.length > 0
                        onClicked: root.currentFolderPath = ""
                    }
                    SidebarIconButton {
                        glyph: "\u270E" // new note
                        tooltipText: "New note"
                        onClicked: {
                            var folder = root.currentFolderPath
                            var ok = noteController.addEntryInFolder("Untitled note", "", folder)
                            if (ok) {
                                root.refresh(false)
                                var newNote = root.allNotes.find(function (n) { return n.folder === folder && n.title === "Untitled note" })
                                if (newNote) {
                                    root.selectNote(newNote)
                                    root.scrollToNote(newNote.id)
                                }
                            }
                        }
                    }
                    SidebarIconButton {
                        glyph: "\u2795" // new folder
                        tooltipText: "New folder"
                        onClicked: {
                            root.newFolderName = ""
                            root.isCreatingFolder = true
                        }
                    }
                    SidebarIconButton {
                        glyph: "\u21C5" // sort
                        tooltipText: root.sortByRecent ? "Sorted by recent" : "Sorted by name"
                        onClicked: {
                            root.sortByRecent = !root.sortByRecent
                            root.buildTree()
                        }
                    }
                    SidebarIconButton {
                        glyph: "\u229F" // collapse all
                        tooltipText: "Collapse all"
                        onClicked: {
                            root.expandedPaths = ({})
                            root.buildTree()
                        }
                    }
                }

                ScrollView {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    clip: true

                    ListView {
                        id: folderListView
                        anchors.fill: parent
                        model: treeModel
                        delegate: TreeDelegate {}
                        spacing: 0
                        clip: true

                        // ---- Inline folder creation ----
                        header: Item {
                            id: headerItem
                            width: parent.width
                            height: root.isCreatingFolder ? 26 : 0
                            visible: root.isCreatingFolder
                            clip: true

                            onVisibleChanged: {
                                if (visible && folderNameInput) {
                                    folderNameInput.text = ""
                                    folderNameInput.forceActiveFocus()
                                }
                            }

                            Rectangle {
                                anchors.fill: parent
                                color: theme.surfaceAlt
                                border.width: 1
                                border.color: theme.tertiary
                                radius: 2

                                TextField {
                                    id: folderNameInput
                                    anchors.fill: parent
                                    anchors.margins: 2
                                    color: theme.textPrimary
                                    font.pixelSize: 13
                                    placeholderText: "Folder name..."
                                    placeholderTextColor: theme.textMuted
                                    text: root.newFolderName
                                    background: Rectangle { color: "transparent" }
                                    onAccepted: {
                                        if (text.trim().length > 0) {
                                            root.createFolder(text)
                                        } else {
                                            root.isCreatingFolder = false
                                            root.newFolderName = ""
                                        }
                                    }
                                    Keys.onEscapePressed: {
                                        root.isCreatingFolder = false
                                        root.newFolderName = ""
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        // ---- Editor (new block editor) ----
        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            visible: noteEditor && noteEditor.model !== null

            // Title
            TextField {
                id: titleField
                Layout.fillWidth: true
                Layout.leftMargin: 24
                Layout.rightMargin: 24
                placeholderText: "Title"
                color: theme.textPrimary
                font.family: theme.headlineFont
                font.pixelSize: 26
                font.bold: true
                background: null
                text: noteEditor ? noteEditor.noteTitle : ""
                onTextChanged: {
                    if (!loadingEditor && noteEditor) {
                        noteEditor.noteTitle = text
                        saveTimer.restart()
                    }
                }
                property bool loadingEditor: false
            }

            // Block list
            BlockList {
                id: blockList
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.margins: 12
            }

            Timer {
                id: saveTimer
                interval: 800
                onTriggered: root.saveCurrentNote()
            }
        }

        // ---- Empty state ----
        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            visible: !noteEditor || noteEditor.model === null
            Label {
                Layout.alignment: Qt.AlignCenter
                text: "Select or create a note to get started."
                color: theme.textMuted
                font.family: theme.bodyFont
                font.pixelSize: 13
            }
        }
    }

    // ---- Tree Delegate (unchanged) ----
    component SidebarIconButton : Rectangle {
        id: iconBtn
        property string glyph: ""
        property string tooltipText: ""
        signal clicked()

        width: 24
        height: 24
        radius: 4
        color: iconArea.containsMouse ? Qt.rgba(1, 1, 1, 0.08) : "transparent"

        Text {
            anchors.centerIn: parent
            text: iconBtn.glyph
            color: theme.textSecondary
            font.pixelSize: 13
        }

        MouseArea {
            id: iconArea
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: Qt.PointingHandCursor
            onClicked: iconBtn.clicked()
        }

        ToolTip.visible: iconArea.containsMouse && iconBtn.tooltipText.length > 0
        ToolTip.text: iconBtn.tooltipText
        ToolTip.delay: 500
    }

    component TreeDelegate : Item {
        id: delegateItem
        property var item: (index >= 0 && index < treeModel.count) ? treeModel.get(index) : null

        width: folderListView.width
        height: 26
        property bool hovered: false

        Rectangle {
            anchors.fill: parent
            color: {
                if (item && item.type === "note" && item.noteId === root.selectedId)
                    return Qt.rgba(theme.tertiary.r, theme.tertiary.g, theme.tertiary.b, 0.16)
                if (hovered) return Qt.rgba(1, 1, 1, 0.06)
                return "transparent"
            }
        }

        Rectangle {
            visible: item && item.type === "note" && item.noteId === root.selectedId
            width: 2
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            color: theme.tertiary
        }

        Item {
            anchors.fill: parent
            anchors.leftMargin: 8 + (item ? item.depth * 16 : 0)
            anchors.rightMargin: 8

            Text {
                id: folderArrow
                visible: item && item.type === "folder"
                text: (item && item.expanded) ? "▼ " : "▶ "
                color: theme.textMuted
                font.pixelSize: 10
                anchors.left: parent.left
                anchors.verticalCenter: parent.verticalCenter
            }

            Text {
                id: itemText
                text: {
                    if (!item) return ""
                    if (item.type === "folder") {
                        return item.expanded ? item.name : item.name + " (" + item.noteCount + ")"
                    } else {
                        return item.name || "Untitled"
                    }
                }
                color: {
                    if (!item) return theme.textSecondary
                    if (item.type === "note" && item.noteId === root.selectedId) return theme.tertiary
                    return theme.textSecondary
                }
                font.pixelSize: (item && item.type === "folder") ? 13 : 12
                font.bold: (item && item.type === "folder")
                anchors.left: parent.left
                anchors.leftMargin: (item && item.type === "folder") ? 16 : 0
                anchors.verticalCenter: parent.verticalCenter
                anchors.right: parent.right
                elide: Text.ElideRight
            }

            MouseArea {
                anchors.fill: parent
                hoverEnabled: true
                onEntered: delegateItem.hovered = true
                onExited: delegateItem.hovered = false
                acceptedButtons: Qt.LeftButton | Qt.RightButton

                onClicked: function(mouse) {
                    if (mouse.button === Qt.RightButton) {
                        if (item && item.type === "folder") {
                            folderContextMenu.targetFolder = item.folderPath
                            folderContextMenu.popup()
                        }
                        return
                    }
                    if (item) {
                        if (item.type === "folder") {
                            root.currentFolderPath = item.folderPath
                            root.toggleFolder(item.folderPath)
                        } else {
                            var note = root.allNotes.find(function (n) { return n.id === item.noteId })
                            if (note) {
                                root.selectNote(note)
                                root.currentFolderPath = note.folder || ""
                            }
                        }
                    }
                }
            }
        }
    }

    // ---- Context Menu (unchanged) ----
    Menu {
        id: folderContextMenu
        property string targetFolder: ""
        MenuItem {
            text: "Rename Folder"
            onTriggered: {
                renameFolderDialog.folderName = folderContextMenu.targetFolder
                renameFolderDialog.open()
            }
        }
        MenuItem {
            text: "Delete Folder"
            onTriggered: {
                if (noteController.deleteFolder(folderContextMenu.targetFolder))
                    root.refresh(false)
            }
        }
        MenuItem {
            text: "New Note Here"
            onTriggered: {
                var folder = folderContextMenu.targetFolder
                var ok = noteController.addEntryInFolder("Untitled note", "", folder)
                if (ok) {
                    root.refresh(false)
                    var newNote = root.allNotes.find(function (n) { return n.folder === folder && n.title === "Untitled note" })
                    if (newNote) {
                        root.selectNote(newNote)
                        root.scrollToNote(newNote.id)
                    }
                }
            }
        }
    }

    // ---- Rename Folder Dialog (unchanged) ----
    Dialog {
        id: renameFolderDialog
        property string folderName: ""
        modal: true
        title: "Rename Folder"
        anchors.centerIn: parent
        width: 300
        background: Rectangle {
            color: theme.surfaceAlt
            radius: 12
            border.color: theme.border
        }
        ColumnLayout {
            spacing: 12
            anchors.fill: parent
            anchors.margins: 16
            Label { text: "New name for '" + renameFolderDialog.folderName + "':"; color: theme.textPrimary; font.pixelSize: 14 }
            StyledTextField {
                id: renameFolderNameField
                Layout.fillWidth: true
                placeholderText: "New folder name"
                text: renameFolderDialog.folderName
                onAccepted: renameFolderConfirmButton.clicked()
            }
            RowLayout {
                Layout.fillWidth: true
                spacing: 8
                Button { text: "Cancel"; onClicked: renameFolderDialog.close() }
                PrimaryButton {
                    id: renameFolderConfirmButton
                    text: "Rename"
                    Layout.fillWidth: true
                    onClicked: {
                        var newName = renameFolderNameField.text.trim()
                        if (newName.length > 0 && newName !== renameFolderDialog.folderName) {
                            if (noteController.renameFolder(renameFolderDialog.folderName, newName)) {
                                var oldPath = renameFolderDialog.folderName
                                var newPath = newName
                                if (root.expandedPaths[oldPath] !== undefined) {
                                    root.expandedPaths[newPath] = root.expandedPaths[oldPath]
                                    delete root.expandedPaths[oldPath]
                                }
                                if (root.currentFolderPath === oldPath) {
                                    root.currentFolderPath = newPath
                                }
                                root.refresh(false)
                                renameFolderDialog.close()
                            }
                        }
                    }
                }
            }
        }
    }
    Component.onCompleted: refresh(false)


    // ---- Removed pagination components ----
}