import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami

QQC2.Dialog {
    id: root

    required property QtObject controller

    title: i18n("Open Archived Note")
    modal: true
    standardButtons: QQC2.Dialog.Close
    width: Math.min(parent ? parent.width - Kirigami.Units.gridUnit : Kirigami.Units.gridUnit * 24, Kirigami.Units.gridUnit * 28)
    height: Math.min(parent ? parent.height - Kirigami.Units.gridUnit : Kirigami.Units.gridUnit * 28, Kirigami.Units.gridUnit * 28)

    contentItem: Item {
        implicitWidth: Kirigami.Units.gridUnit * 24
        implicitHeight: Kirigami.Units.gridUnit * 20

        QQC2.Label {
            anchors.centerIn: parent
            visible: archiveList.count === 0
            text: i18n("No archived notes yet.")
            opacity: 0.7
        }

        ListView {
            id: archiveList

            anchors.fill: parent
            model: root.controller.archiveNotes
            clip: true

            delegate: QQC2.ItemDelegate {
                required property var modelData

                width: ListView.view.width
                text: modelData.fileName
                onClicked: {
                    root.controller.openArchivedNote(modelData.activeNotePath);
                    root.close();
                }

                contentItem: ColumnLayout {
                    spacing: 0

                    QQC2.Label {
                        Layout.fillWidth: true
                        text: modelData.fileName
                        elide: Text.ElideRight
                    }

                    QQC2.Label {
                        Layout.fillWidth: true
                        text: modelData.preview ? modelData.preview : modelData.modified
                        opacity: 0.65
                        elide: Text.ElideRight
                        font: Kirigami.Theme.smallFont
                    }
                }
            }
        }
    }
}
