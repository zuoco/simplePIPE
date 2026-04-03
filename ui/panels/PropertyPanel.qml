// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../components"
import "../style"

CollapsiblePanel {
    id: root

    property var propertyModel
    property bool editMode: true

    title: "属性面板"
    fillHeight: true

    property bool flashing: false
    property color flashColor: "#0078D4"

    function ensureExpandedAndFlash() {
        root.collapsed = false
        flashAnim.restart()
    }

    Theme {
        id: theme
    }

    borderColor: flashing ? flashColor : "#E0E0E0"

    SequentialAnimation {
        id: flashAnim
        running: false

        ScriptAction { script: root.flashing = true }
        PauseAnimation { duration: 150 }
        ScriptAction { script: root.flashing = false }
        PauseAnimation { duration: 150 }
    }

    ListView {
        id: list
        anchors.fill: parent
        clip: true
        model: root.propertyModel
        spacing: 1

        section.property: "group"
        section.criteria: ViewSection.FullString

        section.delegate: Rectangle {
            width: ListView.view.width
            height: 30
            color: "#F7FAFD"
            border.color: "#E4EDF6"
            border.width: 1

            Label {
                anchors.verticalCenter: parent.verticalCenter
                anchors.left: parent.left
                anchors.leftMargin: 8
                text: section
                font.bold: true
                color: theme.textPrimary
            }
        }

        delegate: Rectangle {
            required property string key
            required property string value
            required property bool editable

            width: ListView.view.width
            height: theme.rowHeight
            color: "#FFFFFF"

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 8
                anchors.rightMargin: 8

                Label {
                    text: key
                    color: theme.textPrimary
                    Layout.preferredWidth: 130
                    elide: Text.ElideRight
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    radius: 4
                    color: (editable && root.editMode) ? "#FAFCFF" : "transparent"
                    border.color: (editable && root.editMode) ? "#D6E6F5" : "transparent"
                    border.width: (editable && root.editMode) ? 1 : 0

                    Label {
                        visible: !(editable && root.editMode)
                        anchors.fill: parent
                        anchors.leftMargin: 8
                        anchors.rightMargin: 8
                        verticalAlignment: Text.AlignVCenter
                        text: value + (editable ? "" : " (auto)")
                        color: editable ? theme.textPrimary : theme.textSecondary
                        elide: Text.ElideRight
                    }

                    TextField {
                        visible: editable && root.editMode
                        anchors.fill: parent
                        anchors.margins: 2
                        text: value
                        selectByMouse: true
                        onEditingFinished: {
                            if (!root.propertyModel) {
                                return
                            }

                            const idx = root.propertyModel.index(index, 0)
                            const committed = root.propertyModel.setData(idx, text, 2)
                            if (!committed && root.propertyModel.refresh) {
                                root.propertyModel.refresh()
                            }
                        }
                    }
                }
            }
        }

        footer: Rectangle {
            width: ListView.view.width
            height: list.count === 0 ? 48 : 0
            color: "transparent"
            visible: list.count === 0

            Label {
                anchors.centerIn: parent
                text: "未选择对象"
                color: theme.textSecondary
            }
        }
    }
}
