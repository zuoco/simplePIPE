import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Frame {
    id: root

    property alias title: titleLabel.text
    property bool collapsed: false
    property bool fillHeight: false
    property int headerHeight: 34
    property color panelColor: "#FFFFFF"
    property color borderColor: "#E0E0E0"
    default property alias panelContent: contentContainer.data

    padding: 0

    background: Rectangle {
        color: root.panelColor
        border.color: root.borderColor
        border.width: 1
        radius: 6
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: root.headerHeight
            color: "transparent"

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 10
                anchors.rightMargin: 10

                Label {
                    id: arrowLabel
                    text: root.collapsed ? "▸" : "▾"
                    color: "#666666"
                    font.pixelSize: 12
                    Layout.alignment: Qt.AlignVCenter
                }

                Label {
                    id: titleLabel
                    color: "#333333"
                    font.pixelSize: 13
                    font.bold: true
                    Layout.fillWidth: true
                    elide: Text.ElideRight
                    Layout.alignment: Qt.AlignVCenter
                }
            }

            MouseArea {
                anchors.fill: parent
                onClicked: root.collapsed = !root.collapsed
                cursorShape: Qt.PointingHandCursor
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 1
            visible: !root.collapsed
            color: "#EAEAEA"
        }

        Item {
            id: contentHolder
            Layout.fillWidth: true
            Layout.fillHeight: root.fillHeight
            implicitHeight: root.collapsed ? 0 : contentContainer.implicitHeight
            visible: !root.collapsed

            Item {
                id: contentContainer
                anchors.fill: parent
                anchors.margins: 10
                implicitHeight: childrenRect.height
            }
        }
    }
}
