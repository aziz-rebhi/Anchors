#ifndef NOTESDATABASE_H
#define NOTESDATABASE_H

#include "core/crypto/SecureBuffer.h"
#include <QObject>
#include <QUuid>
#include <QList>
#include <QSqlDatabase>

#include <QSqlQuery>

class Document;

class NotesDatabase : public QObject {
    Q_OBJECT

public:
    static NotesDatabase* instance();

    bool initialize(const QString& dbPath);
    void close();
    bool isOpen() const;
    // Closes and re-opens the same database file. Used after the file has
    // been replaced on disk (backup import/export, full wipe) so that no
    // stale SQLite handle keeps pointing at the old inode.
    bool reset();

    // Note documents are stored as a single encrypted blob in the `data`
    // column. `key` is the session key; content only ever reaches disk as
    // ciphertext.
    bool saveDocument(const Document& doc, const SecureBuffer& key);
    Document* loadDocument(QUuid id, const SecureBuffer& key);
    bool deleteDocument(QUuid id);
    QList<QUuid> allDocumentIds() const;

private:
    explicit NotesDatabase(QObject* parent = nullptr);
    ~NotesDatabase() override;
    bool createTables();
    bool migrateSchema();
    static bool columnExists(const QSqlDatabase& db, const QString& table, const QString& column);

    static NotesDatabase* s_instance;
    QString m_dbPath;
    QSqlDatabase m_db;
    bool m_isOpen = false;
};

#endif // NOTESDATABASE_H
