pragma Singleton
import QtQuick

// Preview: a sunny day.
QtObject {
    readonly property bool available: true
    property var current: ({ temperature: 22, condition: "clear", description: "Sunny", high: 25, low: 16, city: "Milano",
                             hourly: [], daily: [] })
    function refresh() {}
}
