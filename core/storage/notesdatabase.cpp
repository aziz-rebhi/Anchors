#include "notesdatabase.h"
#include "core/models/document.h"
#include <QSqlError>
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
            FOREIGN KEY (block_id) REFERENCES blocks(id) ON DELETE CASCADE
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

// ---- CRUD operations (stubs for now) ----

bool NotesDatabase::saveDocument(const Document& doc)
{
    Q_UNUSED(doc);
    qDebug() << "Saving document" << doc.id().toString();
    return true;
}

Document* NotesDatabase::loadDocument(QUuid id)
{
    Q_UNUSED(id);
    qDebug() << "Loading document" << id.toString();
    return nullptr;
}

bool NotesDatabase::deleteDocument(QUuid id)
{
    Q_UNUSED(id);
    return false;
}

QList<QUuid> NotesDatabase::allDocumentIds() const
{
    return QList<QUuid>();
}