pragma Singleton
import QtQuick

// Preview: the app's files live in memory for as long as the preview runs.
QtObject {
    property var store: ({})
    readonly property string appData: "/data/data/app"
    readonly property string media: "/sdcard"
    function readText(path, maxBytes) { return store[path] || "" }
    function writeText(path, text) {
        const s = Object.assign({}, store)
        s[path] = String(text)
        store = s
        return true
    }
    function exists(path) { return path in store }
    function remove(path) {
        const s = Object.assign({}, store)
        delete s[path]
        store = s
        return true
    }
    function list(dir) { return Object.keys(store).filter(p => p.startsWith(dir + "/")) }
    function copy(paths, dir) { return 0 }
    function move(paths, dir) { return 0 }
}
