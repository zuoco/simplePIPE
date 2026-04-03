// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

import QtQuick

QtObject {
    readonly property color bgPrimary: "#F5F5F5"
    readonly property color bgPanel: "#FFFFFF"
    readonly property color bgViewportTop: "#E8E8E8"
    readonly property color bgViewportBottom: "#D0D0D0"
    readonly property color accent: "#0078D4"
    readonly property color textPrimary: "#333333"
    readonly property color textSecondary: "#888888"
    readonly property color divider: "#E0E0E0"
    readonly property color hover: "#EEF6FD"
    readonly property color cardShadow: "#10000000"

    readonly property int radius: 6
    readonly property int spacing: 8
    readonly property int panelPadding: 10
    readonly property int rowHeight: 36
    readonly property int topBarHeight: 48
    readonly property int statusBarHeight: 30

    readonly property string fontFamily: "Noto Sans CJK SC"
    readonly property int fontSize: 13
    readonly property int titleSize: 15
}
