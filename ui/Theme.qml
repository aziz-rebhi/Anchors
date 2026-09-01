import QtQuick 2.15

QtObject {
    id: root

    // Prefer Qt 6 styleHints; fall back to dark if unavailable
    readonly property bool systemIsDark: {
        try {
            if (Qt.styleHints && Qt.styleHints.colorScheme !== undefined)
                return Qt.styleHints.colorScheme === Qt.Dark
        } catch (e) {}
        return true
    }

    // themeId: "dark" | "light" | "system"
    readonly property bool isDark: {
        if (!settingsController)
            return true
        var id = settingsController.themeId || "dark"
        if (id === "light")
            return false
        if (id === "dark")
            return true
        // "system" (or anything else)
        return root.systemIsDark
    }

    readonly property color accent:
        (settingsController && settingsController.accentColor
         && settingsController.accentColor.length)
            ? settingsController.accentColor
            : (isDark ? "#89b4fa" : "#1d4ed8")

    readonly property color tertiary: accent
    readonly property color secondary: accent

    readonly property color background: isDark ? "#0f0f12" : "#f5f5f7"
    readonly property color surface:    isDark ? "#18181c" : "#ffffff"
    readonly property color surfaceAlt: isDark ? "#222228" : "#ececef"
    readonly property color border:     isDark ? "#2e2e36" : "#d5d5da"

    readonly property color textPrimary:   isDark ? "#f2f2f5" : "#1c1c1e"
    readonly property color textSecondary: isDark ? "#a8a8b3" : "#6c6c70"
    readonly property color textMuted:     isDark ? "#6c6c76" : "#8e8e93"

    readonly property color danger:  isDark ? "#f38ba8" : "#dc2626"
    readonly property color success: isDark ? "#a6e3a1" : "#16a34a"
    readonly property color warning: isDark ? "#fab387" : "#d97706"
    readonly property color neutral: isDark ? "#6c7086" : "#94a3b8"

    readonly property color codeBg:       isDark ? "#1e1e2e" : "#f0f0f3"
    readonly property color codeHeader:   isDark ? "#313244" : "#e4e4ea"
    readonly property color codeText:     isDark ? "#cdd6f4" : "#1c1c1e"
    readonly property color codeMuted:    isDark ? "#6c7086" : "#8e8e93"

    readonly property color onAccent: isDark ? "#0A140A" : "#ffffff"

    readonly property string headlineFont: "Inter, Segoe UI, sans-serif"
    readonly property string bodyFont:     "Inter, Segoe UI, sans-serif"
    readonly property string labelFont:    "Inter, Segoe UI, sans-serif"

    readonly property int radiusSmall:  6
    readonly property int radiusMedium: 10
    readonly property int radiusPill:   999
    readonly property int spacingLarge: 20

    readonly property color hoverFill: isDark ? Qt.rgba(1,1,1,0.08) : Qt.rgba(0,0,0,0.06)

    // Re-evaluate when OS theme changes (Qt 6)
    property var _schemeWatcher: Connections {
        target: (typeof Qt.styleHints !== "undefined") ? Qt.styleHints : null
        function onColorSchemeChanged() {
            // touch a dependency so isDark bindings refresh
            root.systemIsDarkChanged()
        }
    }
}