import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../components"
import "../style"

CollapsiblePanel {
    id: root

    property var tableModel
    property bool editEnabled: true

    title: "工况表 (LoadCase Table)"
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
                model: ["工况名", "类别", "组合方法", "引用工况数"]

                Rectangle {
                    width: index === 0 ? 130 : (index === 2 ? 110 : 100)
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
            id: caseListView
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            model: root.tableModel

            delegate: Row {
                width: caseListView.width
                height: theme.rowHeight
                spacing: 1

                required property int index
                required property var model

                // 工况名列
                EditableCell {
                    width: 130
                    height: theme.rowHeight
                    text: model.name !== undefined ? model.name : ""
                    editable: root.editEnabled
                    selected: caseListView.currentIndex === index

                    onEditCommitted: function(value) {
                        if (root.tableModel) {
                            root.tableModel.setCaseName(index, value)
                        }
                    }
                }

                // 类别列
                Rectangle {
                    width: 100
                    height: theme.rowHeight
                    color: caseListView.currentIndex === index ? "#DCEEFF" : "transparent"

                    Label {
                        anchors.fill: parent
                        anchors.leftMargin: 8
                        verticalAlignment: Text.AlignVCenter
                        text: model.category !== undefined ? model.category : ""
                        color: theme.textSecondary
                        font.pixelSize: 12
                        elide: Text.ElideRight
                    }
                }

                // 组合方法列
                Rectangle {
                    width: 110
                    height: theme.rowHeight
                    color: caseListView.currentIndex === index ? "#DCEEFF" : "transparent"

                    Label {
                        anchors.fill: parent
                        anchors.leftMargin: 8
                        verticalAlignment: Text.AlignVCenter
                        text: model.method !== undefined ? model.method : ""
                        color: theme.textSecondary
                        font.pixelSize: 12
                        elide: Text.ElideRight
                    }
                }

                // 引用工况数列
                Rectangle {
                    width: 100
                    height: theme.rowHeight
                    color: caseListView.currentIndex === index ? "#DCEEFF" : "transparent"

                    Label {
                        anchors.fill: parent
                        anchors.leftMargin: 8
                        verticalAlignment: Text.AlignVCenter
                        text: model.entryCount !== undefined ? String(model.entryCount) : "0"
                        color: theme.textSecondary
                        font.pixelSize: 12
                    }
                }

                MouseArea {
                    anchors.fill: parent
                    z: -1
                    onClicked: caseListView.currentIndex = index
                }
            }
        }
    }
}
