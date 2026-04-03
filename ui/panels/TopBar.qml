// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../components"
import "../style"

Rectangle {
    id: root

    property var workbenchController
    property var appController
    property var onNewRequested
    property var onOpenRequested

    height: theme.topBarHeight
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
        spacing: 8

        Label {
            text: appController && appController.documentName.length > 0
                ? appController.documentName
                : "Untitled"
            font.pixelSize: theme.titleSize
            font.family: theme.fontFamily
            color: theme.textPrimary
            Layout.rightMargin: 16
        }

        TabBar {
            id: wbTabBar
            Layout.preferredWidth: 320
            Layout.fillHeight: true

            Repeater {
                model: workbenchController ? workbenchController.workbenchNames : []
                delegate: TabButton {
                    text: modelData
                    width: Math.max(100, implicitWidth + 20)
                }
            }

            currentIndex: {
                if (!workbenchController || !workbenchController.workbenchNames || workbenchController.workbenchNames.length <= 0) {
                    return -1
                }
                return Math.max(0, workbenchController.workbenchNames.indexOf(workbenchController.activeWorkbench))
            }

            onCurrentIndexChanged: {
                if (workbenchController && currentIndex >= 0 && currentIndex < workbenchController.workbenchNames.length) {
                    var selected = workbenchController.workbenchNames[currentIndex]
                    if (selected !== workbenchController.activeWorkbench) {
                        workbenchController.switchWorkbench(selected)
                    }
                }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            color: "transparent"
            implicitHeight: 1
        }

        IconButton {
            iconText: "N"
            tooltip: "新建 (Ctrl+N)"
            onClicked: {
                if (typeof root.onNewRequested === "function") {
                    root.onNewRequested()
                }
            }
        }

        IconButton {
            iconText: "O"
            tooltip: "打开 (Ctrl+O)"
            onClicked: {
                if (typeof root.onOpenRequested === "function") {
                    root.onOpenRequested()
                }
            }
        }

        IconButton {
            iconText: "S"
            tooltip: "保存 (Ctrl+S)"
        }

        IconButton {
            iconText: "↶"
            tooltip: "撤销 (Ctrl+Z)"
            enabled: appController ? appController.canUndo : false
            onClicked: appController ? appController.undo() : undefined
        }

        IconButton {
            iconText: "↷"
            tooltip: "重做 (Ctrl+Y)"
            enabled: appController ? appController.canRedo : false
            onClicked: appController ? appController.redo() : undefined
        }
    }
}
