#include "quicknotestore.h"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSaveFile>
#include <QTextStream>

namespace {
constexpr auto currentNote = "current.md";
constexpr auto archivePrefix = "archive/";
constexpr auto markdownSuffix = ".md";
}

QuickNoteStore::QuickNoteStore(QObject *parent)
    : QObject(parent)
{
}

QString QuickNoteStore::ping() const
{
    return QStringLiteral("QuickNoteStore ready");
}

QVariantMap QuickNoteStore::ensureStorage(const QString &notesDirectory) const
{
    QString error;
    if (!ensureStorageInternal(notesDirectory, &error)) {
        return errorResult(error);
    }

    QVariantMap result = okResult();
    result.insert(QStringLiteral("currentPath"), QString::fromLatin1(currentNote));
    result.insert(QStringLiteral("archivePath"), QStringLiteral("archive"));
    return result;
}

QVariantMap QuickNoteStore::readNote(const QString &notesDirectory, const QString &activeNotePath) const
{
    QString error;
    if (!ensureStorageInternal(notesDirectory, &error)) {
        return errorResult(error);
    }

    const ValidatedPath validated = validateNotePath(activeNotePath);
    if (!validated.ok) {
        return errorResult(validated.error);
    }

    const QString path = absoluteNotePath(notesDirectory, validated.relativePath);
    QFile file(path);
    if (!file.exists()) {
        return errorResult(QStringLiteral("Note file does not exist: %1").arg(validated.relativePath));
    }
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return errorResult(QStringLiteral("Could not read note: %1").arg(file.errorString()));
    }

    QVariantMap result = okResult();
    result.insert(QStringLiteral("content"), QString::fromUtf8(file.readAll()));
    result.insert(QStringLiteral("activeNotePath"), validated.relativePath);
    result.insert(QStringLiteral("fileName"), validated.fileName);
    result.insert(QStringLiteral("isArchived"), validated.kind == PathKind::Archive);
    return result;
}

QVariantMap QuickNoteStore::writeNote(const QString &notesDirectory, const QString &activeNotePath, const QString &content) const
{
    QString error;
    if (!ensureStorageInternal(notesDirectory, &error)) {
        return errorResult(error);
    }

    const ValidatedPath validated = validateNotePath(activeNotePath);
    if (!validated.ok) {
        return errorResult(validated.error);
    }

    if (!writeFileAtomically(absoluteNotePath(notesDirectory, validated.relativePath), content, &error)) {
        return errorResult(error);
    }

    QVariantMap result = okResult();
    result.insert(QStringLiteral("activeNotePath"), validated.relativePath);
    result.insert(QStringLiteral("fileName"), validated.fileName);
    result.insert(QStringLiteral("savedAt"), QDateTime::currentDateTime().toString(Qt::ISODate));
    return result;
}

QVariantMap QuickNoteStore::archiveDraft(const QString &notesDirectory, const QString &content) const
{
    if (content.trimmed().isEmpty()) {
        return errorResult(QStringLiteral("Nothing to archive."));
    }

    QString error;
    if (!ensureStorageInternal(notesDirectory, &error)) {
        return errorResult(error);
    }

    QDir archiveDir(QDir(notesDirectory).absoluteFilePath(QStringLiteral("archive")));
    const QString stamp = QDateTime::currentDateTime().toString(QStringLiteral("yyyy-MM-dd_HH-mm-ss"));
    QString fileName = stamp + QString::fromLatin1(markdownSuffix);
    int suffix = 2;
    while (archiveDir.exists(fileName)) {
        fileName = QStringLiteral("%1_%2%3").arg(stamp).arg(suffix++).arg(QString::fromLatin1(markdownSuffix));
    }

    const QString archivePath = archiveDir.absoluteFilePath(fileName);
    if (!writeFileAtomically(archivePath, content, &error)) {
        return errorResult(error);
    }
    if (!writeFileAtomically(QDir(notesDirectory).absoluteFilePath(QString::fromLatin1(currentNote)), QString(), &error)) {
        return errorResult(error);
    }

    QVariantMap result = okResult();
    result.insert(QStringLiteral("archiveFileName"), fileName);
    result.insert(QStringLiteral("archiveNotePath"), QStringLiteral("archive/%1").arg(fileName));
    result.insert(QStringLiteral("activeNotePath"), QString::fromLatin1(currentNote));
    result.insert(QStringLiteral("savedAt"), QDateTime::currentDateTime().toString(Qt::ISODate));
    return result;
}

QVariantMap QuickNoteStore::listArchivedNotes(const QString &notesDirectory) const
{
    QString error;
    if (!ensureStorageInternal(notesDirectory, &error)) {
        return errorResult(error);
    }

    QDir archiveDir(QDir(notesDirectory).absoluteFilePath(QStringLiteral("archive")));
    const QFileInfoList entries = archiveDir.entryInfoList(QStringList{QStringLiteral("*.md")}, QDir::Files, QDir::Time);

    QVariantList notes;
    notes.reserve(entries.size());
    for (const QFileInfo &entry : entries) {
        const QString fileName = entry.fileName();
        if (!isValidArchiveFileName(fileName)) {
            continue;
        }

        QVariantMap item;
        item.insert(QStringLiteral("fileName"), fileName);
        item.insert(QStringLiteral("activeNotePath"), QStringLiteral("archive/%1").arg(fileName));
        item.insert(QStringLiteral("modified"), entry.lastModified().toString(Qt::ISODate));
        item.insert(QStringLiteral("preview"), firstLinePreview(entry.absoluteFilePath()));
        notes.append(item);
    }

    QVariantMap result = okResult();
    result.insert(QStringLiteral("notes"), notes);
    return result;
}

