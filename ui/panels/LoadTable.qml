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
    property bool editEnabled: true

    title: "载荷表 (Load Table)"
    fillHeight: true

    Theme {
        id: theme
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // 表头
        Row {
            Layout.fillWidth: true
            Layout.preferredHeight: 28
            spacing: 1

            Repeater {
                model: ["名称", "类型", "参数", "作用对象"]

                Rectangle {
                    width: index === 2 ? 160 : (index === 3 ? 140 : 120)
                    height: 28
                    color: "#F0F2F5"
                    border.color: "#E0E0E0"
                    border.width: 1

                    Label {
                        anchors.centerIn: parent
                        text: modelData
                        font.pixelSize: 12
                        font.bold: true
                        color: theme.textPrimary
                    }
                }
            }
        }

        // 表体
        ListView {
            id: loadListView
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            model: root.tableModel

            delegate: Row {
                width: loadListView.width
                height: theme.rowHeight
                spacing: 1

                required property int index
                required property var model

                // 名称列
                EditableCell {
                    width: 120
                    height: theme.rowHeight
                    text: model.name !== undefined ? model.name : ""
                    editable: root.editEnabled
                    selected: loadListView.currentIndex === index

                    onEditCommitted: function(value) {
                        if (root.tableModel) {
                            root.tableModel.setLoadName(index, value)
                        }
                    }
                }

                // 类型列
                Rectangle {
                    width: 120
                    height: theme.rowHeight
                    color: loadListView.currentIndex === index ? "#DCEEFF" : "transparent"

                    Label {
                        anchors.fill: parent
                        anchors.leftMargin: 8
                        verticalAlignment: Text.AlignVCenter
                        text: model.loadType !== undefined ? model.loadType : ""
                        color: "#888888"
                        font.pixelSize: 12
                        elide: Text.ElideRight
                    }
                }

                // 参数列
                EditableCell {
                    width: 160
                    height: theme.rowHeight
                    text: model.parameters !== undefined ? model.parameters : ""
                    editable: root.editEnabled
                    selected: loadListView.currentIndex === index

                    onEditCommitted: function(value) {
                        if (root.tableModel) {
                            root.tableModel.setLoadParameters(index, value)
                        }
                    }
                }

                // 作用对象列
                Rectangle {
                    width: 140
                    height: theme.rowHeight
                    color: loadListView.currentIndex === index ? "#DCEEFF" : "transparent"

                    Label {
                        anchors.fill: parent
                        anchors.leftMargin: 8
                        verticalAlignment: Text.AlignVCenter
                        text: model.affectedCount !== undefined ? (model.affectedCount + " 个对象") : ""
                        color: theme.textSecondary
                        font.pixelSize: 12
                    }
                }

                MouseArea {
                    anchors.fill: parent
                    z: -1
                    onClicked: loadListView.currentIndex = index
                }
            }
        }
    }
}
