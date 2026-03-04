#include <QMessageBox>
#include <QKeyEvent>
#include <QHeaderView>
#include <QLineEdit>
#include "ui_VertexWidget.h"
#include "jsonio/dbquerybase.h"
#include "jsonqml/models/schema_model.h"
#include "jsonqml/clients/vertex_client.h"
#include "jsonqml/clients/table_client.h"
#include "jsonqml/clients/settings_client.h"
#include "jsonqml/forms/VertexWidget.h"
#include "jsonqml/forms/EdgesWidget.h"
#include "jsonqml/forms/json_view.h"
#include "jsonqml/forms/table_view.h"
#include "jsonqml/forms/FileDialogs.h"
#include "jsonqml/forms/SelectDialog.h"
#include "jsonqml/forms/QueryWidget.h"
#include "jsonqml/forms/TableEditWindow.h"
#include "jsonqml/forms/HelpMainWindow.h"
#include "jsonqml/forms/waitingspinnerwidget.h"

namespace jsonqml {

VertexWidget::VertexWidget(const QString& aschema, const QString& document_id, QWidget *parent):
    QMainWindow(parent),
    ui(new Ui::VertexWidget),
    vertex_client(),
    json_tree(),
    schema_name(aschema)
{
    ui->setupUi(this);
    ui->keySplitter->setStretchFactor(0, 2);
    ui->keySplitter->setStretchFactor(1, 1);
    ui->mainSplitter->setStretchFactor(0, 1);
    ui->mainSplitter->setStretchFactor(1, 4);

    this->setAttribute(Qt::WA_DeleteOnClose); // automatically delete itself when window is closed
    QString title = qApp->applicationName()+" Structured Data Editor and Database Browser";
    this->setWindowTitle(title);

    wait_dialog = new WaitingSpinnerWidget(this, true/*Qt::ApplicationModal*/, true);
    wait_dialog->start();

    vertex_client.reset(new VertexClient(schema_name, document_id, this));
    QObject::connect(vertex_client.get(), &VertexClient::executingChange, this, &VertexWidget::setExecuting);

    // define edit tree view
    QItemDelegate* deleg_schema = new JsonDelegate();
    json_tree.reset(new JsonView(vertex_client.get(), this));
    json_tree->setItemDelegate(deleg_schema);
    json_tree->setColumnWidth(0, 250);
    ui->verticalLayout->addWidget(json_tree.get());

    auto mode = single_selection;
    keys_table_client.reset(new TableClient(vertex_client->tablemodel(), mode, this));
    keys_view.reset(new DBKeysTable(mode, keys_table_client.get(),
                                    [&](const QModelIndex& index){ this->openRecordKey(index); }, this));
    QObject::connect(keys_view.get(), &DBKeysTable::clicked,
                     this, &VertexWidget::openRecordKey);
    QObject::connect(vertex_client.get(), &VertexClient::updatedKeyList, keys_view.get(), &DBKeysTable::move_to_key);
    keys_view->setSelectionBehavior(QAbstractItemView::SelectRows);
    keys_view->horizontalHeader()->setSectionResizeMode( QHeaderView::Interactive );
    ui->horizontalLayout->addWidget(keys_view.get());

    set_actions();
    QObject::connect(&jsonqml::uiSettings(), &jsonqml::Preferences::settingsChanged, this, &VertexWidget::updateViewMenu);
    QObject::connect(vertex_client.get(), &VertexClient::updatedQueryString, ui->queryEdit, &QTextEdit::setText);
    qDebug() << "VertexWidget construct";
}

VertexWidget::~VertexWidget()
{
    delete wait_dialog;
    qDebug() << "VertexWidget destruct";
}

void VertexWidget::openDocument(const QString &key_document)
{
    vertex_client->readEditorId(key_document);
    keys_view->move_to_key(key_document.toStdString());
}

void VertexWidget::changeQuery(jsonio::DBQueryDef query)
{
    vertex_client->tablemodel()->setQuery(*query.condition(), query.fields());
}

void VertexWidget::setExecuting(bool exec)
{
    if(exec) {
        ui_logger->info("VertexWidget start process");
        wait_dialog->start();
    }
    else {
        wait_dialog->stop();
        ui_logger->info("VertexWidget finish process");
    }
}

void VertexWidget::updateViewMenu()
{
    ui->action_Show_comments->setChecked(JsonSchemaModel::showComments);
    ui->action_Display_enums->setChecked(JsonSchemaModel::useEnumNames);
    ui->action_Edit_id->setChecked(JsonSchemaModel::editID);
    ui->actionKeep_Data_Fields_Expanded->setChecked(JsonClient::expanded_fields);
}

std::string VertexWidget::currentKey() const
{
    return  vertex_client->editorId().toStdString();
}

void VertexWidget::openRecordKey(const QModelIndex& index)
{
    vertex_client->readEditorData(keys_table_client->rowIndex(index));
}

// Menu commands -----------------------------------------------------------

void VertexWidget::CmHelpAbout()
{
    helpWin("AboutVertexWidget", "");
}

void VertexWidget::CmHelpContens()
{
    helpWin(schema_name, "");
}

void VertexWidget::CmFollowInEdges()
{
    auto vertex_id = vertex_client->editorId();
    if(!vertex_id.isEmpty()) {
        jsonqml::EdgesWidget* ptr = new jsonqml::EdgesWidget(true, vertex_id);
        ptr->show();
    }
}

void VertexWidget::CmFollowOutEdges()
{
    auto vertex_id = vertex_client->editorId();
    if(!vertex_id.isEmpty()) {
        jsonqml::EdgesWidget* ptr = new jsonqml::EdgesWidget(false, vertex_id);
        ptr->show();
    }
}

void VertexWidget::CmImportJSON()
{
    QString file;
    if(ChooseFileOpen(this, file,
                       "Please, select a file with JSON object", domFilters)) {
        vertex_client->readJson(file);  // yaml, html must be into readJson
    }
}

void VertexWidget::CmExportJSON()
{
    auto file = vertex_client->fileSchemaExt(schema_name, "json");
    if(ChooseFileSave(this, file,
                       "Please, select a file to write the data",
                       domFilters, file))  {
        vertex_client->saveJson(file);  // yaml, html must be into readJson
    }
}

void VertexWidget::CmNew()
{
    vertex_client->jsonmodel()->setupModelData("", schema_name);
}

void VertexWidget::CmClone()
{
    vertex_client->setEditorOid("");
}

void VertexWidget::CmRead()
{
    SelectDialog sel_dlg(single_selection, "Please, select a record to read/view",
                         vertex_client->tablemodel(), this);
    if(sel_dlg.exec())  {
        auto selndx = sel_dlg.selIndex();
        if(selndx>=0) {
            vertex_client->readEditorData(selndx);
            keys_view->move_to_key(vertex_client->tableId(selndx));
        }
    }
}

void VertexWidget::CmUpdate()
{
    vertex_client->updateRecord();
}

void VertexWidget::CmDelete()
{
    auto key = vertex_client->editorId();
    QString msg = QString("Confirm deletion of %1 record?").arg(key);
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Record to delete", msg,
                                  QMessageBox::Yes|QMessageBox::No);
    if(reply == QMessageBox::Yes) {
        vertex_client->deleteRecord(key);
    }
}

