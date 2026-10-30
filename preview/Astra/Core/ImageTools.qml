pragma Singleton
import QtQuick

// Preview: image tools do nothing.
QtObject {
    function crop(path, rect, out) { return false }
    function rotate(path, degrees, out) { return false }
}
