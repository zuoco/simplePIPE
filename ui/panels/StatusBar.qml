import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../style"

Rectangle {
    id: root

    property var appController

    height: theme.statusBarHeight
    color: "#FFFFFF"
    border.color: theme.divider
    border.width: 1

    Theme {
        id: theme
    }

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 10
        anchors.rightMargin: 10
        spacing: 18

        Label {
            text: "Selected: " + (appController ? appController.selectedCount : 0)
            color: theme.textPrimary
        }

        Label {
            text: "Coord: (0.0, 0.0, 0.0)"
            color: theme.textSecondary
        }

        Rectangle {
            Layout.fillWidth: true
            color: "transparent"
            implicitHeight: 1
        }

        Label {
            text: "Zoom: 100%"
            color: theme.textSecondary
        }
    }
}