void VertexWidget::CmBackupQueriedRecordstoFile()
{
    auto keys = select_keys("Please, select a records to backup.");
    if(!keys.empty()) {
        auto file = vertex_client->fileSchemaExt(schema_name, "json");
        if(ChooseFileSave(this, file,
                           "Please, select a file to write (document) records",
                           domFilters, file))  {
            vertex_client->backupRecords(file, std::move(keys));
        }
    }
}

void VertexWidget::CmRestoreMultipleRecordsfromFile()
{
    QString file;
    if(ChooseFileOpen(this, file,
                       "Please, select a file with JSON objects", domFilters)) {
        vertex_client->restoreRecords(file);
    }
}

void VertexWidget::CmBackupGraphtoFile()
{
    auto keys = select_keys("Please, select a records to backup graph.");
    if(!keys.empty()) {
        auto file = vertex_client->fileSchemaExt(schema_name, "json");
        if(ChooseFileSave(this, file,
                           "Please, select a file to write graph data",
                           domFilters, file))  {
            vertex_client->backupGraph(file, std::move(keys));
        }
    }
}

void VertexWidget::CmRestoreGraphfromFile()
{
    QString file;
    if(ChooseFileOpen(this, file,
                       "Please, select a file with JSON graph", domFilters)) {
        vertex_client->restoreGraph(file);
    }
}

