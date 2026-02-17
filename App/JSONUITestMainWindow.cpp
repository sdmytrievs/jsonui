#include <iostream>

#include <QMessageBox>
#include "JSONUITestMainWindow.h"
#include "ui_JSONUITestMainWindow.h"

#include "jsonio/exceptions.h"
#include "jsonio/dbconnect.h"
#include "jsonqml/forms/PreferencesUI.h"
#include "jsonqml/forms/TableEditWindow.h"
#include "jsonqml/forms/VertexWidget.h"
#include "jsonqml/forms/EdgesWidget.h"
#include "jsonqml/forms/JsonEditWidget.h"
#include "jsonqml/forms/SelectDialog.h"
#include "jsonqml/forms/FormatImpexWidget.h"
#include "jsonqml/forms/HelpMainWindow.h"
#include "jsonqml/models/schema_model.h"
#include "jsonqml/arango_database.h"
#include "jsonqml/clients/settings_client.h"

TJSONUITestWin::TJSONUITestWin(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::TJSONUITestWin)
{
    ui->setupUi(this);

    set_actions();
    connect(&jsonqml::uiSettings(), &jsonqml::Preferences::errorChanged,
            this, &TJSONUITestWin::showErrorMessage);
    connect(&jsonqml::uiSettings(), &jsonqml::Preferences::settingsChanged,
            this, &TJSONUITestWin::updateViewMenu);

    // init help window
    new jsonqml::HelpMainWindow(nullptr);
}

TJSONUITestWin::~TJSONUITestWin()
{
    delete ui;
}

//  Connect all actions
void TJSONUITestWin::set_actions()
{
    // File
    connect( ui->actionSet_I_O_Files_Location, &QAction::triggered, this, &TJSONUITestWin::CmSetWorkDir);
    connect( ui->actionNewVertex, &QAction::triggered, this, &TJSONUITestWin::CmNewVertex);
    connect( ui->actionNew_Edge_Editor_Window, &QAction::triggered, this, &TJSONUITestWin::CmNewEdge);
    connect( ui->actionNew_Bson, &QAction::triggered, this, &TJSONUITestWin::CmNewJson);
    connect( ui->actionNew_Collection_Data_Editor, &QAction::triggered, this, &TJSONUITestWin::CmNewCollection);
    connect( ui->actionNew_CSV, &QAction::triggered, this, &TJSONUITestWin::CmNewCsv);
    connect( ui->actionNew_Impex, &QAction::triggered, this, &TJSONUITestWin::CmNewImpexFormat);
    connect( ui->actionE_xit, &QAction::triggered, qApp, &QApplication::quit);

    // Help
    connect( ui->action_Help_About, &QAction::triggered, this, &TJSONUITestWin::CmHelpAbout);
    connect( ui->actionContents, &QAction::triggered, this, &TJSONUITestWin::CmHelpContens);
    connect( ui->actionAuthors, &QAction::triggered, this, &TJSONUITestWin::CmHelpAuthors);
    connect( ui->actionLicense, &QAction::triggered, this, &TJSONUITestWin::CmHelpLicense);
    connect( ui->actionPreferences, &QAction::triggered, this, &TJSONUITestWin::CmSettingth);

    // View
    connect(ui->action_Show_comments, &QAction::toggled,
            [](bool val){ jsonqml::uiSettings().setBoolValue("ShowComments", val); });
    connect( ui->action_Display_enums, &QAction::toggled,
            [](bool val){ jsonqml::uiSettings().setBoolValue("ShowEnumNames", val); });
    connect( ui->action_Edit_id, &QAction::toggled,
            [](bool val){ jsonqml::uiSettings().setBoolValue("CanEdit_id", val); });
}

void TJSONUITestWin::CmHelpContens()
{
    jsonqml::helpWin("jsonui-test", "");
}

void TJSONUITestWin::CmHelpAbout()
{
    jsonqml::helpWin("AboutJsonui", "");
}

void TJSONUITestWin::CmHelpAuthors()
{
    jsonqml::helpWin("AuthorsJsonui", "");
}

void TJSONUITestWin::CmHelpLicense()
{
    jsonqml::helpWin("LicenseJsonui", "");
}

void TJSONUITestWin::updateViewMenu()
{
    ui->action_Show_comments->setChecked(jsonqml::JsonSchemaModel::showComments);
    ui->action_Display_enums->setChecked(jsonqml::JsonSchemaModel::useEnumNames);
    ui->action_Edit_id->setChecked(jsonqml::JsonSchemaModel::editID);
}

void TJSONUITestWin::showErrorMessage()
{
    auto error = jsonqml::uiSettings().lastError();
    if(!error.isEmpty()) {
        QMessageBox::critical(QApplication::activeWindow(), "error", error);
    }
}

void TJSONUITestWin::CmSettingth()
{
    try {
        jsonqml::PreferencesUI dlg(jsonqml::uiSettings(), this);
        if( !dlg.exec() )
            return;
    }
    catch(jsonio::jsonio_exception& e) {
        jsonqml::ui_logger->error("jsonio exception when update settings {}", e.what());
    }
    catch(std::exception& e)  {
        jsonqml::ui_logger->error("std exception when update settings {}", e.what());
    }
}

