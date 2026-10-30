pragma Singleton
import QtQuick

// Preview: the phone line, idle.
QtObject {
    property var state: ({ "calls": [], "sim": true })
    readonly property bool ready: true
    property int revision: 0
    signal event(string name, var data)
    function call(method, params, callback) { if (callback) Qt.callLater(() => callback(null, "not available in the preview")) }
}
