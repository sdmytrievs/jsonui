#include "jsonqml/models/db_keys_model.h"
#include "jsonqml/clients/settings_client.h"

namespace jsonqml {


DBKeysModel::DBKeysModel(DocumentType type, const QString& schema,
                         ArangoDatabase* db_client,
                         QObject *parent):
    DBKeysModel(type, schema, jsonio::DBQueryBase::emptyQuery(),
                {}, db_client, parent)
{}

DBKeysModel::DBKeysModel(DocumentType type, const QString &schema,
                         const jsonio::DBQueryBase &query,
                         const model_line_t &query_fields,
                         ArangoDatabase *db_client,
                         QObject *parent):
    DBQueryModel(type, schema, db_client, parent)
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

    QObject::connect(this, &DBKeysModel::cmDeleteList, dbdocument, &ArangoDBDocument::deleteList);
    QObject::connect(this, &DBKeysModel::cmRestoreRecordsfromFile, dbdocument, &ArangoDBDocument::restoreRecordsfromFile);
    QObject::connect(this, &DBKeysModel::cmBackupRecordstoFile, dbdocument, &ArangoDBDocument::backupRecordstoFile);
    QObject::connect(this, &DBKeysModel::cmBackupGraphtoFile, dbdocument, &ArangoDBDocument::backupGraphtoFile);
    QObject::connect(this, &DBKeysModel::cmRestoreGraphfromFile, dbdocument, &ArangoDBDocument::restoreGraphfromFile);

    setQuery(query, query_fields);
}


DBKeysModel::~DBKeysModel()
{
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
    std::vector<std::string> new_list = transform2std(query_fields);
    updateFields(std::move(new_list));
}

void DBKeysModel::read(std::string doc_id)
{
    if(!doc_id.empty()) {
        emit cmRead(doc_id);
    }
}

void DBKeysModel::read(size_t row)
{
    read(get_id(row));
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

void DBKeysModel::remove(std::string doc_id)
{
    if(!doc_id.empty()) {
        emit cmDelete(doc_id);
    // !!! need update model table for all open
    }
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

void DBKeysModel::backup_records(QString file, std::vector<std::string> &&keys)
{
   emit cmBackupRecordstoFile(file, std::move(keys));
}

void DBKeysModel::restore_records(QString file)
{
    emit cmRestoreRecordsfromFile(file);
}

void DBKeysModel::backup_graph(QString file, std::vector<std::string> &&keys)
{
    emit cmBackupGraphtoFile(file, std::move(keys));
}

void DBKeysModel::restore_graph(QString file)
{
    emit cmRestoreGraphfromFile(file);
}

void DBKeysModel::delete_records(std::vector<std::string> &&keys)
{
    emit cmDeleteList(std::move(keys));
}

std::string DBKeysModel::get_id(size_t row) const
{
    if(row<table.size()) {
        return table[row][0];
    }
    return {};
}

}

