import QtQuick
import QtQuick.Controls

Button {
    id: control

    property bool outlined: false
    property bool uppercase: false

    Theme { id: theme }

    font.family: theme.labelFont
    font.pixelSize: 14
    font.letterSpacing: uppercase ? 1.2 : 0

    contentItem: Text {
        text: uppercase ? control.text.toUpperCase() : control.text
        font: control.font
        color: control.outlined ? theme.tertiary : theme.onAccent
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
    }

    background: Rectangle {
        implicitHeight: 44
        radius: theme.radiusPill
        color: control.outlined
               ? "transparent"
               : (control.pressed ? Qt.darker(theme.tertiary, 1.15) : theme.tertiary)
        border.width: control.outlined ? 1 : 0
        border.color: theme.border
        opacity: control.enabled ? 1.0 : 0.5
    }
}