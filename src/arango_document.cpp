
#include "arango_document_p.h"
#include "jsonqml/clients/settings_client.h"

namespace jsonqml {

extern std::shared_ptr<spdlog::logger> ui_logger;


ArangoDBDocumentPrivate::ArangoDBDocumentPrivate(DocumentType type, const QString &schema,
                                                 ArangoDatabasePrivate* parent_database):
    database(parent_database),
    doc_type(type),
    document_schema_name(schema.toStdString()),
    query_exist(false),
    dbdocument(nullptr),
    last_good_query(jsonio::DBQueryBase::emptyQuery())
{
    if(document_schema_name.empty()) {
        switch(doc_type) {
        case jsonqml::Json:
        case jsonqml::Resource:
        case jsonqml::Schema:
            break;
        case jsonqml::Vertex:
            document_schema_name = ArangoDatabase::anyVertexSchema();
            break;
        case jsonqml::Edge:
            document_schema_name = ArangoDatabase::anyEdgeSchema();
            break;
        }
    }
    is_dbdocument();
}

bool ArangoDBDocumentPrivate::is_dbdocument()
{
    if(!dbdocument && database->db_connected()) {
        const jsonio::DataBase& jsonio_db = database->jsonio_database(doc_type==jsonqml::Resource);

        switch(doc_type) {
        case jsonqml::Json:   // To do

        case jsonqml::Resource:  // schema document with resource database
        case jsonqml::Schema:  {
            jsonio::DBSchemaDocument* new_client = jsonio::DBSchemaDocument::newSchemaDocument(
                        jsonio_db, document_schema_name, ArangoDatabase::collectionFromSchema(document_schema_name));
            dbdocument.reset(new_client);
        }
            break;
        case jsonqml::Vertex: {
            jsonio::DBVertexDocument* new_client = jsonio::DBVertexDocument::newVertexDocument(
                        jsonio_db, document_schema_name);
            dbdocument.reset(new_client);
        }
            break;
        case jsonqml::Edge: {
            jsonio::DBEdgeDocument* new_client = jsonio::DBEdgeDocument::newEdgeDocument(
                        jsonio_db, document_schema_name);
            new_client->setMode(true);  // Do we need change it?
            dbdocument.reset(new_client);
        }
            break;
        }
    }
    return (dbdocument.get() != nullptr);
}

void ArangoDBDocumentPrivate::build_table()
{
    // wait jsonio mutex for finish
    std::this_thread::sleep_for(std::chrono::microseconds(2));
    last_good_query = dbdocument->currentQueryResult().condition();
    last_fields = new_header = dbdocument->currentQueryResult().query().fields();
    new_header.insert(new_header.begin(), "key");
    new_table.clear();
    for(const auto& line: dbdocument->currentQueryResult().queryResult())  {
        new_table.push_back(line.second);
        new_table.back().insert(new_table.back().begin(), line.first);
    }
}

std::string ArangoDBDocumentPrivate::schema_from_id(const std::string &doc_id) const
{
    std::string doc_schema;
    auto names = jsonio::split( doc_id, "/" );
    if(names.size()>1) {
        doc_schema = ArangoDatabase::schemaFromCollection(names.front());
    }
    return doc_schema;
}

void ArangoDBDocumentPrivate::set_query(const jsonio::DBQueryBase &query,
                                        const jsonio::values_t &query_fields)
{
    query_exist = true;
    if(is_dbdocument()) {
        dbdocument->setQuery(query, query_fields);
        build_table();
    }
}

void ArangoDBDocumentPrivate::set_default_query()
{
    set_query(jsonio::DBQueryBase::emptyQuery(), {});
}

void ArangoDBDocumentPrivate::update_query()
{
    if(is_dbdocument()) {
        if(dbdocument->queryDefined()) {
            //dbdocument->updateQuery();
            // query updated into jsonio lib
            build_table();
        }
        else {
            if(query_exist) {
                // restore query after delete document
                dbdocument->setQuery(jsonio::DBQueryBase::emptyQuery(), {});
                build_table();
            }
        }
    }
}

bool ArangoDBDocumentPrivate::update_schema(const std::string& new_schema)
{
    if(new_schema!=document_schema_name)  {
        document_schema_name = new_schema;
        if(is_dbdocument()) {
            dbdocument->resetSchema(document_schema_name, true);
            // updateQuery(); into reset schema
            build_table();
            return true;
        }
    }
    return false;
}

bool ArangoDBDocumentPrivate::clear_document()
{
    bool table_changed=false;
    if(is_dbdocument()) {
        if(dbdocument->queryDefined()) {
            new_table={};
            table_changed=true;
        }
        dbdocument.reset();
    }
    return table_changed;
}

void ArangoDBDocumentPrivate::execute_query(const jsonio::DBQueryBase &query,
                                            const jsonio::values_t &query_fields)
{
    if(is_dbdocument()) {
        new_table = dbdocument->downloadDocuments(query, query_fields);
        last_good_query = query;
        last_fields = new_header = query_fields;
    }
}

std::string ArangoDBDocumentPrivate::read(const std::string& doc_id)
{
    if(is_dbdocument()) {
        dbdocument->readDocument(doc_id);
        return dbdocument->getJson();
    }
    return "";
}

std::string ArangoDBDocumentPrivate::read_query(const std::string &doc_id)
{
    std::string ret_json;
    if(is_dbdocument()) {
        auto id_query = jsonio::DBQueryBase("RETURN DOCUMENT(\"" + doc_id +"\")", jsonio::DBQueryBase::qAQL);
        auto result = dbdocument->selectQuery(id_query);
        if(result.size()>0) {
            ret_json = result[0];
        }
    }
    return ret_json;
}

bool ArangoDBDocumentPrivate::save(const std::string &json_data, std::string& doc_id)
{
    if(is_dbdocument()) {
        doc_id = dbdocument->recFromJson(json_data, false);
        if(!dbdocument->existsDocument(doc_id)) {
            doc_id = dbdocument->createDocument(doc_id);
            return true;
        }
        else   {
            dbdocument->updateDocument(doc_id);
        }
    }
    return false;
}

void ArangoDBDocumentPrivate::remove(const std::string& doc_id)
{
    if(is_dbdocument()) {
        dbdocument->deleteDocument(doc_id);
    }
}

//------------------------------------------------------------------------------------


ArangoDBDocument::ArangoDBDocument(const ArangoDatabase* parent_database,
                                   ArangoDBDocumentPrivate* private_doc):
    QObject()
{
    arango_database = parent_database;
    impl_ptr.reset(private_doc);

    // reset client to new database allocation
    QObject::connect(arango_database, &ArangoDatabase::dbdriveChanged, this, &ArangoDBDocument::reloadQuery);
    QObject::connect(arango_database, &ArangoDatabase::errorConnection, this, &ArangoDBDocument::resetClient);

    QObject::connect(this, &ArangoDBDocument::updatedDocument, arango_database, &ArangoDatabase::afterUpdatedDocument);
    QObject::connect(this, &ArangoDBDocument::deletedDocument, arango_database, &ArangoDatabase::afterDeletedDocument);

    QObject::connect(this, &ArangoDBDocument::isException, &uiSettings(), &Preferences::setError);
}

ArangoDBDocument::~ArangoDBDocument()
{
}

void ArangoDBDocument::lastQueryResult(jsonio::DBQueryBase &query,
                                       jsonio::values_t &fields,
                                       jsonio::values_table_t &data)
{
    ui_logger->debug("lastQueryResult {}", impl_func()->last_good_query.queryString());
    QMutexLocker locker(&result_mutex);
    query = impl_func()->last_good_query;
    fields = std::move(impl_func()->new_header);
    impl_func()->new_header = {};
    data = std::move(impl_func()->new_table);
    impl_func()->new_table = {};
}

void ArangoDBDocument::resetClient()
{
    ui_logger->debug("resetClient");
    if(impl_func()->clear_document()) {
        emit finishedQuery();
    }
}

void ArangoDBDocument::resetSchema(std::string aschema_name)
{
    ui_logger->debug("resetSchema {}", aschema_name);
    emit started();
    QMutexLocker locker(&result_mutex);
    try {
        if(impl_func()->update_schema(aschema_name)) {
            emit finishedQuery(); // only if new schema
        }
    }
    catch(std::exception& e) {
        emit isException(e.what());
    }
    emit finished();
}

void ArangoDBDocument::reloadQuery()
{
    ui_logger->debug("reloadQuery {}", impl_func()->last_good_query.queryString());
    emit started();
    QMutexLocker locker(&result_mutex);
    try {
        impl_func()->update_query();
        emit finishedQuery();
    }
    catch(std::exception& e) {
        emit isException(e.what());
    }
    emit finished();
}

void ArangoDBDocument::changeQuery(jsonio::DBQueryBase query, std::vector<std::string> query_fields)
{
    emit started();
    QMutexLocker locker(&result_mutex);
    try {
        impl_func()->set_query(query, query_fields);
        emit finishedQuery();
    }
    catch(std::exception& e) {
        emit isException(e.what());
    }
    emit finished();
}

void ArangoDBDocument::executeQuery(jsonio::DBQueryBase query, std::vector<std::string> query_fields)
{
    emit started();
    QMutexLocker locker(&result_mutex);
    try {
        impl_func()->execute_query(query, query_fields);
        emit finishedQuery();
    }
    catch(std::exception& e) {
        emit isException(e.what());
    }
    emit finished();
}

void ArangoDBDocument::readDocument(std::string doc_id)
{
    try {
        auto json_data = impl_func()->read(doc_id);
        emit readedDocument(impl_func()->schema_from_id(doc_id), json_data);
        ui_logger->debug("read document {}", doc_id);
    }
    catch(std::exception& e) {
        emit isException(e.what());
    }
}

void ArangoDBDocument::readDocumentQuery(std::string doc_id)
{
    try {
        ui_logger->debug("read document query {}", doc_id);
        auto json_data = impl_func()->read_query(doc_id);
        emit readedDocument(impl_func()->schema_from_id(doc_id), json_data);
    }
    catch(std::exception& e) {
        emit isException(e.what());
    }
}

void ArangoDBDocument::updateDocument(std::string json_document)
{
    try {
        std::string doc_id;
        if(impl_func()->save(json_document, doc_id)) {
            // new doc
            emit updatedOid(doc_id);
        }
        else {
            emit updatedDocument(impl_func()->schema_from_id(doc_id), doc_id);
        }
        // !!! now reset all model, posible check only one row
        impl_func()->build_table();
        emit finishedQuery();
        ui_logger->debug("update document {}", doc_id);
    }
    catch(std::exception& e) {
        emit isException(e.what());
    }
}

void ArangoDBDocument::deleteDocument(std::string doc_id)
{
    try {
        impl_func()->remove(doc_id);
        emit deletedDocument(impl_func()->schema_from_id(doc_id), doc_id);
        ui_logger->debug("delete document {}", doc_id);
    }
    catch(std::exception& e) {
        emit isException(e.what());
    }
}

jsonio::DBQueryBase ArangoDBDocument::allEdgesQuery(const QString &id_vertex, const QString &edge_collections)
{
    return jsonio::DBVertexDocument::allEdgesQuery(id_vertex.toStdString(), edge_collections.toStdString());
}

jsonio::DBQueryBase ArangoDBDocument::outEdgesQuery(const QString &id_vertex, const QString &edge_collections)
{
    return jsonio::DBVertexDocument::outEdgesQuery(id_vertex.toStdString(), edge_collections.toStdString());
}

jsonio::DBQueryBase ArangoDBDocument::inEdgesQuery(const QString &id_vertex, const QString &edge_collections)
{
    return jsonio::DBVertexDocument::inEdgesQuery(id_vertex.toStdString(), edge_collections.toStdString());
}

}
