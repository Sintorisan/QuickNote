#pragma once

#include <QObject>
#include <QVariantList>
#include <QVariantMap>

class QuickNoteStore : public QObject
{
    Q_OBJECT

public:
    explicit QuickNoteStore(QObject *parent = nullptr);

    Q_INVOKABLE QString ping() const;
    Q_INVOKABLE QVariantMap ensureStorage(const QString &notesDirectory) const;
    Q_INVOKABLE QVariantMap readNote(const QString &notesDirectory, const QString &activeNotePath) const;
    Q_INVOKABLE QVariantMap writeNote(const QString &notesDirectory, const QString &activeNotePath, const QString &content) const;
    Q_INVOKABLE QVariantMap archiveDraft(const QString &notesDirectory, const QString &content) const;
    Q_INVOKABLE QVariantMap listArchivedNotes(const QString &notesDirectory) const;
    Q_INVOKABLE QVariantMap clearNote(const QString &notesDirectory, const QString &activeNotePath) const;
    Q_INVOKABLE QVariantMap deleteArchivedNote(const QString &notesDirectory, const QString &archiveFileName) const;
    Q_INVOKABLE bool isValidNotePath(const QString &activeNotePath) const;

private:
    enum class PathKind {
        Current,
        Archive
    };

    struct ValidatedPath {
        bool ok = false;
        PathKind kind = PathKind::Current;
        QString relativePath;
        QString fileName;
        QString error;
    };

    static QVariantMap okResult();
    static QVariantMap errorResult(const QString &message);
    static bool writeFileAtomically(const QString &absolutePath, const QString &content, QString *error);
    static bool ensureStorageInternal(const QString &notesDirectory, QString *error);
    static ValidatedPath validateNotePath(const QString &activeNotePath);
    static bool isValidArchiveFileName(const QString &fileName);
    static QString absoluteNotePath(const QString &notesDirectory, const QString &activeNotePath);
    static QString firstLinePreview(const QString &absolutePath);
};
