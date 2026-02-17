
#include <QDir>
#include <QClipboard>
#include <QtGui/QGuiApplication>

#include "jsonqml/clients/settings_client.h"
#include "jsonqml/clients/help_client.h"
#include "jsonqml/models/schema_model.h"
#include "jsonqml/arango_database.h"
#include "jsonio/io_settings.h"
#include <spdlog/sinks/stdout_color_sinks.h>

namespace jsonqml {

std::string Preferences::jsonui_section_name = "jsonui";
bool Preferences::use_database = true;
bool Preferences::use_schemas = true;

// Thread-safe logger to stdout with colors
std::shared_ptr<spdlog::logger> ui_logger = spdlog::stdout_color_mt("jsonqml");


Preferences& uiSettings()
{
    static  Preferences data;
    return data;
}

/// Private \class Preferences - monitoring preferences into JSONIO and database
class PreferencesPrivate
{
    Q_DISABLE_COPY_MOVE(PreferencesPrivate)

public:
    explicit PreferencesPrivate(Preferences* );

    ~PreferencesPrivate(){}

    void init();
    void save_db_settings(const std::string& db_group);
    void read_db_settings(const std::string& db_group);
    void save_other_settings();
    void read_other_settings();
    bool change_schemas_path(const std::string& path);
    void set_user_dir(const std::string& path);
    void set_bool_settings(const std::string &setting_name, bool value);

protected:

    Preferences* base_ptr;
    Preferences* base_func() { return static_cast<Preferences*>(base_ptr); }
    const Preferences* base_func() const { return static_cast<const Preferences *>(base_ptr); }

    /// Link to JsonioSettings - storing preferences to JSONIO
    jsonio::JsonioSettings& jsonio_settings;
    /// Link to jsonui settings in configuration data
    jsonio::SectionSettings jsonui_group;

