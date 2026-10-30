pragma Singleton
import QtQuick

// Preview: network state (Wi-Fi connected, mobile data on).
QtObject {
    property var state: ({ "wifi": { "enabled": true, "connected": true, "ssid": "Home", "signal": 80 }, "cellular": { "enabled": true, "operator": "astra", "signal": 3 }, "bluetooth": { "enabled": false }, "airplane": false })
    readonly property bool ready: true
    property int revision: 0
    signal event(string name, var data)
    function call(method, params, callback) { if (callback) Qt.callLater(() => callback(null, "not available in the preview")) }
}
