#ifndef EDGECLIENT_P_H
#define EDGECLIENT_P_H

#include "jsonqml/clients/edge_client.h"
#include "vertex_client_p.h"

namespace jsonqml {

class EdgeClientPrivate : public VertexClientPrivate
{
    Q_DISABLE_COPY_MOVE(EdgeClientPrivate)

public:

    explicit EdgeClientPrivate(const jsonio::DBQueryBase& query=jsonio::DBQueryBase::emptyQuery(),
                               const model_line_t& query_fields={});
    virtual ~EdgeClientPrivate() {}

    void init() override;
    void init_keys(const QString& aschema) override;

    QStringList gen_schema_list() const override;
    QString new_list_default_schema() const  override
    {
        return (schema_names_list.empty()? "": schema_names_list[0]);
    }

    QAbstractItemModel* in_key_model() const
    {
        return in_model.get();
    }
    QAbstractItemModel* out_key_model() const
    {
        return out_model.get();
    }

    void update_jsonmodel() override; //?
    void update_keysmodel() override; //?

    void set_edges_for(jsonio::DBQueryBase&& query);
    bool set_json(const std::string& json_string, const QString& schema_name="") override;

    void set_multi_edge(bool val) {
        multi_edge_query = val;
    }

protected:
    friend class EdgeClient;

    QSharedPointer<DBQueryModel> in_model;
    QSharedPointer<DBQueryModel> out_model;
    std::string in_vertex_id;
    std::string out_vertex_id;
    bool multi_edge_query = false;
    jsonio::DBQueryBase in_query;
    jsonio::DBQueryBase out_query;

    jsonio::DBQueryBase make_vertex_query(const std::string& vertex_id) const;
    jsonio::DBQueryBase make_all_vertex_query(const std::string& vertex_collection) const;
    void in_out_execute_query(std::string vertex_id, DBQueryModel *model,
                              jsonio::DBQueryBase& old_query);
};

}

#endif // EDGECLIENT_P_H
