import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import PipeCAD 1.0
import "../components"
import "../style"

Rectangle {
    id: root

    signal inspectRequested()

    color: "transparent"

    Theme {
        id: theme
    }

    Rectangle {
        anchors.fill: parent
        radius: theme.radius
        border.color: theme.divider
        border.width: 1
        gradient: Gradient {
            GradientStop { position: 0.0; color: theme.bgViewportTop }
            GradientStop { position: 1.0; color: theme.bgViewportBottom }
        }
    }

    VsgViewport {
        id: viewport
        objectName: "viewport3d"
        anchors.fill: parent
        anchors.margins: 1
        focus: true

        Keys.onDeletePressed: {
            // Delete shortcut is consumed by C++ bridge and can trigger removal actions later.
        }
    }

    Column {
        spacing: 6
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.margins: 10

        IconButton {
            iconText: "F"
            tooltip: "正视图"
            onClicked: viewport.setViewPreset(0)
        }
        IconButton {
            iconText: "R"
            tooltip: "右视图"
            onClicked: viewport.setViewPreset(1)
        }
        IconButton {
            iconText: "T"
            tooltip: "俯视图"
            onClicked: viewport.setViewPreset(2)
        }
        IconButton {
            iconText: "I"
            tooltip: "轴测图"
            onClicked: viewport.setViewPreset(3)
        }
        IconButton {
            iconText: "⤢"
            tooltip: "Fit All"
            onClicked: viewport.fitAll()
        }
        IconButton {
            iconText: "G"
            tooltip: "网格开关"
            onClicked: viewport.toggleGrid()
        }
    }

    ContextMenu {
        id: viewportMenu
        onInspectRequested: root.inspectRequested()
    }

    MouseArea {
        anchors.fill: parent
        acceptedButtons: Qt.RightButton
        propagateComposedEvents: true
        onPressed: {
            if (mouse.button === Qt.RightButton) {
                viewportMenu.popup()
                mouse.accepted = true
            }
        }
    }
}
