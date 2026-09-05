import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Page {
    id: root
    Theme { id: theme }

    property var allNotes: []
    property string searchText: ""
    property string selectedId: ""
    property var expandedPaths: ({})
    property string currentFolderPath: ""
    property bool sortByRecent: false
    property bool isCreatingFolder: false
    property string newFolderName: ""

    ListModel { id: treeModel }

    property var filteredNotes: {
        var list = allNotes.filter(function (n) {
            if (n.title && n.title.startsWith("_")) return false
            if (searchText.length > 0) {
                var q = searchText.toLowerCase()
                if ((n.title || "").toLowerCase().indexOf(q) < 0 &&
                    (n.content || "").toLowerCase().indexOf(q) < 0)
                    return false
            }
            return true
        })
        list.sort(function (a, b) { return (b.updatedAt || 0) - (a.updatedAt || 0) })
        return list
    }

    function buildTree() {
        treeModel.clear()
        if (allNotes.length === 0) return
        var rootNode = { children: {}, notes: [] }

        function getNode(pathParts) {
            var node = rootNode
            for (var i = 0; i < pathParts.length; i++) {
                var part = pathParts[i]
                if (!node.children[part])
                    node.children[part] = { children: {}, notes: [] }
                node = node.children[part]
            }
            return node
        }

        for (var i = 0; i < allNotes.length; i++) {
            var note = allNotes[i]
            if (note.title && note.title.startsWith("_")) continue
            var folder = note.folder || ""
            var parts = folder.length > 0 ? folder.split("/") : []
            getNode(parts).notes.push(note)
        }

        function computeCounts(node) {
            var count = node.notes.length
            var names = Object.keys(node.children)
            for (var i = 0; i < names.length; i++)
                count += computeCounts(node.children[names[i]])
            node._count = count
            return count
        }
        computeCounts(rootNode)

        function flatten(node, depth, pathParts) {
            var folderNames = Object.keys(node.children).sort()
            for (var i = 0; i < folderNames.length; i++) {
                var name = folderNames[i]
                var child = node.children[name]
                var fullPath = pathParts.concat(name).join("/")
                var isExpanded = expandedPaths[fullPath] === true
                treeModel.append({
                    type: "folder", name: name, depth: depth, expanded: isExpanded,
                    noteId: "", folderPath: fullPath, noteCount: child._count
                })
                if (isExpanded) {
                    var notesInFolder = child.notes.slice()
                    notesInFolder.sort(root.sortByRecent
                        ? function (a, b) { return (b.updatedAt || 0) - (a.updatedAt || 0) }
                        : function (a, b) { return (a.title || "").localeCompare(b.title || "") })
                    for (var j = 0; j < notesInFolder.length; j++) {
                        var n = notesInFolder[j]
                        treeModel.append({
                            type: "note", name: n.title || "Untitled", depth: depth + 1,
                            expanded: false, noteId: n.id, folderPath: fullPath, noteCount: 0
                        })
                    }
                    flatten(child, depth + 1, pathParts.concat(name))
                }
            }
        }
        flatten(rootNode, 0, [])

        var rootNotes = rootNode.notes.slice()
        rootNotes.sort(root.sortByRecent
            ? function (a, b) { return (b.updatedAt || 0) - (a.updatedAt || 0) }
            : function (a, b) { return (a.title || "").localeCompare(b.title || "") })
        for (var k = 0; k < rootNotes.length; k++) {
            var rn = rootNotes[k]
            treeModel.append({
                type: "note", name: rn.title || "Untitled", depth: 0, expanded: false,
                noteId: rn.id, folderPath: "", noteCount: 0
            })
        }
    }

    function toggleFolder(fullPath) {
        var ep = Object.assign({}, expandedPaths)
        ep[fullPath] = !ep[fullPath]
        expandedPaths = ep
        buildTree()
    }

    function selectNoteById(noteId) {
        if (!noteId || !noteId.length)
            return
        var note = root.allNotes.find(function (n) { return n.id === noteId })
        if (!note)
            return
        root.selectNote(note)
        root.scrollToNote(noteId)
    }

    function createFolder(name) {
        if (!name || name.trim().length === 0) {
            isCreatingFolder = false
            newFolderName = ""
            return
        }
        var leaf = name.trim()
        var folderPath = currentFolderPath.length > 0
            ? (currentFolderPath + "/" + leaf)
            : leaf

        // One empty note so the folder path exists in storage
        var newId = noteController.addEntryInFolder("Untitled note", "", folderPath)
        isCreatingFolder = false
        newFolderName = ""
        if (!newId || !newId.length)
            return

        currentFolderPath = folderPath
        var ep = Object.assign({}, expandedPaths)
        var parts = folderPath.split("/")
        var acc = ""
        for (var i = 0; i < parts.length; i++) {
            acc = (i === 0) ? parts[i] : (acc + "/" + parts[i])
            ep[acc] = true
        }
        expandedPaths = ep

        // entriesChanged already refreshed; pick up the new note after tree rebuild
        Qt.callLater(function () {
            root.refresh(false)
            root.selectNoteById(newId)
        })
    }

    function refresh(preserveSelection) {
        allNotes = noteController.entries()
        buildTree()
        if (preserveSelection && selectedId.length > 0) {
            var found = false
            for (var i = 0; i < treeModel.count; i++) {
                if (treeModel.get(i).noteId === selectedId) { found = true; break }
            }
            if (!found) {
                selectedId = ""
                if (noteEditor) noteEditor.loadFromJson("", "")
            }
        }
        isCreatingFolder = false
        newFolderName = ""
    }

    function saveCurrentNote() {
        if (selectedId.length === 0 || !noteEditor || !noteEditor.model) return
        noteController.updateEntry(selectedId, noteEditor.noteTitle, noteEditor.documentToJson())
        allNotes = noteController.entries()
        buildTree()
    }

    function selectNote(note) {
        if (!note) return
        if (selectedId.length > 0 && noteEditor && noteEditor.model)
            saveCurrentNote()
        selectedId = note.id
        titleField.loadingEditor = true
        var content = note.content || ""
        if (content.length === 0 || content === "{}") {
            noteEditor.loadFromContent(note.title || "Untitled note", [""])
        } else {
            noteEditor.loadFromJson(note.title || "", content)
        }
        titleField.loadingEditor = false
    }

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

    function clearSelection() {
        if (selectedId.length > 0 && noteEditor && noteEditor.model)
            saveCurrentNote()
        selectedId = ""
        if (noteEditor) noteEditor.loadFromJson("", "")
    }

    background: Rectangle { color: theme.background }

    RowLayout {
        anchors.fill: parent
        spacing: 0

        // ── Sidebar ─────────────────────────────────────────────
        Rectangle {
            Layout.preferredWidth: 280
            Layout.fillHeight: true
            color: theme.surfaceAlt
            border.width: 1
            border.color: theme.border
            Layout.margins: 8
            Layout.rightMargin: 0
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
                        text: root.currentFolderPath.length > 0 ? ("→ " + root.currentFolderPath) : "NOTES"
                        color: root.currentFolderPath.length > 0 ? theme.tertiary : theme.textMuted
                        font.pixelSize: 10
                        elide: Text.ElideRight
                        Layout.fillWidth: true
                        MouseArea {
                            anchors.fill: parent
                            enabled: root.currentFolderPath.length > 0
                            cursorShape: Qt.PointingHandCursor
                            onClicked: root.currentFolderPath = ""
                        }
                    }
                    SidebarIconButton {
                        glyph: "⌂"; tooltipText: "Root"
                        visible: root.currentFolderPath.length > 0
                        onClicked: root.currentFolderPath = ""
                    }
                    SidebarIconButton {
                        glyph: "✎"; tooltipText: "New note"
                        onClicked: {
                            var folder = root.currentFolderPath
                            var newId = noteController.addEntryInFolder("Untitled note", "", folder)
                            if (!newId || !newId.length)
                                return
                            Qt.callLater(function () {
                                root.refresh(false)
                                root.selectNoteById(newId)
                            })
                        }
                    }
                    SidebarIconButton {
                        glyph: "+"; tooltipText: "New folder"
                        onClicked: { root.newFolderName = ""; root.isCreatingFolder = true }
                    }
                    SidebarIconButton {
                        glyph: "⇅"; tooltipText: root.sortByRecent ? "Recent" : "Name"
                        onClicked: { root.sortByRecent = !root.sortByRecent; root.buildTree() }
                    }
                    SidebarIconButton {
                        glyph: "⊟"; tooltipText: "Collapse all"
                        onClicked: { root.expandedPaths = ({}); root.buildTree() }
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
                        header: Item {
                            width: parent.width
                            height: root.isCreatingFolder ? 26 : 0
                            visible: root.isCreatingFolder
                            onVisibleChanged: if (visible) { folderNameInput.text = ""; folderNameInput.forceActiveFocus() }
                            Rectangle {
                                anchors.fill: parent
                                color: theme.surfaceAlt
                                border.color: theme.tertiary
                                radius: 2
                                TextField {
                                    id: folderNameInput
                                    anchors.fill: parent
                                    anchors.margins: 2
                                    color: theme.textPrimary
                                    font.pixelSize: 13
                                    placeholderText: "Folder name..."
                                    background: Rectangle { color: "transparent" }
                                    onAccepted: root.createFolder(text)
                                    Keys.onEscapePressed: { root.isCreatingFolder = false; root.newFolderName = "" }
                                }
                            }
                        }
                    }
                }
            }
        }

        // ── Editor ──────────────────────────────────────────────
        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            visible: root.selectedId.length > 0 && noteEditor && noteEditor.model !== null

            TextField {
                id: titleField
                property bool loadingEditor: false
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
            }

            EditorToolbar {
                Layout.fillWidth: true
                Layout.preferredHeight: 40
                Layout.leftMargin: 12
                Layout.rightMargin: 12
                Layout.topMargin: 4
                Layout.bottomMargin: 4

                currentBlockId: noteEditor ? (noteEditor.focusedBlockId || "") : ""

                onChangeType: function (t) {
                    if (noteEditor && noteEditor.focusedBlockId)
                        noteEditor.changeBlockType(noteEditor.focusedBlockId, t)
                }
                onInsertType: function (t) {
                    if (!noteEditor) return
                    var id = noteEditor.focusedBlockId
                    if (id && id.length)
                        noteEditor.insertBlockAfter(id, t, "")
                    else
                        noteEditor.insertBlock("", 0, t, "")
                }
            }

            BlockList {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.margins: 12
            }

            Timer {
                id: saveTimer
                interval: 800
                onTriggered: root.saveCurrentNote()
            }
            Connections {
                target: noteEditor
                function onDocumentModified() { saveTimer.restart() }
            }
        }

        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            visible: root.selectedId.length === 0
            Label {
                Layout.alignment: Qt.AlignCenter
                text: "Select or create a note to get started."
                color: theme.textMuted
                font.pixelSize: 13
            }
        }
    }

    component SidebarIconButton: Rectangle {
        property string glyph: ""
        property string tooltipText: ""
        signal clicked()
        width: 24; height: 24; radius: 4
        color: ma.containsMouse ? Qt.rgba(1,1,1,0.08) : "transparent"
        Text { anchors.centerIn: parent; text: parent.glyph; color: theme.textSecondary; font.pixelSize: 13 }
        MouseArea {
            id: ma; anchors.fill: parent; hoverEnabled: true
            cursorShape: Qt.PointingHandCursor; onClicked: parent.clicked()
        }
        ToolTip.visible: ma.containsMouse && tooltipText.length > 0
        ToolTip.text: tooltipText
        ToolTip.delay: 400
    }

    component TreeDelegate: Item {
        property var item: (index >= 0 && index < treeModel.count) ? treeModel.get(index) : null
        width: folderListView.width
        height: 26
        property bool hovered: false

        Rectangle {
            anchors.fill: parent
            color: {
                if (item && item.type === "note" && item.noteId === root.selectedId)
                    return Qt.rgba(theme.tertiary.r, theme.tertiary.g, theme.tertiary.b, 0.16)
                if (hovered) return Qt.rgba(1,1,1,0.06)
                return "transparent"
            }
        }
        Rectangle {
            visible: item && item.type === "note" && item.noteId === root.selectedId
            width: 2; anchors.top: parent.top; anchors.bottom: parent.bottom; anchors.left: parent.left
            color: theme.tertiary
        }
        Item {
            anchors.fill: parent
            anchors.leftMargin: 8 + (item ? item.depth * 16 : 0)
            anchors.rightMargin: 8
            Text {
                visible: item && item.type === "folder"
                text: (item && item.expanded) ? "▼ " : "▶ "
                color: theme.textMuted; font.pixelSize: 10
                anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter
            }
            Text {
                text: {
                    if (!item) return ""
                    if (item.type === "folder")
                        return item.expanded ? item.name : (item.name + " (" + item.noteCount + ")")
                    return item.name || "Untitled"
                }
                color: (item && item.type === "note" && item.noteId === root.selectedId)
                       ? theme.tertiary : theme.textSecondary
                font.pixelSize: item && item.type === "folder" ? 13 : 12
                font.bold: item && item.type === "folder"
                anchors.left: parent.left
                anchors.leftMargin: item && item.type === "folder" ? 16 : 0
                anchors.verticalCenter: parent.verticalCenter
                anchors.right: parent.right
                elide: Text.ElideRight
            }
            MouseArea {
                anchors.fill: parent
                hoverEnabled: true
                acceptedButtons: Qt.LeftButton | Qt.RightButton
                onEntered: parent.parent.hovered = true
                onExited: parent.parent.hovered = false
                onClicked: function (mouse) {
                    if (!item) return
                    if (mouse.button === Qt.RightButton) {
                        if (item.type === "folder") {
                            folderContextMenu.targetFolder = item.folderPath
                            folderContextMenu.popup()
                        } else {
                            noteContextMenu.targetId = item.noteId
                            noteContextMenu.targetTitle = item.name
                            noteContextMenu.targetFolder = item.folderPath
                            noteContextMenu.popup()
                        }
                        return
                    }
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
                var folder = folderContextMenu.targetFolder
                if (root.selectedId.length > 0) {
                    var open = root.allNotes.find(function (n) { return n.id === root.selectedId })
                    if (open) {
                        var f = open.folder || ""
                        if (f === folder || f.indexOf(folder + "/") === 0) {
                            root.selectedId = ""
                            if (noteEditor)
                                noteEditor.loadFromJson("", "")
                        }
                    }
                }
                if (noteController.deleteFolder(folder)) {
                    if (root.currentFolderPath === folder
                        || root.currentFolderPath.indexOf(folder + "/") === 0)
                        root.currentFolderPath = ""
                    root.refresh(false)
                }
            }
        }
        MenuItem {
            text: "New Subfolder"
            onTriggered: {
                root.currentFolderPath = folderContextMenu.targetFolder
                root.newFolderName = ""
                root.isCreatingFolder = true
            }
        }
        MenuItem {
            text: "New Note Here"
            onTriggered: {
                var folder = folderContextMenu.targetFolder
                var newId = noteController.addEntryInFolder("Untitled note", "", folder)
                if (!newId || !newId.length)
                    return
                Qt.callLater(function () {
                    root.refresh(false)
                    root.selectNoteById(newId)
                })
            }
        }
    }

    Menu {
        id: noteContextMenu
        property string targetId: ""
        property string targetTitle: ""
        property string targetFolder: ""
        MenuItem {
            text: "Rename"
            onTriggered: {
                renameNoteDialog.noteId = noteContextMenu.targetId
                renameNoteDialog.noteTitle = noteContextMenu.targetTitle
                renameNoteDialog.open()
            }
        }
        MenuItem {
            text: "Delete"
            onTriggered: {
                if (noteController.deleteEntry(noteContextMenu.targetId)) {
                    if (root.selectedId === noteContextMenu.targetId) {
                        root.selectedId = ""
                        if (noteEditor) noteEditor.loadFromJson("", "")
                    }
                    root.refresh(false)
                }
            }
        }
        MenuItem {
            text: "Move to Root"
            onTriggered: {
                noteController.moveNoteToFolder(noteContextMenu.targetId, "")
                root.refresh(true)
            }
        }
    }

    Dialog {
        id: renameFolderDialog
        property string folderName: ""
        modal: true
        title: "Rename Folder"
        anchors.centerIn: parent
        width: 300
        background: Rectangle { color: theme.surfaceAlt; radius: 12; border.color: theme.border }
        ColumnLayout {
            anchors.fill: parent; anchors.margins: 16; spacing: 12
            Label { text: "New name:"; color: theme.textPrimary }
            StyledTextField {
                id: renameFolderField
                Layout.fillWidth: true
                text: renameFolderDialog.folderName
                onAccepted: renameFolderBtn.clicked()
            }
            RowLayout {
                Button { text: "Cancel"; onClicked: renameFolderDialog.close() }
                PrimaryButton {
                    id: renameFolderBtn
                    text: "Rename"
                    Layout.fillWidth: true
                    onClicked: {
                        var n = renameFolderField.text.trim()
                        if (n.length > 0 && n !== renameFolderDialog.folderName) {
                            if (noteController.renameFolder(renameFolderDialog.folderName, n)) {
                                var ep = Object.assign({}, root.expandedPaths)
                                if (ep[renameFolderDialog.folderName] !== undefined) {
                                    ep[n] = ep[renameFolderDialog.folderName]
                                    delete ep[renameFolderDialog.folderName]
                                    root.expandedPaths = ep
                                }
                                if (root.currentFolderPath === renameFolderDialog.folderName)
                                    root.currentFolderPath = n
                                root.refresh(false)
                                renameFolderDialog.close()
                            }
                        }
                    }
                }
            }
        }
        onOpened: { renameFolderField.text = folderName; renameFolderField.forceActiveFocus() }
    }

    Dialog {
        id: renameNoteDialog
        property string noteId: ""
        property string noteTitle: ""
        modal: true
        title: "Rename Note"
        anchors.centerIn: parent
        width: 320
        background: Rectangle { color: theme.surfaceAlt; radius: 12; border.color: theme.border }
        ColumnLayout {
            anchors.fill: parent; anchors.margins: 16; spacing: 12
            StyledTextField {
                id: renameNoteField
                Layout.fillWidth: true
                text: renameNoteDialog.noteTitle
                onAccepted: renameNoteBtn.clicked()
            }
            RowLayout {
                Button { text: "Cancel"; onClicked: renameNoteDialog.close() }
                PrimaryButton {
                    id: renameNoteBtn
                    text: "Rename"
                    Layout.fillWidth: true
                    onClicked: {
                        var t = renameNoteField.text.trim()
                        if (t.length > 0 && noteController.renameEntry(renameNoteDialog.noteId, t)) {
                            if (root.selectedId === renameNoteDialog.noteId && noteEditor)
                                noteEditor.noteTitle = t
                            root.refresh(true)
                            renameNoteDialog.close()
                        }
                    }
                }
            }
        }
        onOpened: { renameNoteField.text = noteTitle; renameNoteField.forceActiveFocus() }
    }

    Connections {
        target: noteController
        function onEntriesChanged() { root.refresh(true) }
    }

    Component.onCompleted: refresh(false)
}