import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Dialogs

import org.kde.kirigami as Kirigami
import "../org/sintori/quicknote/private" as QuickNotePrivate

Kirigami.FormLayout {
    id: root

    property string cfg_notesDirectory: plasmoid.configuration.notesDirectory
    property string statusText: ""

    function localPathFromUrl(folderUrl) {
        var text = String(folderUrl);
        if (text.indexOf("file://") === 0) {
            text = decodeURIComponent(text.substring("file://".length));
        }
        return text;
    }

    QuickNotePrivate.QuickNoteStore {
        id: store
    }

    QQC2.TextField {
        Kirigami.FormData.label: i18n("Notes folder:")
        text: root.cfg_notesDirectory
        readOnly: true
        placeholderText: i18n("Choose a folder")
    }

    QQC2.Button {
        text: i18n("Choose Folder")
        icon.name: "folder-open"
        onClicked: folderDialog.open()
    }

    QQC2.Label {
        visible: root.statusText !== ""
        text: root.statusText
        wrapMode: Text.WordWrap
    }

    FolderDialog {
        id: folderDialog

        title: i18n("Choose Notes Folder")
        onAccepted: {
            var path = root.localPathFromUrl(selectedFolder);
            var result = store.ensureStorage(path);
            if (result.ok) {
                root.cfg_notesDirectory = path;
                root.statusText = i18n("Storage is ready.");
            } else {
                root.statusText = result.error;
            }
        }
    }
}
