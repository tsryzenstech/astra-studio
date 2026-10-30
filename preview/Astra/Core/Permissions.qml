pragma Singleton
import QtQuick

// Preview: every permission is granted.
QtObject {
    function request(permission, callback) { if (callback) Qt.callLater(() => callback(true)) }
    function check(permission, callback) { if (callback) Qt.callLater(() => callback(true)) }
    function catalog(callback) { if (callback) Qt.callLater(() => callback([])) }
}
