#include <QMessageBox>
#include <QKeyEvent>
#include <QLineEdit>
#include <QHeaderView>
#include <QComboBox>
#include "ui_JsonEditWidget.h"
#include "jsonqml/models/schema_model.h"
#include "jsonqml/clients/vertex_client.h"
#include "jsonqml/clients/settings_client.h"
#include "jsonqml/forms/JsonEditWidget.h"
#include "jsonqml/forms/json_view.h"
#include "jsonqml/forms/FileDialogs.h"
#include "jsonqml/forms/SelectDialog.h"
#include "jsonqml/forms/HelpMainWindow.h"

namespace jsonqml {

JsonEditWidget::JsonEditWidget(const QString& aschema, QWidget *parent):
    QMainWindow(parent),
    ui(new Ui::JsonEditWidget),
    json_client(),
    json_tree(),
    schema_name(aschema),
    collection_name()
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose); // automatically delete itself when window is closed

    QString title = qApp->applicationName()+" Structured Data Editor and Database Browser";
    setWindowTitle(title);

    if(schema_name.isEmpty())  {
        json_client.reset(new JsonClient(this));
        QComboBox* box = new QComboBox(this);
        box->insertItems(0, json_client->schemasList());
        ui->verticalLayout->addWidget(box);

        QObject::connect(box, &QComboBox::currentTextChanged, json_client.get(), &JsonClient::setSchema);
    }
    else {
        collection_name = QString::fromStdString(ArangoDatabase::collectionFromSchema(schema_name.toStdString()));
        DocumentType dtype = Schema;
        auto resource_lst = ArangoDatabase::getResourcesList();
        if(resource_lst.contains(schema_name)) {
            dtype = Resource;
        }
        json_client.reset(new VertexClient(dtype, schema_name, this));
    }

    // define edit tree view
    QItemDelegate* deleg = new JsonDelegate(this);
    json_tree.reset(new JsonView(json_client.get(), this));
    json_tree->setItemDelegate(deleg);
    json_tree->setColumnWidth(0, 250);
    json_tree->expandToDepth(1);
    ui->verticalLayout->addWidget(json_tree.get());

    set_actions();
    connect(&jsonqml::uiSettings(), &jsonqml::Preferences::settingsChanged, this, &JsonEditWidget::updateViewMenu);
    CmNew();
}

JsonEditWidget::~JsonEditWidget()
{
    qDebug() << "JsonEditWidget destruct";
}

void JsonEditWidget::CmNew()
{
    json_client->jsonmodel()->setupModelData("", schema_name);
}

void JsonEditWidget::CmHelpContens()
{
    helpWin(schema_name, "" );
}

void JsonEditWidget::CmHelpAbout()
{
    helpWin("JsonEditJsonui", "");
}

void JsonEditWidget::CmClone()
{
    VertexClient* vclient = dynamic_cast<VertexClient*>(json_client.get());
    if(vclient) {
        vclient->setEditorOid("");
    }
}

void JsonEditWidget::CmRead()
{
    VertexClient* vclient = dynamic_cast<VertexClient*>(json_client.get());
    if(vclient) {
        SelectDialog sel_dlg(single_selection, "Please, select a record to read/view",
                             vclient->tablemodel(), this);
        if(sel_dlg.exec())  {
            auto selndx = sel_dlg.selIndex();
            if(selndx>=0) {
                vclient->readEditorData(selndx);
            }
        }
    }
}

void JsonEditWidget::CmUpdate()
{
    VertexClient* vclient = dynamic_cast<VertexClient*>(json_client.get());
    if(vclient) {
        vclient->updateRecord();
    }
}

void JsonEditWidget::CmDeleteList()
{
    VertexClient* vclient = dynamic_cast<VertexClient*>(json_client.get());
    if(vclient) {
        auto keys = select_keys(vclient, "Please select the records you would like to delete.");
        if(!keys.empty()) {
            vclient->deleteRecords(std::move(keys));
        }
    }
}

void JsonEditWidget::CmBackup()
{
    VertexClient* vclient = dynamic_cast<VertexClient*>(json_client.get());
    if(vclient) {
        auto keys = select_keys(vclient, "Please, select a records to backup.");
        if(!keys.empty()) {
            auto file = vclient->fileSchemaExt(schema_name, "json");
            if(ChooseFileSave(this, file,
                               "Please, select a file to write (document) records",
                               domFilters, file))  {
                vclient->backupRecords(file, std::move(keys));
            }
        }
    }
}

void JsonEditWidget::CmImportJSON()
{
    QString fileName;
    if(ChooseFileOpen(this, fileName,
                       "Please, select a file with JSON object", domFilters)) {
        json_client->readJson(fileName);
    }
}

void JsonEditWidget::CmExportJSON()
{
    QString fileName = json_client->fileSchemaExt(schema_name, "json");
    if(ChooseFileSave(this, fileName,
                       "Please, select a file to write the data",
                       domFilters, fileName)) {
        json_client->saveJson(fileName);
    }
}

std::vector<std::string> JsonEditWidget::select_keys(VertexClient* vclient, const QString& title)
{
    std::vector<std::string> keys;
    SelectDialog sel_dlg(multi_selection, title, vclient->tablemodel(), this);
    if(sel_dlg.exec())  {
        auto ndxs = sel_dlg.allSelected();
        for(auto ndx: ndxs) {
            keys.push_back(vclient->tableId(ndx));
        }
    }
    return keys;
}

