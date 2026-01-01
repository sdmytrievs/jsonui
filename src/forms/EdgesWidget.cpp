#include <QMessageBox>
#include <QKeyEvent>
#include <QHeaderView>
#include <QLineEdit>
#include "ui_EdgesWidget.h"
#include "jsonqml/forms/EdgesWidget.h"
#include "jsonqml/forms/json_view.h"
#include "jsonqml/forms/table_view.h"
#include "jsonqml/forms/FileDialogs.h"
#include "jsonqml/forms/SelectDialog.h"
#include "jsonqml/forms/waitingspinnerwidget.h"
#include "jsonqml/clients/edge_client.h"
#include "jsonqml/clients/table_client.h"
#include "jsonqml/clients/settings_client.h"
#include "jsonqml/models/schema_model.h"

namespace jsonqml {


EdgesWidget::EdgesWidget(const QString& aschema, QWidget *parent ):
    QMainWindow(parent),
    ui(new Ui::EdgesWidget),
    edge_client(),
    json_tree(),
    schema_name(aschema),
    is_default_query(true)
{
    auto edges_list = ArangoDatabase::getEdgesList();
    if( schema_name.isEmpty() && !edges_list.empty()) {
        schema_name = edges_list[0];
    }
    wait_dialog = new WaitingSpinnerWidget(this, true/*Qt::ApplicationModal*/, true);
    wait_dialog->start();

    edge_client.reset(new EdgeClient(schema_name, this));
    QObject::connect(edge_client.get(), &VertexClient::executingChange, this, &EdgesWidget::setExecuting);

    define_window();
}

EdgesWidget::EdgesWidget(bool in_edges, const QString &vertex_id, QWidget *parent):
    QMainWindow(parent),
    ui(new Ui::EdgesWidget),
    edge_client(),
    json_tree(),
    schema_name(),
    is_default_query(false)
{
    auto edges_list = ArangoDatabase::getEdgesList();
    if( schema_name.isEmpty() && !edges_list.empty()) {
        schema_name = edges_list[0];
    }
    wait_dialog = new WaitingSpinnerWidget(this, true/*Qt::ApplicationModal*/, true);
    wait_dialog->start();

    edge_client.reset(new EdgeClient(schema_name, in_edges, vertex_id, this));
    QObject::connect(edge_client.get(), &VertexClient::executingChange, this, &EdgesWidget::setExecuting);

    define_window();
    ui->typeBox->setEnabled(false);
}

EdgesWidget::~EdgesWidget()
{
    delete wait_dialog;
    qDebug() << "EdgesWidget destruct";
}

void EdgesWidget::setExecuting()
{
    if(edge_client->queryExecuting()) {
        ui_logger->info("EdgesWidget start process");
        wait_dialog->start();
    }
    else {
        wait_dialog->stop();
        ui_logger->info("EdgesWidget finish process");
    }
}

void EdgesWidget::updateViewMenu()
{
    ui->action_Show_comments->setChecked(JsonSchemaModel::showComments);
    ui->action_Display_enums->setChecked(JsonSchemaModel::useEnumNames);
    ui->action_Edit_id->setChecked(JsonSchemaModel::editID);
    ui->actionKeep_Data_Fields_Expanded->setChecked(JsonView::expandedFields);
}

void EdgesWidget::typeChanged(const QString& text)
{
    pLineTask->setText(text);
    schema_name = text;
    edge_client->setSchema(schema_name);
}

void EdgesWidget::openIncoming(const QString &vertex_id)
{
    edge_client->setIncomingEdges(vertex_id);
    edge_client->readEditorData(0);
}

void EdgesWidget::openOutgoing(const QString &vertex_id)
{
    edge_client->setOutgoingEdges(vertex_id);
    edge_client->readEditorData(0);
}

std::string EdgesWidget::currentKey() const
{
    return  edge_client->editorId().toStdString();
}

void EdgesWidget::openRecordKey(const QModelIndex& index)
{
    edge_client->readEditorData(index.row());
}


// Menu commands -----------------------------------------------------------

void EdgesWidget::CmHelpContens()
{
  //  helpWin("Edges", schema_name);
}

void EdgesWidget::CmImportJSON()
{
    QString file;
    if(ChooseFileOpen(this, file,
                       "Please, select a file with JSON object", domFilters)) {
        edge_client->readJson(file);  // yaml, html must be into readJson
    }
}

void EdgesWidget::CmExportJSON()
{
    auto file = edge_client->fileSchemaExt(schema_name, "json");
    if(ChooseFileSave(this, file,
                       "Please, select a file to write the data",
                       domFilters, file))  {
        edge_client->saveJson(file);  // yaml, html must be into readJson
    }
}

void EdgesWidget::CmNew()
{
    edge_client->jsonmodel()->setupModelData("", schema_name);
}

void EdgesWidget::CmClone()
{
    edge_client->setEditorOid("");
}

void EdgesWidget::CmRead()
{
    SelectDialog sel_dlg(single_selection, "Please, select a record to read/view",
                         edge_client->tablemodel(), this);
    if(sel_dlg.exec())  {
        auto selndx = sel_dlg.selIndex();
        if(selndx>=0) {
            edge_client->readEditorData(selndx);
            keys_view->move_to_key(edge_client->tableId(selndx));
        }
    }
}

void EdgesWidget::CmUpdate()
{
    edge_client->updateRecord();
}

void EdgesWidget::CmDelete()
{
    auto key = edge_client->editorId();
    QString msg = QString("Confirm deletion of %1 record?").arg(key);
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Record to delete", msg,
                                  QMessageBox::Yes|QMessageBox::No);
    if(reply == QMessageBox::Yes) {
        edge_client->deleteRecord(key);
    }
}

