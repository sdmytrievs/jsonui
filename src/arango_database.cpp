#include "arango_document_p.h"
#include "jsonio/dbdriverarango.h"

namespace jsonqml {
extern std::shared_ptr<spdlog::logger> ui_logger;


std::string ArangoDatabase::resources_database_name = "resources";

std::map<std::string, ArangoDatabase::CollectionData> ArangoDatabase::user_defined_collections=
{
    {"queries", {{"Query"}, {"name", "qschema", "comment"}}},
    {"docpages", {{"DocPages"}, {"type", "name", "ext"}}},
    {"impexdefs", {{"ImpexFormat"}, {"direction", "name", "impexschema", "schema", "comment"}}}
};
std::map<std::string, ArangoDatabase::CollectionData> ArangoDatabase::defined_collections=
        ArangoDatabase::user_defined_collections;


ArangoDatabase& arango_db()
{
    static  ArangoDatabase data;
    return data;
}

//------------------------------------------------------------------

DatabaseSettings::DatabaseSettings()
{
    db_url = arangocpp::ArangoDBConnection::local_server_endpoint;
    db_name = arangocpp::ArangoDBConnection::local_server_database;
    db_user = arangocpp::ArangoDBConnection::local_server_username;
    db_user_password = arangocpp::ArangoDBConnection::local_server_password;
    db_access = false;
    db_create = true;
}

void DatabaseSettings::save_settings(const std::string &db_group, jsonio::JsonioSettings &jsonio_settings)
{
    auto db_section =jsonio_settings.section(jsonio::arangodb_section(db_group));
    db_section.setValue("DB_URL", db_url.toStdString());
    db_section.setValue("DBName", db_name.toStdString());
    db_section.setValue("DBUser", db_user.toStdString());
    db_section.setValue("DBUserPassword", db_user_password.toStdString());
    db_section.setValue("DBAccess", (db_access? "ro" : "rw"));
    db_section.setValue("DBCreate", db_create);

    jsonio_settings.setValue(jsonio::arangodb_section("UseArangoDBInstance"), db_group);
    jsonio_settings.section(Preferences::jsonui_section_name).setValue("CurrentDBConnection", db_group);
}

void DatabaseSettings::read_settings(const std::string &db_group, jsonio::JsonioSettings &jsonio_settings)
{
    auto db_section =jsonio_settings.section(jsonio::arangodb_section(db_group));

    if(db_group.find("Local")!=std::string::npos) {
        db_url = QString::fromStdString(db_section.value("DB_URL", std::string(arangocpp::ArangoDBConnection::local_server_endpoint)));
        db_name = QString::fromStdString(db_section.value("DBName", std::string(arangocpp::ArangoDBConnection::local_server_database)));
        db_user = QString::fromStdString(db_section.value("DBUser", std::string(arangocpp::ArangoDBConnection::local_server_username)));
        db_user_password = QString::fromStdString(db_section.value("DBUserPassword", std::string(arangocpp::ArangoDBConnection::local_server_password)));
        auto access = db_section.value("DBAccess", std::string("rw"));
        db_access = (access=="ro");
        db_create = db_section.value("DBCreate", true);
    }
    else {
        db_url = QString::fromStdString(db_section.value("DB_URL", std::string(arangocpp::ArangoDBConnection::remote_server_endpoint)));
        db_name = QString::fromStdString(db_section.value("DBName", std::string(arangocpp::ArangoDBConnection::remote_server_database)));
        db_user = QString::fromStdString(db_section.value("DBUser", std::string(arangocpp::ArangoDBConnection::remote_server_username)));
        db_user_password = QString::fromStdString(db_section.value("DBUserPassword", std::string(arangocpp::ArangoDBConnection::remote_server_password)));
        auto access = db_section.value("DBAccess", std::string("ro"));
        db_access = (access=="ro");
        db_create = db_section.value("DBCreate", false);
    }
}

arangocpp::ArangoDBConnection fromSettings(struct DatabaseSettings db_data)
{
    arangocpp::ArangoDBConnection connect_data;
    connect_data.serverUrl = db_data.db_url.toStdString();
    connect_data.databaseName = db_data.db_name.toStdString();
    connect_data.user.name = db_data.db_user.toStdString();
    connect_data.user.password = db_data.db_user_password.toStdString();
    connect_data.user.access = ( db_data.db_access ? "ro": "rw");
    return connect_data;
}

//------------------------------------------------------------------


ArangoDatabasePrivate::ArangoDatabasePrivate():
    jsonio_settings(jsonio::ioSettings()),
    jsonui_group(jsonio_settings.section(Preferences::jsonui_section_name)),
    work_database(nullptr),
    resourse_database(nullptr)
{
    jsonio_settings.set_module_level("jsonqml", "debug");
    init();
}

ArangoDatabasePrivate::~ArangoDatabasePrivate()
{

}

void ArangoDatabasePrivate::init()
{
    auto dbconnection = jsonui_group.value<std::string>("CurrentDBConnection","ArangoDBLocal");
    db_data.db_connect_current = QString::fromStdString(dbconnection);
    db_data.read_settings(dbconnection, jsonio_settings);
    std::string err_message;
    update_database(err_message);
}

void ArangoDatabasePrivate::update_root_client(const std::string& db_group)
{
    auto  root_connect= jsonio::getFromSettings(jsonio_settings.section(jsonio::arangodb_section(db_group)), true);
    root_client.reset(new arangocpp::ArangoDBRootClient(root_connect));
    // To do: Check existence and true credentials, if error set nullptr
}

void ArangoDatabasePrivate::get_system_lists(const std::string& db_group,
                                             QStringList& db_all_databases,
                                             QStringList& db_all_users)
{
    update_root_client(db_group);
    auto users = root_connect()->userNames();
    std::transform(users.begin(), users.end(),
                   std::back_inserter(db_all_users),
                   [](const std::string &v){ return QString::fromStdString(v); });
    auto dbnames = root_connect()->databaseNames();
    std::transform(dbnames.begin(), dbnames.end(),
                   std::back_inserter(db_all_databases),
                   [](const std::string &v){ return QString::fromStdString(v); });
}

void ArangoDatabasePrivate::create_collection_if_no_exist(jsonio::AbstractDBDriver* db_driver)
{
    //for(const auto& vertex_col: jsonio::DataBase::usedVertexCollections()) {
    //    db_driver->create_collection(vertex_col.second, "vertex");
    //}
    for(const auto& edge_col: jsonio::DataBase::usedEdgeCollections()) {
        db_driver->create_collection(edge_col.second, "edge");
    }
}

void ArangoDatabasePrivate::update_resource_database()
{
    try {
        arangocpp::ArangoDBConnection connect_data = fromSettings(db_data);
        connect_data.databaseName = ArangoDatabase::resources_database_name;
        auto db_driver = std::make_shared<jsonio::ArangoDBClient>(connect_data);
        if(!db_driver->connected()) {
            resourse_database.reset();
            ui_logger->error("Resource db connect error {}", db_driver->status());
            return;
        }
        if(resourse_database.get() == nullptr) {
            resourse_database.reset(new jsonio::DataBase(db_driver));
        }
        else {
            resourse_database->updateDriver(db_driver);
        }
    }
    catch(std::exception& e) {
        resourse_database.reset();
        throw;
    }
}

void ArangoDatabasePrivate::update_work_database(const arangocpp::ArangoDBConnection& db_link, std::string& error_message)
{
    try {
        if(db_data.db_create && !root_connect()->existDatabase(db_link.databaseName)) {
            root_connect()->createDatabase(db_link.databaseName, {db_link.user});
        }

        auto db_driver = std::make_shared<jsonio::ArangoDBClient>(db_link);
        if(!db_driver->connected()) {
            work_database.reset();
            error_message += db_driver->status();
            ui_logger->error("Database connect error {}", db_driver->status());
            return;
        }

        create_collection_if_no_exist(db_driver.get());
        if(work_database.get() == nullptr) {
            work_database.reset(new jsonio::DataBase(db_driver));
        }
        else {
            work_database->updateDriver(db_driver);
        }
    }
    catch(std::exception& e) {
        work_database.reset();
        throw;
    }
}

bool ArangoDatabasePrivate::update_database(std::string& error_message)
{
    try {
        error_message.clear();
        arangocpp::ArangoDBConnection db_connect = fromSettings(db_data);

        if(work_database.get()!=nullptr) { // compare with old
            jsonio::ArangoDBClient* old_db_link = dynamic_cast<jsonio::ArangoDBClient*>(work_database->theDriver());
            if(old_db_link && db_connect==old_db_link->connect_data()) {
                return false;  // nothing changed
            }
        }

        if(db_data.db_create) { // try link as root
            update_root_client(db_data.db_connect_current.toStdString());
        }
        update_work_database(db_connect, error_message);
        if(resourse_database) { // change only if used before
          update_resource_database();
        }

        ui_logger->info("Update database connect to: {} {}", db_connect.serverUrl, db_connect.databaseName);
        return true;
    }
    catch(std::exception& e) {
        error_message += e.what();
        ui_logger->error("Exception when updating database {}", e.what());
    }
    return true; // clear connection
}

//----------------------------------------------------------------------------

ArangoDatabase::ArangoDatabase()
{
    impl_ptr.reset(new ArangoDatabasePrivate());
    resetCollectionsList();
    qDebug() << "ArangoDatabase construct";
}

ArangoDatabase::~ArangoDatabase()
{
    qDebug() << "ArangoDatabase destruct";
}

bool ArangoDatabase::dbConnected() const
{
    return impl_func()->db_connected();
}

void ArangoDatabase::afterUpdatedDocument(std::string schema_name, std::string doc_id)
{
    Q_UNUSED(schema_name);
    Q_UNUSED(doc_id);
}

void ArangoDatabase::afterDeletedDocument(std::string schema_name, std::string doc_id)
{
    Q_UNUSED(schema_name);
    Q_UNUSED(doc_id);
}

void ArangoDatabase::ConnectFromSettings()
{
    try {
        std::string err_mess;
        auto d = impl_func();
        auto dbconnection = d->jsonui_group.value<std::string>("CurrentDBConnection","ArangoDBLocal");
        d->db_data.read_settings(dbconnection, d->jsonio_settings);
        emit dbConnectChanged();

        if(d->update_database(err_mess)) {
            if(err_mess.empty()) {
                emit dbdriveChanged();
            }
            else {
                emit errorConnection(QString::fromStdString(err_mess));
            }
        }
    }
    catch(std::exception& e) {
        uiSettings().setError(e.what());
        ui_logger->error("Exception when updating database {}", e.what());
    }
}

ArangoDBDocument* ArangoDatabase::createDocument(DocumentType type, const QString &document_schema_name)
{
    ArangoDBDocument* new_doc = new ArangoDBDocument(this,
             new ArangoDBDocumentPrivate(type, document_schema_name, impl_func()));
    return new_doc;
}

QString ArangoDatabase::dbConnect()
{
    return impl_func()->db_data.db_connect_current;
}

QString ArangoDatabase::dbUrl()
{
    return impl_func()->db_data.db_url;
}

QString ArangoDatabase::dbName()
{
    return impl_func()->db_data.db_name;
}

QString ArangoDatabase::dbUser()
{
    return impl_func()->db_data.db_user;
}

QString ArangoDatabase::dbUserPassword()
{
    return impl_func()->db_data.db_user_password;
}

bool ArangoDatabase::dbAccess()
{
    return impl_func()->db_data.db_access;
}

bool ArangoDatabase::isCreate()
{
    return impl_func()->db_data.db_create;
}

void ArangoDatabase::getRootLists(const std::string &db_group, QStringList &all_databases, QStringList &all_users)
{
    return impl_func()->get_system_lists(db_group, all_databases, all_users);
}

//------------------------------------------------------------------

void ArangoDatabase::resetCollectionsList()
{
    defined_collections = user_defined_collections;
    for(const auto& collection: jsonio::DataBase::usedVertexCollections()) {
        defined_collections[collection.second] = {collection.first, make_query_fields(collection.first)};
    }
    for(const auto& collection: jsonio::DataBase::usedEdgeCollections()) {
        defined_collections[collection.second] = {collection.first, make_query_fields(collection.first)};
    }
}

jsonio::values_t ArangoDatabase::make_query_fields(std::string schema_name) const
{
    jsonio::values_t key_fields = {"_label", "_id"};
    const jsonio::StructDef* schema_struct=jsonio::ioSettings().Schema().getStruct(schema_name);

    if(schema_struct != nullptr) {
        auto ids_or_names = schema_struct->getSelectedList();
        if(!ids_or_names.empty()) {
            key_fields.clear();
            key_fields.push_back("_label");
            for(const auto& id_or_key: ids_or_names) {
                auto pos = id_or_key.find_first_not_of("0123456789.");
                if(pos == std::string::npos)  {
                    auto ids = jsonio::split_int(id_or_key, ".");
                    auto the_name = schema_struct->getPathFromIds(ids);
                    key_fields.push_back(the_name);
                }
                else  {
                    key_fields.push_back( id_or_key );
                }
            }
        }
    }
    return key_fields;
}

QStringList ArangoDatabase::getEdgesList()
{
    auto std_schema_list = jsonio::DataBase::getEdgesList();
    QStringList new_list;
    //new_list.append(no_schema_name);
    std::transform(std_schema_list.begin(), std_schema_list.end(),
                   std::back_inserter(new_list),
                   [](const std::string &v){ return QString::fromStdString(v); });
    return new_list;
}

QStringList ArangoDatabase::getVertexesList()
{
    auto std_schema_list = jsonio::DataBase::getVertexesList();
    QStringList new_list;
    //new_list.append(no_schema_name);
    std::transform(std_schema_list.begin(), std_schema_list.end(),
                   std::back_inserter(new_list),
                   [](const std::string &v){ return QString::fromStdString(v); });
    return new_list;
}

std::string ArangoDatabase::collectionFromSchema(const std::string &schema_name)
{
    for(const auto& item: defined_collections) {
        if(item.second.schema_name==schema_name) {
            return item.first;
        }
    }
    return "";
}

std::string ArangoDatabase::schemaFromCollection(const std::string &collection_name)
{
    auto it = defined_collections.find(collection_name);
    if(it==defined_collections.end()) {
        return "";
    }
    else {
        return it->second.schema_name;
    }
}

jsonio::values_t ArangoDatabase::fieldsFromCollection(const std::string &collection_name)
{
    auto it = defined_collections.find(collection_name);
    if(it==defined_collections.end()) {
        return {"_id", "_key"};
    }
    else {
        return it->second.default_query_fields;
    }
}

std::string ArangoDatabase::anyVertexSchema()
{
    auto list = jsonio::DataBase::usedVertexCollections();
    if(!list.empty()) {
        return  list.begin()->first;
    }
    else {
        return "";
    }
}

std::string ArangoDatabase::anyEdgeSchema()
{
    auto list = jsonio::DataBase::usedEdgeCollections();
    if(!list.empty()) {
        return  list.begin()->first;
    }
    else {
        return "";
    }
}

}
