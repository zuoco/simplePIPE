import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import PipeCAD 1.0
import "../components"
import "../style"

Item {
    id: root

    property alias vtkViewport: viewport
    property var appController: null

    Theme {
        id: theme
    }

    Rectangle {
        anchors.fill: parent
        radius: theme.radius
        border.color: theme.divider
        border.width: 1
        color: theme.bgViewportTop
    }

    VtkViewportElement {
        id: viewport
        objectName: "vtkViewportElement"
        anchors.fill: parent
        anchors.margins: 1
        focus: true
    }

    // Only basic overlay for VTK specific controls
    Column {
        spacing: 6
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.margins: 10

        IconButton {
            iconText: "\u2922"
            tooltip: "Fit All"
            // onClicked: viewManager.fitAll() // handled by C++ bridge
        }
    }
}
