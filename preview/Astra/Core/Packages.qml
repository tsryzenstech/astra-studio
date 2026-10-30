pragma Singleton
import QtQuick

// Preview: the installed apps.
QtObject {
    property int count: 0
    signal changed()
    function list() { return [] }
    function get(pkg) { return ({ package: pkg, name: pkg, iconUrl: "" }) }
    function targets(mime, kind) { return [] }
    function launch(pkg, args) { console.log("Packages.launch", pkg, JSON.stringify(args || {})) }
    function widget(key) { return ({}) }
}
