#include <QMessageBox>
#include <QKeyEvent>
#include <QHeaderView>
#include <QLineEdit>
#include "ui_VertexWidget.h"
#include "jsonqml/forms/VertexWidget.h"
#include "jsonqml/forms/json_view.h"
#include "jsonqml/forms/table_view.h"
#include "jsonqml/forms/FileDialogs.h"
#include "jsonqml/forms/SelectDialog.h"
#include "jsonqml/forms/waitingspinnerwidget.h"
#include "jsonqml/clients/vertex_client.h"
#include "jsonqml/clients/table_client.h"
#include "jsonqml/clients/settings_client.h"
#include "jsonqml/models/schema_model.h"
#include "jsonqml/models/db_keys_model.h"
#include "jsonio/dbconnect.h"

namespace jsonqml {

void VertexWidget::updt_view_menu()
{
    ui->action_Show_comments->setChecked(JsonSchemaModel::showComments);
    ui->action_Display_enums->setChecked(JsonSchemaModel::useEnumNames);
    ui->action_Edit_id->setChecked(JsonSchemaModel::editID);
    ui->actionKeep_Data_Fields_Expanded->setChecked(JsonView::expandedFields);
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
    /// QObject::connect( ui->action_Help_About, &QAction::triggered, this, &VertexWidget::CmHelpAbout);
    /// QObject::connect( ui->actionContents, &QAction::triggered, this, &VertexWidget::CmHelpContens);
    /// QObject::connect( ui->actionAuthors, &QAction::triggered, this, &VertexWidget::CmHelpAuthors);
    /// QObject::connect( ui->actionLicense, &QAction::triggered, this, &VertexWidget::CmHelpLicense);

    // View
    /// QObject::connect( ui->action_Show_comments, &QAction::toggled, &uiSettings(), &UIPreferences::CmShowComments);
    /// QObject::connect( ui->action_Display_enums, &QAction::toggled, &uiSettings(), &UIPreferences::CmDisplayEnums);
    /// QObject::connect( ui->action_Edit_id, &QAction::toggled, &uiSettings(), &UIPreferences::CmEditID);
    /// QObject::connect( ui->actionKeep_Data_Fields_Expanded, &QAction::toggled, &uiSettings(), &UIPreferences::CmEditExpanded);
    updt_view_menu();
    QObject::connect(ui->actionFollow_Edges, &QAction::triggered, this, &VertexWidget::CmFollowOutEdges);
    QObject::connect(ui->actionFollow_Edges_Incoming, &QAction::triggered, this, &VertexWidget::CmFollowInEdges);

    // Record
    QObject::connect(ui->actionNext_Record, &QAction::triggered, keys_view.get(), &DBKeysTable::CmNext);
    QObject::connect(ui->actionPrevious_Record, &QAction::triggered, keys_view.get(), &DBKeysTable::CmPrevious);
    connect(vertex_client.get(), &VertexClient::updatedKeyList, keys_view.get(), &DBKeysTable::move_to_key);
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


VertexWidget::VertexWidget(const QString& aschema, QWidget *parent):
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

    vertex_client.reset(new VertexClient(schema_name, this));
    QObject::connect(vertex_client.get(), &VertexClient::executingChange, this, &VertexWidget::setExecuting);

    // define edit tree view
    QItemDelegate* deleg_schema = new JsonDelegate();
    json_tree.reset(new JsonView(vertex_client.get(), this));
    json_tree->setItemDelegate(deleg_schema);
    json_tree->setColumnWidth(0, 250);
    json_tree->expandToDepth(2);
    ui->verticalLayout->addWidget(json_tree.get());

    auto mode = single_selection;
    keys_table_client.reset(new TableClient(vertex_client->tablemodel(), mode, this));
    keys_view.reset(new DBKeysTable(mode, keys_table_client.get(),
                                    [&](const QModelIndex& index){ this->openRecordKey(index); }, this));
    QObject::connect(keys_view.get(), &DBKeysTable::clicked,
                     this, &VertexWidget::openRecordKey);
    keys_view->setSelectionBehavior(QAbstractItemView::SelectRows);
    keys_view->horizontalHeader()->setSectionResizeMode( QHeaderView::Interactive );
    ui->horizontalLayout->addWidget(keys_view.get());

    set_actions();
    qDebug() << "VertexWidget construct";
}

VertexWidget::~VertexWidget()
{
    delete wait_dialog;
    qDebug() << "VertexWidget destruct";
}

/// Start long database process
void VertexWidget::setExecuting()
{
    if(vertex_client->queryExecuting()) {
        ui_logger->info("VertexWidget start process");
        wait_dialog->start();
    }
    else {
        wait_dialog->stop();
        ui_logger->info("VertexWidget finish process");
    }
}

std::string VertexWidget::currentKey() const
{
    return  vertex_client->editorId().toStdString();
}

void VertexWidget::openRecordKey(const QModelIndex& index)
{
    vertex_client->readEditorData(index.row());
}

// Menu commands -----------------------------------------------------------

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

} // namespace jsonui


