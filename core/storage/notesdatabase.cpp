#include "notesdatabase.h"
#include "core/models/document.h"
#include <QSqlError>
#include <QSqlRecord>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDateTime>
#include <QDebug>

NotesDatabase* NotesDatabase::s_instance = nullptr;

NotesDatabase* NotesDatabase::instance()
{
    if (!s_instance)
        s_instance = new NotesDatabase();
    return s_instance;
}

NotesDatabase::NotesDatabase(QObject* parent)
    : QObject(parent)
{
}

NotesDatabase::~NotesDatabase()
{
    close();
}

bool NotesDatabase::initialize(const QString& dbPath)
{
    if (m_isOpen)
        return true;

    m_db = QSqlDatabase::addDatabase("QSQLITE", "notes_connection");
    m_db.setDatabaseName(dbPath);

    if (!m_db.open()) {
        qWarning() << "Failed to open database:" << m_db.lastError().text();
        return false;
    }

    // Enable foreign key enforcement
    QSqlQuery pragma(m_db);
    if (!pragma.exec("PRAGMA foreign_keys = ON;")) {
        qWarning() << "Failed to enable foreign keys:" << pragma.lastError().text();
    }

    m_isOpen = true;

    if (!createTables()) {
        qWarning() << "Failed to create tables";
        return false;
    }

    return true;
}

void NotesDatabase::close()
{
    if (m_isOpen) {
        m_db.close();
        m_isOpen = false;
    }
    QSqlDatabase::removeDatabase("notes_connection");
}

bool NotesDatabase::isOpen() const
{
    return m_isOpen;
}

bool NotesDatabase::createTables()
{
    if (!m_isOpen)
        return false;

    QStringList queries;
    queries << R"(
        CREATE TABLE IF NOT EXISTS notes (
            id TEXT PRIMARY KEY,
            title TEXT NOT NULL,
            created_at INTEGER NOT NULL,
            updated_at INTEGER NOT NULL,
            icon TEXT,
            cover_image TEXT,
            is_deleted INTEGER DEFAULT 0
        )
    )";
    queries << R"(
        CREATE TABLE IF NOT EXISTS blocks (
            id TEXT PRIMARY KEY,
            note_id TEXT NOT NULL,
            parent_id TEXT,
            type TEXT NOT NULL,
            order_index INTEGER NOT NULL,
            content TEXT,
            created_at INTEGER NOT NULL,
            updated_at INTEGER NOT NULL,
            FOREIGN KEY (note_id) REFERENCES notes(id) ON DELETE CASCADE,
            FOREIGN KEY (parent_id) REFERENCES blocks(id) ON DELETE CASCADE
        )
    )";
    queries << R"(
        CREATE TABLE IF NOT EXISTS images (
            block_id TEXT PRIMARY KEY,
            file_path TEXT,
            caption TEXT,
            width INTEGER,
            height INTEGER,
            FOREIGN KEY (block_id) REFERENCES blocks(id) ON DELETE CASCADE
        )
    )";
    queries << R"(
        CREATE TABLE IF NOT EXISTS code_blocks (
            block_id TEXT PRIMARY KEY,
            language TEXT,
            code TEXT,
            FOREIGN KEY (block_id) REFERENCES blocks(id) ON DELETE CASCADE
        )
    )";
    queries << R"(
        CREATE TABLE IF NOT EXISTS tables (
            block_id TEXT PRIMARY KEY,
            rows INTEGER,
            cols INTEGER,
            FOREIGN KEY (block_id) REFERENCES tables(id) ON DELETE CASCADE
        )
    )";
    queries << R"(
        CREATE TABLE IF NOT EXISTS table_cells (
            table_block_id TEXT,
            row_index INTEGER,
            col_index INTEGER,
            content TEXT,
            PRIMARY KEY (table_block_id, row_index, col_index),
            FOREIGN KEY (table_block_id) REFERENCES tables(block_id) ON DELETE CASCADE
        )
    )";

    QSqlQuery query(m_db);
    for (const QString& sql : queries) {
        if (!query.exec(sql)) {
            qWarning() << "Failed to create table:" << query.lastError().text();
            return false;
        }
    }

    qDebug() << "SQLite tables created successfully";
    return true;
}

// ----------------------------------------------------------------
// CRUD operations
// ----------------------------------------------------------------

