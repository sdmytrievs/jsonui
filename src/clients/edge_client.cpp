
#include "edge_client_p.h"
#include "jsonqml/clients/settings_client.h"
#include "jsonqml/models/schema_model.h"
#include "jsonqml/models/db_keys_model.h"
//#include "jsonio/io_settings.h"
#include "jsonio/dbconnect.h"

namespace jsonqml {

const std::vector<std::string> all_edges_fields = {"_label", "_from", "_to", "_id"};
const std::vector<std::string> all_vertex_fields = {"_label", "_id"};


EdgeClientPrivate::EdgeClientPrivate(const jsonio::DBQueryBase& query,
                                     const model_line_t& query_fields):
    VertexClientPrivate(Edge, query, query_fields),
    in_model(nullptr),
    out_model(nullptr)
{
}

void EdgeClientPrivate::init()
{
    uiSettings();
    qDebug() << "EdgeClientPrivate::init";
    schema_names_list = gen_schema_list();
}

void EdgeClientPrivate::init_keys(const QString &aschema)
{
    if(schema_names_list.empty()) { // not defined schema data
        return;
    }
    auto def_schema= aschema;
    if(def_schema.isEmpty()) {
        // set first schema as inital
        def_schema = new_list_default_schema();
    }
    update_schema(def_schema);

    // alloc editor model
    in_query = jsonio::DBQueryBase::emptyQuery();
    out_query = jsonio::DBQueryBase::emptyQuery();
    json_tree_model.reset(new JsonSchemaModel(current_schema_name, header_names));
    in_model.reset(new DBQueryModel(Vertex, ""));
    out_model.reset(new DBQueryModel(Vertex, ""));

    // define keys model and start query
    update_keysmodel();
}

QStringList EdgeClientPrivate::gen_schema_list() const
{
    return ArangoDatabase::getEdgesList();
}

void EdgeClientPrivate::update_jsonmodel()
{
    if(json_tree_model) {
        json_tree_model->setupModelData("", current_schema_name);
    }
    else {
        in_query = jsonio::DBQueryBase::emptyQuery();
        out_query = jsonio::DBQueryBase::emptyQuery();
        json_tree_model.reset(new JsonSchemaModel(current_schema_name, header_names));
        in_model.reset(new DBQueryModel(Vertex, ""));
        out_model.reset(new DBQueryModel(Vertex, ""));
    }
}

void EdgeClientPrivate::update_keysmodel()
{
    if(keys_model.get()==nullptr) {
        keys_model.reset(new DBKeysModel(Edge, current_schema_name, init_query, init_fields));
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

jsonio::DBQueryBase EdgeClientPrivate::make_vertex_query(const std::string& vertex_id) const
{
    return jsonio::DBQueryBase("RETURN DOCUMENT(\"" + vertex_id +"\")", jsonio::DBQueryBase::qAQL);
}

jsonio::DBQueryBase EdgeClientPrivate::make_all_vertex_query(const std::string& vertex_collection) const
{
    std::string label = vertex_collection;
    label.pop_back();
    std::string AQL_query = "FOR u IN " + vertex_collection;
    AQL_query += "\nFILTER u._label == '" + label + "' ";
    return jsonio::DBQueryBase(AQL_query, jsonio::DBQueryBase::qAQL);
}

void EdgeClientPrivate::in_out_execute_query(std::string vertex_id, DBQueryModel* model,
                                             jsonio::DBQueryBase& old_query)
{
    if(model) {
        jsonio::DBQueryBase new_query;
        auto query_fields = all_vertex_fields;
        std::string vertex_collection;
        auto names = jsonio::split(vertex_id, "/");
        if(names.size()>1) {
            vertex_collection = names.front();
            query_fields = ArangoDatabase::fieldsFromCollection(vertex_collection);
        }
        if(multi_edge_query || vertex_collection.empty()) {
            new_query = make_vertex_query(vertex_id);
        }
        else {
            new_query = make_all_vertex_query(vertex_collection);
        }
        if(new_query!=old_query) {
            old_query = new_query;
            model->executeQuery(new_query, query_fields);
        }
    }
}

bool EdgeClientPrivate::set_json(const std::string& json_string, const QString& schema_name)
{
    if(json_tree_model) {
        json_tree_model->setupModelData(json_string, schema_name);
        json_tree_model->current_object().get_value_via_path<std::string>("_to", out_vertex_id, "");
        json_tree_model->current_object().get_value_via_path<std::string>("_from", in_vertex_id, "");
        in_out_execute_query(in_vertex_id, in_model.get(), in_query);
        in_out_execute_query(out_vertex_id, out_model.get(), out_query);
    }
    return true;
}

QString EdgeClient::vertex_schema_from_id(const QString& key_id)
{
    auto names = key_id.split("/");
    if(names.size()<=1)
        return "";
    auto label = names.front().toStdString();
    label.pop_back(); // delete last "s"
    return QString::fromStdString(jsonio::DataBase::getVertexName(label));
}

//---------------------------------------------------------------------

EdgeClient::EdgeClient(const QString& aschema, EdgeClientPrivate *impl, QObject *parent):
    VertexClient(aschema, impl, parent)
{
    if(impl_func()->in_model) {
        connect(impl_func()->in_model.get(), &DBKeysModel::executingChange,
                this, &VertexClient::executingChange, Qt::UniqueConnection);
        connect(impl_func()->in_model.get(), &DBKeysModel::updatedKeyList,
                this, &EdgeClient::updateInList, Qt::UniqueConnection);

    }
    if(impl_func()->out_model) {
        connect(impl_func()->out_model.get(), &DBKeysModel::executingChange,
                this, &VertexClient::executingChange, Qt::UniqueConnection);
        connect(impl_func()->out_model.get(), &DBKeysModel::updatedKeyList,
                this, &EdgeClient::updateOutList, Qt::UniqueConnection);
    }
}

EdgeClient::EdgeClient(const QString& aschema, QObject *parent):
    EdgeClient(aschema, new EdgeClientPrivate(), parent)
{
    // virtual init into JsonClient
}

EdgeClient::EdgeClient(const QString &aschema,
                       const jsonio::DBQueryBase &query,
                       const model_line_t &query_fields,
                       QObject *parent):
    EdgeClient(aschema, new EdgeClientPrivate(query, query_fields), parent)
{
}

EdgeClient::EdgeClient(const QString &aschema, bool in_edges,
                       const QString &vertex_id, QObject *parent):
    EdgeClient(aschema,
               new EdgeClientPrivate((in_edges? ArangoDBDocument::inEdgesQuery(vertex_id, ""):
                                                ArangoDBDocument::outEdgesQuery(vertex_id, "")),
                                      all_edges_fields),
               parent)
{
    impl_func()->set_multi_edge(true);
}

void EdgeClient::updateInList()
{
    auto editor_id = impl_func()->in_vertex_id;
    if(!editor_id.empty()) {
        emit updatedInList(editor_id);
    }
}

void EdgeClient::updateOutList()
{
    auto editor_id = impl_func()->out_vertex_id;
    if(!editor_id.empty()) {
        emit updatedOutList(editor_id);
    }
}

QAbstractItemModel *EdgeClient::inmodel()
{
    return impl_func()->in_model.get();
}

QAbstractItemModel *EdgeClient::outmodel()
{
    return impl_func()->out_model.get();
}

DBQueryModel *EdgeClient::intablemodel()
{
    return impl_func()->in_model.get();
}

DBQueryModel *EdgeClient::outtablemodel()
{
    return impl_func()->out_model.get();
}

void jsonqml::EdgeClient::setIncomingEdges(QString vertex_id)
{
    if(!vertex_id.isEmpty()) {
        impl_func()->set_multi_edge(true);
        impl_func()->set_edges_for(ArangoDBDocument::inEdgesQuery(vertex_id, ""));
        emit keysModelChanged();
    }
}

void jsonqml::EdgeClient::setOutgoingEdges(QString vertex_id)
{
    if(!vertex_id.isEmpty()) {
        impl_func()->set_multi_edge(true);
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
