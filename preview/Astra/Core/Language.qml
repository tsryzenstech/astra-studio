pragma Singleton
import QtQuick

// Preview: the system language.
QtObject {
    property string current: "en"
    property var available: ["en", "it"]
    function nameOf(code) { return code === "it" ? "Italiano" : "English" }
    function set(code) { current = code }
}
