#ifndef ARANGODB_H
#define ARANGODB_H

#include <map>
#include <string>
#include <QObject>
#include <QQmlEngine>
#include "jsonio/dbquerybase.h"

namespace jsonqml {


class ArangoDBDocument;
class ArangoDatabasePrivate;

enum DocumentType { Json, Schema, Vertex, Edge, Resource };


class ArangoDatabase : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString dbConnect READ dbConnect NOTIFY dbConnectChanged)
    Q_PROPERTY(bool dbConnected READ dbConnected NOTIFY dbdriveChanged)

    Q_PROPERTY(QString dbUrl READ dbUrl NOTIFY dbConnectChanged)
    Q_PROPERTY(QString dbName READ dbName NOTIFY dbConnectChanged)
    Q_PROPERTY(QString dbUser READ dbUser NOTIFY dbConnectChanged)
    Q_PROPERTY(QString dbUserPassword READ dbUserPassword NOTIFY dbConnectChanged)
    Q_PROPERTY(bool    dbAccess READ dbAccess NOTIFY dbConnectChanged)
    Q_PROPERTY(bool    isCreate READ isCreate NOTIFY dbConnectChanged)

signals:
    /// Notify about the changed database driver
    void dbdriveChanged();
    /// Notify about the last error that occurred in the database.
    void errorConnection(QString err);

    void dbConnectChanged();

    /// Refresh current model table if schema (updateKeyList)
    void cmReloadTable(std::string schema_name, std::string doc_id);

public slots:

    /// Notify about updating the document (may require updating of related ones)
    virtual void afterUpdatedDocument(std::string schema_name, std::string doc_id);
    /// Notify about document deletion (may require deletion of related ones)
    virtual void afterDeletedDocument(bool is_vertex, std::string schema_name, std::string doc_id);
    /// Notify about of loading graph documents (load records to different collections)
    virtual void afterLoadedGraph();

    /// Restore collection descriptors after scemasPathChanged
    void resetCollectionsList();
    /// Change db driver according new settings
    void ConnectFromSettings();

public:

    /// Default resourse Database name
    static std::string resources_database_name;

    /// Constructor
    explicit ArangoDatabase();

    /// Destructor
    ~ArangoDatabase();

    QString dbConnect();
    QString dbUrl();
    QString dbName();
    QString dbUser();
    QString dbUserPassword();
    bool    dbAccess();
    bool    isCreate();

    /// The current worker Database exists and checked connection for the current credentials group
    bool dbConnected() const;

    ArangoDBDocument* createDocument(DocumentType type, const QString& document_schema_name="");

    void getRootLists(const std::string& db_group,
                          QStringList& db_all_databases,
                          QStringList& db_all_users);


    static void defineCollection(const std::string& collection_name,
                                 const std::string& schema_name,
                                 jsonio::values_t&& query_fields)
    {
        user_defined_collections[collection_name]= {schema_name, std::move(query_fields)};
    }
    static QStringList getEdgesList();
    static QStringList getVertexesList();
    static QStringList getResourcesList();
    static QStringList getSchemasList();
    static jsonio::values_t getCollectionList(bool with_chema=true);
    static std::string collectionFromSchema(const std::string& schema_name);
    static std::string schemaFromCollection(const std::string& collection_name);
    static jsonio::values_t fieldsFromCollection(const std::string& collection_name);
    static std::string anyVertexSchema();
    static std::string anyEdgeSchema();

protected:

    struct CollectionData {
        std::string schema_name;
        jsonio::values_t default_query_fields;
    };
    static std::map<std::string, CollectionData> user_defined_collections;
    static std::map<std::string, CollectionData> defined_collections;
    jsonio::values_t make_query_fields(std::string schema_name) const;


    friend ArangoDatabase& arango_db();
    friend class ArangoDatabasePrivate;

    QScopedPointer<ArangoDatabasePrivate> impl_ptr;
    ArangoDatabasePrivate* impl_func()
              { return reinterpret_cast<ArangoDatabasePrivate *>(impl_ptr.get()); }
    const ArangoDatabasePrivate* impl_func() const
              { return reinterpret_cast<const ArangoDatabasePrivate *>(impl_ptr.get()); }

};

/// Default ArangoDatabase object
extern ArangoDatabase& arango_db();


}

#endif // ARANGODB_H