void VertexWidget::CmDeleteSelect()
{
    auto keys = select_keys("Please select the records you would like to delete.");
    if(!keys.empty()) {
        vertex_client->deleteRecords(std::move(keys));
    }
}

void VertexWidget::CmDisplaySearchResult()
{
    try  {
        if(!result_window)  {
            result_window.reset(new TableEditWidget("Vertex Query Result window", TableFlag::RowSortingEnabled, this));
        }
        else {
            result_window->raise();
        }
        model_line_t header = vertex_client->tablemodel()->getHeader();
        model_table_t table = vertex_client->tablemodel()->getTable();
        result_window->resetTable(std::move(table), header);
        result_window->show();
     }
    catch(std::exception& e)
    {
        QMessageBox::critical(this, "CmDisplaySearchResult", e.what());
    }
}

void VertexWidget::CmSearchQuery()
{
    try {
        auto query = vertex_client->tablemodel()->query();
        auto query_fields = vertex_client->tablemodel()->queryFields();
        query_fields.erase(query_fields.begin());
        if(!query_window) {
            query_window.reset(new QueryWidget("Vertex Query Widget", {schema_name}, query, query_fields, this));
            QObject::connect(query_window.get(), &QueryWidget::queryChanged, this, &VertexWidget::changeQuery);
        }
        else {
            query_window->updateQuery({schema_name}, query, query_fields);
            query_window->raise();
        }
        query_window->show();
    }
    catch(std::exception& e) {
        QMessageBox::critical( this, "CmSearchQuery", e.what() );
    }
}

// private -----------------------------------------------------------

void VertexWidget::closeEvent(QCloseEvent* e)
{
    if(query_window) {
        query_window->close();
    }
    if(result_window) {
        result_window->close();
    }
    QMainWindow::closeEvent(e);
}

std::vector<std::string> VertexWidget::select_keys(const QString& title)
{
    std::vector<std::string> keys;
    SelectDialog sel_dlg(multi_selection, title, vertex_client->tablemodel(), this);
    if(sel_dlg.exec())  {
        auto ndxs = sel_dlg.allSelected();
        for(auto ndx: ndxs) {
            keys.push_back(vertex_client->tableId(ndx));
        }
    }
    return keys;
}

