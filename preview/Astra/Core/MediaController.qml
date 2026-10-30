pragma Singleton
import QtQuick

// Preview: nothing is playing.
QtObject {
    property bool active: false
    property var session: null
    function command(name) {}
}
