#include <QMessageBox>
#include "ui_FormatImpexWidget.h"
#include "jsonqml/models/schema_model.h"
#include "jsonqml/clients/vertex_client.h"
#include "jsonqml/clients/settings_client.h"
#include "jsonqml/forms/FormatImpexWidget.h"
#include "jsonqml/forms/SelectDialog.h"
#include "jsonqml/forms/FileDialogs.h"
#include "jsonqml/forms/json_view.h"
#include "jsonqml/forms/HelpMainWindow.h"

namespace jsonqml {

QString FormatImpexWidget::impex_shema_name = "ImpexFormat";

FormatImpexWidget::FormatImpexWidget(ImpexMode mode,
                                     const QString &run_schema, QWidget *parent):
    QMainWindow(parent),
    ui(new Ui::FormatImpexWidget),
    impex_client(),
    recipe_client(),
    use_mode(mode),
    parent_schema_name(run_schema)
{
    ui->setupUi(this);
    ui->splitter->setStretchFactor(0, 1);
    ui->splitter->setStretchFactor(1, 2);

    setAttribute(Qt::WA_DeleteOnClose); // automatically delete itself when window is closed

    QString title = qApp->applicationName()+" Structured Data Editor and Database Browser";
    setWindowTitle(title);

    //set up main parameters
    const jsonio::StructDef* strct_def= jsonio::ioSettings().Schema().getStruct("FormatStructDataFile");
    if(strct_def != nullptr) {
        ui->impexschema->addItem("FormatStructDataFile");
    }
    strct_def = jsonio::ioSettings().Schema().getStruct("FormatKeyValueFile");
    if(strct_def != nullptr) {
        ui->impexschema->addItem("FormatKeyValueFile");
    }
    strct_def = jsonio::ioSettings().Schema().getStruct("FormatTableFile");
    if(strct_def != nullptr) {
        ui->impexschema->addItem("FormatTableFile");
    }
    strct_def = jsonio::ioSettings().Schema().getStruct("FormatTextFile");
    if(strct_def != nullptr) {
        ui->impexschema->addItem("FormatTextFile");
    }
    jsonio::JSONIO_THROW_IF(ui->impexschema->count()<1, "impex", 11, " no impex formats defined" );

    schema_name = ui->impexschema->currentText();
    run_funct = [](std::string, std::string, std::string, std::string, bool ) {};

    //define schema checkbox
    //ui->schemaBox->insertItems(0, ArangoDatabase::getEdgesList());
    ui->schemaBox->insertItems(0, ArangoDatabase::getVertexesList());
    if(!parent_schema_name.isEmpty()) {
        ui->schemaBox->setCurrentText(parent_schema_name);
    }
    if(use_mode) {
        ui->direcBox->setCurrentIndex(use_mode-1);
    }

    impex_client.reset(new VertexClient(Resource, impex_shema_name, this));
    connect(impex_client.get(), &VertexClient::editorChanged,
            this, &FormatImpexWidget::editor_changed);
    recipe_client.reset(new JsonClient(this));

    // define edit tree view
    QItemDelegate* deleg = new JsonDelegate(this);
    json_tree.reset(new JsonView(recipe_client.get(), this));
    json_tree->setItemDelegate(deleg);
    json_tree->setColumnWidth(0, 250);
    json_tree->expandToDepth(1);
    ui->gridLayout_3->addWidget(json_tree.get(), 1, 0, 1, 2);

    set_actions();
    connect(&jsonqml::uiSettings(), &jsonqml::Preferences::settingsChanged,
            this, &FormatImpexWidget::updateViewMenu);
    CmNew();
}

FormatImpexWidget::~FormatImpexWidget()
{}

void FormatImpexWidget::setExecuteFunction(ExecuteImpex_f afunc)
{
    run_funct = afunc;
}

void FormatImpexWidget::CmHelpContens()
{
    helpWin(schema_name, "" );
}

void FormatImpexWidget::CmHelpAbout()
{
    helpWin("ImpexWidgetJsonui", "");
}

void FormatImpexWidget::CmClone()
{
    ui->keyNoEdit->setText("");
}

void FormatImpexWidget::CmNew()
{
    ui->keyNoEdit->setText("");
    ui->nameEdit->setText("new_impex_name");
    ui->descEdit->setText("");
    recipe_client->setEditorData(schema_name.toStdString(), "");
}

void FormatImpexWidget::CmRead()
{
    SelectDialog sel_dlg(single_selection, "Please, select a record to read/view",
                         impex_client->tablemodel(), this);
    if(sel_dlg.exec())  {
        auto selndx = sel_dlg.selIndex();
        if(selndx>=0) {
            impex_client->readEditorData(selndx);
        }
    }
}

void FormatImpexWidget::CmUpdate()
{
    std::string doc_json = to_json_node();
    impex_client->setEditorData(impex_shema_name.toStdString(), doc_json);
    impex_client->updateRecord();
}

void FormatImpexWidget::CmDeleteList()
{
    std::vector<std::string> keys;
    SelectDialog sel_dlg(multi_selection, "Please select the records you would like to delete.",
                         impex_client->tablemodel(), this);
    if(sel_dlg.exec())  {
        auto ndxs = sel_dlg.allSelected();
        for(auto ndx: ndxs) {
            keys.push_back(impex_client->tableId(ndx));
        }
    }
    if(!keys.empty()) {
        impex_client->deleteRecords(std::move(keys));
    }
}

void FormatImpexWidget::CmImportJSON()
{
    QString fileName;
    if(ChooseFileOpen(this, fileName,
                       "Please, select a file with JSON object", domFilters)) {
        recipe_client->readJson(fileName);
    }
}

void FormatImpexWidget::CmExportJSON()
{
    QString fileName = recipe_client->fileSchemaExt(schema_name, "json");
    if(ChooseFileSave(this, fileName,
                       "Please, select a file to write the data",
                       domFilters, fileName)) {
        recipe_client->saveJson(fileName);
    }
}

void FormatImpexWidget::CmSelectDataFile()
{
    QString fileName = ui->datafileEdit->text();
    if(ChooseFileOpen(this, fileName,
                       "Please, select Input File Path", textFilters)) {
        ui->datafileEdit->setText(fileName);
    }
}

void FormatImpexWidget::CmRunScript()
{
//#ifndef IMPEX_OFF
    try {
        std::string datafile = ui->datafileEdit->text().toStdString();
        jsonio::JSONIO_THROW_IF(datafile.empty(), "impex", 13, " undefined file with data to import");
        bool overwrite = ui->overwriteBox->isChecked();
        std::string impex_data_name = ui->schemaBox->currentText().toStdString();
        recipe_client->getValue("label", impex_data_name, impex_data_name);
        run_funct(impex_data_name, schema_name.toStdString(),
                  recipe_client->jsonmodel()->current_object().dump(true),
                  datafile, overwrite);
    }
    catch(jsonio::jsonio_exception& e) {
        QMessageBox::critical( this, "jsonio_exception", e.what() );
    }
    catch(std::exception& e) {
        QMessageBox::critical( this, "std::exception", e.what() );
    }
//#endif
}

void FormatImpexWidget::updateViewMenu()
{
    ui->action_Show_comments->setChecked(JsonSchemaModel::showComments);
    ui->action_Display_enums->setChecked(JsonSchemaModel::useEnumNames);
    ui->action_Edit_id->setChecked(JsonSchemaModel::editID);
    ui->actionKeep_Data_Fields_Expanded->setChecked(JsonClient::expanded_fields);
}

void FormatImpexWidget::schema_changed(const QString& text)
{
    schema_name = text;
    recipe_client->setSchema(schema_name);
    //recipe_client->setEditorData("", "");
}

void FormatImpexWidget::editor_changed()
{
    from_json_node(impex_client->jsonmodel()->current_object());
}

void FormatImpexWidget::set_actions()
{
    QObject::connect(ui->impexschema, &QComboBox::currentTextChanged, this, &FormatImpexWidget::schema_changed);
    QObject::QObject::connect(ui->datafileButton, &QToolButton::clicked, this, &FormatImpexWidget::CmSelectDataFile);

    // File
    QObject::connect(ui->actionNew , &QAction::triggered, this, &FormatImpexWidget::CmNew);
    QObject::connect(ui->actionClone, &QAction::triggered, this, &FormatImpexWidget::CmClone);
    QObject::connect(ui->actionE_xit, &QAction::triggered, this, &FormatImpexWidget::close);
    QObject::connect(ui->actionExport_File, &QAction::triggered, this, &FormatImpexWidget::CmExportJSON);
    QObject::connect(ui->actionImport_File, &QAction::triggered, this, &FormatImpexWidget::CmImportJSON);
    QObject::connect(ui->actionSelect_Data_File, &QAction::triggered, this, &FormatImpexWidget::CmSelectDataFile);
#ifndef IMPEX_OFF
    QObject::connect(ui->action_Run_script, &QAction::triggered, this, &FormatImpexWidget::CmRunScript);
#else
    ui->action_Run_script->setDisabled(true);
#endif

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
    QObject::connect(ui->action_Help_About, &QAction::triggered, this, &FormatImpexWidget::CmHelpAbout);
    QObject::connect(ui->actionContents, &QAction::triggered, this, &FormatImpexWidget::CmHelpContens);
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

    QObject::connect(ui->action_Update, &QAction::triggered, this, &FormatImpexWidget::CmUpdate);
    QObject::connect(ui->action_Delete, &QAction::triggered, this, &FormatImpexWidget::CmDeleteList);
    QObject::connect(ui->action_Read, &QAction::triggered, this, &FormatImpexWidget::CmRead);

    if(use_mode == FormatImpexWidget::editMode) {
        ui->runToolBar->hide();
        ui->action_Run_script->setDisabled(true);
        ui->actionSelect_Data_File->setDisabled(true);
        ui->datafileButton->setDisabled(true);
        ui->datafileEdit->setDisabled(true);
        ui->overwriteBox->setDisabled(true);
    }
    else {
        ui->direcBox->setEnabled(false);
        ui->schemaBox->setEnabled(false);
    }
}

std::string FormatImpexWidget::to_json_node()
{
    auto  impex_data = jsonio::JsonFree::object();
    impex_data["_id"] = ui->keyNoEdit->text().toStdString();
    impex_data["name"] = ui->nameEdit->text().toStdString();
    impex_data["impexschema"] = ui->impexschema->currentText().toStdString();
    impex_data["schema"] = ui->schemaBox->currentText().toStdString();
    impex_data["comment"] = ui->descEdit->text().toStdString();
    impex_data["direction"] = ui->direcBox->currentIndex();
    impex_data["impex"] = recipe_client->jsonmodel()->current_object().dump();
    return impex_data.dump();
}

void FormatImpexWidget::from_json_node(const jsonio::JsonBase& object)
{
    // setup data from json
    std::string schema, getdata;

    object.get_value_via_path<std::string>("_id", getdata, "");
    ui->keyNoEdit->setText(QString::fromStdString(getdata));
    object.get_value_via_path<std::string>("name", getdata, "new_impex");
    ui->nameEdit->setText(QString::fromStdString(getdata));
    object.get_value_via_path<std::string>("impexschema", schema, "");
    ui->impexschema->setCurrentText(QString::fromStdString(schema));
    object.get_value_via_path<std::string>("schema", getdata, "");
    ui->schemaBox->setCurrentText(QString::fromStdString(getdata));
    object.get_value_via_path<std::string>("comment", getdata, "");
    ui->descEdit->setText(QString::fromStdString(getdata));

    int direc;
    object.get_value_via_path("direction", direc, 0);
    ui->direcBox->setCurrentIndex(direc);

    // read impex
    object.get_value_via_path<std::string>("impex", getdata, "");
    schema_name = schema.c_str();
    recipe_client->setSchema(schema_name);
    recipe_client->setEditorData(schema, getdata);
    // not used now
    //object.get_value_via_path<std::string>("format", getdata, "");
    //object.get_value_via_path<std::string>("extension", getdata, "");
}


} // namespace jsonui
