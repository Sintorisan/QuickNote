import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Dialogs
import QtQuick.Layouts

import org.kde.kirigami as Kirigami

Item {
    id: root

    required property QtObject controller

    Layout.minimumWidth: Kirigami.Units.gridUnit * 18
    Layout.minimumHeight: Kirigami.Units.gridUnit * 24
    Layout.preferredWidth: Kirigami.Units.gridUnit * 24
    Layout.preferredHeight: Kirigami.Units.gridUnit * 30

    readonly property int margin: Kirigami.Units.smallSpacing * 2
    readonly property int radius: Kirigami.Units.cornerRadius
    readonly property color panelColor: Qt.rgba(
        Kirigami.Theme.backgroundColor.r,
        Kirigami.Theme.backgroundColor.g,
        Kirigami.Theme.backgroundColor.b,
        0.86
    )
    readonly property color softPanelColor: Qt.rgba(
        Kirigami.Theme.alternateBackgroundColor.r,
        Kirigami.Theme.alternateBackgroundColor.g,
        Kirigami.Theme.alternateBackgroundColor.b,
        0.72
    )

    function ask(title, text, callback) {
        confirmDialog.title = title;
        confirmDialog.text = text;
        confirmDialog.acceptedCallback = callback;
        confirmDialog.open();
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: root.margin
        spacing: Kirigami.Units.smallSpacing * 1.5

        RowLayout {
            Layout.fillWidth: true
            spacing: Kirigami.Units.smallSpacing * 1.5

            QQC2.Label {
                Layout.fillWidth: true
                text: i18n("Quick Note")
                font.weight: Font.DemiBold
                font.pointSize: Kirigami.Theme.defaultFont.pointSize + 1
                elide: Text.ElideRight
            }

            QQC2.ToolButton {
                icon.name: root.controller.keepPopupOpen ? "window-unpin" : "window-pin"
                text: i18n("Pin")
                display: QQC2.AbstractButton.IconOnly
                checkable: true
                checked: root.controller.keepPopupOpen
                QQC2.ToolTip.text: root.controller.keepPopupOpen ? i18n("Allow popup to close") : i18n("Keep popup open")
                QQC2.ToolTip.visible: hovered
                onClicked: root.controller.keepPopupOpen = !root.controller.keepPopupOpen
            }
        }

        QQC2.Pane {
            Layout.fillWidth: true
            padding: Kirigami.Units.smallSpacing

            background: Rectangle {
                color: root.softPanelColor
                radius: root.radius
                border.color: Kirigami.Theme.disabledTextColor
                border.width: 1
                opacity: 0.75
            }

            RowLayout {
                anchors.fill: parent
                spacing: Kirigami.Units.smallSpacing

                QQC2.Button {
                    Layout.fillWidth: true
                    text: i18n("Archive")
                    icon.name: "document-save"
                    enabled: root.controller.storageReady
                    onClicked: {
                        if (root.controller.isArchivedNote) {
                            if (root.controller.saveNow(i18n("Autosaved"))) {
                                root.ask(
                                    i18n("Finish archived note?"),
                                    i18n("Save changes and return to a fresh draft?"),
                                    function() { root.controller.finishArchivedNote(); }
                                );
                            }
                        } else if (root.controller.editorText.trim() === "") {
                            root.controller.archiveDraft();
                        } else {
                            root.ask(i18n("Archive this note?"), i18n("Archive this note?"), function() {
                                root.controller.archiveDraft();
                            });
                        }
                    }
                }

                QQC2.Button {
                    Layout.fillWidth: true
                    text: i18n("Open")
                    icon.name: "document-open"
                    enabled: root.controller.storageReady
                    onClicked: {
                        root.controller.refreshArchiveList();
                        archiveList.open();
                    }
                }

                QQC2.Button {
                    Layout.fillWidth: true
                    text: i18n("Clear")
                    icon.name: "edit-clear"
                    flat: true
                    enabled: root.controller.storageReady && (root.controller.isArchivedNote || editor.length > 0)
                    onClicked: root.ask(
                        root.controller.isArchivedNote ? i18n("Clear archived note") : i18n("Clear draft"),
                        root.controller.isArchivedNote
                            ? i18n("Clear this archived note? The file will remain, but its content will be emptied.")
                            : i18n("Clear this draft?"),
                        function() { root.controller.clearActiveNote(); }
                    )
                }

                QQC2.Button {
                    Layout.fillWidth: true
                    text: i18n("Delete")
                    icon.name: "edit-delete"
                    flat: true
                    enabled: root.controller.storageReady && (root.controller.isArchivedNote || editor.length > 0)
                    opacity: enabled ? 0.86 : 0.45
                    onClicked: root.ask(
                        root.controller.isArchivedNote ? i18n("Delete archived note") : i18n("Delete draft"),
                        root.controller.isArchivedNote ? i18n("Delete this archived note?") : i18n("Delete this draft?"),
                        function() { root.controller.deleteActiveNote(); }
                    )
                }
            }
        }

        QQC2.Pane {
            Layout.fillWidth: true
            Layout.fillHeight: true
            padding: Kirigami.Units.smallSpacing

            background: Rectangle {
                color: root.panelColor
                radius: root.radius
                border.color: Kirigami.Theme.disabledTextColor
                border.width: 1
                opacity: 0.9
            }

            QQC2.ScrollView {
                id: editorScrollView
                anchors.fill: parent
                visible: root.controller.storageReady
                clip: true
                QQC2.ScrollBar.vertical.policy: QQC2.ScrollBar.AsNeeded
                QQC2.ScrollBar.horizontal.policy: QQC2.ScrollBar.AlwaysOff

                QQC2.TextArea {
                    id: editor

                    width: editorScrollView.availableWidth
                    text: root.controller.editorText
                    placeholderText: i18n("Quick note")
                    wrapMode: TextEdit.Wrap
                    selectByMouse: true
                    leftPadding: Kirigami.Units.smallSpacing * 1.5
                    rightPadding: Kirigami.Units.smallSpacing * 1.5
                    topPadding: Kirigami.Units.smallSpacing * 1.5
                    bottomPadding: Kirigami.Units.smallSpacing * 1.5
                    background: Rectangle {
                        color: "transparent"
                        radius: root.radius
                    }
                    onTextChanged: root.controller.editorChanged(text)
                    Component.onCompleted: forceActiveFocus()
                }
            }

            ColumnLayout {
                anchors.centerIn: parent
                width: Math.min(parent.width, Kirigami.Units.gridUnit * 18)
                visible: !root.controller.storageReady
                spacing: Kirigami.Units.smallSpacing

                QQC2.Label {
                    Layout.fillWidth: true
                    text: root.controller.message || i18n("Choose a folder to store your notes.")
                    horizontalAlignment: Text.AlignHCenter
                    wrapMode: Text.WordWrap
                }

                QQC2.Button {
                    Layout.alignment: Qt.AlignHCenter
                    text: i18n("Choose Folder")
                    icon.name: "folder-open"
                    onClicked: folderDialog.open()
                }
            }
        }

        QQC2.Label {
            Layout.fillWidth: true
            text: root.controller.message !== "" ? root.controller.message : root.controller.footerText
            opacity: 0.58
            elide: Text.ElideRight
            font: Kirigami.Theme.smallFont
            leftPadding: Kirigami.Units.smallSpacing
            rightPadding: Kirigami.Units.smallSpacing
        }
    }

    FolderDialog {
        id: folderDialog
        title: i18n("Choose Notes Folder")
        onAccepted: root.controller.setNotesFolderFromUrl(selectedFolder)
    }

    ArchiveList {
        id: archiveList
        controller: root.controller
    }

    ConfirmDialog {
        id: confirmDialog
    }
}
