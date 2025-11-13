
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QMimeData>
#include <QDrag>
#include <QtPrintSupport/QPrinter>
#include <QPrintDialog>
#include <QHeaderView>

#include "ui_GraphDialog.h"
#include "jsonqml/clients/chart_client.h"
#include "jsonqml/forms/GraphDialog.h"
#include "jsonqml/forms/LegendDialog.h"
#include "jsonqml/forms/SymbolDialog.h"
#include "jsonqml/forms/chart_view.h"
#include "jsonqml/forms/FileDialogs.h"
#include "src/charts/markershapes.h"

namespace jsonqml {

/// The constructor
GraphDialog::GraphDialog(ChartClient *client, const QString& title, QWidget *parent):
    QDialog(parent),
    ui(new Ui::GraphDialogData),
    chart_client(client),
    is_fragment(false)
{
    ui->setupUi(this);
    setWindowTitle( title );

    ui->splitter->setStretchFactor(0, 4);
    ui->splitter->setStretchFactor(1, 0);

    // Define legend table
    legend_table.reset(new DragTableWidget(this));
    legend_table->setSelectionMode(QAbstractItemView::NoSelection);
    legend_table->setColumnCount(3);
    legend_table->setColumnWidth(0,35);
    legend_table->setColumnWidth(1,35);
    legend_table->horizontalHeader()->setStretchLastSection(true);
    legend_table->verticalHeader()->setVisible(false);
    legend_table->horizontalHeader()->setVisible(false);

    LabelDelegate *legend_delegate = new LabelDelegate( this );
    legend_table->setItemDelegate(legend_delegate);
    ui->verticalLayout->addWidget(legend_table.get());

    // define plot window
    plot.reset(new PlotChartView(chart_client, this));
    ui->verticalLayout_2->addWidget(plot.get());

    // Insert labels in legend box
    auto graph_type = chart_client->chartData()->graphType();
    if(graph_type == LineChart || graph_type == AreaChart) {
        show_legend();
    }

    button_fragment =ui->buttonBox->addButton("&Fragment", QDialogButtonBox::ActionRole );
    QPushButton* bcustomize =ui->buttonBox->addButton("&Customize", QDialogButtonBox::ActionRole );
    QPushButton* bprint =ui->buttonBox->addButton("&Print", QDialogButtonBox::ActionRole );
    QPushButton* bsave_image =ui->buttonBox->addButton("&Save Image", QDialogButtonBox::ActionRole );

    //QObject::connect( ui->buttonBox, &QDialogButtonBox::rejected, this, &GraphDialog::reject);
    QObject::connect( ui->buttonBox->button(QDialogButtonBox::Close), &QPushButton::clicked, this, &GraphDialog::close);
    QObject::connect( ui->buttonBox, &QDialogButtonBox::helpRequested, this, &GraphDialog::CmHelp);

    QObject::connect( button_fragment, &QPushButton::clicked, this, &GraphDialog::CmFragment);
    QObject::connect( bcustomize, &QPushButton::clicked, this, &GraphDialog::CmLegend);
    QObject::connect( bprint, &QPushButton::clicked, this, &GraphDialog::CmPrint);
    QObject::connect( bsave_image, &QPushButton::clicked, this, &GraphDialog::CmSaveImage);

    connect(legend_table.get(), &DragTableWidget::cellClicked, this, &GraphDialog::changeIcon);
    QObject::connect(plot.get(), &PlotChartView::fragmentChanged, this, &GraphDialog::updateFragment);
}

GraphDialog::~GraphDialog()
{
}

void GraphDialog::UpdatePlots(const QString& title)
{
    setWindowTitle(title);
    plot->updateAll();
}

void GraphDialog::UpdateAll(const QString& title)
{
    if(!title.isEmpty()) {
        setWindowTitle(title);
    }
    // Insert labels in legend box
    auto graph_type = chart_client->chartData()->graphType();
    if(graph_type == LineChart || graph_type == AreaChart) {
        show_legend();
    }
    plot->updateAll();
}

// slots ------------------------------------

void GraphDialog::CmLegend()
{
    LegendDialog legend(this);
    legend.exec();
}

void GraphDialog::CmFragment()
{
    if(is_fragment == false) {
        is_fragment = true;
        button_fragment->setText("&Full");
    }
    else {
        is_fragment = false;
        button_fragment->setText("&Fragment");
    }
    plot->updateFragment(is_fragment);
}

void GraphDialog::CmSaveImage()
{
    QString file_name = "image.pdf";
    if( ChooseFileSave(this, file_name, "Saving Graphics Image", ChartClient::imageFilters().join(";;")) ) {
        plot->renderDocument(file_name);
    }
}

// Print graph screen to printer device
void GraphDialog::CmPrint()
{
    QPrinter printer( QPrinter::PrinterResolution/*QPrinter::HighResolution*/ );

    QPrintDialog dialog(&printer);
    if(dialog.exec()) {
        printer.setDocName(chart_client->chartData()->graphTitle());
        QPainter p(&printer);
        plot->render(&p);
    }

}

void GraphDialog::CmHelp()
{
    // helpWin( "GraphDialogJsonui", "" );
}

// Insert labels in legend box
void GraphDialog::show_legend()
{
    QTableWidgetItem *item0, *item1;
    Qt::ItemFlags flags;

    legend_table->setAcceptDrops(true);
    legend_table->setEditTriggers( QAbstractItemView::DoubleClicked|QAbstractItemView::AnyKeyPressed );

    LegendModel *legend_model = chart_client->editLegendModel();
    legend_table->clear();
    legend_table->setRowCount(legend_model->rowCount());

    for(int row = 0; row < legend_model->rowCount(); ++row) {
        legend_table->setRowHeight(row, 21/*htF(ftString, 0)+4*/);

        QModelIndex index = legend_model->index(row, 0);

        QIcon icon = markerShapeIcon(legend_model->lineData(row));
        item0 = new QTableWidgetItem(icon, "");
        flags = item0->flags();
        item0->setFlags(flags & ~Qt::ItemIsEditable);
        item0->setToolTip("Legend column 1");
        legend_table->setItem(row, 0, item0 );

        int abs_index = legend_model->data(index, LegendModel::AbscissaIndexRole).toInt();
        QString abs_name = legend_model->abscissaIndexName(abs_index-2);
        item1 = new QTableWidgetItem(tr("%1").arg(abs_name));
        item1->setToolTip("Legend column 2");
        legend_table->setItem(row, 1, item1);

        QString name = legend_model->data(index, LegendModel::NameRole).toString();
        item1 = new QTableWidgetItem(tr("%1").arg(name));
        item1->setToolTip("Legend column 3");
        legend_table->setItem(row, 2, item1 );
    }
}

void GraphDialog::changeIcon(int row, int column)
{
    if(column == 0) {
        auto graph_type = chart_client->chartData()->graphType();
        if(graph_type == LineChart || graph_type == AreaChart) {
            LegendModel *legend_model = chart_client->editLegendModel();
            SymbolDialog cd(legend_model->lineData(row), this);
            if(cd.exec()) {
                legend_model->setLineData(row, cd.plotLine());
                auto icon = markerShapeIcon(legend_model->lineData(row));
                legend_table->item(row, column)->setIcon(icon);
            }
        }
    }
    else if(column == 2) {
        highlightRow(row);
    }
}

void GraphDialog::highlightRow(int row)
{
    if(active_row == row) { // toggle
        restoreRow();
        return;
    }
    restoreRow();
    active_row = row;
    plot->highlightLine(row, true);
}

void GraphDialog::restoreRow()
{
    if(active_row != -1) {
        plot->highlightLine(active_row, false);
    }
    active_row = -1;
}

void GraphDialog::updateFragment(QRectF  rect)
{
    auto graph_data = chart_client->chartData();
    graph_data->setfxMin(rect.left());
    graph_data->setfxMax(rect.right());
    graph_data->setfyMin(rect.bottom());
    graph_data->setfyMax(rect.top());
    is_fragment = true;
    button_fragment->setText("&Full");
    plot->updateFragment(is_fragment);
    //emit dataChanged(gr_data);
}

//=======================================================================================

void DragTableWidget::start_drag(/*Qt::DropActions supportedActions*/)
{
    if(currentColumn() == 2)  {
        QTableWidgetItem *item = currentItem();
        QString text_ = item->text();

        QFontMetrics fm(this->font());
        QPixmap pixmap(fm.horizontalAdvance(text_)+2, fm.height()+2);
        pixmap.fill(QColor(0, 0, 0, 0));
        QPainter dc(&pixmap);
        dc.drawText(2, fm.height()-2, text_);
        dc.end();

        QMimeData * mimeData = new QMimeData;
        mimeData->setText(text_);
        QDrag *drag = new QDrag(this);
        drag->setMimeData(mimeData);
        drag->setPixmap(pixmap/*QPixmap::grabWidget(this)*/);
        //drag->setHotSpot( QPoint(0, pixmap.height()) );
        drag->setHotSpot(QPoint( pixmap.width()/2, pixmap.height()/2 ));
        drag->exec();
    }
}

void DragTableWidget::mousePressEvent( QMouseEvent *e )
{
    if(e->buttons() & Qt::LeftButton) {
        start_pos = e->pos();
    }
    QTableWidget::mousePressEvent(e);
}

void DragTableWidget::mouseMoveEvent( QMouseEvent *e )
{
    if(e->buttons() & Qt::LeftButton) {
        int delta = (e->pos()-start_pos).manhattanLength();
        if(delta >= QApplication::startDragDistance()) {
            start_drag();
        }
    }
    QTableWidget::mouseMoveEvent(e);
}

void DragTableWidget::focusOutEvent(QFocusEvent *event)
{
    top_dlg->restoreRow();
    QTableWidget::focusOutEvent(event);
}

//--------------------------------------------------------------------------

// Editing  Table of Labels
QWidget *LabelDelegate::createEditor(QWidget *parent,
                                     const QStyleOptionViewItem &,
                                     const QModelIndex &index) const
{

    if(index.column() == 2)  {
        QLineEdit *editor =  new QLineEdit(parent);
        editor->setMaxLength( 30 );
        return editor;
    }
    else if(index.column() == 1) {
        QStringList abs_list = graph_dlg->chart_client->abscissaList(index.row());
        QComboBox* editor =  new QComboBox(parent);
        editor->addItems(abs_list);
        return editor;
    }
    return nullptr;
}

void LabelDelegate::setModelData(QWidget *editor,
                                 QAbstractItemModel *model,
                                 const QModelIndex &index) const
{
    LegendModel *legend_model = graph_dlg->chart_client->editLegendModel();
    QModelIndex legend_index = legend_model->index(index.row(), 0);

    if(index.column() == 1) {
        QComboBox *box_edit = dynamic_cast<QComboBox*>(editor);
        if(box_edit) {
            model->setData(index, box_edit->currentText(), Qt::EditRole);
            legend_model->setData(legend_index, box_edit->currentIndex(), LegendModel::AbscissaIndexRole);
        }
    }
    else if(index.column() == 2) {
        QItemDelegate::setModelData(editor, model, index);
        QLineEdit *line_edit = dynamic_cast<QLineEdit*>(editor);
        if(line_edit) {
            legend_model->setData(legend_index, line_edit->text(), LegendModel::NameRole);
        }
    }
}

} // namespace jsonui

//--------------------- End of GraphDialog.cpp ---------------------------
