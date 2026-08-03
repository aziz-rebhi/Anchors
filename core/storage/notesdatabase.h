#ifndef NOTESDATABASE_H
#define NOTESDATABASE_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QUuid>
#include <QList>

class Document; // forward declaration

class NotesDatabase : public QObject
{
    Q_OBJECT
public:
    static NotesDatabase* instance();
    bool initialize(const QString& dbPath);
    void close();
    bool isOpen() const;

    // Table creation
    bool createTables();

    // CRUD operations
    bool saveDocument(const Document& doc);
    Document* loadDocument(QUuid id);
    bool deleteDocument(QUuid id);
    QList<QUuid> allDocumentIds() const;

private:
    explicit NotesDatabase(QObject* parent = nullptr);
    ~NotesDatabase();

    static NotesDatabase* s_instance;
    QSqlDatabase m_db;
    bool m_isOpen = false;
};

#endif // NOTESDATABASE_H