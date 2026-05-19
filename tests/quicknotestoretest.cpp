#include "quicknotestore.h"

#include <QDir>
#include <QFile>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QTemporaryDir>
#include <QTest>

#include <memory>

class QuickNoteStoreTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void qmlImportCreatesStore();
    void validatesNotePathsStrictly();
    void ensuresStorageAndWritesAtomically();
    void archivesAndListsNewestFirst();
    void deletesArchivedNoteAndReturnsToDraft();
};

void QuickNoteStoreTest::qmlImportCreatesStore()
{
    QQmlEngine engine;
    engine.addImportPath(QStringLiteral(QUICKNOTE_QML_IMPORT_PATH));

    QQmlComponent component(&engine);
    component.setData(R"(
import "org/sintori/quicknote/private" as QuickNotePrivate
QuickNotePrivate.QuickNoteStore {}
)",
                      QUrl::fromLocalFile(QStringLiteral(QUICKNOTE_QML_IMPORT_PATH) + QStringLiteral("/ImportProbe.qml")));

    std::unique_ptr<QObject> object(component.create());
    QVERIFY2(object, qPrintable(component.errorString()));

    QString ping;
    QVERIFY(QMetaObject::invokeMethod(object.get(), "ping", Q_RETURN_ARG(QString, ping)));
    QCOMPARE(ping, QStringLiteral("QuickNoteStore ready"));
}

void QuickNoteStoreTest::validatesNotePathsStrictly()
{
    QuickNoteStore store;
    QVERIFY(store.isValidNotePath(QStringLiteral("current.md")));
    QVERIFY(store.isValidNotePath(QStringLiteral("archive/2026-05-19_08-42-10.md")));

    QVERIFY(!store.isValidNotePath(QString()));
    QVERIFY(!store.isValidNotePath(QStringLiteral("/tmp/note.md")));
    QVERIFY(!store.isValidNotePath(QStringLiteral("../current.md")));
    QVERIFY(!store.isValidNotePath(QStringLiteral("archive/../note.md")));
    QVERIFY(!store.isValidNotePath(QStringLiteral("archive/nested/note.md")));
    QVERIFY(!store.isValidNotePath(QStringLiteral("archive\\note.md")));
    QVERIFY(!store.isValidNotePath(QStringLiteral("archive/note.txt")));
}

void QuickNoteStoreTest::ensuresStorageAndWritesAtomically()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    QuickNoteStore store;
    QVariantMap result = store.ensureStorage(tempDir.path());
    QVERIFY2(result.value(QStringLiteral("ok")).toBool(), qPrintable(result.value(QStringLiteral("error")).toString()));
    QVERIFY(QFile::exists(QDir(tempDir.path()).absoluteFilePath(QStringLiteral("current.md"))));
    QVERIFY(QDir(tempDir.path()).exists(QStringLiteral("archive")));

    result = store.writeNote(tempDir.path(), QStringLiteral("current.md"), QStringLiteral("hello"));
    QVERIFY2(result.value(QStringLiteral("ok")).toBool(), qPrintable(result.value(QStringLiteral("error")).toString()));

    result = store.readNote(tempDir.path(), QStringLiteral("current.md"));
    QVERIFY2(result.value(QStringLiteral("ok")).toBool(), qPrintable(result.value(QStringLiteral("error")).toString()));
    QCOMPARE(result.value(QStringLiteral("content")).toString(), QStringLiteral("hello"));

    result = store.writeNote(tempDir.path(), QStringLiteral("archive/../bad.md"), QStringLiteral("bad"));
    QVERIFY(!result.value(QStringLiteral("ok")).toBool());
}

void QuickNoteStoreTest::archivesAndListsNewestFirst()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    QuickNoteStore store;
    QVariantMap result = store.archiveDraft(tempDir.path(), QStringLiteral("first\nbody"));
    QVERIFY2(result.value(QStringLiteral("ok")).toBool(), qPrintable(result.value(QStringLiteral("error")).toString()));
    const QString firstName = result.value(QStringLiteral("archiveFileName")).toString();
    QVERIFY(firstName.endsWith(QStringLiteral(".md")));

    QTest::qWait(1100);
    result = store.archiveDraft(tempDir.path(), QStringLiteral("second"));
    QVERIFY2(result.value(QStringLiteral("ok")).toBool(), qPrintable(result.value(QStringLiteral("error")).toString()));
    const QString secondName = result.value(QStringLiteral("archiveFileName")).toString();
    QVERIFY(secondName.endsWith(QStringLiteral(".md")));
    QVERIFY(firstName != secondName);

    result = store.listArchivedNotes(tempDir.path());
    QVERIFY2(result.value(QStringLiteral("ok")).toBool(), qPrintable(result.value(QStringLiteral("error")).toString()));
    const QVariantList notes = result.value(QStringLiteral("notes")).toList();
    QCOMPARE(notes.size(), 2);
    QCOMPARE(notes.at(0).toMap().value(QStringLiteral("fileName")).toString(), secondName);
    QCOMPARE(notes.at(1).toMap().value(QStringLiteral("fileName")).toString(), firstName);

    result = store.archiveDraft(tempDir.path(), QStringLiteral("   "));
    QVERIFY(!result.value(QStringLiteral("ok")).toBool());
    QCOMPARE(result.value(QStringLiteral("error")).toString(), QStringLiteral("Nothing to archive."));
}

void QuickNoteStoreTest::deletesArchivedNoteAndReturnsToDraft()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    QuickNoteStore store;
    QVariantMap result = store.archiveDraft(tempDir.path(), QStringLiteral("delete me"));
    QVERIFY2(result.value(QStringLiteral("ok")).toBool(), qPrintable(result.value(QStringLiteral("error")).toString()));
    const QString fileName = result.value(QStringLiteral("archiveFileName")).toString();
    QVERIFY(QFile::exists(QDir(tempDir.path()).absoluteFilePath(QStringLiteral("archive/%1").arg(fileName))));

    result = store.deleteArchivedNote(tempDir.path(), fileName);
    QVERIFY2(result.value(QStringLiteral("ok")).toBool(), qPrintable(result.value(QStringLiteral("error")).toString()));
    QCOMPARE(result.value(QStringLiteral("activeNotePath")).toString(), QStringLiteral("current.md"));
    QVERIFY(!QFile::exists(QDir(tempDir.path()).absoluteFilePath(QStringLiteral("archive/%1").arg(fileName))));

    result = store.deleteArchivedNote(tempDir.path(), QStringLiteral("../bad.md"));
    QVERIFY(!result.value(QStringLiteral("ok")).toBool());
}

QTEST_MAIN(QuickNoteStoreTest)

#include "quicknotestoretest.moc"
