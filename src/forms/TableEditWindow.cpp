#include <QMessageBox>
#include <QFile>
#include "ui_TableEditWindow.h"
#include "jsonqml/forms/TableEditWindow.h"
#include "jsonqml/forms/table_view.h"
#include "jsonqml/forms/FileDialogs.h"
#include "jsonqml/forms/GraphDialog.h"
#include "jsonqml/clients/chart_client.h"
#include "jsonqml/clients/settings_client.h"

namespace jsonqml {


TableEditWidget::TableEditWidget(const QString& title,
                                 int mode, QWidget *parent):
    QMainWindow(parent),
    ui(new Ui::TableEditWidget),
    table_client(nullptr),
    table_view(nullptr),
    graph_dlg(nullptr),
    widow_title(title),  mode_using(mode)
{
    ui->setupUi(this);

    setWindowTitle(title);
    //setAttribute(Qt::WA_DeleteOnClose); // automatically delete itself when window is closed

    if(mode_using & TableFlag::GraphDataEnabled) {
        table_client.reset(new ChartClient(mode, this));
    }
    else {
        table_client.reset(new CSVClient(mode, this));
    }

    table_view.reset(new MatrixTable(mode, table_client.get(), this));
    ui->verticalLayout->addWidget(table_view.get());

    if( !(mode_using & TableFlag::NoMenuEnabled) ) {
        QObject::disconnect(table_view.get(), &MatrixTable::customContextMenuRequested,
                            table_view.get(), &MatrixTable::slotPopupContextMenu);
        QObject::connect(table_view.get(), &MatrixTable::customContextMenuRequested,
                         this, &TableEditWidget::slotPopupContextMenu);
    }
    set_actions();
}

TableEditWidget::~TableEditWidget()
{}

// Menu commands -----------------------------------------------------------

/// Write all table to csv file
void TableEditWidget::CmExportCSV()
{
    QString file_name;
    if(ChooseFileSave(this, file_name, "Please, select file to write", csvFilters)) {
        table_client->saveCSV(file_name);
    }
    if(!uiSettings().lastError().isEmpty())  {
        QMessageBox::critical(this, "Export CSV", uiSettings().lastError());
    }
}

/// Read table from csv file
void TableEditWidget::CmImportCSV()
{
    QString file_name;
    if(ChooseFileOpen(this, file_name, "Please, select file with csv object", csvFilters)) {
        table_client->readCSV(file_name);
        close_graph();
    }
    if(!uiSettings().lastError().isEmpty())  {
        QMessageBox::critical(this, "Import CSV", uiSettings().lastError());
    }
}

void TableEditWidget::CmPlotTable()
{
    try {
        show_graph_data();
    }
    catch(jsonio::jsonio_exception& e) {
        QMessageBox::critical( this, "jsonio_exception", e.what() );
    }
    catch(std::exception& e) {
        QMessageBox::critical( this, "std::exception", e.what() );
    }
}


void TableEditWidget::closeEvent(QCloseEvent* )
{
    close_graph();
}

void TableEditWidget::slotPopupContextMenu(const QPoint &pos)
{
    if(mode_using & TableFlag::NoMenuEnabled) {
        return;
    }

    QAction* act;
    QMenu *menu = new QMenu(this);
    QModelIndex index = table_view->indexAt(pos);
    table_view->makePopupContextMenu(menu, index);

    if(mode_using & TableFlag::GraphDataEnabled) {
        bool addsep = false;
        auto section = index.column();
        ChartClient* chart_client = dynamic_cast<ChartClient*>(table_client.get());

        if( chart_client->isNumber(section) ) {
            menu->addSeparator();
            addsep = true;
            act =  new QAction( "Toggle &X", this );
            act->setShortcut(this->tr("Ctrl+X"));
            act->setStatusTip("Mark columns as X (toggle)");
            QObject::connect(act, &QAction::triggered, this, &TableEditWidget::toggleX);
            menu->addAction(act);
        }

        if( chart_client->isNumber(section) )  {
            if( !addsep )
                menu->addSeparator();
            act =  new QAction("Toggle &Y", this );
            act->setShortcut(this->tr("Ctrl+Y"));
            act->setStatusTip("Mark columns as Y (toggle)");
            QObject::connect(act, &QAction::triggered, this, &TableEditWidget::toggleY);
            menu->addAction(act);
        }
    }
    menu->exec(mapToGlobal(pos));
    delete menu;
}

void TableEditWidget::toggleX()
{
    auto section = table_view->currentIndex().column();
    ChartClient* chart_client = dynamic_cast<ChartClient*>(table_client.get());
    if(chart_client && section>=0) {
        chart_client->toggleX(section);
    }
}

void TableEditWidget::toggleY()
{
    auto section = table_view->currentIndex().column();
    ChartClient* chart_client = dynamic_cast<ChartClient*>(table_client.get());
    if(chart_client && section>=0) {
        chart_client->toggleY(section);
    }
}

//  Connect all actions
void TableEditWidget::set_actions()
{
    // File
    QObject::connect(ui->action_Help, &QAction::triggered, this, &TableEditWidget::CmHelp);
    QObject::connect(ui->actionClose, &QAction::triggered, this, &TableEditWidget::close);
    QObject::connect(ui->actionExport_to_CSV_File, &QAction::triggered, this, &TableEditWidget::CmExportCSV);

    if( mode_using & TableFlag::NoMenuEnabled ) {
        ui->actionExport_Selected_to_CSV_File->setVisible(false);
        ui->actionSelect_row->setVisible(false);
        ui->actionSelect_co_lumn->setVisible(false);
        ui->actionSelect_all->setVisible(false);
        ui->action_Copy->setVisible(false);
        ui->actionCopy_wit_h_names->setVisible(false);
    }
    else {
        QObject::connect(ui->actionSelect_row, &QAction::triggered, table_view.get(), &MatrixTable::selRow);
        QObject::connect(ui->actionSelect_co_lumn, &QAction::triggered, table_view.get(), &MatrixTable::selColumn);
        QObject::connect(ui->actionSelect_all, &QAction::triggered, table_view.get(), &MatrixTable::selectAll);
        QObject::connect(ui->action_Copy, &QAction::triggered, table_view.get(), &MatrixTable::copyData);
        QObject::connect(ui->actionCopy_wit_h_names, &QAction::triggered, table_view.get(), &MatrixTable::copyDataHeader);
    }

    if( /*is csv_model ?*/ (mode_using & TableFlag::TableIsEditable) && !(mode_using & TableFlag::NoMenuEnabled) ) {
        QObject::connect(ui->actionImport_from_CVS, &QAction::triggered, this, &TableEditWidget::CmImportCSV);
    }
    else {
        ui->actionImport_from_CVS->setVisible(false);
    }

    if( (mode_using & TableFlag::TableIsEditable) && !(mode_using & TableFlag::NoMenuEnabled) ) {
        QObject::connect(ui->actionCalculator, &QAction::triggered, table_view.get(), &MatrixTable::runCalc);
        QObject::connect(ui->actionC_ut, &QAction::triggered, table_view.get(), &MatrixTable::cutData);
        QObject::connect(ui->action_Paste, &QAction::triggered, table_view.get(), &MatrixTable::pasteData);
        QObject::connect(ui->actionPaste_transposed, &QAction::triggered, table_view.get(), &MatrixTable::pasteTransposedData);
        QObject::connect(ui->actionClear, &QAction::triggered, table_view.get(), &MatrixTable::clearData);
    }
    else {
        ui->actionCalculator->setVisible(false);
        ui->actionC_ut->setVisible(false);
        ui->action_Paste->setVisible(false);
        ui->actionPaste_transposed->setVisible(false);
        ui->actionClear->setVisible(false);
    }

    // tools
    if( (mode_using & TableFlag::GraphDataEnabled) && !(mode_using & TableFlag::NoMenuEnabled))  {
        QObject::connect(ui->actionMark_Columns_as_X, &QAction::triggered, this, &TableEditWidget::toggleX);
        QObject::connect(ui->actionMark_Columns_as_Y, &QAction::triggered, this, &TableEditWidget::toggleY);
        QObject::connect(ui->actionPlot_Results, &QAction::triggered, this, &TableEditWidget::CmPlotTable);
    }
    else {
        ui->menu_Tools->menuAction()->setVisible(false);
    }
}

void TableEditWidget::close_graph()
{
    if(graph_dlg) {
        graph_dlg->close();
    }
}

void TableEditWidget::show_graph_data()
{
    QString title = "Graph for window: ";
    title  += widow_title;
    ChartClient* chart_client = dynamic_cast<ChartClient*>(table_client.get());

    if(!graph_dlg)  {
        graph_dlg.reset(new GraphDialog(chart_client, title));
    }
    else {
        graph_dlg->raise();
    }
    graph_dlg->show();
    graph_dlg->UpdateAll(title);
}

} // namespace jsonui