    void apply_changes_to_static();
};

PreferencesPrivate::PreferencesPrivate(Preferences* parent):
    base_ptr(parent),
    jsonio_settings(jsonio::ioSettings()),
    jsonui_group(jsonio_settings.section(Preferences::jsonui_section_name))
{
    jsonio_settings.set_module_level("jsonqml", "debug");
    init();
}

void PreferencesPrivate::init()
{
    if(Preferences::use_schemas) {
        read_other_settings();
    }
    if(Preferences::use_database) {
        auto dbconnection = jsonui_group.value<std::string>("CurrentDBConnection","ArangoDBLocal");
        base_func()->db_data.db_connect_current = QString::fromStdString(dbconnection);
        read_db_settings(dbconnection);
    }
}

bool PreferencesPrivate::change_schemas_path(const std::string& path)
{
    jsonio_settings.setValue<std::string>(jsonio::common_section("SchemasDirectory"), path);
    if(jsonio_settings.updateSchemaDir()) {
        ui_logger->debug("Changed schemas path to: {}", path);
        return true;
    }
    return false;
}

void PreferencesPrivate::set_user_dir(const std::string& path)
{
    jsonio_settings.setUserDir(path);
}

void PreferencesPrivate::apply_changes_to_static()
{
    // load main programm settingth
    JsonSchemaModel::showComments = jsonui_group.value("ShowComments", JsonSchemaModel::showComments);
    JsonSchemaModel::useEnumNames = jsonui_group.value("ShowEnumNames", JsonSchemaModel::useEnumNames);
    JsonSchemaModel::editID = jsonui_group.value("CanEdit_id", JsonSchemaModel::editID);
    JsonClient::expanded_fields = jsonui_group.value("KeepExpanded", JsonClient::expanded_fields);
    HelpClient::edit_help = jsonui_group.value("CanEditDocPages", HelpClient::edit_help);
    ui_logger->debug("Applied changes to internal static values");
}

void PreferencesPrivate::save_other_settings()
{
    const auto q = base_func();
    jsonio_settings.setValue(jsonio::common_section("ResourcesDirectory"), q->resources_directory.toStdString());
    jsonio_settings.setValue(jsonio::common_section("SchemasDirectory"), q->schemas_directory.toStdString());
    jsonio_settings.setUserDir(q->work_directory.toStdString());

    jsonui_group.setValue("ShowComments", q->show_comments);
    jsonui_group.setValue("ShowEnumNames", q->show_enumnames);
    jsonui_group.setValue("CanEdit_id", q->can_edit_id);
    jsonui_group.setValue("KeepExpanded", q->keep_expanded);
    jsonui_group.setValue("CanEditDocPages", q->can_edit_doc_pages);

    apply_changes_to_static();
}

void PreferencesPrivate::set_bool_settings(const std::string& setting_name, bool value)
{
    jsonui_group.setValue(setting_name, value);
    apply_changes_to_static();
}

void PreferencesPrivate::save_db_settings(const std::string& db_group)
{
    base_func()->db_data.save_settings(db_group, jsonio_settings);
}

void PreferencesPrivate::read_other_settings()
{
    auto q = base_func();
    q->resources_directory = QString::fromStdString(jsonio_settings.value<std::string>(jsonio::common_section("ResourcesDirectory"), ""));
    q->schemas_directory = QString::fromStdString(jsonio_settings.value<std::string>(jsonio::common_section("SchemasDirectory" ), ""));
    q->work_directory = QString::fromStdString(jsonio_settings.value<std::string>(jsonio::common_section("WorkDirectoryPath" ), "."));

    q->show_comments = jsonui_group.value("ShowComments", JsonSchemaModel::showComments);
    q->show_enumnames = jsonui_group.value("ShowEnumNames", JsonSchemaModel::useEnumNames);
    q->can_edit_id = jsonui_group.value("CanEdit_id", JsonSchemaModel::editID);
    q->keep_expanded = jsonui_group.value("KeepExpanded", JsonClient::expanded_fields);
    q->can_edit_doc_pages = jsonui_group.value("CanEditDocPages", HelpClient::edit_help);

    apply_changes_to_static();
}

void PreferencesPrivate::read_db_settings(const std::string& db_group)
{
    auto q = base_func();
    q->db_data.read_settings(db_group, jsonio_settings);
    q->RefreshLists(db_group);
    ui_logger->debug("Changed db credentials to: {}", db_group);
}

//----------------------------------------------------------------------------

Preferences::Preferences()
{
    setDBConnectList();
    impl_ptr.reset(new PreferencesPrivate(this));

    if(Preferences::use_database) {
        QObject::connect(this, &Preferences::scemasPathChanged, &arango_db(), &ArangoDatabase::resetCollectionsList);
        QObject::connect(this, &Preferences::dbdriverChanged, &arango_db(), &ArangoDatabase::ConnectFromSettings);
        QObject::connect(&arango_db(), &ArangoDatabase::errorConnection, this, &Preferences::setError);
        QObject::connect(&arango_db(), &ArangoDatabase::dbdriveChanged, this, &Preferences::openVertex);
    }
    qDebug() << "Preferences construct";
}

Preferences::~Preferences()
{
    qDebug() << "Preferences destruct";
}

void Preferences::setDBConnectList()
{
    db_connect_list = {"ArangoDBLocal", "ArangoDBRemote"};
    // To do: restore list from settings
}

void Preferences::changeDBConnect(const QString &db_group)
{
    setError(QString());
    try {
        if(Preferences::use_database) {
            db_data.db_connect_current = db_group;
            auto dbconnection = db_group.toStdString();
            impl_func()->read_db_settings(dbconnection);
            emit dbConnectChanged();
        }
    }
    catch(std::exception& e) {
        setError(e.what());
    }
}

void Preferences::changeScemasPath(const QString &url)
{
    if(url.isEmpty()) {
        return;
    }
    setError(QString());
    try {
        auto path = handleFileChosen(url);
        if(impl_func()->change_schemas_path(path.toStdString())) {
            schemas_directory = path;
            emit scemasPathChanged();
        }
    }
    catch(std::exception& e) {
        setError(e.what());
    }
}

void Preferences::applyChanges()
{
    auto d = impl_func();
    setError(QString());
    try {
        if(Preferences::use_schemas) {
            d->save_other_settings();
            if(d->change_schemas_path(schemas_directory.toStdString())) {
                emit scemasPathChanged();
            }
        }
        if(Preferences::use_database) {
            auto dbconnection = db_data.db_connect_current.toStdString();
            d->save_db_settings(dbconnection);
            // tell database to reload settings
            emit dbdriverChanged();
        }
        emit settingsChanged();
        emit modelChanged();
    }
    catch(std::exception& e) {
        setError(e.what());
    }
}

void Preferences::RefreshLists(const std::string& db_group)
{
    // ask root client arango_db to refresh lists for new settings group,
    try {  // try generate list of all databases
        db_all_databases.clear();
        db_all_users.clear();
        if(db_data.db_create) {
            arango_db().getRootLists(db_group, db_all_databases, db_all_users);
        }
    }
    catch(std::exception& e) {
        ui_logger->warn("Error connection as root to host: {}", e.what());
    }

    if(db_all_databases.indexOf(db_data.db_name)<0) {
        db_all_databases.append(db_data.db_name);
    }
    if(db_all_users.indexOf(db_data.db_user)<0) {
        db_all_users.append(db_data.db_user);
    }
    emit dbNamesListChanged();
    emit dbUsersListChanged();
}

bool Preferences::dbConnected()
{
    return arango_db().dbConnected();
}

QString Preferences::lastError() const
{
    return err_message;
}

void Preferences::setError(const QString& error)
{
    qDebug() << "Error " << error;
    err_message = error;
    emit errorChanged();
}

QString Preferences::dbConnectCurrent() const
{
    return db_data.db_connect_current;
}

QString Preferences::dbUrl() const
{
    return db_data.db_url;
}

QString Preferences::dbName() const
{
    return db_data.db_name;
}

QString Preferences::dbUser() const
{
    return db_data.db_user;
}

QString Preferences::dbUserPassword() const
{
    return db_data.db_user_password;
}

bool Preferences::dbAccess() const
{
    return db_data.db_access;
}

bool Preferences::isCreate() const
{
    return db_data.db_create;
}

void Preferences::setConnectCurrent(const QString &val)
{
    db_data.db_connect_current = val;
}

void Preferences::setUrl(const QString &val)
{
    db_data.db_url = val;
}

void Preferences::setName(const QString &val)
{
    db_data.db_name = val;
}

void Preferences::setUser(const QString &val)
{
    db_data.db_user = val;
}

void Preferences::setUserPassword(const QString &val)
{
    db_data.db_user_password = val;
}

void Preferences::setAccess(bool val)
{
    db_data.db_access = val;
}

void Preferences::setCreate(bool val)
{
    db_data.db_create = val;
}

void Preferences::addDBName(const QString &new_name)
{
    db_all_databases.append(new_name);
    emit dbNamesListChanged();
}

void Preferences::addDBUser(const QString &new_user)
{
    db_all_users.append(new_user);
    emit dbUsersListChanged();
}

QString Preferences::handleFileChosen(const QString &urls)
{
    QString abs_path;
    const QUrl url(urls);
    if (url.isLocalFile()) {
        abs_path = QDir::toNativeSeparators(url.toLocalFile());
    } else {
        abs_path = urls;
    }
    if(url.isLocalFile() || url.isRelative()) {
        setWorkPath(abs_path); // save last used dir
    }
    return abs_path;
}

QUrl Preferences::workDir() const
{
    return QUrl::fromLocalFile(work_directory);
}

QString Preferences::addWorkDir(const QString &file_path) const
{
    QString full_path;
    if( file_path.contains('/') || file_path.contains('\\') ) {
        full_path = file_path;
    }
    else {
        full_path  =  work_directory+"/"+file_path;
    }
    return full_path;
}

void Preferences::setWorkPath(const QString &path)
{
    QFileInfo flinfo(path);
    work_directory = flinfo.dir().path();
    impl_func()->set_user_dir(work_directory.toStdString());
    emit workDirChanged();
}

void Preferences::setBoolValue(const QString &name, bool value)
{
    impl_func()->set_bool_settings(name.toStdString(), value);
    emit settingsChanged();
    if(name=="ShowComments" || name =="ShowEnumNames") {
        emit modelChanged();
    }
}

QString Preferences::paste()
{
    return QGuiApplication::clipboard()->text(QClipboard::Clipboard);
}

void Preferences::copy(const QString& clip_text)
{
    QGuiApplication::clipboard()->setText(clip_text/*, QClipboard::Clipboard*/);
}


QStringList transform2qt(const std::vector<std::string>& std_list)
{
    QStringList new_list;
    std::transform(std_list.begin(), std_list.end(),
                   std::back_inserter(new_list),
                   [](const std::string &v){ return QString::fromStdString(v); });
    return new_list;
}

std::vector<std::string> transform2std(const QStringList& qt_list)
{
    std::vector<std::string> new_list;
    std::transform(qt_list.begin(), qt_list.end(),
                   std::back_inserter(new_list),
                   [](const QString &v){ return v.toStdString(); });
    return new_list;
}
}
