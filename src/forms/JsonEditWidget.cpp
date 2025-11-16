#include <QMessageBox>
#include <QKeyEvent>
#include <QLineEdit>
#include <QHeaderView>
#include "ui_JsonEditWidget.h"
#include "jsonqml/forms/JsonEditWidget.h"
#include "jsonqml/forms/json_view.h"
#include "jsonqml/forms/FileDialogs.h"
#include "jsonqml/clients/json_client.h"
#include "jsonqml/clients/settings_client.h"

#include "jsonio/dbjsondoc.h"
#ifndef IMPEX_OFF
#include "jsonimpex/yaml_xml2file.h"
#else
#include "jsonio/txt2file.h"
#endif

namespace jsonqml {


//  Connect all actions
void JsonEditWidget::set_actions()
{
    // File
    QObject::connect(ui->actionNew , &QAction::triggered, this, &JsonEditWidget::CmNew);
    QObject::connect(ui->action_Clone_Structured_Data_Object , &QAction::triggered, this, &JsonEditWidget::CmClone);
    QObject::connect(ui->actionE_xit, &QAction::triggered, this, &JsonEditWidget::close);
    QObject::connect(ui->actionExport_Json_File, &QAction::triggered, this, &JsonEditWidget::CmExportJSON);
    QObject::connect(ui->actionImport_Json_File, &QAction::triggered, this, &JsonEditWidget::CmImportJSON);
#ifndef IMPEX_OFF
    QObject::connect(ui->action_Export_YAML_File, &QAction::triggered, this, &JsonEditWidget::CmExportYAML);
    QObject::connect(ui->actionImport_YAML_File, &QAction::triggered, this, &JsonEditWidget::CmImportYAML);
    QObject::connect(ui->actionExport_XML_File, &QAction::triggered, this, &JsonEditWidget::CmExportXML);
    QObject::connect(ui->actionImport_XML_File, &QAction::triggered, this, &JsonEditWidget::CmImportXML);
#else
    ui->action_Export_YAML_File->setVisible(false);
    ui->actionImport_YAML_File->setVisible(false);
    ui->actionExport_XML_File->setVisible(false);
    ui->actionImport_XML_File->setVisible(false);
#endif

    // Edit
    QObject::connect(ui->actionAdd_one_field, &QAction::triggered, json_tree.get(), &JsonView::CmAddObject);
    QObject::connect(ui->action_Clone_Current, &QAction::triggered, json_tree.get(), &JsonView::CmCloneObject);
    QObject::connect(ui->action_Delete_field, &QAction::triggered, json_tree.get(), &JsonView::CmDelObject);
    QObject::connect(ui->action_Delete_fields, &QAction::triggered, json_tree.get(), &JsonView::CmDelObjects);
    if(!schema_name.isEmpty()) {
        QObject::connect(ui->actionRemove_Alternatives_Union, &QAction::triggered, json_tree.get(), &JsonView::CmDelObjectsUnion);
        QObject::connect(ui->action_Add_fields, &QAction::triggered, json_tree.get(), &JsonView::CmAddObjects);
        QObject::connect(ui->actionReset_Data_to_Defaults, &QAction::triggered, json_tree.get(), &JsonView::CmResetObject);
    }
    else {
        ui->actionRemove_Alternatives_Union->setVisible(false);
        ui->action_Add_fields->setVisible(false);
        ui->actionReset_Data_to_Defaults->setVisible(false);
    }

    QObject::connect(ui->action_Resize_array, &QAction::triggered, json_tree.get(), &JsonView::CmResizeArray);
    QObject::connect(ui->action_Calculator, &QAction::triggered, json_tree.get(), &JsonView::CmCalc);
    QObject::connect(ui->actionCopy_Field_Path, &QAction::triggered, json_tree.get(), &JsonView::CopyFieldPath);
    QObject::connect(ui->actionCopy_Field, &QAction::triggered, json_tree.get(), &JsonView::CopyField);
    QObject::connect(ui->actionPaste_Field, &QAction::triggered, json_tree.get(), &JsonView::PasteField);

    // Help
    /*   QObject::connect(ui->action_Help_About, &QAction::triggered, this, &JsonEditWidget::CmHelpAbout);
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
        QObject::connect( ui->action_Create, &QAction::triggered, this, &JsonEditWidget::CmCreateInsert);
        QObject::connect( ui->action_Delete, &QAction::triggered, this, &JsonEditWidget::CmDelete);
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

JsonEditWidget::JsonEditWidget(const QString& aschema, const QString& collect, QWidget *parent):
    QMainWindow(parent),
    ui(new Ui::JsonEditWidget),
    json_client(),
    json_tree(),
    schema_name(aschema),
    collection_name(collect)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose); // automatically delete itself when window is closed

    QString title = qApp->applicationName()+" Structured Data Editor and Database Browser";
    setWindowTitle(title);

    json_client.reset(new JsonClient(this));
    if(!schema_name.isEmpty())  {
        json_client->setSchema(schema_name);
    }

    // define edit tree view
    QItemDelegate* deleg_schema = nullptr;
    if(schema_name.isEmpty()) {
        deleg_schema = new JsonFreeDelegate();
    }
    else {
        deleg_schema = new JsonSchemaDelegate();
    }
    json_tree.reset(new JsonView(json_client.get(), this));
    json_tree->setItemDelegate(deleg_schema);
    json_tree->setColumnWidth(0, 250);
    json_tree->expandToDepth(0);
    ui->verticalLayout->addWidget(json_tree.get());

    set_actions();
    // set up default data
    CmNew();
}

JsonEditWidget::~JsonEditWidget()
{}

// Menu commands -----------------------------------------------------------

// Set default json record
void JsonEditWidget::CmNew()
{
    try {
        json_client->jsonmodel()->setupModelData("", schema_name);
    }
    catch( jsonio::jsonio_exception& e)
    {
        QMessageBox::critical( this, "jsonio_exception", e.what() );
    }
    catch(std::exception& e)
    {
        QMessageBox::critical( this, "std::exception", e.what() );
    }
}

// Clone json record (clear _id)
void JsonEditWidget::CmClone()
{
    try {
        json_tree->setOid("");
        json_tree->setValueViaPath("_rev", std::string(""));
        json_tree->hide();
        json_tree->show();
    }
    catch(jsonio::jsonio_exception& e)
    {
        QMessageBox::critical( this, "jsonio_exception", e.what() );
    }
    catch(std::exception& e)
    {
        QMessageBox::critical( this, "std::exception", e.what() );
    }
}

// Save current record to DB file as new
void JsonEditWidget::CmCreateInsert()
{
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
    }
}


// Read new record from DB
void JsonEditWidget::CmRead()
{
    try {
        if( dbcollection.get() == nullptr )
            return;

        // Select key to load
        auto reckey = SelectKeyFrom(dbcollection.get(), this, "Please, select a record to read/view" );
        if(!reckey.empty()) {
            dbcollection->readDocument(reckey);
            reset_json( dbcollection->getJson(true), "" );
            contents_changed = false;
        }
    }
    catch( jsonio::jsonio_exception& e)
    {
        QMessageBox::critical( this, "jsonio_exception", e.what() );
    }
    catch(std::exception& e)
    {
        QMessageBox::critical( this, "std::exception", e.what() );
    }
}

// Save current record to DB file
void JsonEditWidget::CmUpdate()
{
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
}

// Delete current record from DB
void JsonEditWidget::CmDelete()
{
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
}


/// Read bson record from json file fileName
void JsonEditWidget::CmImportJSON()
{
    try {
        QString fileName;
        if(  ChooseFileOpen( this, fileName,
                             "Please, select a file with JSON object", jsonFilters  ))
        {
#ifndef IMPEX_OFF
            jsonio17::JsonYamlXMLFile file( fileName );
            file.set_type( jsonio17::TxtFile::Json );
#else
            jsonio::JsonFile file(fileName);
#endif
         json_client->jsonmodel()->setupModelData(file.load_json(), ""/*schema_name*/);
         //   if( !reset_json( file.load_json(), ""/*schema_from_name(fileName)*/) )
         //       jsonio::JSONIO_THROW( "editors", 10, "Try to read another schema format file "+ fileName );

