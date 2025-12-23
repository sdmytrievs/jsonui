#include <QMessageBox>
#include <QKeyEvent>
#include <QLineEdit>
#include <QHeaderView>
#include <QComboBox>
#include "ui_JsonEditWidget.h"
#include "jsonqml/forms/JsonEditWidget.h"
#include "jsonqml/forms/json_view.h"
#include "jsonqml/forms/FileDialogs.h"
#include "jsonqml/forms/SelectDialog.h"
#include "jsonqml/clients/vertex_client.h"
#include "jsonqml/clients/settings_client.h"

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
    json_tree->expandToDepth(0);
    ui->verticalLayout->addWidget(json_tree.get());

    set_actions();
    CmNew();
}

JsonEditWidget::~JsonEditWidget()
{
    qDebug() << "JsonEditWidget destruct";
}

// Set default json record
void JsonEditWidget::CmNew()
{
    json_client->jsonmodel()->setupModelData("", schema_name);
}

// Clone json record (clear _id)
void JsonEditWidget::CmClone()
{
    VertexClient* vclient = dynamic_cast<VertexClient*>(json_client.get());
    if(vclient) {
        vclient->setEditorOid("");
    }
}

// Save current record to DB file as new
void JsonEditWidget::CmBackup()
{
    /*
    try {
        if( dbcollection.get() == nullptr )
            return;
        std::string key = current_key();
        std::string rid = dbcollection->createDocument( key );
        json_tree->updateModelData(dbcollection->getJson(), schema_name );
    }
    catch( jsonio::jsonio_exception& e)
    {
        QMessageBox::critical( this, "jsonio_exception", e.what() );
    }
    catch(std::exception& e)
    {
        QMessageBox::critical( this, "std::exception", e.what() );
    }*/
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

// Save current record to DB file
void JsonEditWidget::CmUpdate()
{
    /*
    try {
        if( dbcollection.get() == nullptr )
            return;
        std::string key = current_key();
        dbcollection->updateDocument( key );
        contents_changed =  false;
    }
    catch( jsonio17::jsonio_exception& e)
    {
        QMessageBox::critical( this, "jsonio_exception", e.what() );
    }
    catch(std::exception& e)
    {
        QMessageBox::critical( this, "std::exception", e.what() );
    }
   */
}

// Delete current record from DB
void JsonEditWidget::CmDeleteList()
{ /*
    try {
        if(dbcollection.get() == nullptr)
            return;
        // get current key
        std::string key = current_key();
        QString msg = QString("Confirm deletion of %1 record?").arg( key.c_str() );
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question( this, "Record to delete", msg,
                                      QMessageBox::Yes|QMessageBox::No);
        if (reply == QMessageBox::No)
            return;
        dbcollection->deleteDocument(key);
        contents_changed =  false;
    }
    catch( jsonio::jsonio_exception& e)
    {
        QMessageBox::critical( this, "jsonio_exception", e.what() );
    }
    catch(std::exception& e)
    {
        QMessageBox::critical( this, "std::exception", e.what() );
    }
  */
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
    /*
    QObject::connect(ui->action_Help_About, &QAction::triggered, this, &JsonEditWidget::CmHelpAbout);
    QObject::connect(ui->actionContents, &QAction::triggered, this, &JsonEditWidget::CmHelpContens);
    QObject::connect(ui->actionAuthors, &QAction::triggered, this, &JsonEditWidget::CmHelpAuthors);
    QObject::connect(ui->actionLicense, &QAction::triggered, this, &JsonEditWidget::CmHelpLicense);

    // View
    QObject::connect( ui->action_Show_comments, &QAction::toggled, &uiSettings(), &UIPreferences::CmShowComments);
    QObject::connect( ui->action_Display_enums, &QAction::toggled, &uiSettings(), &UIPreferences::CmDisplayEnums);
    QObject::connect( ui->action_Edit_id, &QAction::toggled, &uiSettings(), &UIPreferences::CmEditID);
    QObject::connect( ui->actionKeep_Data_Fields_Expanded, &QAction::toggled, &uiSettings(), &UIPreferences::CmEditExpanded);
    */

    if(!collection_name.isEmpty()) {
        QObject::connect( ui->action_Update, &QAction::triggered, this, &JsonEditWidget::CmUpdate);
        QObject::connect( ui->action_Backup, &QAction::triggered, this, &JsonEditWidget::CmBackup);
        QObject::connect( ui->action_Delete_list, &QAction::triggered, this, &JsonEditWidget::CmDeleteList);
        QObject::connect( ui->action_Read, &QAction::triggered, this, &JsonEditWidget::CmRead);
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

