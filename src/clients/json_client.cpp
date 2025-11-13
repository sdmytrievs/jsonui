#include "jsonqml/clients/json_client.h"
#include "jsonqml/clients/settings_client.h"
#include "jsonqml/models/json_model.h"
#include "jsonqml/models/schema_model.h"
#include "jsonio/io_settings.h"
#include "json_client_p.h"

namespace jsonqml {

const QString JsonClientPrivate::no_schema_name = "None";

JsonClientPrivate::JsonClientPrivate():
    json_tree_model(nullptr)
{
    header_names << "key" << "value" << "comment";
}

void JsonClientPrivate::init()
{
    schema_names_list = gen_schema_list();
    update_jsonmodel();
}

QStringList JsonClientPrivate::gen_schema_list() const
{
    auto std_schema_list = jsonio::ioSettings().Schema().getStructs(false);
    QStringList new_list;
    new_list.append(no_schema_name);
    std::transform(std_schema_list.begin(), std_schema_list.end(),
                   std::back_inserter(new_list),
                   [](const std::string &v){ return QString::fromStdString(v); });
    return new_list;
}

bool JsonClientPrivate::update_schema(QString new_schema)
{
    if(!schema_names_list.contains(new_schema)) {
        new_schema = no_schema_name;
    }
    if(current_schema_name != new_schema) {
        current_schema_name = new_schema;
        return true;
    }
    return false;
}

void JsonClientPrivate::update_jsonmodel()
{
    if(no_schema_model(current_schema_name)) {
        new_tree_model.reset(new JsonFreeModel(header_names));
    }
    else {
        new_tree_model.reset(new JsonSchemaModel(current_schema_name, header_names));
    }
    json_tree_model.swap(new_tree_model);
}

const jsonio::JsonBase &JsonClientPrivate::get_json(const QString&) const
{
    return json_tree_model->current_object();
}

bool JsonClientPrivate::set_json(const std::string& json_string, const QString& schema_name)
{
    //qDebug() << "Set json " << schema_name << " " << current_schema_name;
    if(!no_schema_model(current_schema_name) &&
            !no_schema_model(schema_name) &&
            schema_name!=current_schema_name) {
        return false;
    }
    update_schema(schema_name);
    json_tree_model->setupModelData(json_string, current_schema_name);
    return true;
}

//--------------------------------------------------------------------------

JsonClient::JsonClient(JsonClientPrivate* impl, QObject *parent):
    QObject(parent),
    impl_ptr(impl)
{
    impl_func()->init();

    QObject::connect(this, &JsonClient::schemaChanged, this, &JsonClient::setModelSchema);
    QObject::connect(&uiSettings(), &Preferences::scemasPathChanged, this, &JsonClient::updateSchemaList);
}

JsonClient::JsonClient(QObject *parent):
    JsonClient(new JsonClientPrivate, parent)
{
}

JsonClient::~JsonClient() {}

const QString &JsonClient::schema()
{
    return impl_func()->current_schema_name;
}

const QStringList &JsonClient::schemasList()
{
    return impl_func()->schema_names_list;
}

JsonBaseModel *JsonClient::jsonmodel()
{
    return impl_func()->json_tree_model.get();
}

const QStringList &JsonClient::headerNames()
{
    return impl_func()->header_names;
}

void JsonClient::setModelSchema()
{
    emit jsonModelAboutChanged();
    impl_func()->update_jsonmodel();
    emit jsonModelChanged();
    impl_func()->new_tree_model.reset();
}

void JsonClient::updateSchemaList()
{
    auto new_list = impl_func()->gen_schema_list();
    setSchemaList(new_list);
}

void JsonClient::setSchema(const QString &new_schema)
{
    if(impl_func()->update_schema(new_schema)) {
        qDebug() << "setSchema " << new_schema;
        emit schemaChanged();
    }
}

void JsonClient::setSchemaList(const QStringList &new_list)
{
    if(impl_func()->schema_names_list != new_list) {
        impl_func()->schema_names_list = new_list;
        emit listChanged();
        // set up no schema selected
        setSchema(impl_func()->new_list_default_schema());
    }
}

void JsonClient::readJson(const QString &url)
{
    if(url.isEmpty()) {
        return;
    }
    uiSettings().setError(QString());
    try {
        auto path = uiSettings().handleFileChosen(url);
        auto file_schema = schemaFromPath(path);
        jsonio::JsonFile file(path.toStdString());
        auto json_string = file.load_json();
        impl_func()->set_json(json_string, file_schema);
    }
    catch(std::exception& e) {
        uiSettings().setError(e.what());
    }
}

void JsonClient::saveJson(const QString &url)
{
    if(url.isEmpty()) {
        return;
    }
    uiSettings().setError(QString());
    try {
        auto path = uiSettings().handleFileChosen(url);
        jsonio::JsonFile file(path.toStdString());
        file.save(impl_func()->get_json());
    }
    catch(std::exception& e) {
        uiSettings().setError(e.what());
    }
}

//  static ------------------------------------------------------------

QString JsonClient::fileSchemaExt(const QString& schema_name, const QString& ext)
{
    QString ret("*.");
    if(!JsonClientPrivate::no_schema_model(schema_name)) {
        ret += schema_name+ ".";
    }
    return ret+ext;
}

QString JsonClient::schemaFromPath(const QString& file_path)
{
    auto pose = file_path.lastIndexOf('.');
    if(pose>0) {
        auto posb = file_path.lastIndexOf('.', pose-1);
        if(posb>0) {
            return file_path.mid(posb+1, pose-posb-1);
        }
    }
    return QString();
}

}
