#include "jsonqml/models/db_keys_model.h"
#include "jsonqml/clients/settings_client.h"

namespace jsonqml {


DBKeysModel::DBKeysModel(DocumentType type, const QString& schema,
                         ArangoDatabase* db_client,
                         QObject *parent)
    :DBQueryModel(type, schema, db_client, parent)
{
    // tell the editor to change Oid
    QObject::connect(dbdocument, &ArangoDBDocument::updatedOid, this, &DBKeysModel::updatedOid);
    // tell the editor to change document
    QObject::connect(dbdocument, &ArangoDBDocument::readedDocument, this, &DBKeysModel::readedDocument);

    QObject::connect(this, &DBKeysModel::cmResetSchema, dbdocument, &ArangoDBDocument::resetSchema);
    QObject::connect(this, &DBKeysModel::cmReloadQuery, dbdocument, &ArangoDBDocument::reloadQuery);
    QObject::connect(this, &DBKeysModel::cmChangeQuery, dbdocument, &ArangoDBDocument::changeQuery);

    QObject::connect(this, &DBKeysModel::cmRead, dbdocument, &ArangoDBDocument::readDocument);
    QObject::connect(this, &DBKeysModel::cmReadQuery, dbdocument, &ArangoDBDocument::readDocumentQuery);
    QObject::connect(this, &DBKeysModel::cmUpdate, dbdocument, &ArangoDBDocument::updateDocument);
    QObject::connect(this, &DBKeysModel::cmDelete, dbdocument, &ArangoDBDocument::deleteDocument);

    // load default query
    setQuery(jsonio::DBQueryBase::emptyQuery(), {});
}


DBKeysModel::~DBKeysModel()
{
}

void DBKeysModel::updateKeyList()
{
    DBQueryModel::updateKeyList();
    // read first record after change all list
    read(0);
}

void DBKeysModel::updateQuery()
{
    uiSettings().setError(QString());
    emit cmReloadQuery();
}

void DBKeysModel::resetSchema(QString new_schema_name)
{
    uiSettings().setError(QString());
    emit cmResetSchema(new_schema_name.toStdString());
}

void DBKeysModel::setQuery(const jsonio::DBQueryBase& query,
                           const std::vector<std::string>& query_fields)
{
    uiSettings().setError(QString());
    emit cmChangeQuery(query, query_fields);
}

void DBKeysModel::updateQuery(const jsonio::DBQueryBase &aquery)
{
    setQuery(aquery, queryFields());
}

void DBKeysModel::updateFields(const std::vector<std::string> &query_fields)
{
    setQuery(query(), query_fields);
}

void DBKeysModel::updateQuery(const QString &query)
{
    updateQuery(jsonio::DBQueryBase(query.toStdString(), jsonio::DBQueryBase::qAQL));
}

void DBKeysModel::updateFields(const QStringList &query_fields)
{
    std::vector<std::string> new_list;
    std::transform(query_fields.begin(), query_fields.end(),
                   std::back_inserter(new_list),
                   [](const QString &v){ return v.toStdString(); });
    updateFields(std::move(new_list));
}

void DBKeysModel::read(size_t row)
{
    auto doc_id = get_id(row);
    if(!doc_id.empty()) {
        emit cmRead(doc_id);
    }
}

void DBKeysModel::read_query(std::string doc_id)
{
    if(!doc_id.empty()) {
        emit cmReadQuery(doc_id);
    }
}

void DBKeysModel::save(const std::string &json_data)
{
    // reload all model after save -> undifined updated/saved row number
    emit cmUpdate(json_data);
}

void DBKeysModel::remove(size_t row)
{
    auto doc_id = get_id(row);
    if(!doc_id.empty()) {
        emit cmDelete(doc_id);

        beginRemoveRows(QModelIndex(), row, row);
        table.erase(table.begin()+row);
        endRemoveRows();
    }
}

std::string DBKeysModel::get_id(size_t row) const
{
    if(row<table.size()) {
        return table[row][0];
    }
    return {};
}

}

