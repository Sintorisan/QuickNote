import QtQuick
import QtQuick.Controls as QQC2

QQC2.Dialog {
    id: root

    property string text: ""
    property var acceptedCallback: null

    modal: true
    standardButtons: QQC2.Dialog.Ok | QQC2.Dialog.Cancel

    QQC2.Label {
        text: root.text
        wrapMode: Text.WordWrap
        width: Math.min(parent ? parent.width : 320, 320)
    }

    onAccepted: {
        if (acceptedCallback) {
            acceptedCallback();
        }
        acceptedCallback = null;
    }

    onRejected: acceptedCallback = null
}