void VertexWidget::set_actions()
{
    // File
    QObject::connect( ui->actionNew, &QAction::triggered, this, &VertexWidget::CmNew);
    QObject::connect( ui->action_Clone_Structured_Data_Object, &QAction::triggered, this, &VertexWidget::CmClone);
    QObject::connect( ui->actionE_xit, &QAction::triggered, this, &VertexWidget::close);
    QObject::connect( ui->actionExport_Json_File, &QAction::triggered, this, &VertexWidget::CmExportJSON);
    QObject::connect( ui->actionImport_Json_File, &QAction::triggered, this, &VertexWidget::CmImportJSON);

    // Edit
    QObject::connect(ui->actionAdd_one_field, &QAction::triggered, json_tree.get(), &JsonView::CmAddObject);
    QObject::connect(ui->action_Clone_Current, &QAction::triggered, json_tree.get(), &JsonView::CmCloneObject);
    QObject::connect(ui->action_Delete_field, &QAction::triggered, json_tree.get(), &JsonView::CmDelObject);
    QObject::connect(ui->actionRemove_Alternatives_Union, &QAction::triggered, json_tree.get(), &JsonView::CmDelObjectsUnion);
    QObject::connect(ui->actionReset_Data_to_Defaults, &QAction::triggered, json_tree.get(), &JsonView::CmResetObject);
    QObject::connect(ui->action_Resize_array, &QAction::triggered, json_tree.get(), &JsonView::CmResizeArray);
    QObject::connect(ui->action_Calculator, &QAction::triggered, json_tree.get(), &JsonView::CmCalc);
    QObject::connect(ui->actionCopy_Field_Path, &QAction::triggered, json_tree.get(), &JsonView::CopyFieldPath);
    QObject::connect(ui->actionCopy_Field, &QAction::triggered, json_tree.get(), &JsonView::CopyField);
    QObject::connect(ui->actionPaste_Field, &QAction::triggered, json_tree.get(), &JsonView::PasteField);

    // Help
    QObject::connect(ui->action_Help_About, &QAction::triggered, this, &VertexWidget::CmHelpAbout);
    QObject::connect(ui->actionContents, &QAction::triggered, this, &VertexWidget::CmHelpContens);
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
    QObject::connect(ui->actionFollow_Edges, &QAction::triggered, this, &VertexWidget::CmFollowOutEdges);
    QObject::connect(ui->actionFollow_Edges_Incoming, &QAction::triggered, this, &VertexWidget::CmFollowInEdges);

    // Record
    QObject::connect(ui->actionNext_Record, &QAction::triggered, keys_view.get(), &DBKeysTable::CmNext);
    QObject::connect(ui->actionPrevious_Record, &QAction::triggered, keys_view.get(), &DBKeysTable::CmPrevious);
    QObject::connect(ui->action_Update, &QAction::triggered, this, &VertexWidget::CmUpdate);
    QObject::connect(ui->action_Delete, &QAction::triggered, this, &VertexWidget::CmDelete);
    QObject::connect(ui->action_Read, &QAction::triggered, this, &VertexWidget::CmRead);
#ifndef IMPEX_OFF
    QObject::connect(ui->actionImport, &QAction::triggered, this, &VertexWidget::CmImportFormat);
    QObject::connect(ui->actionUpdateFormat, &QAction::triggered, this, &VertexWidget::CmUpdateFormat);
    QObject::connect(ui->actionExport, &QAction::triggered, this, &VertexWidget::CmExportFormat);
#else
    ui->actionImport->setVisible(false);
    ui->actionUpdateFormat->setVisible(false);
    ui->actionExport->setVisible(false);
#endif
    QObject::connect(ui->actionDeleteMultiple, &QAction::triggered, this, &VertexWidget::CmDeleteSelect);
    QObject::connect(ui->actionSearch_Results, &QAction::triggered, this, &VertexWidget::CmDisplaySearchResult);
    QObject::connect(ui->actionSearch, &QAction::triggered, this, &VertexWidget::CmSearchQuery);

    QObject::connect(ui->actionBackup_Queried_Records_to_File, &QAction::triggered,
                     this,  &VertexWidget::CmBackupQueriedRecordstoFile);
    QObject::connect(ui->actionRestore_Multiple_Records_from_File, &QAction::triggered,
                     this, &VertexWidget::CmRestoreMultipleRecordsfromFile);

    QObject::connect(ui->actionBackup_Graph_to_File, &QAction::triggered,
                     this,  &VertexWidget::CmBackupGraphtoFile);
    QObject::connect(ui->actionRestore_Graph_from_File, &QAction::triggered,
                     this, &VertexWidget::CmRestoreGraphfromFile);

    QLineEdit* pLineTask = new QLineEdit(ui->nameToolBar);
    pLineTask->setEnabled(true);
    pLineTask->setFocusPolicy(Qt::ClickFocus);
    pLineTask->setReadOnly(true);
    pLineTask->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    pLineTask->setText(schema_name);
    ui->nameToolBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    ui->nameToolBar->addWidget(pLineTask); // setStretchableWidget(pLine);
}

} // namespace jsonui


