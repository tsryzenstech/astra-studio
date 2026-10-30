pragma Singleton
import QtQuick

// Preview: the running app. Astra Studio sets packageName and accent from
// the project's app.json.
QtObject {
    property string packageName: "com.example.app"
    property var args: ({})
    property color accent: "transparent"
    readonly property string dataDir: "/data/data/" + packageName
    readonly property string mediaDir: "/sdcard"
    signal intent(var args)
    signal mediaChanged()
    function quit() {}
    function media(filters) { return [] }
    function mediaAsync(filters, callback) { Qt.callLater(() => callback([])) }
    function removeMedia(path) { return false }
    function imageInfo(path) { return ({}) }
}
