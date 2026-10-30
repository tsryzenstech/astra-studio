pragma Singleton
import QtQuick

// Preview: the display.
QtObject {
    property var state: ({ "modes": [] })
    readonly property bool ready: true
    property int revision: 0
    signal event(string name, var data)
    function call(method, params, callback) { if (callback) Qt.callLater(() => callback(null, "not available in the preview")) }
}