QVariantMap QuickNoteStore::clearNote(const QString &notesDirectory, const QString &activeNotePath) const
{
    return writeNote(notesDirectory, activeNotePath, QString());
}

QVariantMap QuickNoteStore::deleteArchivedNote(const QString &notesDirectory, const QString &archiveFileName) const
{
    QString error;
    if (!ensureStorageInternal(notesDirectory, &error)) {
        return errorResult(error);
    }
    if (!isValidArchiveFileName(archiveFileName)) {
        return errorResult(QStringLiteral("Invalid archive filename."));
    }

    const QString archivePath = QDir(notesDirectory).absoluteFilePath(QStringLiteral("archive/%1").arg(archiveFileName));
    QFile file(archivePath);
    if (file.exists() && !file.remove()) {
        return errorResult(QStringLiteral("Could not delete archived note: %1").arg(file.errorString()));
    }
    if (!writeFileAtomically(QDir(notesDirectory).absoluteFilePath(QString::fromLatin1(currentNote)), QString(), &error)) {
        return errorResult(error);
    }

    QVariantMap result = okResult();
    result.insert(QStringLiteral("activeNotePath"), QString::fromLatin1(currentNote));
    return result;
}

bool QuickNoteStore::isValidNotePath(const QString &activeNotePath) const
{
    return validateNotePath(activeNotePath).ok;
}

QVariantMap QuickNoteStore::okResult()
{
    return QVariantMap{{QStringLiteral("ok"), true}, {QStringLiteral("error"), QString()}};
}

QVariantMap QuickNoteStore::errorResult(const QString &message)
{
    return QVariantMap{{QStringLiteral("ok"), false}, {QStringLiteral("error"), message}};
}

bool QuickNoteStore::writeFileAtomically(const QString &absolutePath, const QString &content, QString *error)
{
    QSaveFile file(absolutePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        if (error) {
            *error = QStringLiteral("Could not open note for writing: %1").arg(file.errorString());
        }
        return false;
    }

    file.write(content.toUtf8());
    if (!file.commit()) {
        if (error) {
            *error = QStringLiteral("Could not save note: %1").arg(file.errorString());
        }
        return false;
    }
    return true;
}

bool QuickNoteStore::ensureStorageInternal(const QString &notesDirectory, QString *error)
{
    if (notesDirectory.trimmed().isEmpty()) {
        if (error) {
            *error = QStringLiteral("Choose a notes folder first.");
        }
        return false;
    }

    QDir dir(notesDirectory);
    if (!dir.exists() && !dir.mkpath(QStringLiteral("."))) {
        if (error) {
            *error = QStringLiteral("Could not create notes folder.");
        }
        return false;
    }
    if (!dir.exists(QStringLiteral("archive")) && !dir.mkdir(QStringLiteral("archive"))) {
        if (error) {
            *error = QStringLiteral("Could not create archive folder.");
        }
        return false;
    }

    QFileInfo currentInfo(dir.absoluteFilePath(QString::fromLatin1(currentNote)));
    if (currentInfo.exists() && !currentInfo.isFile()) {
        if (error) {
            *error = QStringLiteral("current.md exists but is not a file.");
        }
        return false;
    }
    if (!currentInfo.exists()) {
        return writeFileAtomically(currentInfo.absoluteFilePath(), QString(), error);
    }

    return true;
}

QuickNoteStore::ValidatedPath QuickNoteStore::validateNotePath(const QString &activeNotePath)
{
    ValidatedPath result;
    const QString path = activeNotePath.trimmed();

    if (path == QString::fromLatin1(currentNote)) {
        result.ok = true;
        result.kind = PathKind::Current;
        result.relativePath = QString::fromLatin1(currentNote);
        result.fileName = QString::fromLatin1(currentNote);
        return result;
    }

    if (path.isEmpty()) {
        result.error = QStringLiteral("Missing note path.");
        return result;
    }
    if (QDir::isAbsolutePath(path) || path.startsWith(QLatin1Char('/')) || path.contains(QLatin1Char('\\'))) {
        result.error = QStringLiteral("Invalid note path: absolute paths and separators are not allowed.");
        return result;
    }
    if (path.contains(QStringLiteral(".."))) {
        result.error = QStringLiteral("Invalid note path: traversal is not allowed.");
        return result;
    }
    if (!path.startsWith(QString::fromLatin1(archivePrefix))) {
        result.error = QStringLiteral("Invalid note path.");
        return result;
    }

    const QString fileName = path.mid(QString::fromLatin1(archivePrefix).size());
    if (!isValidArchiveFileName(fileName)) {
        result.error = QStringLiteral("Invalid archive filename.");
        return result;
    }

    result.ok = true;
    result.kind = PathKind::Archive;
    result.relativePath = QStringLiteral("archive/%1").arg(fileName);
    result.fileName = fileName;
    return result;
}

bool QuickNoteStore::isValidArchiveFileName(const QString &fileName)
{
    if (fileName.isEmpty() || fileName == QStringLiteral(".") || fileName == QStringLiteral("..")) {
        return false;
    }
    if (!fileName.endsWith(QString::fromLatin1(markdownSuffix))) {
        return false;
    }
    if (fileName.contains(QLatin1Char('/')) || fileName.contains(QLatin1Char('\\')) || fileName.contains(QStringLiteral(".."))) {
        return false;
    }
    return true;
}

QString QuickNoteStore::absoluteNotePath(const QString &notesDirectory, const QString &activeNotePath)
{
    return QDir(notesDirectory).absoluteFilePath(activeNotePath);
}

QString QuickNoteStore::firstLinePreview(const QString &absolutePath)
{
    QFile file(absolutePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return QString();
    }

    QTextStream stream(&file);
    return stream.readLine().trimmed();
}
