// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../components"
import "../style"

// ComponentToolStrip — 元件插入工具条
// 双列图标条：左列 Fittings（贴近视口），右列 Accessories（贴近参数面板）
Rectangle {
    id: root

    property var appController: null

    // 固定宽度：两列各 30px + 内边距
    SplitView.minimumWidth: 68
    SplitView.preferredWidth: 68
    SplitView.maximumWidth: 68

    color: "#F8FAFB"
    border.color: "#D8E4EF"
    border.width: 1

    Theme { id: theme }

    // 分组线标签
    ColumnLayout {
        anchors.fill: parent
        anchors.topMargin: 6
        anchors.bottomMargin: 6
        anchors.leftMargin: 2
        anchors.rightMargin: 2
        spacing: 0

        // 分组标题
        Text {
            Layout.fillWidth: true
            text: "构件"
            font.pixelSize: 10
            color: theme.textSecondary
            horizontalAlignment: Text.AlignHCenter
            elide: Text.ElideNone
        }

        Item { Layout.preferredHeight: 4 }

        // 双列图标区
        Row {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 2

            // ── Fittings 列（紧贴视口侧，即左侧）──
            Column {
                width: 32
                spacing: 3

                // 列标题
                Text {
                    width: 32
                    text: "管件"
                    font.pixelSize: 9
                    color: theme.textSecondary
                    horizontalAlignment: Text.AlignHCenter
                }

                IconButton {
                    iconText: "—"
                    tooltip: "插入直管段 (insert-pipe)"
                    onClicked: if (root.appController) root.appController.insertComponent("insert-pipe")
                }
                IconButton {
                    iconText: "↩"
                    tooltip: "插入弯头 (insert-elbow)"
                    onClicked: if (root.appController) root.appController.insertComponent("insert-elbow")
                }
                IconButton {
                    iconText: "⊤"
                    tooltip: "插入三通 (insert-tee)"
                    onClicked: if (root.appController) root.appController.insertComponent("insert-tee")
                }
                IconButton {
                    iconText: "▷"
                    tooltip: "插入大小头 (insert-reducer)"
                    onClicked: if (root.appController) root.appController.insertComponent("insert-reducer")
                }
                IconButton {
                    iconText: "⊗"
                    tooltip: "插入阀门 (insert-valve)"
                    onClicked: if (root.appController) root.appController.insertComponent("insert-valve")
                }
                IconButton {
                    iconText: "⊞"
                    tooltip: "插入法兰 (insert-flange)"
                    onClicked: if (root.appController) root.appController.insertComponent("insert-flange")
                }
            }

            // ── Accessories 列（紧贴参数面板侧，即右侧）──
            Column {
                width: 32
                spacing: 3

                // 列标题
                Text {
                    width: 32
                    text: "附件"
                    font.pixelSize: 9
                    color: theme.textSecondary
                    horizontalAlignment: Text.AlignHCenter
                }

                IconButton {
                    iconText: "△"
                    tooltip: "插入刚性支撑/锚固 (insert-rigid-support)"
                    onClicked: if (root.appController) root.appController.insertComponent("insert-rigid-support")
                }
                IconButton {
                    iconText: "⌀"
                    tooltip: "插入弹簧支吊架 (insert-spring-hanger)"
                    onClicked: if (root.appController) root.appController.insertComponent("insert-spring-hanger")
                }
                IconButton {
                    iconText: "→"
                    tooltip: "插入导向约束 (insert-guide)"
                    onClicked: if (root.appController) root.appController.insertComponent("insert-guide")
                }
                IconButton {
                    iconText: "⊟"
                    tooltip: "插入位移限位器 (insert-restraint)"
                    onClicked: if (root.appController) root.appController.insertComponent("insert-restraint")
                }
                IconButton {
                    iconText: "I"
                    tooltip: "插入结构梁/支架 (insert-beam)"
                    onClicked: if (root.appController) root.appController.insertComponent("insert-beam")
                }
            }
        }
    }
}

