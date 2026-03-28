import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Dialog {
    id: root

    modal: true
    title: "新建工程"
    standardButtons: Dialog.Ok | Dialog.Cancel

    property alias projectName: projectNameField.text

    contentItem: ColumnLayout {
        spacing: 8

        Label {
            text: "工程名"
        }

        TextField {
            id: projectNameField
            Layout.fillWidth: true
            placeholderText: "请输入工程名称"
            selectByMouse: true
        }
    }
}
