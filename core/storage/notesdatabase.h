#ifndef NOTESDATABASE_H
#define NOTESDATABASE_H

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

    bool saveDocument(const Document& doc);
    Document* loadDocument(QUuid id);
    bool deleteDocument(QUuid id);
    QList<QUuid> allDocumentIds() const;

private:
    explicit NotesDatabase(QObject* parent = nullptr);
    ~NotesDatabase() override;
    bool createTables();

    static NotesDatabase* s_instance;
    QSqlDatabase m_db;
    bool m_isOpen = false;
};

#endif // NOTESDATABASE_H
