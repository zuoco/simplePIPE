// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

import QtQuick
import QtQuick.Controls

ToolButton {
    id: root

    property string iconText: ""
    property string tooltip: ""

    text: iconText.length > 0 ? iconText : "?"
    font.pixelSize: 14
    implicitWidth: 30
    implicitHeight: 30

    background: Rectangle {
        radius: 4
        color: root.down ? "#CFE5F8" : (root.hovered ? "#EEF6FD" : "transparent")
        border.color: root.hovered ? "#B7D7F3" : "transparent"
        border.width: 1
    }

    ToolTip.visible: hovered
    ToolTip.text: tooltip
    ToolTip.delay: 250
}
