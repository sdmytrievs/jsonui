
#ifndef ARANGODOC_H
#define ARANGODOC_H

#include <QMutex>
#include "jsonqml/arango_database.h"

namespace jsonqml {

class ArangoDBDocumentPrivate;

/// \class ArangoDBDocument
/// \brief The ArangoDBDocument class object to work with database document.
class ArangoDBDocument: public QObject
{
    Q_OBJECT

signals:
    /// Notify the started any db request
    void started();
    /// Notify the finished any db request
    void finished();
    /// Report a request that has been executed with an exception
    void isException(const QString& msg);

    /// Notify the finished query process
    void finishedQuery();

    /// Open new arango document result
    void readedDocument(std::string schema_name, std::string doc_json);
    /// Result saving new Arango document
    void updatedOid(std::string doc_id);
    /// Notify about updating the document (may require updating of related ones)
    void updatedDocument(std::string schema_name, std::string doc_id);
    /// Notify about document deletion (may require deletion of related ones)
    void deletedDocument(bool is_vertex, std::string schema_name, std::string doc_id);
    /// Notify about of loading graph documents
    /// Load records to different collections (reload all open )
    void loadedGraph();

public slots:

    /// Reset current DB client
    virtual void resetClient();

    /// Reset current DB schema
    virtual void resetSchema(std::string aschema_name);
    /// Refresh current query data (updateKeyList)
    virtual void reloadQuery();
    /// Refresh current query table after update_line
    virtual void reloadTable(std::string schema_name, std::string doc_id);
    /// Update query
    virtual void changeQuery(jsonio::DBQueryBase query, std::vector<std::string> query_fields);
    /// Execute select query (do not change internal document selection)
    virtual void executeQuery(jsonio::DBQueryBase query, std::vector<std::string> query_fields);

    /// Open document
    virtual void readDocument(std::string doc_id);
    /// Open document (read document from any collection)
    virtual void readDocumentQuery(std::string doc_id);

    /// Save json Document to database
    virtual void updateDocument(std::string json_document);
    /// Delete keyDocument document from database
    virtual void deleteDocument(std::string doc_id);

    /// Delete documents from database
    virtual void deleteList(std::vector<std::string> keys);
    /// Read multiple records from file
    virtual void restoreRecordsfromFile(QString file);
    /// Write multiple records to file
    virtual void backupRecordstoFile(QString file, std::vector<std::string> keys);
    /// Write graph to file
    void backupGraphtoFile(QString file, std::vector<std::string> keys);
    /// Read multiple records from file
    void restoreGraphfromFile(QString file);

public:

    ~ArangoDBDocument();
    void lastQueryResult(jsonio::DBQueryBase& query,
                         jsonio::values_t& fields,
                         jsonio::values_table_t& data);

    /// Generate all edges query ( edge_collections - comma-separated edges collections list )
    static jsonio::DBQueryBase allEdgesQuery(const QString& id_vertex, const QString& edge_collections = "");
    /// Generate the outgoing edges query ( edge_collections - comma-separated edges collections list ).
    static jsonio::DBQueryBase outEdgesQuery(const QString& id_vertex, const QString& edge_collections = "");
    /// Generate the incoming edges query ( edge_collections - comma-separated edges collections list ).
    static jsonio::DBQueryBase inEdgesQuery(const QString& id_vertex, const QString& edge_collections = "");

protected:

    friend class ArangoDatabase;
    explicit ArangoDBDocument(const ArangoDatabase* parent_database,
                              ArangoDBDocumentPrivate* private_doc);

    QMutex result_mutex;
    const ArangoDatabase* arango_database=nullptr;

    friend class ArangoDBDocumentPrivate;

    QScopedPointer<ArangoDBDocumentPrivate> impl_ptr;
    ArangoDBDocumentPrivate* impl_func()
              { return reinterpret_cast<ArangoDBDocumentPrivate *>(impl_ptr.get()); }
    const ArangoDBDocumentPrivate* impl_func() const
              { return reinterpret_cast<const ArangoDBDocumentPrivate *>(impl_ptr.get()); }

};

}

#endif // ARANGODOC_H
