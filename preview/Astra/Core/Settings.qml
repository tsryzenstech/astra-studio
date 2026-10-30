pragma Singleton
import QtQuick

// Preview: the system settings store. Astra Studio changes "theme.mode",
// "theme.accent" and "display.font_scale" from its toolbar.
QtObject {
    property var values: ({ "theme.mode": "dark", "theme.accent": "orange", "display.font_scale": 1 })
    function setValue(key, value) {
        const v = Object.assign({}, values)
        v[key] = value
        values = v
    }
    function value(key, fallback) { return key in values ? values[key] : fallback }
}
