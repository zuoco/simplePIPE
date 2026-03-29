import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../components"
import "../style"

CollapsiblePanel {
    id: root

    property var analysisModel

    title: "分析树"
    fillHeight: true

    Theme {
        id: theme
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 4
        spacing: 2

        // === 载荷 (Loads) 分组 ===
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 28
            color: loadFolderMouse.containsMouse ? "#F0F5FF" : "transparent"
            radius: 4

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 6
                spacing: 4

                Label {
                    text: loadSection.visible ? "▾" : "▸"
                    color: theme.textSecondary
                    font.pixelSize: 12
                }

                Label {
                    text: "🏋"
                    font.pixelSize: 14
                }

                Label {
                    text: "载荷 (Loads)"
                    color: theme.textPrimary
                    font.pixelSize: 13
                    font.bold: true
                    Layout.fillWidth: true
                }
            }

            MouseArea {
                id: loadFolderMouse
                anchors.fill: parent
                hoverEnabled: true
                onClicked: loadSection.visible = !loadSection.visible
            }
        }

        ListView {
            id: loadSection
            Layout.fillWidth: true
            Layout.preferredHeight: contentHeight
            visible: true
            interactive: false
            model: root.analysisModel ? root.analysisModel.loadNames : []

            delegate: Rectangle {
                width: loadSection.width
                height: 24
                color: loadItemMouse.containsMouse ? "#F6FAFE" : "transparent"

                Row {
                    anchors.fill: parent
                    anchors.leftMargin: 24
                    spacing: 6

                    Label {
                        anchors.verticalCenter: parent.verticalCenter
                        text: modelData
                        color: theme.textPrimary
                        font.pixelSize: 12
                    }
                }

                MouseArea {
                    id: loadItemMouse
                    anchors.fill: parent
                    hoverEnabled: true
                    onClicked: {
                        if (root.analysisModel) {
                            root.analysisModel.selectLoad(index)
                        }
                    }
                }
            }
        }

        // === 基本工况 (Load Cases) 分组 ===
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 28
            color: caseFolderMouse.containsMouse ? "#F0F5FF" : "transparent"
            radius: 4

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 6
                spacing: 4

                Label {
                    text: caseSection.visible ? "▾" : "▸"
                    color: theme.textSecondary
                    font.pixelSize: 12
                }

                Label {
                    text: "📋"
                    font.pixelSize: 14
                }

                Label {
                    text: "基本工况 (Load Cases)"
                    color: theme.textPrimary
                    font.pixelSize: 13
                    font.bold: true
                    Layout.fillWidth: true
                }
            }

            MouseArea {
                id: caseFolderMouse
                anchors.fill: parent
                hoverEnabled: true
                onClicked: caseSection.visible = !caseSection.visible
            }
        }

        ListView {
            id: caseSection
            Layout.fillWidth: true
            Layout.preferredHeight: contentHeight
            visible: true
            interactive: false
            model: root.analysisModel ? root.analysisModel.caseNames : []

            delegate: Rectangle {
                width: caseSection.width
                height: 24
                color: caseItemMouse.containsMouse ? "#F6FAFE" : "transparent"

                Row {
                    anchors.fill: parent
                    anchors.leftMargin: 24
                    spacing: 6

                    Label {
                        anchors.verticalCenter: parent.verticalCenter
                        text: modelData
                        color: theme.textPrimary
                        font.pixelSize: 12
                    }
                }

                MouseArea {
                    id: caseItemMouse
                    anchors.fill: parent
                    hoverEnabled: true
                    onClicked: {
                        if (root.analysisModel) {
                            root.analysisModel.selectCase(index)
                        }
                    }
                }
            }
        }

        // === 组合工况 (Combinations) 分组 ===
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 28
            color: comboFolderMouse.containsMouse ? "#F0F5FF" : "transparent"
            radius: 4

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 6
                spacing: 4

                Label {
                    text: comboSection.visible ? "▾" : "▸"
                    color: theme.textSecondary
                    font.pixelSize: 12
                }

                Label {
                    text: "🔗"
                    font.pixelSize: 14
                }

                Label {
                    text: "组合工况 (Combinations)"
                    color: theme.textPrimary
                    font.pixelSize: 13
                    font.bold: true
                    Layout.fillWidth: true
                }
            }

            MouseArea {
                id: comboFolderMouse
                anchors.fill: parent
                hoverEnabled: true
                onClicked: comboSection.visible = !comboSection.visible
            }
        }

        ListView {
            id: comboSection
            Layout.fillWidth: true
            Layout.preferredHeight: contentHeight
            visible: true
            interactive: false
            model: root.analysisModel ? root.analysisModel.combinationNames : []

            delegate: Rectangle {
                width: comboSection.width
                height: 24
                color: comboItemMouse.containsMouse ? "#F6FAFE" : "transparent"

                Row {
                    anchors.fill: parent
                    anchors.leftMargin: 24
                    spacing: 6

                    Label {
                        anchors.verticalCenter: parent.verticalCenter
                        text: modelData
                        color: theme.textPrimary
                        font.pixelSize: 12
                    }
                }

                MouseArea {
                    id: comboItemMouse
                    anchors.fill: parent
                    hoverEnabled: true
                    onClicked: {
                        if (root.analysisModel) {
                            root.analysisModel.selectCombination(index)
                        }
                    }
                }
            }
        }

        Item {
            Layout.fillHeight: true
        }
    }
}
