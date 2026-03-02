#include <QLabel>
#include <QMessageBox>
#include <QFileInfo>
#include <QDesktopServices>
#include <QPrinter>
#include <QPrintDialog>
#include "ui_HelpMainWindow.h"
#include "HelpEditDialog.h"
#include "jsonqml/forms/HelpMainWindow.h"
#include "jsonqml/forms/SelectDialog.h"
#include "jsonqml/clients/help_client.h"
#include "jsonqml/clients/settings_client.h"
#include "../jsonui_version.h"

namespace jsonqml {

HelpMainWindow* HelpMainWindow::pDia = nullptr;

void helpWin(const QString &name, const QString &item)
{
    if(HelpMainWindow::pDia) {
        HelpMainWindow::pDia->showHelp(name, item);
        HelpMainWindow::pDia->show();
        HelpMainWindow::pDia->raise();
    }
}

HelpMainWindow::HelpMainWindow(QWidget* parent):
    QMainWindow(parent),
    ui(new Ui::HelpWindowData),
    help_client()
{
    ui->setupUi(this);
    ui->splitter->setStretchFactor(0, 500);
    setWindowTitle(" Help Viewer");
    pDia = this;

    help_client.reset(new HelpClient(this));
    help_view.reset(new HelpTextBrowser(help_client.get(), this));
    ui->verticalLayout_2->addWidget(help_view.get());

    set_actions();
}

HelpMainWindow::~HelpMainWindow()
{}

void HelpMainWindow::showHelp(const QString &name, const QString &item)
{
    QString doc_url = name+".md";
    // if(!item.isEmpty()) {
    //     doc_url +="#"+item;
    // }
    help_view->setSource(doc_url);
    help_view->scrollToAnchor("#"+item);
    ui->actionCreate_Update_Docpage_into_DB->setEnabled(false);
}

void HelpMainWindow::helpAbout()
{
    showHelp("helpAbout", "");
}

void HelpMainWindow::helpOnHelp()
{
    showHelp("helpOnHelp", "how-to-use-gem-selektor-application");
}

void HelpMainWindow::helpVersion()
{
    QMessageBox::information(this,  QString("JSONUI v.") + JSONUI_VERSION,
                             QString("\nThis is jsonui code package v.") + JSONUI_VERSION +
                                 "\n\nusing Qt6 framework v." + JSONUI_QT_VERSION +
                                 "\n\nOS " + JSONUI_OSX  + " compiler  " + JSONUI_COMPILER_ID + " " + JSONUI_COMPILER_VERSION +
                                 "\n\nFor GEMS R&D community, GPL v.3\n\n"
                                 "(c) 2026,  GEMS Development Team\n\n"
                                 "      Paul Scherrer Institute" );
}

void HelpMainWindow::CmEdit()
{
    if(!help_client->canEdit()) {
        return;
    }
    auto old_text = help_view->toMarkdown();
    HelpEditDialog dlg(help_client.get(), old_text, this);
    QObject::connect(&dlg, &HelpEditDialog::updateMarkdown, this, &HelpMainWindow::updateDocPage);

    if(dlg.exec()) {
        ui->actionCreate_Update_Docpage_into_DB->setEnabled(true);
    }
    else {
        updateDocPage(old_text);
    }
}

void HelpMainWindow::CmRead()
{
    // if(!help_client->canEdit()) {
    //     return;
    // }
    SelectDialog sel_dlg(single_selection, "Please, select a record to read/view",
                         help_client->tablemodel(), this);
    if(sel_dlg.exec())  {
        auto selndx = sel_dlg.selIndex();
        if(selndx>=0) {
            auto doc_id = help_client->tableId(selndx);
            int type=0;
            auto url = help_client->urlfromOid(doc_id, type);
            if(type==0) {
                help_view->setSource(url);
                ui->actionCreate_Update_Docpage_into_DB->setEnabled(false);
            }
        }
    }
}

void HelpMainWindow::CmUpdate()
{
    if(!help_client->canEdit()) {
        return;
    }
    help_client->saveMarkdownDocument(adress_line->text(), help_view->toMarkdown());
    ui->actionCreate_Update_Docpage_into_DB->setEnabled(false);
}

void HelpMainWindow::CmDeleteList()
{
    if(!help_client->canEdit()) {
        return;
    }
    std::vector<std::string> keys;
    SelectDialog sel_dlg(multi_selection, "Please select the records you would like to delete.",
                         help_client->tablemodel(), this);
    if(sel_dlg.exec())  {
        auto ndxs = sel_dlg.allSelected();
        for(auto ndx: ndxs) {
            keys.push_back(help_client->tableId(ndx));
        }
    }
    if(!keys.empty()) {
        help_client->deleteRecords(std::move(keys));
    }
}

void HelpMainWindow::action_find_next()
{
    if(find_line && !find_line->text().isEmpty()) {
        QTextDocument::FindFlags flg;
        if(ui->action_Case_sensetiv->isChecked()) {
            flg |= QTextDocument::FindCaseSensitively;
        }
        help_view->find(find_line->text(), flg);
    }
}

void HelpMainWindow::action_find_previous()
{
    if(find_line && !find_line->text().isEmpty()) {
        QTextDocument::FindFlags flg;
        flg |= QTextDocument::FindBackward;
        if(ui->action_Case_sensetiv->isChecked()) {
            flg |= QTextDocument::FindCaseSensitively;
        }
        help_view->find(find_line->text(), flg);
    }
}

void HelpMainWindow::updateDocPage(const QString &mdtext)
{
    help_view->setMarkdown(mdtext);
}

void HelpMainWindow::set_actions()
{
    QObject::connect(ui->actionHome, &QAction::triggered, [&]() { showHelp("start", ""); } );
    QObject::connect(ui->actionBack, &QAction::triggered, help_view.get(), &QTextBrowser::backward);
    QObject::connect(ui->actionForward, &QAction::triggered, help_view.get(), &QTextBrowser::forward);
    QObject::connect(help_view.get(), &QTextBrowser::forwardAvailable, ui->actionForward, &QAction::setEnabled);
    QObject::connect(help_view.get(), &QTextBrowser::backwardAvailable, ui->actionBack, &QAction::setEnabled);
    ui->actionForward->setEnabled(false);
    ui->actionBack->setEnabled(false);

    QObject::connect(ui->action_Find, &QAction::triggered, this, &HelpMainWindow::action_find_next);
    QObject::connect(ui->actionFind_Next, &QAction::triggered, this, &HelpMainWindow::action_find_next);
    QObject::connect(ui->actionFind_Previous, &QAction::triggered, this, &HelpMainWindow::action_find_previous);

    QObject::connect(ui->actionZoom_In, &QAction::triggered, [&]() { help_view->zoomIn(1); });
    QObject::connect(ui->actionZoom_Out, &QAction::triggered, [&]() { help_view->zoomOut(1); });

    QObject::connect(ui->action_About, &QAction::triggered, this, &HelpMainWindow::helpAbout);
    QObject::connect(ui->actionVersion, &QAction::triggered, this, &HelpMainWindow::helpVersion);
    QObject::connect(ui->actionHelp_on_Help, &QAction::triggered, this, &HelpMainWindow::helpOnHelp);
    QObject::connect(ui->action_Print, &QAction::triggered, help_view.get(), &HelpTextBrowser::helpPrint);

    // Database
    if(help_client->canEdit()) {
        QObject::connect(ui->action_Read_another_Docpage_from_DB, &QAction::triggered, this, &HelpMainWindow::CmRead);
        QObject::connect(ui->action_Edit_DocPage,&QAction::triggered, this, &HelpMainWindow::CmEdit);
        QObject::connect(ui->actionCreate_Update_Docpage_into_DB, &QAction::triggered, this, &HelpMainWindow::CmUpdate);
        QObject::connect(ui->action_Delete_Docpages_from_DB, &QAction::triggered, this, &HelpMainWindow::CmDeleteList);
    }
    else {
        ui->menu_Database->menuAction()->setVisible(false);
    }

    QLabel *label_2 = new QLabel(ui->toolFind);
    label_2->setText("Find:");
    ui->toolFind->addWidget(label_2);

    find_line.reset(new QLineEdit(ui->toolFind));
    find_line->setEnabled(true);
    find_line->setFocusPolicy(Qt::ClickFocus);
    ui->toolFind->addWidget(find_line.get());
    ui->toolFind->addAction(ui->actionFind_Previous);
    ui->toolFind->addAction(ui->actionFind_Next);

    if(help_client->canEdit()) {
        QLabel *label = new QLabel(ui->toolAddress);
        label->setText("Address:");
        ui->toolAddress->addWidget(label);

        adress_line.reset(new QLineEdit(ui->toolAddress));
        adress_line->setEnabled(true);
        adress_line->setFocusPolicy(Qt::ClickFocus);
        adress_line->setReadOnly(true);
        adress_line->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        ui->toolAddress->addWidget(adress_line.get());
        QObject::connect(help_client.get(), &HelpClient::currentIdChanged, adress_line.get(), &QLineEdit::setText);
    }
}

QVariant HelpTextBrowser::loadResource(int type, const QUrl &name)
{
    ui_logger->info("loadResource {} {}", type, name.toString().toStdString());
    return  help_client->readResource(type, name);
}

void HelpTextBrowser::doSetSource(const QUrl &url, QTextDocument::ResourceType type)
{
    if(!url.isRelative() && url.scheme() != QString("qrc") && url.scheme() != QString("file")) {
        QDesktopServices::openUrl(url);
    }
    else {
        QTextBrowser::doSetSource(url, type);
    }
}

void HelpTextBrowser::helpPrint()
{
    QPrinter printer(QPrinter::PrinterResolution/*QPrinter::HighResolution*/);
    QPrintDialog dlg(&printer, this);
    dlg.setWindowTitle(tr("Print Help Page"));
    if(dlg.exec()) {
        print(&printer);
    }
}

} // namespace jsonui


