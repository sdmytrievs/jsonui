#include <QFileDialog>
#include <QMessageBox>
#include "ui_PreferencesUI.h"
#include "jsonqml/forms/PreferencesUI.h"

namespace jsonqml {

PreferencesUI::PreferencesUI(jsonqml::Preferences& asettings, QWidget* parent):
    QDialog(parent),
    ui(new Ui::PreferencesUI),
    settings(asettings)
{
    ui->setupUi(this);

    ui->resourcesEdit->setText(settings.resources_directory);
    ui->schemasEdit->setText(settings.schemas_directory);
    ui->usersEdit->setText(settings.work_directory);

    ui->commentBox->setChecked(settings.show_comments);
    ui->enumBox->setChecked(settings.show_enumnames);
    ui->idBox->setChecked(settings.can_edit_id);
    ui->expandedBox->setChecked(settings.keep_expanded);

    ui->dbGroupBox->addItems(settings.db_connect_list);
    ui->dbGroupBox->setEditText(settings.dbConnectCurrent());
    load_db_data();

    QObject::connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &PreferencesUI::CmSave);
    QObject::connect(ui->buttonBox, &QDialogButtonBox::helpRequested, this, &PreferencesUI::CmHelp);
    QObject::connect(ui->usersButton, &QToolButton::clicked, this, &PreferencesUI::CmProjectDir);
    QObject::connect(ui->schemasButton, &QToolButton::clicked, this, &PreferencesUI::CmSchemasDir);
    QObject::connect(ui->resourcesButton, &QToolButton::clicked, this, &PreferencesUI::CmResourcesDir);

    QObject::connect(ui->dbGroupBox, &QComboBox::currentTextChanged, this, &PreferencesUI::ChangeDBGroup);
    QObject::connect(ui->dbUrl, &QLineEdit::textEdited, this, &PreferencesUI::ChangedDBUrl);
}

PreferencesUI::~PreferencesUI()
{ }

void PreferencesUI::CmSave()
{
    cm_save();
    settings.applyChanges();
    if(!settings.lastError().isEmpty())  {
        QMessageBox::critical(this, "Preferences update", settings.lastError());
    }
}

void PreferencesUI::CmProjectDir()
{
    QString dir = QFileDialog::getExistingDirectory(this, "Select Project Directory", ui->usersEdit->text(),
                                                    QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    ui->usersEdit->setText(dir);
}

void PreferencesUI::CmSchemasDir()
{
    QString dir = QFileDialog::getExistingDirectory(this, "Select Schemas Directory", ui->schemasEdit->text(),
                                                    QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    ui->schemasEdit->setText(dir);
}

void PreferencesUI::CmResourcesDir()
{
    QString dir = QFileDialog::getExistingDirectory(this, "Select Resource Directory", ui->resourcesEdit->text(),
                                                    QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks );
    ui->resourcesEdit->setText(dir);
}

void PreferencesUI::ChangeDBGroup(const QString &name)
{
    settings.changeDBConnect(name);
    load_db_data();
}

void PreferencesUI::ChangedDBUrl(const QString &url)
{
    // not full implemented
    auto db_group =ui->dbGroupBox->currentText().toStdString();
    save_db_data();
    // !!! from private save_db_settings(db_group);
    settings.RefreshLists(ui->dbGroupBox->currentText().toStdString());
    load_db_data();
}

void PreferencesUI::load_db_data()
{
    ui->dbUrl->setText(settings.dbUrl());
    ui->dbnameBox->clear();
    ui->dbnameBox->addItems(settings.db_all_databases);
    ui->dbnameBox->setEditText(settings.dbName());
    ui->dbuserBox->clear();
    ui->dbuserBox->addItems(settings.db_all_users);
    ui->dbuserBox->setEditText(settings.dbUser());
    ui->dbuserPassword->setText(settings.dbUserPassword());

    ui->readonlyBox->setChecked(settings.dbAccess());
    ui->createdbBox->setChecked(settings.isCreate());
}

void PreferencesUI::save_db_data()
{
    settings.setConnectCurrent(ui->dbGroupBox->currentText());
    settings.setUrl(ui->dbUrl->text());
    settings.setName(ui->dbnameBox->currentText());
    settings.setUser(ui->dbuserBox->currentText());
    settings.setUserPassword(ui->dbuserPassword->text());

    settings.setAccess(ui->readonlyBox->isChecked());
    settings.setCreate(ui->createdbBox->isChecked());
}

void PreferencesUI::cm_save()
{
    settings.resources_directory = ui->resourcesEdit->text();
    settings.schemas_directory = ui->schemasEdit->text();
    settings.setWorkPath(ui->usersEdit->text());

    settings.show_comments = ui->commentBox->isChecked();
    settings.show_enumnames = ui->enumBox->isChecked();
    settings.can_edit_id = ui->idBox->isChecked();
    settings.keep_expanded = ui->expandedBox->isChecked();

    save_db_data();
}

} // namespace jsonui
