
#include "vertex_client_p.h"
#include "jsonqml/clients/settings_client.h"
#include "jsonqml/models/schema_model.h"
#include "jsonqml/models/db_keys_model.h"
#include "jsonio/dbconnect.h"


namespace jsonqml {

extern std::shared_ptr<spdlog::logger> ui_logger;

VertexClientPrivate::VertexClientPrivate():
    JsonClientPrivate(),
    keys_model(nullptr),
    sort_proxy_model(nullptr)
{
}

void VertexClientPrivate::init()
{
    uiSettings();
    qDebug() << "VertexClientPrivate::init";
    schema_names_list = gen_schema_list();
    if(schema_names_list.empty()) { // not defined schema data
        // try use  DBJsonDocument and JsonFree editor
        return;
    }
    // set first schema as inital
    update_schema(new_list_default_schema());
    // alloc editor model
    json_tree_model.reset(new JsonSchemaModel(current_schema_name, header_names));

    // define keys model and start query
    update_keysmodel();
}

QStringList VertexClientPrivate::gen_schema_list() const
{
    return ArangoDatabase::getVertexesList();
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
        keys_model.reset(new DBKeysModel(Vertex, current_schema_name));
        if(sorting_enabled()) {
            sort_proxy_model.reset(new SortFilterProxyModel());
            sort_proxy_model->setSourceModel(keys_model.get());
        }
    }
    else {
        keys_model->resetSchema(current_schema_name);
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
   // json_tree_model->current_object().set_value_via_path("_id", doc_id);
}

bool VertexClientPrivate::set_json(const std::string& json_string, const QString& schema_name)
{
    if(json_tree_model) {
        json_tree_model->setupModelData(json_string, schema_name);
    }
    return true;
}

//-------------------------------------------------------------------------

VertexClient::VertexClient(VertexClientPrivate *impl, QObject *parent):
    JsonClient(impl, parent)
{
    if(impl_func()->keys_model) {
            connect(impl_func()->keys_model.get(), &DBKeysModel::readedDocument,
                    this, &VertexClient::setEditorData, Qt::UniqueConnection);
            connect(impl_func()->keys_model.get(), &DBKeysModel::updatedOid,
                    this, &VertexClient::setEditorOid, Qt::UniqueConnection);
            connect(impl_func()->keys_model.get(), &DBKeysModel::executingChange,
                    this, &VertexClient::executingChange, Qt::UniqueConnection);
        }
}

VertexClient::VertexClient(QObject *parent):
    VertexClient(new VertexClientPrivate(), parent)
{
    // virtual init into JsonClient
}

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
    }
}

void VertexClient::setEditorData(std::string schema_name, std::string doc_json)
{
    try {
        impl_func()->set_editor_data(schema_name, doc_json);
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

QAbstractItemModel *VertexClient::keysmodel()
{
    return impl_func()->keys_list_model();
}

bool VertexClient::sortingEnabled()
{
    return impl_func()->sorting_enabled();
}

}
