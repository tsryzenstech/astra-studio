pragma Singleton
import QtQuick

// Preview: two sample contacts.
QtObject {
    property var state: ({ "contacts": [ { "id": 1, "name": "Alex Rossi", "phone": "+39 333 000 0001", "favorite": true }, { "id": 2, "name": "Sam Bianchi", "phone": "+39 333 000 0002" } ] })
    readonly property bool ready: true
    property int revision: 0
    signal event(string name, var data)
    function call(method, params, callback) { if (callback) Qt.callLater(() => callback(null, "not available in the preview")) }
}
