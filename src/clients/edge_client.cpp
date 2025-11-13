
#include "edge_client_p.h"
#include "jsonqml/clients/settings_client.h"
#include "jsonqml/models/schema_model.h"
#include "jsonqml/models/db_keys_model.h"
#include "jsonio/io_settings.h"
#include "jsonio/dbconnect.h"

namespace jsonqml {

EdgeClientPrivate::EdgeClientPrivate():
    VertexClientPrivate(),
    in_model(nullptr),
    out_model(nullptr)
{
}

void EdgeClientPrivate::init()
{
    uiSettings();
    qDebug() << "EdgeClientPrivate::init";
    schema_names_list = gen_schema_list();
    if(schema_names_list.empty()) { // not defined schema data
        return;
    }
    // set first schema as inital
    update_schema(new_list_default_schema());

    // alloc editor model
    json_tree_model.reset(new JsonSchemaModel(current_schema_name, header_names));
    in_model.reset(new DBQueryModel(Vertex, ""));
    out_model.reset(new DBQueryModel(Vertex, ""));

    // define keys model and start query
    update_keysmodel();
}

QStringList EdgeClientPrivate::gen_schema_list() const
{
    return ArangoDatabase::getEdgesList();;
}

void EdgeClientPrivate::update_jsonmodel()
{
    if(json_tree_model) {
        json_tree_model->setupModelData("", current_schema_name);
    }
    else {
        json_tree_model.reset(new JsonSchemaModel(current_schema_name, header_names));
        in_model.reset(new DBQueryModel(Vertex, ""));
        out_model.reset(new DBQueryModel(Vertex, ""));
    }
}

void EdgeClientPrivate::update_keysmodel()
{
    if(keys_model.get()==nullptr) {
        keys_model.reset(new DBKeysModel(Edge, current_schema_name));
        if(sorting_enabled()) {
            sort_proxy_model.reset(new SortFilterProxyModel());
            sort_proxy_model->setSourceModel(keys_model.get());
        }
    }
    else {
        keys_model->resetSchema(current_schema_name);
    }
    qDebug() << "update_keysmodel" << keys_model->rowCount() << " " << keys_model->columnCount();
}

void EdgeClientPrivate::set_edges_for(jsonio::DBQueryBase &&query)
{
    if(keys_model) {
        keys_model->setQuery(query, all_edges_fields);
    }
}

jsonio::DBQueryBase EdgeClientPrivate::make_vertex_query(std::string vertex_id) const
{
    return jsonio::DBQueryBase("RETURN DOCUMENT(\"" + vertex_id +"\")", jsonio::DBQueryBase::qAQL);
}

bool EdgeClientPrivate::set_json(const std::string& json_string, const QString& schema_name)
{
    if(json_tree_model) {
        json_tree_model->setupModelData(json_string, schema_name);
        json_tree_model->current_object().get_value_via_path<std::string>("_to", out_vertex_id, "");
        json_tree_model->current_object().get_value_via_path<std::string>("_from", in_vertex_id, "");
        if(in_model) {
            auto query_fields = all_vertex_fields;
            auto names = jsonio::split(in_vertex_id, "/");
            if(names.size()>1) {
                query_fields = ArangoDatabase::fieldsFromCollection(names.front());
            }
            in_model->executeQuery(make_vertex_query(in_vertex_id), query_fields);
        }
        if(out_model) {
            auto query_fields = all_vertex_fields;
            auto names = jsonio::split(out_vertex_id, "/");
            if(names.size()>1) {
                query_fields = ArangoDatabase::fieldsFromCollection(names.front());
            }
            out_model->executeQuery(make_vertex_query(out_vertex_id), query_fields);
        }
    }
    return true;
}


//---------------------------------------------------------------------

EdgeClient::EdgeClient(EdgeClientPrivate *impl, QObject *parent):
    VertexClient(impl, parent)
{
}

EdgeClient::EdgeClient(QObject *parent):
    EdgeClient(new EdgeClientPrivate(), parent)
{
    // virtual init into JsonClient
}

QAbstractItemModel *EdgeClient::inmodel()
{
    return impl_func()->in_model.get();
}

QAbstractItemModel *EdgeClient::outmodel()
{
    return impl_func()->out_model.get();
}

void jsonqml::EdgeClient::setIncomingEdges(QString vertex_id)
{
    if(!vertex_id.isEmpty()) {
        impl_func()->set_edges_for(ArangoDBDocument::inEdgesQuery(vertex_id, ""));
        emit keysModelChanged();
    }
}

void jsonqml::EdgeClient::setOutgoingEdges(QString vertex_id)
{
    if(!vertex_id.isEmpty()) {
        impl_func()->set_edges_for(ArangoDBDocument::outEdgesQuery(vertex_id, ""));
        emit keysModelChanged();
    }
}

QString EdgeClient::incomingVertex() const
{
    return QString::fromStdString(impl_func()->in_vertex_id);
}

QString EdgeClient::outgoingVertex() const
{
    return QString::fromStdString(impl_func()->out_vertex_id);
}

}