void JsonEditWidget::updateViewMenu()
{
    ui->action_Show_comments->setChecked(JsonSchemaModel::showComments);
    ui->action_Display_enums->setChecked(JsonSchemaModel::useEnumNames);
    ui->action_Edit_id->setChecked(JsonSchemaModel::editID);
    ui->actionKeep_Data_Fields_Expanded->setChecked(JsonClient::expanded_fields);
}

void JsonEditWidget::set_actions()
{
    // File
    QObject::connect(ui->actionNew , &QAction::triggered, this, &JsonEditWidget::CmNew);
    QObject::connect(ui->action_Clone_Structured_Data_Object , &QAction::triggered, this, &JsonEditWidget::CmClone);
    QObject::connect(ui->actionE_xit, &QAction::triggered, this, &JsonEditWidget::close);
    QObject::connect(ui->actionExport_Json_File, &QAction::triggered, this, &JsonEditWidget::CmExportJSON);
    QObject::connect(ui->actionImport_Json_File, &QAction::triggered, this, &JsonEditWidget::CmImportJSON);

    // Edit
    QObject::connect(ui->actionAdd_one_field, &QAction::triggered, json_tree.get(), &JsonView::CmAddObject);
    QObject::connect(ui->action_Clone_Current, &QAction::triggered, json_tree.get(), &JsonView::CmCloneObject);
    QObject::connect(ui->action_Delete_field, &QAction::triggered, json_tree.get(), &JsonView::CmDelObject);
    QObject::connect(ui->actionReset_Data_to_Defaults, &QAction::triggered, json_tree.get(), &JsonView::CmResetObject);
    if(!schema_name.isEmpty()) {
        QObject::connect(ui->actionRemove_Alternatives_Union, &QAction::triggered, json_tree.get(), &JsonView::CmDelObjectsUnion);
    }
    else {
        ui->actionRemove_Alternatives_Union->setVisible(false);
    }

    QObject::connect(ui->action_Resize_array, &QAction::triggered, json_tree.get(), &JsonView::CmResizeArray);
    QObject::connect(ui->action_Calculator, &QAction::triggered, json_tree.get(), &JsonView::CmCalc);
    QObject::connect(ui->actionCopy_Field_Path, &QAction::triggered, json_tree.get(), &JsonView::CopyFieldPath);
    QObject::connect(ui->actionCopy_Field, &QAction::triggered, json_tree.get(), &JsonView::CopyField);
    QObject::connect(ui->actionPaste_Field, &QAction::triggered, json_tree.get(), &JsonView::PasteField);

    // Help
    QObject::connect(ui->action_Help_About, &QAction::triggered, this, &JsonEditWidget::CmHelpAbout);
    QObject::connect(ui->actionContents, &QAction::triggered, this, &JsonEditWidget::CmHelpContens);
    QObject::connect(ui->actionAuthors, &QAction::triggered, HelpMainWindow::pDia, &HelpMainWindow::helpAuthors);
    QObject::connect(ui->actionLicense, &QAction::triggered, HelpMainWindow::pDia, &HelpMainWindow::helpLicense);
    QObject::connect( ui->actionVersion, &QAction::triggered, HelpMainWindow::pDia, &HelpMainWindow::helpVersion);

    // View
    QObject::connect(ui->action_Show_comments, &QAction::toggled,
                     [](bool val){ jsonqml::uiSettings().setBoolValue("ShowComments", val); });
    QObject::connect(ui->action_Display_enums, &QAction::toggled,
                     [](bool val){ jsonqml::uiSettings().setBoolValue("ShowEnumNames", val); });
    QObject::connect(ui->action_Edit_id, &QAction::toggled,
                     [](bool val){ jsonqml::uiSettings().setBoolValue("CanEdit_id", val); });
    QObject::connect(ui->actionKeep_Data_Fields_Expanded, &QAction::toggled,
                     [](bool val){ jsonqml::uiSettings().setBoolValue("KeepExpanded", val); });
    updateViewMenu();

    if(!collection_name.isEmpty()) {
        QObject::connect(ui->action_Update, &QAction::triggered, this, &JsonEditWidget::CmUpdate);
        QObject::connect(ui->action_Backup, &QAction::triggered, this, &JsonEditWidget::CmBackup);
        QObject::connect(ui->action_Delete_list, &QAction::triggered, this, &JsonEditWidget::CmDeleteList);
        QObject::connect(ui->action_Read, &QAction::triggered, this, &JsonEditWidget::CmRead);
    }
    else {
        ui->menu_Database->menuAction()->setVisible(false);
    }

    if(!schema_name.isEmpty()) {
        QLineEdit* pLineTask = new QLineEdit(ui->nameToolBar);
        pLineTask->setEnabled(true);
        pLineTask->setFocusPolicy(Qt::ClickFocus);
        pLineTask->setReadOnly(true);
        pLineTask->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        QString title =  schema_name;
        pLineTask->setText(title);
        ui->nameToolBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        ui->nameToolBar->addWidget(pLineTask); // setStretchableWidget( pLine );
    }
    else  {
        ui->mainToolBar->hide();
    }
}

} // namespace jsonui

