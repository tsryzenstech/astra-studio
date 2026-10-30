pragma Singleton
import QtQuick

// Preview: the device. Values of a Redmi Note 10 Pro.
QtObject {
    property real brightness: 0.7
    property int batteryLevel: 82
    property bool charging: false
    property string model: "Redmi Note 10 Pro"
    function property(key, fallback) { return fallback !== undefined ? fallback : "" }
    function setBrightness(v) { brightness = v }
    function call(method, params, callback) { if (callback) Qt.callLater(() => callback(null, "")) }
    function power(what) {}
    function vibrate(ms) {}
}
