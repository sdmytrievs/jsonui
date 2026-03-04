#pragma once

#include <QDialog>
#include "jsonqml/clients/settings_client.h"

namespace Ui {
class PreferencesUI;
}

namespace jsonqml {

/// \class  Preferences  dialog for insert/edit settings
class PreferencesUI : public QDialog
{
    Q_OBJECT

public slots:
    void CmSave();
    void CmProjectDir();
    void CmSchemasDir();
    void CmResourcesDir();
    void CmHelp();

    void ChangeDBGroup(const QString &name);
    void ChangedDBUrl(const QString &url);

public:
    explicit PreferencesUI(jsonqml::Preferences& asettings, QWidget *parent = nullptr);
    ~PreferencesUI();

private:
    QScopedPointer<Ui::PreferencesUI> ui;
    jsonqml::Preferences& settings;

    void cm_save();
    void load_db_data();
    void save_db_data();
};

} // namespace jsonui