void EdgesWidget::set_actions()
{
    // File
    QObject::connect(ui->actionNew , &QAction::triggered, this, &EdgesWidget::CmNew);
    QObject::connect(ui->action_Clone_Structured_Data_Object , &QAction::triggered, this, &EdgesWidget::CmClone);
    QObject::connect(ui->actionE_xit, &QAction::triggered, this, &EdgesWidget::close);
    QObject::connect(ui->actionExport_Json_File, &QAction::triggered, this, &EdgesWidget::CmExportJSON);
    QObject::connect(ui->actionImport_Json_File, &QAction::triggered, this, &EdgesWidget::CmImportJSON);

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
    //QObject::connect(ui->action_Help_About, &QAction::triggered, this, &EdgesWidget::CmHelpAbout);
    QObject::connect(ui->actionContents, &QAction::triggered, this, &EdgesWidget::CmHelpContens);
    //QObject::connect(ui->actionAuthors, &QAction::triggered, this, &EdgesWidget::CmHelpAuthors);
    //QObject::connect(ui->actionLicense, &QAction::triggered, this, &EdgesWidget::CmHelpLicense);

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
    QObject::connect(ui->actionFollow_In_Vertex, &QAction::triggered, this, &EdgesWidget::CmFollowInVertex);
    QObject::connect(ui->actionFollow_Out_Vertex, &QAction::triggered, this, &EdgesWidget::CmFollowOutVertex);

    // Record
    QObject::connect(ui->actionNext_Record, &QAction::triggered, keys_view.get(), &DBKeysTable::CmNext);
    QObject::connect(ui->actionPrevious_Record, &QAction::triggered, keys_view.get(), &DBKeysTable::CmPrevious);
    QObject::connect(ui->action_Update, &QAction::triggered, this, &EdgesWidget::CmUpdate);
    QObject::connect(ui->action_Delete, &QAction::triggered, this, &EdgesWidget::CmDelete);
    QObject::connect(ui->actionDeleteMultiple, &QAction::triggered, this, &EdgesWidget::CmDeleteSelect);
    QObject::connect(ui->actionSearch_Results, &QAction::triggered, this, &EdgesWidget::CmDisplaySearchResult);
    if(is_default_query) {
        ui->actionSearch->setDisabled(true);
        ui->action_Read->setDisabled(true);
    }
    else {
        QObject::connect(ui->actionSearch, &QAction::triggered, this, &EdgesWidget::CmSearchQuery);
        QObject::connect(ui->action_Read, &QAction::triggered, this, &EdgesWidget::CmRead);
    }
    QObject::connect(ui->actionBackup_Queried_Records_to_File, &QAction::triggered,
                     this,  &EdgesWidget::CmBackupQueriedRecordstoFile);
    QObject::connect(ui->actionRestore_Multiple_Records_from_File, &QAction::triggered,
                     this, &EdgesWidget::CmRestoreMultipleRecordsfromFile);
    QObject::connect(ui->actionBackup_Graph_to_File, &QAction::triggered,
                     this,  &EdgesWidget::CmBackupGraphtoFile);
    QObject::connect(ui->actionRestore_Graph_from_File, &QAction::triggered,
                     this, &EdgesWidget::CmRestoreGraphfromFile);
    QObject::connect(ui->actionIncoming_Vertex_Query, &QAction::triggered, this, &EdgesWidget::CmIncomingQuery);
    QObject::connect(ui->actionOutgoing_Vertex_Query, &QAction::triggered, this, &EdgesWidget::CmOutgoingQuery);

    pLineTask = new QLineEdit(ui->nameToolBar);
    pLineTask->setEnabled(true);
    pLineTask->setFocusPolicy(Qt::ClickFocus);
    pLineTask->setReadOnly(true);
    pLineTask->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    pLineTask->setText(schema_name);
    ui->nameToolBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    ui->nameToolBar->addWidget(pLineTask); // setStretchableWidget( pLine );
}

