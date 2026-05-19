import QtQuick
import "org/sintori/quicknote/private" as QuickNotePrivate

Item {
    id: root

    property string notesDirectory: ""
    property string activeNotePath: "current.md"
    property string editorText: ""
    property string autosaveStatus: "Setup"
    property string message: ""
    property string lastSavedAt: ""
    property bool storageReady: false
    property bool loading: false
    property bool keepPopupOpen: false
    property var archiveNotes: []

    readonly property bool isArchivedNote: activeNotePath.indexOf("archive/") === 0
    readonly property string activeFileName: isArchivedNote ? activeNotePath.substring("archive/".length) : "current.md"
    readonly property string noteStateLabel: isArchivedNote ? i18n("Archived") : i18n("Draft")
    readonly property string footerText: noteStateLabel + " · " + autosaveStatus + " · " + activeFileName

    signal archiveListUpdated()

    QuickNotePrivate.QuickNoteStore {
        id: store
    }

    Timer {
        id: autosaveTimer
        interval: 500
        repeat: false
        onTriggered: root.saveNow(i18n("Autosaved"))
    }

    onNotesDirectoryChanged: initialize()

    function initialize() {
        autosaveTimer.stop();
        if (notesDirectory.trim() === "") {
            storageReady = false;
            autosaveStatus = i18n("Setup");
            message = i18n("Choose a folder to store your notes.");
            editorText = "";
            return;
        }

        var ensured = store.ensureStorage(notesDirectory);
        if (!ensured.ok) {
            storageReady = false;
            autosaveStatus = i18n("Error");
            message = ensured.error;
            return;
        }

        storageReady = true;
        if (!store.isValidNotePath(activeNotePath)) {
            activeNotePath = "current.md";
            message = i18n("Missing note restored to current.md.");
        }
        loadActiveNote();
    }

    function setNotesFolderFromUrl(folderUrl) {
        var text = String(folderUrl);
        if (text.indexOf("file://") === 0) {
            text = decodeURIComponent(text.substring("file://".length));
        }
        notesDirectory = text;
        activeNotePath = "current.md";
        initialize();
    }

    function loadActiveNote() {
        if (!storageReady) {
            return;
        }

        var result = store.readNote(notesDirectory, activeNotePath);
        if (!result.ok && activeNotePath !== "current.md") {
            activeNotePath = "current.md";
            result = store.readNote(notesDirectory, activeNotePath);
        }
        if (!result.ok) {
            autosaveStatus = i18n("Error");
            message = result.error;
            return;
        }

        loading = true;
        editorText = result.content;
        loading = false;
        autosaveStatus = i18n("Autosaved");
        message = "";
    }

    function editorChanged(text) {
        if (loading || !storageReady) {
            return;
        }
        editorText = text;
        autosaveStatus = i18n("Unsaved");
        autosaveTimer.restart();
    }

    function saveNow(statusText) {
        if (!storageReady) {
            return false;
        }

        autosaveTimer.stop();
        autosaveStatus = i18n("Saving...");
        var result = store.writeNote(notesDirectory, activeNotePath, editorText);
        if (!result.ok) {
            autosaveStatus = i18n("Error");
            message = result.error;
            return false;
        }

        autosaveStatus = statusText || i18n("Autosaved");
        lastSavedAt = result.savedAt || "";
        message = "";
        return true;
    }

    function archiveDraft() {
        if (!storageReady) {
            return false;
        }
        if (isArchivedNote) {
            return false;
        }
        if (editorText.trim() === "") {
            message = i18n("Nothing to archive.");
            autosaveStatus = i18n("Autosaved");
            return false;
        }

        autosaveTimer.stop();
        autosaveStatus = i18n("Saving...");
        var result = store.archiveDraft(notesDirectory, editorText);
        if (!result.ok) {
            autosaveStatus = i18n("Error");
            message = result.error;
            return false;
        }

        activeNotePath = "current.md";
        loading = true;
        editorText = "";
        loading = false;
        autosaveStatus = i18n("Autosaved");
        message = i18n("Archived %1").arg(result.archiveFileName);
        return true;
    }

    function finishArchivedNote() {
        if (!storageReady || !isArchivedNote) {
            return false;
        }

        autosaveTimer.stop();
        autosaveStatus = i18n("Saving...");

        var saveResult = store.writeNote(notesDirectory, activeNotePath, editorText);
        if (!saveResult.ok) {
            autosaveStatus = i18n("Error");
            message = saveResult.error;
            return false;
        }

        var clearResult = store.clearNote(notesDirectory, "current.md");
        if (!clearResult.ok) {
            autosaveStatus = i18n("Error");
            message = clearResult.error;
            return false;
        }

        activeNotePath = "current.md";
        loading = true;
        editorText = "";
        loading = false;
        autosaveStatus = i18n("Autosaved");
        lastSavedAt = saveResult.savedAt || "";
        message = i18n("Archived note finished.");
        return true;
    }

    function refreshArchiveList() {
        if (!storageReady) {
            archiveNotes = [];
            archiveListUpdated();
            return;
        }
        saveNow(i18n("Autosaved"));
        var result = store.listArchivedNotes(notesDirectory);
        if (!result.ok) {
            autosaveStatus = i18n("Error");
            message = result.error;
            archiveNotes = [];
        } else {
            archiveNotes = result.notes;
        }
        archiveListUpdated();
    }

    function openArchivedNote(notePath) {
        if (!storageReady) {
            return;
        }
        saveNow(i18n("Autosaved"));
        activeNotePath = notePath;
        loadActiveNote();
    }

    function clearActiveNote() {
        if (!storageReady) {
            return;
        }
        autosaveTimer.stop();
        var result = store.clearNote(notesDirectory, activeNotePath);
        if (!result.ok) {
            autosaveStatus = i18n("Error");
            message = result.error;
            return;
        }

        loading = true;
        editorText = "";
        loading = false;
        autosaveStatus = i18n("Autosaved");
        message = "";
    }

    function deleteActiveNote() {
        if (!storageReady) {
            return;
        }

        if (!isArchivedNote) {
            clearActiveNote();
            return;
        }

        autosaveTimer.stop();
        var result = store.deleteArchivedNote(notesDirectory, activeFileName);
        if (!result.ok) {
            autosaveStatus = i18n("Error");
            message = result.error;
            return;
        }

        activeNotePath = "current.md";
        loading = true;
        editorText = "";
        loading = false;
        autosaveStatus = i18n("Autosaved");
        message = "";
    }
}
