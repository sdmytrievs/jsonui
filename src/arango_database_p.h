#ifndef ARANGODB_P_H
#define ARANGODB_P_H

#include "jsonqml/clients/settings_client.h"
#include "jsonqml/arango_database.h"
#include "arango-cpp/arangoconnect.h"
#include "jsonio/dbconnect.h"

namespace jsonqml {

/// Private \class ArangoDatabase - monitoring database
class ArangoDatabasePrivate
{
    Q_DISABLE_COPY_MOVE(ArangoDatabasePrivate)

    friend class ArangoDatabase;
public:

    /// Constructor
    explicit ArangoDatabasePrivate();

    /// Destructor
    virtual ~ArangoDatabasePrivate();

    void init();
    /// Update the current database driver. Return false if credentials are not changed;
    /// otherwise, return true. In case of an error, return the not-empty error message.
    bool update_database(std::string& error_message);

    /// Current work Database exist
    bool db_connected() const
    {
        return (work_database.get()!=nullptr && work_database->connected());
    }

    /// Current work Database
    const jsonio::DataBase& database() const
    {
        jsonio::JSONIO_THROW_IF(work_database.get()==nullptr, "Preferences", 10,
                                  " the database connection is not defined.");
        return *work_database.get();
    }

    /// Current resourse Database
    const jsonio::DataBase& resources_database()
    {
       if(!resourse_database) { // create connect only if used
           update_resource_database();
        }
        jsonio::JSONIO_THROW_IF(resourse_database.get()==nullptr, "Preferences", 11,
                                  " the resources database connection is not defined.");
        return *resourse_database.get();
    }

    /// Current jsonio  work Database
    const jsonio::DataBase& jsonio_database(bool is_resource=false)
    {
        if(is_resource) {
            return resources_database();
        }
        return database();
    }

protected:

    /// Current database credentials
    struct DatabaseSettings db_data;

    /// Link to JsonioSettings - storing preferences to JSONIO
    jsonio::JsonioSettings& jsonio_settings;
    /// Link to jsonui settings in configuration data
    jsonio::SectionSettings jsonui_group;

    /// Current work Database
    std::shared_ptr<jsonio::DataBase> work_database;
    /// Current resourse Database
    std::shared_ptr<jsonio::DataBase> resourse_database;
    /// Current ArangoDB root client
    std::shared_ptr<arangocpp::ArangoDBRootClient> root_client;

    /// Current root database connection
    arangocpp::ArangoDBRootClient* root_connect() const
    {
        jsonio::JSONIO_THROW_IF(root_client.get()==nullptr, "Preferences", 12,
                                " the root database connection is not defined.");
        return root_client.get();
    }

    void update_work_database(const arangocpp::ArangoDBConnection& db_data,
                              std::string& error_message);
    void update_resource_database();
    void update_root_client(const std::string& db_group);
    void create_collection_if_no_exist(jsonio::AbstractDBDriver* db_driver);
    void get_system_lists(const std::string& db_group,
                          QStringList& db_all_databases,
                          QStringList& db_all_users);
};

}
#endif // ARANGODB_P_H
