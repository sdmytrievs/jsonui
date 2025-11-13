
#ifndef DKEYSMODEL_H
#define DKEYSMODEL_H

#include "jsonqml/models/db_query_model.h"

namespace jsonqml {

/// \class DBKeysModel
/// \brief The DBKeysModel class provides a read-only data model for ArangoDB(noSQL) collection data sets.
///
/// DBKeysModel is a high-level interface for executing selection statements and traversing the result set.
/// It is built on top of the lower-level jsonio and can provide data to view classes such as QTableView.
class DBKeysModel: public DBQueryModel
{
    Q_OBJECT

signals:
   void updatedOid(std::string doc_id);
   /// Open new arango document result
   void readedDocument(std::string schema_name, std::string doc_json);

   /// Reset current DB schema
   void cmResetSchema(std::string aschema_name);
   /// Refresh current query data (updateKeyList)
   void cmReloadQuery();
   /// Update query
   void cmChangeQuery(jsonio::DBQueryBase query, std::vector<std::string> query_fields);

   /// Open document
   void cmRead(std::string doc_id);
   /// Open document (read document from any collection)
   void cmReadQuery(std::string doc_id);
   /// Save json Document to database
   void cmUpdate(std::string json_document);
   /// Delete keyDocument document from database
   void cmDelete(std::string doc_id);

public slots:
    /// Refresh  documents keys list for table
    void updateKeyList() override;

    void updateQuery();
    void resetSchema(QString new_schema_name);

public:
    explicit DBKeysModel(DocumentType type, const QString& schema,
                         ArangoDatabase* db_client=&arango_db(),
                         QObject *parent = nullptr);
    /// Destroys the object and frees any allocated resources.
    virtual ~DBKeysModel();

    Q_INVOKABLE void updateQuery(const QString& query);
    Q_INVOKABLE void updateFields(const QStringList& query_fields);
    void updateQuery(const jsonio::DBQueryBase& query);
    void updateFields(const model_line_t& query_fields);

    /// Set the query \a query for the given database connection \a  db.
    /// settings->lastError() can be used to retrieve verbose information if there
    ///    was an error setting the query.
    void setQuery(const jsonio::DBQueryBase& query,
                  const model_line_t& query_fields);

    Q_INVOKABLE std::string get_id(size_t row) const;
    Q_INVOKABLE void read(size_t row);
    Q_INVOKABLE void read_query(std::string doc_id);
    Q_INVOKABLE void save(const std::string& json_data);
    Q_INVOKABLE void remove(size_t row);
};

}

#endif // DKEYSMODEL_H
