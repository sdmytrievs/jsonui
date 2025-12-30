#ifndef SETTINGS_H
#define SETTINGS_H

#include <string>
#include <QObject>
#include <QQmlEngine>

#include "jsonio/io_settings.h"

namespace jsonqml {

extern std::shared_ptr<spdlog::logger> ui_logger;

QStringList transform2qt(const std::vector<std::string>& std_list);
std::vector<std::string> transform2std(const QStringList& qt_list);

struct DatabaseSettings
{
    /// Current database credentials group
    QString db_connect_current;

    /// The connection's database URL
    QString db_url;
    /// The connection's database name
    QString db_name;
    /// The connection's user name
    QString db_user;
    /// The connection's user password name
    QString db_user_password;
    /// Database access  "rw" (read&write) or "ro" (read-only if true)
    bool db_access;
    /// The flag to creating the empty database if they are not present
    /// Work if defined root credentials
    bool    db_create;

    DatabaseSettings();
    void save_settings(const std::string& db_group, jsonio::JsonioSettings& jsonio_settings);
    void read_settings(const std::string& db_group, jsonio::JsonioSettings& jsonio_settings);
};

class PreferencesPrivate;

class Preferences : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool withDatabase MEMBER use_database CONSTANT)
    Q_PROPERTY(bool withSchemas MEMBER use_schemas CONSTANT)

    Q_PROPERTY(QString error READ lastError WRITE setError NOTIFY errorChanged)

    Q_PROPERTY(QString resourcesDir MEMBER resources_directory NOTIFY settingsChanged)
    Q_PROPERTY(QString schemasDir MEMBER schemas_directory NOTIFY scemasPathChanged)
    Q_PROPERTY(QUrl workDir READ workDir NOTIFY workDirChanged)

    Q_PROPERTY(bool showComments MEMBER show_comments NOTIFY settingsChanged)
    Q_PROPERTY(bool showEnumNames MEMBER show_enumnames NOTIFY settingsChanged)
    Q_PROPERTY(bool canEditID MEMBER can_edit_id NOTIFY settingsChanged)
    Q_PROPERTY(bool keepExpanded MEMBER keep_expanded NOTIFY settingsChanged)
    Q_PROPERTY(bool canEditDocPages MEMBER can_edit_doc_pages NOTIFY settingsChanged)

    Q_PROPERTY(QStringList dbConnectList MEMBER db_connect_list NOTIFY dbConnectListChanged)

    Q_PROPERTY(QString dbConnect READ dbConnectCurrent WRITE setConnectCurrent NOTIFY dbConnectChanged)
    Q_PROPERTY(QString dbUrl READ dbUrl WRITE setUrl NOTIFY dbConnectChanged)
    Q_PROPERTY(QString dbName READ dbName WRITE setName NOTIFY dbConnectChanged)
    Q_PROPERTY(QString dbUser READ dbUser WRITE setUser NOTIFY dbConnectChanged)
    Q_PROPERTY(QString dbUserPassword READ dbUserPassword WRITE setUserPassword NOTIFY dbConnectChanged)
    Q_PROPERTY(bool    dbAccess READ dbAccess WRITE setAccess NOTIFY dbConnectChanged)
    Q_PROPERTY(bool    isCreate READ isCreate WRITE setCreate NOTIFY dbConnectChanged)

    Q_PROPERTY(QStringList dbNamesList MEMBER db_all_databases NOTIFY dbNamesListChanged)
    Q_PROPERTY(QStringList dbUsersList MEMBER db_all_users NOTIFY dbUsersListChanged)

signals:
    void errorChanged();
    void settingsChanged();
    void modelChanged();
    void dbConnectListChanged();
    void dbConnectChanged();
    void dbNamesListChanged();
    void dbUsersListChanged();

    void workDirChanged();
    void scemasPathChanged();
    void dbdriverChanged();
    void openVertex();

public slots:
    /// Setting the value of the last error that occurred
    /// and emitting a signal about the error.
    void setError(const QString& error);

private slots:
    void setDBConnectList();

public:
    /// Jsonui section name in resource file
    static std::string jsonui_section_name;
    /// Use link to database in settings
    static bool use_database;
    /// Use link to database in settings
    static bool use_schemas;

    /// Constructor
    explicit Preferences();
    /// Destructor
    ~Preferences();

    ///  Returns information about the last error that occurred
    Q_INVOKABLE QString lastError() const;

    /// Download new schemas from the folder with the path
    Q_INVOKABLE void changeScemasPath(const QString& path);

    /// Switching current database credentials from the list
    Q_INVOKABLE void changeDBConnect(const QString& db_group);

    /// Apply and save changes according to data in properties
    Q_INVOKABLE void applyChanges();

    Q_INVOKABLE bool dbConnected();

    /// Adde new database
    Q_INVOKABLE void addDBName(const QString& new_name);
    /// Adde new user
    Q_INVOKABLE void addDBUser(const QString& new_user);

    /// Get the path to the previous working directory
    QUrl workDir() const;
    /// Set the path as the working directory
    Q_INVOKABLE void setWorkPath(const QString &path);
    /// Converting the path of this URL formatted as a local file path,
    /// if necessary, and save it as the current directory
    Q_INVOKABLE QString handleFileChosen(const QString &url);

    Q_INVOKABLE QString paste();
    Q_INVOKABLE void copy(const QString& clip_text);

    QString dbConnectCurrent() const;
    QString dbUrl() const;
    QString dbName() const;
    QString dbUser() const;
    QString dbUserPassword() const;
    bool    dbAccess() const;
    bool    isCreate() const;

    void setConnectCurrent(const QString& val);
    void setUrl(const QString& val);
    void setName(const QString& val);
    void setUser(const QString& val);
    void setUserPassword(const QString& val);
    void setAccess(bool val);
    void setCreate(bool val);

    void RefreshLists(const std::string &db_group);
    QString addWorkDir(const QString &path) const;

    Q_INVOKABLE void setBoolValue(const QString &name, bool value);

protected:
    friend Preferences& uiSettings();
    friend class PreferencesPrivate;
    friend class PreferencesUI;

    /// The information about the last error that occurred
    QString err_message;

    /// The directory with resources
    QString resources_directory;
    /// The directory with json schemas
    QString schemas_directory;
    /// The last working directory
    QString work_directory;

    /// The flag to show schema comments in the editor
    bool show_comments;
    /// The flag to show enum names in the editor
    bool show_enumnames;
    /// The flag to edit system data
    bool can_edit_id;
    /// The flag to keep data fields expanded
    bool keep_expanded;
    /// The flag to edit docpages
    bool can_edit_doc_pages;

    /// List of all defined database connections (credentials)
    QStringList db_connect_list;
    /// Current database credentials
    struct DatabaseSettings db_data;
    /// List of all existing databases according URL
    /// Work if defined root credentials and set flag db_create
    QStringList db_all_databases;
    /// List of all existing users according URL
    /// Work if defined root credentials and set flag db_create
    QStringList db_all_users;

    QScopedPointer<PreferencesPrivate> impl_ptr;
    PreferencesPrivate* impl_func()
    { return reinterpret_cast<PreferencesPrivate *>(impl_ptr.get()); }
    const PreferencesPrivate* impl_func() const
    { return reinterpret_cast<const PreferencesPrivate *>(impl_ptr.get()); }

};

/// Function to connect to only one Preferences object
extern Preferences& uiSettings();

inline std::string jsonui_section(const std::string& item)
{
    return  Preferences::jsonui_section_name+"."+item;
}

}

#endif // SETTINGS_H
