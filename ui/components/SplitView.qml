// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

import QtQuick
import QtQuick.Controls as QQC

QQC.SplitView {
    id: root

    property color handleColor: "#E0E0E0"
    property color handleHoverColor: "#B8D8F5"

    handle: Rectangle {
        implicitWidth: 6
        implicitHeight: 6
        color: QQC.SplitHandle.pressed
            ? root.handleHoverColor
            : (QQC.SplitHandle.hovered ? "#D8EAFB" : root.handleColor)

        Rectangle {
            anchors.centerIn: parent
            width: 2
            height: parent.height > parent.width ? 18 : 2
            color: "#9AA7B2"
            radius: 1
        }
    }
}