            contents_changed = true;
        }
    }
    catch( jsonio::jsonio_exception& e)
    {
        QMessageBox::critical( this, "jsonio_exception", e.what() );
    }
    catch(std::exception& e)
    {
        QMessageBox::critical( this, "std::exception", e.what() );
    }
}

/// Write bson record to json file fileName
void JsonEditWidget::CmExportJSON()
{
    try {
        QString fileName = file_schema_ext( schema_name, "json" );
        if(  ChooseFileSave( this, fileName,
                             "Please, select a file to write the data", jsonFilters, fileName  ))
        {
#ifndef IMPEX_OFF
            jsonio17::JsonYamlXMLFile file( fileName );
            file.set_type( jsonio17::TxtFile::Json );
#else
           jsonio:: JsonFile file( fileName );
#endif
            file.save( json_tree->current_object() );
        }
    }
    catch( jsonio::jsonio_exception& e)
    {
        QMessageBox::critical( this, "jsonio_exception", e.what() );
    }
    catch(std::exception& e)
    {
        QMessageBox::critical( this, "std::exception", e.what() );
    }
}

#ifndef IMPEX_OFF

/// Read bson record from yaml file fileName
void JsonEditWidget::CmImportYAML()
{
    try {
        auto d = impl_func();

        std::string fileName;
        if(  ChooseFileOpen( this, fileName,
                             "Please, select a file with YAML data object", yamlFilters  ))
        {
            jsonio17::JsonYamlXMLFile file( fileName );
            file.set_type( jsonio17::TxtFile::Yaml );
            if( !d->reset_json( file.load_json(), ""/*schema_from_name(fileName)*/) )
                jsonio17::JSONIO_THROW( "editors", 11, "Try to read another schema format file "+ fileName );
            d->set_content_state( true );
        }
    }
    catch( jsonio17::jsonio_exception& e)
    {
        QMessageBox::critical( this, "jsonio_exception", e.what() );
    }
    catch(std::exception& e)
    {
        QMessageBox::critical( this, "std::exception", e.what() );
    }
}

