// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../components"
import "../style"

CollapsiblePanel {
    id: root

    property var tableModel
    property var propertyModel
    property bool editMode: true

    title: "参数化面板"
    fillHeight: true

    Theme {
        id: theme
    }

    function ensurePropertyPanelVisibleAndFlash() {
        root.collapsed = false
        propertyPanel.collapsed = false
        propertyPanel.ensureExpandedAndFlash()
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 8

        SplitView {
            id: rightSplit
            Layout.fillWidth: true
            Layout.fillHeight: true
            orientation: Qt.Vertical

            PipePointTable {
                id: pipePointTable
                objectName: "pipePointTablePanel"
                tableModel: root.tableModel
                SplitView.minimumHeight: 200
                SplitView.preferredHeight: 300
            }

            PropertyPanel {
                id: propertyPanel
                objectName: "propertyPanel"
                propertyModel: root.propertyModel
                editMode: root.editMode
                SplitView.minimumHeight: 220
                SplitView.fillHeight: true
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 34
            radius: 4
            color: "#F7FAFD"
            border.color: "#E1EAF3"
            border.width: 1

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 8
                anchors.rightMargin: 8

                Label {
                    text: "属性模式"
                    color: theme.textSecondary
                    font.pixelSize: 12
                }

                Item {
                    Layout.fillWidth: true
                }

                Button {
                    text: root.editMode ? "切换为只读模式" : "切换为编辑模式"
                    onClicked: root.editMode = !root.editMode
                }
            }
        }
    }
}