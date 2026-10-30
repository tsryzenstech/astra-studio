// SPDX-License-Identifier: Apache-2.0
import QtQuick
import Astra.Core
import Astra.Controls

// Astra Studio's preview: the app being edited, on a phone-sized screen
// with astraOS's real components. Astra Studio sets source and reloads.
Rectangle {
    id: host
    color: "#1b1b1f"

    property url source
    property int revision: 0
    property string error: ""
    property real screenWidth: 393
    property real screenHeight: 873
    signal loaded()

    function reload() {
        loader.sourceComponent = undefined
        error = ""
        if (source.toString() === "")
            return
        const c = Qt.createComponent(source)
        if (c.status === Component.Error) {
            error = c.errorString()
            return
        }
        loader.sourceComponent = c
        loaded()
    }

    Rectangle {
        id: phone
        anchors.centerIn: parent
        width: host.screenWidth
        height: host.screenHeight
        scale: Math.min(1, (host.width - 32) / width, (host.height - 32) / height)
        radius: Theme.screenRadius
        color: Theme.background
        clip: true
        border.width: 3
        border.color: "#3a3a40"

        Loader {
            id: loader
            anchors.fill: parent
            onStatusChanged: if (status === Loader.Error) host.error = qsTr("The app could not be created (see Output).")
        }

        // Status bar
        Item {
            width: parent.width
            height: Theme.statusBarHeight
            Text {
                x: 32
                anchors.verticalCenter: parent.verticalCenter
                text: Qt.formatTime(new Date(), "HH:mm")
                font.family: Theme.fontFamily
                font.pixelSize: 15
                font.weight: Font.DemiBold
                color: Theme.text
            }
            BatteryGlyph {
                anchors.right: parent.right
                anchors.rightMargin: 28
                anchors.verticalCenter: parent.verticalCenter
                level: System.batteryLevel
                color: Theme.text
            }
        }
        // Home indicator
        Rectangle {
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 8
            width: 134
            height: 5
            radius: 3
            color: Theme.text
            opacity: 0.9
        }

        Rectangle {
            anchors.fill: parent
            visible: host.error !== ""
            color: Theme.background
            Text {
                anchors.fill: parent
                anchors.margins: 24
                anchors.topMargin: 80
                wrapMode: Text.Wrap
                text: host.error
                font.family: "Consolas"
                font.pixelSize: 13
                color: Theme.danger
            }
        }
    }
}
