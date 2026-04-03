// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import PipeCAD 1.0
import "../components"
import "../style"

Rectangle {
    id: root

    signal inspectRequested()
    signal modifyRequested()
    signal viewModeRequested()
    signal deleteRequested()
    signal boxSelectFinished(int startX, int startY, int endX, int endY, bool append)

    property alias vsgViewport: viewport
    property var appController: null

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
            iconText: "\u2922"
            tooltip: "Fit All"
            onClicked: viewport.fitAll()
        }
        IconButton {
            iconText: "G"
            tooltip: "网格开关"
            onClicked: viewport.toggleGrid()
        }
    }

    // Box selection rectangle overlay
    Rectangle {
        id: selectionRect
        visible: false
        color: selectionRectArea.isWindow ? "#200078D4" : "#20FF6600"
        border.color: selectionRectArea.isWindow ? "#0078D4" : "#FF6600"
        border.width: selectionRectArea.isWindow ? 1 : 1
        border.pixelAligned: true

        // Dashed border for Crossing mode (right-to-left)
        Rectangle {
            anchors.fill: parent
            visible: !selectionRectArea.isWindow
            color: "transparent"
            border.color: selectionRectArea.isWindow ? "#0078D4" : "#FF6600"
            border.width: 1
            opacity: 0.6
        }
    }

    // Box selection mouse area (non-Ctrl left drag)
    MouseArea {
        id: selectionRectArea
        anchors.fill: parent
        acceptedButtons: Qt.LeftButton
        propagateComposedEvents: true

        property bool dragging: false
        property int startX: 0
        property int startY: 0
        property bool isWindow: true

        onPressed: function(mouse) {
            if (mouse.modifiers & Qt.ControlModifier) {
                // Ctrl+drag = orbit rotation, pass through
                mouse.accepted = false
                return
            }
            startX = mouse.x
            startY = mouse.y
            dragging = false
            mouse.accepted = false  // allow click-through for single clicks initially
        }

        onPositionChanged: function(mouse) {
            if (mouse.modifiers & Qt.ControlModifier) {
                mouse.accepted = false
                return
            }
            var dx = Math.abs(mouse.x - startX)
            var dy = Math.abs(mouse.y - startY)
            if (!dragging && (dx > 5 || dy > 5)) {
                dragging = true
            }
            if (dragging) {
                isWindow = (mouse.x >= startX)
                var rx = Math.min(startX, mouse.x)
                var ry = Math.min(startY, mouse.y)
                var rw = Math.abs(mouse.x - startX)
                var rh = Math.abs(mouse.y - startY)
                selectionRect.x = rx
                selectionRect.y = ry
                selectionRect.width = rw
                selectionRect.height = rh
                selectionRect.visible = true
            }
        }

        onReleased: function(mouse) {
            if (dragging) {
                selectionRect.visible = false
                var append = (mouse.modifiers & Qt.ShiftModifier) ? true : false
                root.boxSelectFinished(startX, startY, mouse.x, mouse.y, append)
                dragging = false
            }
        }
    }

    ContextMenu {
        id: viewportMenu
        hasSelection: appController ? appController.hasSelection : false
        onModifyRequested: root.modifyRequested()
        onViewRequested: root.viewModeRequested()
        onDeleteRequested: root.deleteRequested()
    }

    MouseArea {
        anchors.fill: parent
        acceptedButtons: Qt.RightButton
        propagateComposedEvents: true
        onPressed: function(mouse) {
            if (mouse.button === Qt.RightButton) {
                viewportMenu.popup()
                mouse.accepted = true
            }
        }
    }
}