void EdgesWidget::define_window()
{
    ui->setupUi(this);
    ui->keySplitter->setStretchFactor(0, 1);
    ui->mainSplitter->setStretchFactor(0, 0);
    ui->mainSplitter->setStretchFactor(1, 0);
    ui->mainSplitter->setStretchFactor(2, 1);
    ui->typeBox->setEnabled(true);

    this->setAttribute(Qt::WA_DeleteOnClose); // automatically delete itself when window is closed
    QString title = qApp->applicationName()+" Structured Data Editor and Database Browser";
    this->setWindowTitle(title);

    ui->typeBox->insertItems(0, edge_client->schemasList());
    ui->typeBox->setCurrentText(schema_name);

    // define edit tree view
    QItemDelegate* deleg_schema = new JsonDelegate();
    json_tree.reset(new JsonView(edge_client.get(), this));
    json_tree->setItemDelegate(deleg_schema);
    json_tree->setColumnWidth(0, 250);
    json_tree->expandToDepth(2);
    json_tree->setColumnWidth( 0, 150 );
    json_tree->expandToDepth(1);
    ui->gridLayout->addWidget(json_tree.get(), 1, 0, 1, 2);

    auto mode = single_selection;
    keys_table_client.reset(new TableClient(edge_client->tablemodel(), mode, this));
    keys_view.reset(new DBKeysTable(mode, keys_table_client.get(),
                                    [&](const QModelIndex& index){ this->openRecordKey(index); }, this));
    QObject::connect(keys_view.get(), &DBKeysTable::clicked,
                     this, &EdgesWidget::openRecordKey);
    keys_view->setSelectionBehavior(QAbstractItemView::SelectRows);
    keys_view->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    ui->keySplitter->insertWidget(0, keys_view.get());
    ui->keySplitter->setStretchFactor(0, 4);

    in_keys_table_client.reset(new TableClient(edge_client->outtablemodel(), mode, this));
    in_keys_view.reset(new DBKeysTable(mode, in_keys_table_client.get(), [](const QModelIndex&){;}, this));
    QObject::connect(in_keys_view.get(), &DBKeysTable::doubleClicked, this, &EdgesWidget::addIncoming);
    in_keys_view->setSelectionBehavior(QAbstractItemView::SelectRows);
    in_keys_view->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    ui->verticalLayout_2->addWidget(in_keys_view.get());

    out_keys_table_client.reset(new TableClient(edge_client->intablemodel(), mode, this));
    out_keys_view.reset(new DBKeysTable(mode, out_keys_table_client.get(), [](const QModelIndex&){;}, this));
    QObject::connect(out_keys_view.get(), &DBKeysTable::doubleClicked, this, &EdgesWidget::addOutgoing);
    out_keys_view->setSelectionBehavior(QAbstractItemView::SelectRows);
    out_keys_view->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    ui->verticalLayout->addWidget(out_keys_view.get());

    set_actions();
    connect(&jsonqml::uiSettings(), &jsonqml::Preferences::settingsChanged, this, &EdgesWidget::updateViewMenu);
    QObject::connect(ui->typeBox, &QComboBox::currentTextChanged, this, &EdgesWidget::typeChanged);

    qDebug() << "EdgesWidget construct";
}


/*
void EdgesWidgetPrivate::reset_type_box( const QString& text )
{
    QObject::disconnect( qUi->typeBox, &QComboBox::currentTextChanged,
                         this, &EdgesWidgetPrivate::type_changed);
    qUi->typeBox->setCurrentText(text);
    pLineTask->setText(text);
    current_schema_name = text.toStdString();
    QObject::connect( qUi->typeBox, &QComboBox::currentTextChanged,
                      this, &EdgesWidgetPrivate::type_changed);
}*/


} // namespace jsonui