bool NotesDatabase::saveDocument(const Document& doc)
{
    if (!m_isOpen) return false;

    QJsonObject docJson = doc.toJson();
    QString docId = docJson["id"].toString();

    m_db.transaction();

    QSqlQuery query(m_db);

    // Upsert the note row
    query.prepare(R"(
        INSERT INTO notes (id, title, created_at, updated_at, is_deleted)
        VALUES (?, ?, ?, ?, 0)
        ON CONFLICT(id) DO UPDATE SET
            title = excluded.title,
            updated_at = excluded.updated_at,
            is_deleted = 0
    )");
    query.addBindValue(docId);
    query.addBindValue(docJson["title"].toString());
    qint64 now = QDateTime::currentSecsSinceEpoch();
    query.addBindValue(now);
    query.addBindValue(now);

    if (!query.exec()) {
        qWarning() << "saveDocument note upsert failed:" << query.lastError().text();
        m_db.rollback();
        return false;
    }

    // Delete existing blocks for this note (clean re-insert)
    query.prepare("DELETE FROM blocks WHERE note_id = ?");
    query.addBindValue(docId);
    if (!query.exec()) {
        qWarning() << "saveDocument block delete failed:" << query.lastError().text();
        m_db.rollback();
        return false;
    }

    // Insert each block
    QJsonArray blocksArr = docJson["blocks"].toArray();
    for (const QJsonValue& val : blocksArr) {
        QJsonObject blockObj = val.toObject();

        query.prepare(R"(
            INSERT INTO blocks (id, note_id, parent_id, type, order_index, content, created_at, updated_at)
            VALUES (?, ?, ?, ?, ?, ?, ?, ?)
        )");
        query.addBindValue(blockObj["id"].toString());
        query.addBindValue(docId);
        query.addBindValue(blockObj["parentId"].toString());
        query.addBindValue(blockObj["blockType"].toString());
        query.addBindValue(blockObj["orderIndex"].toInt());

        // Store block data as compact JSON in the content column
        QJsonObject dataObj = blockObj["data"].toObject();
        query.addBindValue(QString::fromUtf8(
            QJsonDocument(dataObj).toJson(QJsonDocument::Compact)));

        query.addBindValue(now);
        query.addBindValue(now);

        if (!query.exec()) {
            qWarning() << "saveDocument block insert failed:" << query.lastError().text();
            m_db.rollback();
            return false;
        }
    }

    if (!m_db.commit()) {
        qWarning() << "saveDocument commit failed:" << m_db.lastError().text();
        return false;
    }

    return true;
}

Document* NotesDatabase::loadDocument(QUuid id)
{
    if (!m_isOpen) return nullptr;

    QString idStr = id.toString(QUuid::WithoutBraces);
    QSqlQuery query(m_db);

    // Load the note row
    query.prepare("SELECT id, title FROM notes WHERE id = ? AND is_deleted = 0");
    query.addBindValue(idStr);
    if (!query.exec() || !query.next()) {
        qDebug() << "loadDocument: note not found" << idStr;
        return nullptr;
    }

    QJsonObject docJson;
    docJson["id"]    = query.value(0).toString();
    docJson["title"] = query.value(1).toString();

    // Load blocks ordered by order_index
    query.prepare("SELECT id, parent_id, type, order_index, content "
                  "FROM blocks WHERE note_id = ? ORDER BY order_index");
    query.addBindValue(idStr);
    if (!query.exec()) {
        qWarning() << "loadDocument blocks query failed:" << query.lastError().text();
        return nullptr;
    }

    QJsonArray blocksArr;
    while (query.next()) {
        QJsonObject blockJson;
        blockJson["id"]         = query.value(0).toString();
        blockJson["parentId"]   = query.value(1).toString();
        blockJson["blockType"]  = query.value(2).toString();
        blockJson["orderIndex"] = query.value(3).toInt();

        // Parse block data from JSON content
        QString contentStr = query.value(4).toString();
        if (!contentStr.isEmpty()) {
            QJsonParseError err;
            QJsonDocument dataDoc = QJsonDocument::fromJson(contentStr.toUtf8(), &err);
            if (err.error == QJsonParseError::NoError) {
                blockJson["data"] = dataDoc.object();
            } else {
                qWarning() << "loadDocument: bad block JSON for" << blockJson["id"].toString()
                << err.errorString();
                blockJson["data"] = QJsonObject{{"type", "paragraph"}, {"text", ""}};
            }
        } else {
            blockJson["data"] = QJsonObject{{"type", "paragraph"}, {"text", ""}};
        }

        blocksArr.append(blockJson);
    }

    docJson["blocks"] = blocksArr;

    return Document::fromJson(docJson);
}

bool NotesDatabase::deleteDocument(QUuid id)
{
    if (!m_isOpen) return false;

    QString idStr = id.toString(QUuid::WithoutBraces);
    QSqlQuery query(m_db);

    // Soft delete — keeps data recoverable
    query.prepare("UPDATE notes SET is_deleted = 1, updated_at = ? WHERE id = ?");
    query.addBindValue(QDateTime::currentSecsSinceEpoch());
    query.addBindValue(idStr);

    if (!query.exec()) {
        qWarning() << "deleteDocument failed:" << query.lastError().text();
        return false;
    }

    return query.numRowsAffected() > 0;
}

QList<QUuid> NotesDatabase::allDocumentIds() const
{
    QList<QUuid> ids;
    if (!m_isOpen) return ids;

    QSqlQuery query(m_db);
    if (!query.exec("SELECT id FROM notes WHERE is_deleted = 0 ORDER BY updated_at DESC")) {
        qWarning() << "allDocumentIds failed:" << query.lastError().text();
        return ids;
    }

    while (query.next()) {
        QUuid id = QUuid::fromString(query.value(0).toString());
        if (!id.isNull())
            ids.append(id);
    }

    return ids;
}