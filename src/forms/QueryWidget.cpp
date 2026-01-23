#include <QMessageBox>
#include "ui_QueryWidget.h"
#include "jsonqml/forms/QueryWidget.h"
#include "jsonqml/forms/SelectDialog.h"
#include "jsonqml/forms/FileDialogs.h"
//#include "service/SchemaSelectDialog.h"
#include "jsonqml/forms/query_view.h"
#include "jsonqml/clients/vertex_client.h"
#ifndef IMPEX_OFF
#include "jsonimpex/yaml_xml2file.h"
#endif

namespace jsonqml {

std::string QueryWidget::query_collection_name = "queries";

QueryWidget::QueryWidget(const QString &atitle, const QStringList& schemas,
                         const jsonio::DBQueryDef& query, QWidget* parent):
    QMainWindow(parent),
    ui(new Ui::QueryWidget),
    query_client(),
    query_edit(),
    schema_name(schemas[0]),
    default_query(query),
    query_description(query)
{
    ui->setupUi(this);
    //this->setAttribute(Qt::WA_DeleteOnClose); // automatically delete itself when window is closed
    QString title = qApp->applicationName()+atitle;
    this->setWindowTitle(title);

    ui->splitterSelect->setStretchFactor(0, 1);
    ui->splitterSelect->setStretchFactor(1, 2);
    ui->qSchema->insertItems(0, schemas);

    // set up select model
    ///lstmodel = new StringVectorModel(query_description.fields(), this );
    ///ui->fieldsList->setModel(lstmodel);
    //ui->fieldsList->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->fieldsList->setSelectionMode(QAbstractItemView::MultiSelection);
    ui->fieldsList->setEditTriggers( QAbstractItemView::NoEditTriggers );

    // set up query edit mode
    query_edit.reset(new QueryView(ui->frameTree));
    query_model = new QueryModel(this);
    QueryDelegate* query_deleg = new QueryDelegate();
    query_edit->setModel(query_model);
    query_edit->setItemDelegate(query_deleg);
    query_edit->setColumnWidth(0, 350);
    ui->gridLayout_2->addWidget(query_edit.get(), 1, 0, 1, 1);

    // set up where part
    show_query();
    set_actions();
}

QueryWidget::~QueryWidget()
{}

std::string QueryWidget::getSchema() const
{
    return ui->qSchema->currentText().toStdString();
}

const jsonio::DBQueryDef& QueryWidget::getQuery()
{
    return get_query();
}

void QueryWidget::CmHelp()
{
///    helpWin(schema_name, "" );
}

void QueryWidget::CmHelpAbout()
{
///    helpWin( "QueryWidgetJsonui", "" );
}

void QueryWidget::CmLoadQuery()
{
    QString file;
    if(ChooseFileOpen(this, file,
                       "Please, select a file with JSON object", domFilters)) {
        query_client->readJson(file);  // yaml, html must be into readJson
    }
    from_json_node(query_client->jsonmodel()->current_object());
}

void QueryWidget::CmSaveQuery()
{
    std::string doc_json = to_json_node();
    query_client->setEditorData(query_schema, doc_json);

    auto file = query_client->fileSchemaExt(schema_name, "json");
    if(ChooseFileSave(this, file,
                       "Please, select a file to write the query",
                       domFilters, file))  {
        query_client->saveJson(file);  // yaml, html must be into readJson
    }
}

void QueryWidget::CmClone()
{
    query_client->setEditorOid("");
    qId->setText("");
}

void QueryWidget::CmRead()
{
    SelectDialog sel_dlg(single_selection, "Please, select a record to read/view",
                         query_client->tablemodel(), this);
    if(sel_dlg.exec())  {
        auto selndx = sel_dlg.selIndex();
        if(selndx>=0) {
            query_client->readEditorData(selndx);
            from_json_node(query_client->jsonmodel()->current_object());  /// posible exception
        }
    }
}

void QueryWidget::CmUpdate()
{
    std::string doc_json = to_json_node();
    query_client->setEditorData(query_schema, doc_json);
    query_client->updateRecord();
}

void QueryWidget::CmDeleteList()
{
    std::vector<std::string> keys;
    SelectDialog sel_dlg(multi_selection, "Please select the records you would like to delete.",
                         query_client->tablemodel(), this);
    if(sel_dlg.exec())  {
        auto ndxs = sel_dlg.allSelected();
        for(auto ndx: ndxs) {
            keys.push_back(query_client->tableId(ndx));
        }
    }
    if(!keys.empty()) {
        query_client->deleteRecords(std::move(keys));
    }
}


void QueryWidget::CmWhereApply()
{
    auto collection = ui->qSchema->currentText().toStdString();
    collection = ArangoDatabase::collectionFromSchema(collection);
    auto qrstr = query_model->getQuery(get_query_type(), collection);
    if(!qrstr.empty()) {
        ui->queryText->setText(QString::fromStdString(qrstr.queryString()));
    }
}

void QueryWidget::CmWhereRestore()
{
    auto selection = query_description.fields();
    query_description = default_query;
    query_description.setFields(selection);
    show_query();
}

void QueryWidget::CmAddWhereFields()
{
    // try {
    //     std::vector<std::string> f_list;
    //     SchemaSelectDialog dlg( this, "Please, mark data fields for the query table",
    //                            schema_name, f_list);
    //     if( dlg.exec() )
    //     {
    //         f_list = dlg.allSelected();
    //         auto f_desc = dlg.allSelectedTypes();
    //         query_edit->CmAddObjects(f_list, f_desc);
    //     }
    // }
    // catch(jsonio::jsonio_exception& e)
    // {
    //     QMessageBox::critical( this, "jsonio_exception", e.what() );
    // }
    // catch(std::exception& e)
    // {
    //     QMessageBox::critical( this, "std::exception", e.what() );
    // }
}

void QueryWidget::CmSelectFields()
{
    // try {
    //     SchemaSelectDialog dlg( this, "Please, mark data fields for the ouput table",
    //                            schema_name, query_description.fields());
    //     if( dlg.exec() )   {
    //         query_description.setFields( dlg.allSelected());
    //         lstmodel->setStringList(query_description.fields() );
    //     }
    // }
    // catch( jsonio::jsonio_exception& e)
    // {
    //     QMessageBox::critical( this, "jsonio_exception", e.what() );
    // }
    // catch(std::exception& e)
    // {
    //     QMessageBox::critical( this, "std::exception", e.what() );
    // }
}

void QueryWidget::CmAddSelectFields()
{
    // try  {
    //     std::vector<std::string> f_list;
    //     SchemaSelectDialog dlg( this, "Please, mark addition data fields for the ouput table",
    //                            schema_name, f_list);
    //     if( dlg.exec() )
    //     {
    //         f_list = dlg.allSelected();
    //         query_description.addFields(f_list);
    //         lstmodel->setStringList(query_description.fields());
    //     }
    // }
    // catch( jsonio::jsonio_exception& e)
    // {
    //     QMessageBox::critical( this, "jsonio_exception", e.what() );
    // }
    // catch(std::exception& e)
    // {
    //     QMessageBox::critical( this, "std::exception", e.what() );
    // }
}


void QueryWidget::AQL_toggled(bool checked)
{
    ui->actionAdd_and->setEnabled(checked);
    ui->actionAdd_or->setEnabled(checked);
    ui->actionAdd_not->setEnabled(checked);

    ui->actionAdd_gt->setEnabled(checked);
    ui->actionAdd_gte->setEnabled(checked);
    ui->actionAdd_lt->setEnabled(checked);
    ui->actionAdd_lte->setEnabled(checked);
    ui->actionAdd_noteq->setEnabled(checked);
    ui->actionAdd_eq->setEnabled(checked);

    ui->actionAdd_in->setEnabled(checked);
    ui->actionAdd_nin->setEnabled(checked);
    ui->actionAdd_like->setEnabled(checked);
    ui->actionAdd_regexp->setEnabled(checked);
    ui->actionAdd_not_regexp->setEnabled(checked);

    ui->actionAdd_List_Fieldpaths->setEnabled(!checked);

    if(query_edit->AQLmode(checked))  { // changed mode
        if((checked && default_query.condition()->type() == jsonio::DBQueryBase::qAQL)
            || (!checked && default_query.condition()->type() == jsonio::DBQueryBase::qTemplate)) {
            ui->queryText->setText(QString::fromStdString(default_query.condition()->queryString()));
        }
        else {
            ui->queryText->setText("");
        }
    }
}

void QueryWidget::schema_changed(const QString& text)
{
    if(schema_name != text) {
        schema_name = text;
        query_client->tablemodel()->updateQuery(new_schema_query());
    }
}

void QueryWidget::select_restore()
{
    query_description.setFields(default_query.fields());
    /// lstmodel->setStringList(query_description.fields());
}


//----------------------------------------------------------------

void QueryWidget::show_query()
{
    ui->qName->setText(QString::fromStdString(query_description.name()));
    ui->qComment->setText(QString::fromStdString(query_description.comment()));
    ui->qSchema->setCurrentText(QString::fromStdString(query_description.schema()));

    auto type = query_description.condition()->type();
    if(type == jsonio::DBQueryBase::qAQL) {
        ui->pAQL->setChecked(true);
    }
    /* else if(type == jsonio::DBQueryBase::qEJDB) {
            ui->pEJDB->setChecked(true);
    }*/
    else {
        ui->pTemplate->setChecked(true);
    }
    AQL_toggled(ui->pAQL->isChecked());

    /// lstmodel->setStringList(query_description.fields());
    ui->queryText->setText(QString::fromStdString(query_description.condition()->queryString()));

    // clear internal
    qId->setText("");
    query_edit->ClearAll();
}

void QueryWidget::set_actions()
{
    QObject::connect(ui->qSchema, &QComboBox::currentTextChanged, this, &QueryWidget::schema_changed);
    QObject::connect(ui->pAQL, &QRadioButton::toggled, this, &QueryWidget::AQL_toggled);

    // File
    QObject::connect(ui->action_Save_Current_Query_to_File, &QAction::triggered, this, &QueryWidget::CmSaveQuery);
    QObject::connect(ui->action_Load_Query_from_File, &QAction::triggered, this, &QueryWidget::CmLoadQuery);
    //QObject::connect(ui->action_Execute_Current_Query, &QAction::triggered, this, &QueryWidget::CmExecute);
    //QObject::connect(ui->action_Accept_curent_Query, &QAction::triggered, this, &QueryWidget::CmApplay);
    QObject::connect(ui->action_Close_Cancel, &QAction::triggered, this, &QueryWidget::close);

    // Select part
    QObject::connect(ui->action_Select_Fieldpaths, &QAction::triggered, this, &QueryWidget::CmSelectFields);
    QObject::connect(ui->action_Add_Fieldpaths, &QAction::triggered, this, &QueryWidget::CmAddSelectFields);
    QObject::connect(ui->action_restote_select, &QAction::triggered, this, &QueryWidget::select_restore);

    // Generate part
    QObject::connect(ui->action_Apply_from_Tree, &QAction::triggered, this, &QueryWidget::CmWhereApply);
    QObject::connect(ui->action_Restore_Query, &QAction::triggered, this, &QueryWidget::CmWhereRestore);
    QObject::connect(ui->action_Clear_Tree, &QAction::triggered, query_edit.get(), &QueryView::ClearAll);

    // FILTER
    QObject::connect(ui->actionAdd_List_Fieldpaths, &QAction::triggered, this, &QueryWidget::CmAddWhereFields);
    QObject::connect(ui->actionAdd_Fieldpath, &QAction::triggered, query_edit.get(), &QueryView::CmAddField);
    QObject::connect(ui->actionDelete_Line, &QAction::triggered, query_edit.get(), &QueryView::CmDelLine);

    QObject::connect(ui->actionAdd_and, &QAction::triggered, query_edit.get(), &QueryView::CmAddAnd);
    QObject::connect(ui->actionAdd_or, &QAction::triggered, query_edit.get(), &QueryView::CmAddOr);
    QObject::connect(ui->actionAdd_not, &QAction::triggered, query_edit.get(), &QueryView::CmAddNot);

    QObject::connect(ui->actionAdd_eq, &QAction::triggered, query_edit.get(), &QueryView::CmAddEq);
    QObject::connect(ui->actionAdd_noteq, &QAction::triggered, query_edit.get(), &QueryView::CmAddNotEq);
    QObject::connect(ui->actionAdd_gt, &QAction::triggered, query_edit.get(), &QueryView::CmAddGt);
    QObject::connect(ui->actionAdd_gte, &QAction::triggered, query_edit.get(), &QueryView::CmAddGte);
    QObject::connect(ui->actionAdd_lt, &QAction::triggered, query_edit.get(), &QueryView::CmAddLt);
    QObject::connect(ui->actionAdd_lte, &QAction::triggered, query_edit.get(), &QueryView::CmAddLte);

    QObject::connect(ui->actionAdd_in, &QAction::triggered, query_edit.get(), &QueryView::CmAddIn);
    QObject::connect(ui->actionAdd_nin, &QAction::triggered, query_edit.get(), &QueryView::CmAddNin);
    QObject::connect(ui->actionAdd_like, &QAction::triggered, query_edit.get(), &QueryView::CmAddLike);
    QObject::connect(ui->actionAdd_regexp, &QAction::triggered, query_edit.get(), &QueryView::CmAddRegexp);
    QObject::connect(ui->actionAdd_not_regexp, &QAction::triggered, query_edit.get(), &QueryView::CmAddNotRegexp);

    // Database
    QObject::connect(ui->action_Update, &QAction::triggered, this, &QueryWidget::CmUpdate);
    QObject::connect(ui->action_Clone, &QAction::triggered, this, &QueryWidget::CmClone);
    QObject::connect(ui->action_DeleteList, &QAction::triggered, this, &QueryWidget::CmDeleteList);
    QObject::connect(ui->action_Read, &QAction::triggered, this, &QueryWidget::CmRead);

    // Help
    QObject::connect(ui->actionHelp_About, &QAction::triggered, this, &QueryWidget::CmHelpAbout);
    QObject::connect(ui->action_Help, &QAction::triggered, this, &QueryWidget::CmHelp);

    qId.reset(new QLineEdit(ui->nameToolBar));
    qId->setEnabled(true);
    qId->setReadOnly(true);
    qId->setFocusPolicy(Qt::StrongFocus);
    qId->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    qId->setText(QString{});
    ui->nameToolBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    QLabel *aname = new QLabel("_id",ui->nameToolBar);
    ui->nameToolBar->addWidget(aname);
    ui->nameToolBar->addWidget(qId.get()); // setStretchableWidget( pLine );
}

std::string QueryWidget::to_json_node()
{
    auto object = jsonio::JsonFree::object();
    jsonio::DBQueryDef quer = get_query();
    quer.toJson(object);
    object.set_oid(qId->text().toStdString());

    // Filter
    object.set_value_via_path("filter_generator", query_model->getFILTER());
    return object.dump();
}

void QueryWidget::from_json_node(const jsonio::JsonBase& object)
{
    query_description.fromJson(object);
    show_query();
    std::string stroid;
    object.get_value_via_path<std::string>("_id", stroid, "");
    qId->setText(QString::fromStdString(stroid));

    // Filter
    std::string filter;
    object.get_value_via_path<std::string>("filter_generator", filter, "" );
    if(!filter.empty()) {
        query_model->setFILTER(filter);
    }
}

const jsonio::DBQueryDef& QueryWidget::get_query()
{
    query_description.setName(ui->qName->text().toStdString());
    query_description.setComment(ui->qComment->text().toStdString());
    query_description.setSchema(ui->qSchema->currentText().toStdString());
    std::string query_text = ui->queryText->toPlainText().toStdString();
    query_description.setCondition( std::make_shared<jsonio::DBQueryBase>(query_text, get_query_type()));

    return query_description;
}

jsonio::DBQueryBase::QType QueryWidget::get_query_type() const
{
    jsonio::DBQueryBase::QType type = jsonio::DBQueryBase::qTemplate;
    if(ui->pAQL->isChecked()) {
        type = jsonio::DBQueryBase::qAQL;
    }
    return type;
}

jsonio::DBQueryBase QueryWidget::new_schema_query()
{
    std::string queryString = "";
    if(!schema_name.isEmpty()) {
        queryString = "{ \"qschema\": \"";
        queryString += schema_name.toStdString();
        queryString += "\" }";
    }
    return jsonio::DBQueryBase(queryString, jsonio::DBQueryBase::qTemplate);
}

// bool QueryWidget::select_line( std::string& fldpath, const jsonio17::FieldDef** flddata )
// {
//     const auto d = impl_func();
//     SchemaSelectDialog dlg( this, "Please, mark data fields for the query table",
//                            d->schema_name );
//     if( !dlg.exec() )
//         return false;
//     fldpath = dlg.selIndex(flddata);
//     return !fldpath.empty();
// }

} // namespace jsonui
