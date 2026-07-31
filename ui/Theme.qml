import QtQuick

// Design tokens pulled from the Anchor color palette + Stitch mockups.
// Usage: instantiate locally in any page in this folder, e.g.
//   Theme { id: theme }
//   Rectangle { color: theme.background }
//
// This is a plain QtObject rather than a `pragma Singleton` - simpler to
// wire up in a qmake (non-CMake-module) project, and since every property
// here is `readonly`, instantiating it per-page costs nothing and carries
// no shared-state risk.
QtObject {
    // --- Brand palette ---
    readonly property color primary: "#0B3D0B"     // deep green
    readonly property color secondary: "#2E8B57"   // medium green (buttons, accents)
    readonly property color tertiary: "#4F9F4F"    // soft green (highlights)
    readonly property color neutral: "#757872"     // gray

    // --- Surfaces ---
    readonly property color background: "#000000"
    readonly property color surface: "#111311"
    readonly property color surfaceAlt: "#1B1D1B"
    readonly property color border: "#2A2E2A"

    // --- Text ---
    readonly property color textPrimary: "#D1E7D1"
    readonly property color textSecondary: "#8C948C"
    readonly property color textMuted: "#5C625C"

    // --- Status ---
    readonly property color danger: "#E0574C"
    readonly property color warning: "#E0A15C"
    readonly property color success: tertiary

    // --- Typography ---
    // Falls back to the platform default automatically if these aren't
    // installed. Send the .ttf/.otf files and I'll wire up FontLoader for
    // a pixel-exact match instead of relying on font-name lookup.
    readonly property string headlineFont: "Hanken Grotesk"
    readonly property string bodyFont: "Hanken Grotesk"
    readonly property string labelFont: "Geist"

    // --- Shape ---
    readonly property int radiusSmall: 8
    readonly property int radiusMedium: 12
    readonly property int radiusLarge: 20
    readonly property int radiusPill: 999

    // --- Spacing ---
    readonly property int spacingSmall: 8
    readonly property int spacingMedium: 16
    readonly property int spacingLarge: 24

    readonly property string iconFont: "Material Symbols Outlined"

}
