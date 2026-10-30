pragma Singleton
import QtQuick

// Preview: notifications are printed to Astra Studio's output.
QtObject {
    property var list: []
    property int unread: 0
    function post(title, body, options) {
        console.log("Notification:", title, "-", body)
        return Date.now()
    }
    function dismiss(id) {}
    function clear() {}
}
