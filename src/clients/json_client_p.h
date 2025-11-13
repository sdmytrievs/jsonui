#ifndef JSONCLIENT_P_H
#define JSONCLIENT_P_H

#include "jsonqml/clients/json_client.h"
#include "jsonqml/models/base_model.h"

namespace jsonqml {

class JsonClientPrivate
{

    Q_DISABLE_COPY_MOVE(JsonClientPrivate)

public:
    static bool no_schema_model(const QString& schema_name)
    {
        return (schema_name.isEmpty() || schema_name==no_schema_name);
    }

    explicit JsonClientPrivate();
    virtual ~JsonClientPrivate() {}

    virtual void init();

    virtual QStringList gen_schema_list() const;
    virtual QString new_list_default_schema() const
    {
        return (schema_names_list.empty()? no_schema_name: schema_names_list[0]);
    }
    virtual bool update_schema(QString new_schema);
    virtual void update_jsonmodel();

    virtual bool set_json(const std::string& json_string, const QString& schema_name="");
    virtual const jsonio::JsonBase& get_json(const QString& schema_name="") const;

protected:
    friend class JsonClient;

    static const QString no_schema_name;

    QStringList schema_names_list={no_schema_name};
    QString current_schema_name=no_schema_name;

    QStringList header_names;
    QSharedPointer<JsonBaseModel> json_tree_model;
    // protect qml error messages when swap models
    QSharedPointer<JsonBaseModel> new_tree_model;
};

}

#endif // JSONCLIENT_P_H
