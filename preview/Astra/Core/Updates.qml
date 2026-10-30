pragma Singleton
import QtQuick

// Preview: system updates, up to date.
QtObject {
    property var state: ({ "available": false })
    readonly property bool ready: true
    property int revision: 0
    signal event(string name, var data)
    function call(method, params, callback) { if (callback) Qt.callLater(() => callback(null, "not available in the preview")) }
}
