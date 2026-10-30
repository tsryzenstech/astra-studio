pragma Singleton
import QtQuick

// Preview: system dialogs answer with their first button.
QtObject {
    function show(options, callback) {
        console.log("Dialog:", options && options.title)
        const first = options && options.buttons && options.buttons.length ? (options.buttons[0].id || options.buttons[0]) : "ok"
        if (callback) Qt.callLater(() => callback(first))
    }
}
