
#ifndef DBQUERYMODEL_H
#define DBQUERYMODEL_H

#include <QThread>
#include "jsonqml/models/select_model.h"
#include "jsonqml/arango_document.h"
#include "jsonio/dbquerybase.h"


namespace jsonqml {


/// \class DBQueryModel
/// \brief The DBQueryModel class provides a read-only data model for ArangoDB(noSQL) result sets.
///
/// DBQueryModel is a high-level interface for executing selection statements and traversing the result set.
/// It is built on top of the lower-level jsonio and can provide data to view classes such as QTableView.
class DBQueryModel: public SelectModel
{

    Q_OBJECT

    Q_PROPERTY(bool queryExecuting READ queryExecuting NOTIFY executingChange)

signals:
    void executingChange();
    /// Execute the query \a query for the given database connection.
    void CmExecuteQuery(const jsonio::DBQueryBase& query,const std::vector<std::string>& query_fields);

public slots:
    /// Refresh  documents keys list for table
    virtual void updateKeyList();

public:
    explicit DBQueryModel(DocumentType type, const QString& schema,
                          ArangoDatabase* db_client=&arango_db(),
                          QObject *parent = nullptr);

    virtual ~DBQueryModel();

    /// Execute the query \a query for the given database connection.
    /// uiSettings().lastError() can retrieve verbose information if there is an error in the query.
    virtual void executeQuery(const jsonio::DBQueryBase& query, const model_line_t& query_fields);

    /// Returns a reference to the const jsonio::DBQueryBase object associated with this model.
    const jsonio::DBQueryBase& query() const;
    /// Returns a reference to the query fields associated with this model.
    const model_line_t& queryFields() const;

    Q_INVOKABLE QString lastQuery() const;
    Q_INVOKABLE QStringList lastQueryFields() const;

    Q_INVOKABLE bool queryExecuting();

protected:
    jsonio::DBQueryBase data_query;

    void resetTable(model_table_t&& table_data, const model_line_t& header_data) override;

    ArangoDBDocument* dbdocument;
    bool query_executing;
    /// Thead moved db request to
    QThread db_thread;

    void set_executing(bool val);
    void reset_dbclient(DocumentType type, const QString& schema, ArangoDatabase* db_link);
};

}

#endif // DBQUERYMODEL_H
