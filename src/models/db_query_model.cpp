
#include "jsonqml/models/db_query_model.h"
#include "jsonqml/clients/settings_client.h"
#include "jsonio/dbschemadoc.h"

namespace jsonqml {


DBQueryModel::DBQueryModel(DocumentType type,
                           const QString& schema,
                           ArangoDatabase* db_client,
                           QObject *parent)
    : SelectModel(parent),
      dbdocument(nullptr)
{
    reset_dbclient(type, schema, db_client);
}

DBQueryModel::~DBQueryModel()
{
    db_thread.quit();
    db_thread.wait();
}

void DBQueryModel::reset_dbclient(DocumentType type, const QString& schema,
                                  ArangoDatabase* db_client)
{
    qRegisterMetaType<std::vector<std::string> >("std::vector<std::string>");
    qRegisterMetaType<jsonio::DBQueryBase>("jsonio::DBQueryBase");
    qRegisterMetaType<std::string>("std::string");

    try {
        dbdocument = db_client->createDocument(type, schema);

        QObject::connect(dbdocument, &ArangoDBDocument::finishedQuery, this, &DBQueryModel::updateKeyList);
        QObject::connect(dbdocument, &ArangoDBDocument::finished, [&]() { set_executing(false); } );
        QObject::connect(dbdocument, &ArangoDBDocument::started, [&]() { set_executing(true); } );
        QObject::connect(this, &DBQueryModel::CmExecuteQuery, dbdocument, &ArangoDBDocument::executeQuery);

        // thread functions
        dbdocument->moveToThread(&db_thread);
        QObject::connect(&db_thread, &QThread::finished, dbdocument, &QObject::deleteLater);
        db_thread.start();
    }
    catch(std::exception& e) {
        uiSettings().setError(QString(e.what()));
    }
}

void DBQueryModel::updateKeyList()
{
    std::vector<std::string> new_query_fields;
    jsonio::values_table_t new_data_table;

    dbdocument->lastQueryResult(data_query, new_query_fields, new_data_table);
    resetTable(std::move(new_data_table), std::move(new_query_fields));
    set_executing(false);
}

void DBQueryModel::resetTable(model_table_t&& table_data,
                              const model_line_t& header_data)
{
    beginResetModel();
    table = std::move(table_data);
    header = header_data;
    endResetModel();
}

void DBQueryModel::executeQuery(const jsonio::DBQueryBase& query,
                           const std::vector<std::string>& query_fields)
{
    uiSettings().setError(QString());
    emit CmExecuteQuery(query, query_fields);
}

void DBQueryModel::set_executing(bool val)
{
    query_executing = val;
    emit executingChange();
}

const jsonio::DBQueryBase &DBQueryModel::query() const
{
    return  data_query;
}

const model_line_t &DBQueryModel::queryFields() const
{
    return header;
}

QString DBQueryModel::lastQuery() const
{
    return QString::fromStdString(data_query.queryString());
}

QStringList DBQueryModel::lastQueryFields() const
{
    QStringList new_list;
    std::transform(header.begin(), header.end(),
                   std::back_inserter(new_list),
                   [](const std::string &v){ return QString::fromStdString(v); });
    return new_list;
}

bool DBQueryModel::queryExecuting()
{
    return query_executing;
}

}