void TJSONUITestWin::CmNewCsv()
{
    try {
        if(!csv_window) {
            csv_window.reset(new jsonqml::TableEditWidget("CSV editor ",
                                                          jsonqml::TableFlag::TableIsEditable |
                                                              jsonqml::TableFlag::GraphDataEnabled |
                                                              jsonqml::TableFlag::RowSortingEnabled ));
        }
        else {
            csv_window->raise();
        }
        csv_window->show();
        csv_window->CmImportCSV();
    }
    catch( jsonio::jsonio_exception& e) {
        QMessageBox::critical( this, "jsonio_exception", e.what() );
    }
    catch(std::exception& e) {
        QMessageBox::critical( this, "std::exception", e.what() );
    }
}

void TJSONUITestWin::CmNewCollection()
{
    try {
        std::string testschema{};
        //auto lst =  jsonqml::transform2std(jsonqml::ArangoDatabase::getResourcesList());
        auto lst =  jsonqml::transform2std(jsonqml::ArangoDatabase::getSchemasList());

        if(!lst.empty()) {
            jsonqml::SelectDialog sel_dlg(jsonqml::single_selection,
                                          "Please, select collection to edit data", lst, ";" );
            if(sel_dlg.exec()) {
                testschema = lst[sel_dlg.getSelectedIndex()];
            }
        }

        if(!testschema.empty()) {
            jsonqml::JsonEditWidget* ptr = new jsonqml::JsonEditWidget(QString::fromStdString(testschema), this);
            ptr->show();
        }
    }
    catch(jsonio::jsonio_exception& e) {
        QMessageBox::critical(this, "jsonio_exception", e.what());
    }
    catch(std::exception& e) {
        QMessageBox::critical(this, "std::exception", e.what());
    }
}

void TJSONUITestWin::CmNewJson()
{
    try {
        jsonqml::JsonEditWidget* ptr = new jsonqml::JsonEditWidget(QString(), this);
        ptr->show();
    }
    catch(jsonio::jsonio_exception& e) {
        QMessageBox::critical(this, "jsonio_exception", e.what());
    }
    catch(std::exception& e) {
        QMessageBox::critical(this, "std::exception", e.what());
    }
}

void TJSONUITestWin::CmNewVertex()
{
    try {
        std::string testschema ="";

        auto vertex_lst = jsonio::DataBase::getVertexesList();
        if(!vertex_lst.empty()) {
            jsonqml::SelectDialog sel_dlg(jsonqml::single_selection, "Please, select collection to edit data", vertex_lst, ";" );
            if(sel_dlg.exec()) {
                testschema = vertex_lst[sel_dlg.getSelectedIndex()];
            }
        }
        if(!testschema.empty()) {
            jsonqml::VertexWidget* ptr = new jsonqml::VertexWidget(QString::fromStdString(testschema), "", this);
            ptr->show();
        }
    }
    catch(jsonio::jsonio_exception& e) {
        QMessageBox::critical(this, "jsonio_exception", e.what());
    }
    catch(std::exception& e) {
        QMessageBox::critical(this, "std::exception", e.what());
    }
}

void TJSONUITestWin::CmNewEdge()
{
    try {
        std::string testschema ="";

        auto edges_lst = jsonio::DataBase::getEdgesList();
        if(!edges_lst.empty()) {
            jsonqml::SelectDialog sel_dlg(jsonqml::single_selection, "Please, select collection to edit data", edges_lst, ";" );
            if(sel_dlg.exec()) {
                testschema = edges_lst[sel_dlg.getSelectedIndex()];
            }
        }
        if(!testschema.empty()) {
            jsonqml::EdgesWidget* ptr = new jsonqml::EdgesWidget(QString::fromStdString(testschema), this);
            ptr->show();
        }
    }
    catch(jsonio::jsonio_exception& e) {
        QMessageBox::critical(this, "jsonio_exception", e.what());
    }
    catch(std::exception& e) {
        QMessageBox::critical(this, "std::exception", e.what());
    }
}

void TJSONUITestWin::CmNewImpexFormat()
{
    try {
        jsonqml::FormatImpexWidget* ptr = new jsonqml::FormatImpexWidget(jsonqml::FormatImpexWidget::editMode);
        ptr->show();

        //std::string testschema ="";
        // auto vertex_lst = jsonio::DataBase::getVertexesList();
        // if(!vertex_lst.empty()) {
        //     jsonqml::SelectDialog sel_dlg(jsonqml::single_selection, "Please, select collection to edit data", vertex_lst, ";" );
        //     if(sel_dlg.exec()) {
        //         testschema = vertex_lst[sel_dlg.getSelectedIndex()];
        //     }
        // }
        // std::vector<std::string> lst = { "_id", "properties.name", "properties.class_"};
        // std::shared_ptr<jsonio::DBQueryBase> query = std::make_shared<jsonio::DBQueryBase>();
        // jsonio::DBQueryDef test_query(query, lst);
        // if(!testschema.empty()) {
        //     jsonqml::QueryWidget* ptr = new jsonqml::QueryWidget("", {QString::fromStdString(testschema)}, test_query, this);
        //     ptr->show();
        // }
    }
    catch(jsonio::jsonio_exception& e) {
        QMessageBox::critical(this, "jsonio_exception", e.what());
    }
    catch(std::exception& e) {
        QMessageBox::critical(this, "std::exception", e.what());
    }
}

void TJSONUITestWin::closeEvent(QCloseEvent *e)
{
    if(jsonqml::HelpMainWindow::pDia) {
        jsonqml::HelpMainWindow::pDia->close();
        delete jsonqml::HelpMainWindow::pDia;
    }
    QWidget::closeEvent(e);
}

