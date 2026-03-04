
#include <QMimeData>
#include <QDragEnterEvent>
#include <QDrag>
#include <QtCharts/QAreaSeries>
#include <QtCharts/QScatterSeries>
#include <QGraphicsLayout>
#include <QRubberBand>
#include <QFileInfo>
#include <QSizeF>
#include <QPrinter>

#include "jsonqml/forms/chart_view.h"
#include "jsonqml/clients/chart_client.h"

namespace jsonqml {

PlotChartView::PlotChartView(ChartClient *client, QWidget *parent) :
    QChartView(new QChart(), parent),
    chart_client(client),
    chart_decorator(client->seriesDecorator())
{
    setRubberBand(QChartView::RectangleRubberBand);
    setAcceptDrops(true);

    axisX = new QValueAxis;
    chart()->addAxis(axisX, Qt::AlignBottom);
    axisY = new QValueAxis;
    chart()->addAxis(axisY, Qt::AlignLeft);

    show_plot();

    chart()->setDropShadowEnabled(false);
    chart()->legend()->hide();
    chart()->layout()->setContentsMargins(0, 0, 0, 0);
    chart()->setMargins( QMargins( 5,5, 0, 0) );
    // chart()->legend()->setMarkerShape(QLegend::MarkerShapeFromSeries);

    rubberBand = findChild<QRubberBand *>();

    connect(chart_client->editLegendModel(), &LegendModel::dataChanged, this, &PlotChartView::modelDataChanged);
}

PlotChartView::~PlotChartView()
{
}

void PlotChartView::updateAll()
{
    show_plot();
}

void PlotChartView::updateLines()
{
    show_plot_internal();
}

void PlotChartView::updateFragment(bool is_fragment)
{
    chart_decorator->setFragent(is_fragment);
}

void PlotChartView::modelDataChanged(const QModelIndex &topLeft, const QModelIndex &, const QList<int> &)
{
    chart_client->applyLegend();
    auto graph_type = chart_client->chartData()->graphType();
    int row = topLeft.row();

    qDebug() << "modelDataChanged  " << row;
    if(graph_type == LineChart) {
        chart_decorator->updateSeriesLine(row);
    }
    else if(graph_type == AreaChart) {
        updateLines();
    }
}

void PlotChartView::highlightLine(size_t line, bool enable)
{
    chart_decorator->highlightSeries(line, enable);
}

void PlotChartView::dragEnterEvent(QDragEnterEvent *event)
{
    if(event->mimeData()->hasFormat("text/plain")) {
        if (event->source() == this) {
            event->setDropAction(Qt::MoveAction);
            event->accept();
        }
        else {
            event->acceptProposedAction();
        }
    }
    else {
        event->ignore();
    }
}

void PlotChartView::dragMoveEvent(QDragMoveEvent *event)
{
    if (event->mimeData()->hasFormat("text/plain")) {
        if (event->source() == this) {
            event->setDropAction(Qt::MoveAction);
            event->accept();
        }
        else {
            event->acceptProposedAction();
        }
    }
    else {
        event->ignore();
    }
}

void PlotChartView::dropEvent( QDropEvent* event )
{
    if(event->mimeData()->hasFormat("text/plain")) {
        QString drop_text = event->mimeData()->text();
        auto posF =  event->position();
        chart_decorator->dropLabel(posF, drop_text);
    }
}

void PlotChartView::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->button() == Qt::RightButton) {
        event->accept();
        return;
    }
    else {
        if(event->button() == Qt::LeftButton) {
            if(rubberBand && rubberBand->isVisible()) {
                if(rubberBand->height()<3 || rubberBand->width()<3) {
                    rubberBand->hide();
                    event->accept();
                    return;
                }

                QPointF fp = chart()->mapToValue(rubberBand->geometry().topLeft());
                QPointF tp = chart()->mapToValue(rubberBand->geometry().bottomRight());
                QRectF  rect(fp, tp);
                emit fragmentChanged(rect);
            }
        }
    }

    QChartView::mouseReleaseEvent(event);
}

void PlotChartView::renderDocument(const QString &file_name)
{
    const QString fmt = QFileInfo(file_name).suffix().toLower();
    if(fmt == "pdf") {
#ifndef QT_NO_PRINTER
        chart_decorator->renderPdf(file_name);
#endif
    }
    else if(fmt == "svg") {
        chart_decorator->renderSvg(size(), file_name);
    }
    else {
        QPixmap picture = grab();
        picture.save(file_name);
    }
}

void PlotChartView::show_plot_lines()
{
    for(int i = 0; i <= chart_decorator->size(); i++) {
        auto series = new QScatterSeries;
        chart()->addSeries(series);
        series->attachAxis(axisX);
        series->attachAxis(axisY);
        chart_decorator->updateSeries(i, series);
    }
}

void PlotChartView::show_area_chart()
{
    for(int i = 0; i <= chart_decorator->size(); i++) {
        auto area = new QAreaSeries();
        chart()->addSeries(area);
        area->attachAxis(axisX);
        area->attachAxis(axisY);
        chart_decorator->updateAreaSeries(i, area);
    }
}

void PlotChartView::show_plot_internal()
{
    chart()->removeAllSeries();
    switch(chart_client->chartData()->graphType()) {
    case LineChart:
        show_plot_lines();
        break;
    case AreaChart:
        show_area_chart();
        break;
    case BarChart:
    case Isolines:
    case lines_3D:
        break;
    }
    chart_decorator->updateMinMax();
}

void PlotChartView::show_plot()
{
    grid_decorate();
    show_plot_internal();
}

void PlotChartView::grid_decorate()
{
    auto axis_font = chart_client->chartData()->axisFont();
    auto title_font = axis_font;
    title_font.setPointSize(title_font.pointSize()+2);

    chart()->setFont(axis_font);
    chart()->setTitleFont(title_font);
    chart()->setTitle(chart_client->chartData()->title);
    chart()->setBackgroundBrush(chart_client->chartData()->backgroundColor());

    axisX->setTickCount(chart_client->chartData()->axis_typeX+1);
    //axisX->setMinorTickCount(4);
    axisX->setTitleFont(title_font);
    axisX->setLabelsFont(axis_font);
    axisX->setTitleText(chart_client->chartData()->xname);
    auto penX = axisX->linePen();
    penX.setWidth(3);
    penX.setColor(Qt::darkGray);
    axisX->setLinePen(penX);

    axisY->setTickCount(chart_client->chartData()->axis_typeY+1);
    //axisY->setMinorTickCount(4);
    axisY->setTitleFont(title_font);
    axisY->setLabelsFont(axis_font);
    axisY->setTitleText(chart_client->chartData()->yname);
    axisY->setLinePen(penX);
}

} // namespace jsonui