/// Write bson record to YAML file fileName
void JsonEditWidget::CmExportYAML()
{
    try {
        const auto d = impl_func();
        std::string fileName = file_schema_ext( d->current_schema_name, "yaml" );
        if(  ChooseFileSave( this, fileName,
                             "Please, select a file to write the data", yamlFilters, fileName  ))
        {
            jsonio17::JsonYamlXMLFile file( fileName );
            file.set_type( jsonio17::TxtFile::Yaml );
            file.save( d->json_tree->current_object() );
        }
    }
    catch( jsonio17::jsonio_exception& e)
    {
        QMessageBox::critical( this, "jsonio_exception", e.what() );
    }
    catch(std::exception& e)
    {
        QMessageBox::critical( this, "std::exception", e.what() );
    }
}

/// Read bson record from xml file fileName
void JsonEditWidget::CmImportXML()
{
    try {
        auto d = impl_func();

        std::string fileName;
        if(  ChooseFileOpen( this, fileName,
                             "Please, select a file with xml object to read", xmlFilters  ))
        {
            jsonio17::JsonYamlXMLFile file( fileName );
            file.set_type( jsonio17::TxtFile::XML );
            if( !d->reset_json( file.load_json(), ""/*schema_from_name(fileName)*/) )
                jsonio17::JSONIO_THROW( "editors", 12, "Try to read another schema format file "+ fileName );
            d->set_content_state( true );
        }
    }
    catch( jsonio17::jsonio_exception& e)
    {
        QMessageBox::critical( this, "jsonio_exception", e.what() );
    }
    catch(std::exception& e)
    {
        QMessageBox::critical( this, "std::exception", e.what() );
    }
}

/// Write bson record to file fileName
void JsonEditWidget::CmExportXML()
{
    try {
        const auto d = impl_func();
        std::string fileName = file_schema_ext( d->current_schema_name, "xml" );
        if(  ChooseFileSave( this, fileName,
                             "Please, select a file to write the data", xmlFilters, fileName  ))
        {
            jsonio17::JsonYamlXMLFile file( fileName );
            file.set_type( jsonio17::TxtFile::XML );
            file.save( d->json_tree->current_object() );
        }
    }
    catch( jsonio17::jsonio_exception& e)
    {
        QMessageBox::critical( this, "jsonio_exception", e.what() );
    }
    catch(std::exception& e)
    {
        QMessageBox::critical( this, "std::exception", e.what() );
    }
}

#endif


} // namespace jsonui

