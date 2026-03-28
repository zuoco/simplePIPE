import QtQuick
import QtQuick.Controls

Rectangle {
    id: root

    property string text: ""
    property bool editable: true
    property bool selected: false
    property color readonlyColor: "#888888"
    property color normalTextColor: "#333333"

    signal editCommitted(string value)

    color: selected ? "#DCEEFF" : "transparent"
    border.width: 0

    Text {
        id: displayText
        anchors.fill: parent
        anchors.leftMargin: 8
        anchors.rightMargin: 8
        verticalAlignment: Text.AlignVCenter
        text: root.text
        color: root.editable ? root.normalTextColor : root.readonlyColor
        elide: Text.ElideRight
        visible: !editor.visible
    }

    TextField {
        id: editor
        anchors.fill: parent
        anchors.margins: 2
        visible: false
        text: root.text
        selectByMouse: true

        onAccepted: {
            root.editCommitted(text)
            visible = false
        }
        onEditingFinished: {
            if (visible) {
                root.editCommitted(text)
                visible = false
            }
        }
    }

    MouseArea {
        anchors.fill: parent
        acceptedButtons: Qt.LeftButton
        onDoubleClicked: {
            if (!root.editable) {
                return
            }
            editor.text = root.text
            editor.visible = true
            editor.forceActiveFocus()
            editor.selectAll()
        }
    }
}
