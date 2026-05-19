pragma ComponentBehavior: Bound

import QtQuick

import org.kde.kirigami as Kirigami
import org.kde.plasma.core as PlasmaCore
import org.kde.plasma.plasmoid

PlasmoidItem {
    id: root

    Plasmoid.icon: "accessories-text-editor"
    Plasmoid.status: PlasmaCore.Types.ActiveStatus
    toolTipMainText: i18n("Quick Note")
    toolTipSubText: i18n("Open a quick Markdown note")
    switchWidth: Kirigami.Units.gridUnit * 18
    switchHeight: Kirigami.Units.gridUnit * 24
    hideOnWindowDeactivate: !noteController.keepPopupOpen

    NoteController {
        id: noteController

        notesDirectory: root.plasmoid.configuration.notesDirectory
        activeNotePath: root.plasmoid.configuration.activeNotePath || "current.md"
        keepPopupOpen: root.plasmoid.configuration.keepPopupOpen

        onNotesDirectoryChanged: root.plasmoid.configuration.notesDirectory = notesDirectory
        onActiveNotePathChanged: root.plasmoid.configuration.activeNotePath = activeNotePath
        onKeepPopupOpenChanged: root.plasmoid.configuration.keepPopupOpen = keepPopupOpen

        Component.onCompleted: {
            initialize();
        }
    }

    compactRepresentation: Kirigami.Icon {
        source: root.Plasmoid.icon
        active: compactMouseArea.containsMouse

        MouseArea {
            id: compactMouseArea
            anchors.fill: parent
            hoverEnabled: true
            onClicked: root.expanded = !root.expanded
        }
    }

    fullRepresentation: NoteSurface {
        controller: noteController
    }

    PlasmaCore.Action {
        id: toggleQuickNoteAction
        text: i18n("Toggle Quick Note")
        icon.name: "accessories-text-editor"
        onTriggered: root.expanded = !root.expanded
    }

    Plasmoid.contextualActions: [
        toggleQuickNoteAction
    ]

    Component.onCompleted: {
        Plasmoid.setInternalAction("toggleQuickNote", toggleQuickNoteAction);
    }
}
