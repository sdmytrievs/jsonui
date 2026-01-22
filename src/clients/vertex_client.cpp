
#include "vertex_client_p.h"
#include "jsonqml/clients/settings_client.h"
#include "jsonqml/models/schema_model.h"
#include "jsonqml/models/db_keys_model.h"
#include "jsonio/dbconnect.h"


namespace jsonqml {

extern std::shared_ptr<spdlog::logger> ui_logger;

VertexClientPrivate::VertexClientPrivate(DocumentType dtype,
                                         const jsonio::DBQueryBase& query,
                                         const model_line_t& query_fields):
    JsonClientPrivate(),
    doc_type(dtype),
    keys_model(nullptr),
    sort_proxy_model(nullptr),
    init_query(query),
    init_fields(query_fields)
{
}

void VertexClientPrivate::init()
{
    uiSettings();
    qDebug() << "VertexClientPrivate::init";
    schema_names_list = gen_schema_list();
}

void VertexClientPrivate::init_keys(const QString& aschema)
{
    qDebug() << "VertexClientPrivate::init_keys";
    if(schema_names_list.empty()) { // not defined schema data
        // try use  DBJsonDocument and JsonFree editor
        return;
    }
    auto def_schema= aschema;
    if(def_schema.isEmpty()) {
        // set first schema as inital
        def_schema = new_list_default_schema();
    }
    update_schema(def_schema);
    // alloc editor model
    json_tree_model.reset(new JsonSchemaModel(current_schema_name, header_names));

    // define keys model and start query
    update_keysmodel();
}

QStringList VertexClientPrivate::gen_schema_list() const
{
    QStringList lst;

    switch(doc_type) {
    case Json:
    case Schema:
        lst = ArangoDatabase::getSchemasList();
        break;
    case Vertex:
        lst = ArangoDatabase::getVertexesList();
        break;
    case Edge:
        lst = ArangoDatabase::getEdgesList();
        break;
    case Resource:
        lst = ArangoDatabase::getResourcesList();
        break;
    }
    return lst;
}

QAbstractItemModel *VertexClientPrivate::keys_list_model() const
{
    if(sorting_enabled()) {
        return sort_proxy_model.get();
    }
    return keys_model.get();
}

bool VertexClientPrivate::sorting_enabled() const
{
    return keys_table_mode&RowSortingEnabled;
}

bool VertexClientPrivate::update_schema(QString new_schema)
{
    if(!schema_names_list.contains(new_schema)) {
        new_schema = new_list_default_schema();
    }
    if(current_schema_name != new_schema) {
        current_schema_name = new_schema;
        return true;
    }
    return false;
}

void VertexClientPrivate::update_jsonmodel()
{
    if(json_tree_model) {
        json_tree_model->setupModelData("", current_schema_name);
    }
    else {
        json_tree_model.reset(new JsonSchemaModel(current_schema_name, header_names));
    }
}

void VertexClientPrivate::update_keysmodel()
{
    if(keys_model.get()==nullptr) {
        // could be zero, if schema list empty
        // alloc when refresh list
        keys_model.reset(new DBKeysModel(doc_type, current_schema_name, init_query, init_fields));
        if(sorting_enabled()) {
            sort_proxy_model.reset(new SortFilterProxyModel());
            sort_proxy_model->setSourceModel(keys_model.get());
        }
        qDebug() << "VertexClientPrivate rows " << keys_model->rowCount();
    }
    else {
        keys_model->resetSchema(current_schema_name);
        qDebug() << "VertexClientPrivate2 rows " << keys_model->rowCount();
    }
    qDebug() << "update_keysmodel";
}

void VertexClientPrivate::read_editor_data(int row)
{
    if(row>=0 && sorting_enabled() && sort_proxy_model) {
        row = sort_proxy_model->mapToSource(sort_proxy_model->index(row,0)).row();
    }

    if(row>=0 && keys_model && keys_model->rowCount()>row) {
        keys_model->read(row);
    }
    else {
        set_json("", current_schema_name);
    }
}

void VertexClientPrivate::set_editor_data(std::string schema_name, std::string doc_json)
{
    set_json(doc_json, QString::fromStdString(schema_name));
}

std::string VertexClientPrivate::id_from_editor()
{
    std::string vertex_id;
    if(json_tree_model) {
        json_tree_model->current_object().get_value_via_path("_id", vertex_id, vertex_id);
    }
    return vertex_id;
}

void VertexClientPrivate::id_to_editor(std::string doc_id)
{
    json_tree_model->setOid(doc_id);
}

bool VertexClientPrivate::set_json(const std::string& json_string, const QString& schema_name)
{
    if(json_tree_model) {
        json_tree_model->setupModelData(json_string, schema_name);
    }
    return true;
}

//-------------------------------------------------------------------------

VertexClient::VertexClient(const QString& aschema, VertexClientPrivate *impl, QObject *parent):
    JsonClient(impl, parent)
{
    impl_func()->init_keys(aschema);
    if(impl_func()->keys_model) {
            connect(impl_func()->keys_model.get(), &DBKeysModel::readedDocument,
                    this, &VertexClient::setEditorData, Qt::UniqueConnection);
            connect(impl_func()->keys_model.get(), &DBKeysModel::updatedOid,
                    this, &VertexClient::setEditorOid, Qt::UniqueConnection);
            connect(impl_func()->keys_model.get(), &DBKeysModel::executingChange,
                    this, &VertexClient::executingChange, Qt::UniqueConnection);
            connect(impl_func()->keys_model.get(), &DBKeysModel::updatedKeyList,
                    this, &VertexClient::updateKeyList, Qt::UniqueConnection);
        }
}

VertexClient::VertexClient(const QString& aschema, const QString& doc_id, QObject *parent):
    VertexClient(aschema, new VertexClientPrivate(Vertex), parent)
{
    start_doc_id = doc_id.toStdString();
    // virtual init into JsonClient
}

VertexClient::VertexClient(DocumentType dtype, const QString &aschema,
                           const jsonio::DBQueryBase &query,
                           const model_line_t &query_fields,
                           QObject *parent):
     VertexClient(aschema, new VertexClientPrivate(dtype, query, query_fields), parent)
{ }

VertexClient::VertexClient(DocumentType dtype, const QString &aschema, QObject *parent):
    VertexClient(aschema, new VertexClientPrivate(dtype), parent)
{ }

bool VertexClient::queryExecuting()
{
    if(impl_func()->keys_model) {
        return impl_func()->keys_model->queryExecuting();
    }
    return false;
}

void VertexClient::setModelSchema()
{
    impl_func()->update_jsonmodel();
    emit jsonModelChanged();

    impl_func()->update_keysmodel();
    emit keysModelChanged();

    if(impl_func()->keys_model) {
        connect(impl_func()->keys_model.get(), &DBKeysModel::readedDocument,
                this, &VertexClient::setEditorData, Qt::UniqueConnection);
        connect(impl_func()->keys_model.get(), &DBKeysModel::updatedOid,
                this, &VertexClient::setEditorOid, Qt::UniqueConnection);
        connect(impl_func()->keys_model.get(), &DBKeysModel::executingChange,
                this, &VertexClient::executingChange, Qt::UniqueConnection);
        connect(impl_func()->keys_model.get(), &DBKeysModel::updatedKeyList,
                this, &VertexClient::updateKeyList, Qt::UniqueConnection);
    }
}

void VertexClient::setEditorData(std::string schema_name, std::string doc_json)
{
    try {
        impl_func()->set_editor_data(schema_name, doc_json);
        emit editorChanged();
    }
    catch(std::exception& e) {
        uiSettings().setError(e.what());
    }
}

void VertexClient::setEditorOid(std::string doc_id)
{
    try {
        impl_func()->id_to_editor(doc_id);
    }
    catch(std::exception& e) {
        uiSettings().setError(e.what());
    }
}

void VertexClient::updateKeyList()
{
    std::string editor_id;
    if(!start_doc_id.empty()) {
        editor_id = start_doc_id;
        start_doc_id = "";
    }
    else {
        editor_id = impl_func()->id_from_editor();
    }
    if(editor_id.empty()) {
        readEditorData(0);
        editor_id = tableId(0);
    }
    if(!editor_id.empty()) {
        emit updatedKeyList(editor_id);
    }
}

void VertexClient::readEditorData(int row)
{
    if(row>=0) {
        impl_func()->read_editor_data(row);
    }
}

void VertexClient::readEditorId(QString vertex_id)
{
    if(impl_func()->keys_model) {
        impl_func()->keys_model->read_query(vertex_id.toStdString());
    }
}

QString VertexClient::editorId() {
   return QString::fromStdString(impl_func()->id_from_editor());
}

std::string VertexClient::tableId(size_t row) const
{
    if(impl_func()->keys_model) {
        return impl_func()->keys_model->get_id(row);
    }
    return {};
}

void VertexClient::deleteRecord(QString vertex_id)
{
    if(impl_func()->keys_model) {
        impl_func()->keys_model->remove(vertex_id.toStdString());
    }
}

void VertexClient::updateRecord()
{
    if(impl_func()->keys_model) {
        impl_func()->keys_model->save(impl_func()->get_json().dump());
    }
}

void VertexClient::backupRecords(QString file, std::vector<std::string> &&keys)
{
    if(impl_func()->keys_model) {
        impl_func()->keys_model->backup_records(file, std::move(keys));
    }
}

void VertexClient::restoreRecords(QString file)
{
    if(impl_func()->keys_model) {
        impl_func()->keys_model->restore_records(file);
    }
}

void VertexClient::backupGraph(QString file, std::vector<std::string> &&keys)
{
    if(impl_func()->keys_model) {
        impl_func()->keys_model->backup_graph(file, std::move(keys));
    }
}

void VertexClient::restoreGraph(QString file)
{
    if(impl_func()->keys_model) {
        impl_func()->keys_model->restore_graph(file);
    }
}

void VertexClient::deleteRecords(std::vector<std::string> &&keys)
{
    if(impl_func()->keys_model) {
        impl_func()->keys_model->delete_records(std::move(keys));
    }
}

QAbstractItemModel *VertexClient::keysmodel()
{
    return impl_func()->keys_list_model();
}

DBKeysModel *VertexClient::tablemodel()
{
    return impl_func()->keys_model.get();
}

bool VertexClient::sortingEnabled()
{
    return impl_func()->sorting_enabled();
}

}
