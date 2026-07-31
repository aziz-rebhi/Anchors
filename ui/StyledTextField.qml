import QtQuick
import QtQuick.Controls

// Dark, rounded input field matching the lock/search fields in the
// mockups. Behaves like a normal TextField - `text`, `echoMode`,
// `placeholderText`, `onAccepted` all work as usual.
TextField {
    id: control

    Theme { id: theme }

    color: theme.textPrimary
    placeholderTextColor: theme.textMuted
    font.family: theme.bodyFont
    font.pixelSize: 15
    selectByMouse: true
    leftPadding: 16
    rightPadding: 16
    implicitHeight: 44

    background: Rectangle {
        radius: theme.radiusMedium
        color: theme.surfaceAlt
        border.width: 1
        border.color: control.activeFocus ? theme.tertiary : theme.border
    }
}
