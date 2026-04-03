// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

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
        spacing: 0

        // Left zone: selected object info
        Label {
            Layout.preferredWidth: parent.width * 0.4
            elide: Text.ElideRight
            text: appController ? appController.selectionInfo : ""
            color: theme.textPrimary
            font.pixelSize: 12
        }

        Rectangle {
            width: 1
            Layout.fillHeight: true
            Layout.topMargin: 4
            Layout.bottomMargin: 4
            color: theme.divider
        }

        // Center zone: mouse 3D world coordinates
        Label {
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignHCenter
            text: appController ? appController.mouseCoord : "(0.0, 0.0, 0.0)"
            color: theme.textSecondary
            font.pixelSize: 12
        }

        Rectangle {
            width: 1
            Layout.fillHeight: true
            Layout.topMargin: 4
            Layout.bottomMargin: 4
            color: theme.divider
        }

        // Right zone: zoom level
        Label {
            Layout.preferredWidth: 80
            horizontalAlignment: Text.AlignRight
            text: "Zoom: " + (appController ? Math.round(appController.zoomLevel) : 100) + "%"
            color: theme.textSecondary
            font.pixelSize: 12
        }
    }
}
