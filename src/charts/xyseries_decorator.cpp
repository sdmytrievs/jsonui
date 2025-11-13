#include <QPainter>
#include <QPrinter>
#include <QFontMetrics>
#include <QtCharts/QChart>
#include <QtSvg/QSvgGenerator>
#include "jsonqml/charts/xyseries_decorator.h"
#include "markershapes.h"
#ifndef NO_JSONIO
#include "jsonio/service.h"
#else
namespace jsonio {
template<typename T>
bool essentiallyEqual( const T& a, const T& b, const T& epsilon = std::numeric_limits<T>::epsilon() )
{
    return fabs(a - b) <= ( (fabs(a) > fabs(b) ? fabs(b) : fabs(a)) * epsilon);
}
}
#endif

namespace jsonqml {

QXYSeriesDecorator::QXYSeriesDecorator(const ChartData *graphdata, QObject *parent) :
    QObject(parent), chart_data(graphdata)
{
    connect(this, &QXYSeriesDecorator::fragmentChanged, this, &QXYSeriesDecorator::updateMinMax);
}

QXYSeriesDecorator::~QXYSeriesDecorator()
{ }

void QXYSeriesDecorator::updateMinMax()
{
    if(!axisX || !axisY) {
        return;
    }

    if(is_fragment) {
        if(!jsonio::essentiallyEqual(chart_data->fxMin(), chart_data->fxMax()) &&
            !jsonio::essentiallyEqual(chart_data->fyMin(), chart_data->fyMax()))  {
            axisY->setRange(chart_data->fyMin(), chart_data->fyMax());
            axisX->setRange(chart_data->fxMin(), chart_data->fxMax());
        }
        else  {
            generate_min_max_region();
            axisX->setRange(generated_region[0]/2, generated_region[1]/2);
            axisY->setRange(generated_region[2]/2, generated_region[3]/2);
        }
    }
    else  {
        if(!jsonio::essentiallyEqual(chart_data->xMin(), chart_data->xMax()) &&
            !jsonio::essentiallyEqual(chart_data->yMin(), chart_data->yMax()))  {
            axisY->setRange(chart_data->yMin(), chart_data->yMax());
            axisX->setRange(chart_data->xMin(), chart_data->xMax());
        }
        else  {
            generate_min_max_region();
            axisX->setRange(generated_region[0], generated_region[1]);
            axisY->setRange(generated_region[2], generated_region[3]);
        }
    }
}

void QXYSeriesDecorator::highlightSeries(size_t line, bool enable)
{
    if(chart_data->graphType() == LineChart) {
        if(line>=point_series.size()) {
            return;
        }
        // update series lines
        auto  linedata = chart_data->lineData(line);
        QXYSeries *series = line_series[line];
        if(series) {
            QPen pen = series->pen();
            getLinePen(pen, linedata);
            if(enable) {
                pen.setWidth(linedata.penSize()*2);
            }
            series->setPen(pen);
        }

        QScatterSeries *scatterseries = point_series[line];
        if(scatterseries) {
            auto shsize = linedata.markerSize();
            if(enable) {
                shsize *=2;
            }
            scatterseries->setLightMarker(markerShapeImage(linedata));
            scatterseries->setMarkerSize(shsize);
        }
    }
    else if(chart_data->graphType() == AreaChart) {
        if(line>=area_series.size()) {
            return;
        }
        if(area_series[line]) {
            area_series[line]->setOpacity( (enable ? 1: 0.5) );
        }
    }
}

void QXYSeriesDecorator::updateSeries(size_t nline, QScatterSeries *series)
{
    // extract data from QML
    data_from_chart_view(nline, series);

    // get chart settings
    size_t modelline;
    int nplot =  chart_data->plot(nline, &modelline);
    if(nplot < 0) {
        return;
    }
    auto* datamodel = chart_data->modelData(static_cast<size_t>(nplot));
    const auto& linedata = chart_data->lineData(nline);

    if(nline >= point_series.size()) {
        resize_series();
    }
    // set up points
    update_scatter_series(series, linedata);
    point_series[nline] = series;
    point_mapper[nline].reset(map_series_line(series, datamodel, modelline, linedata.xColumn()));
    // set up lines
    line_series[nline] = new_series_line(linedata);
    line_mapper[nline].reset(map_series_line(line_series[nline], datamodel, modelline, linedata.xColumn()));
}

void QXYSeriesDecorator::updateSeriesLine(size_t nline)
{
    if(nline >=point_series.size()) {
        return;
    }
    const auto& linedata = chart_data->lineData(nline);
    update_scatter_series(point_series[nline], linedata);
    point_mapper[nline]->setXColumn(linedata.xColumn()+1);
    update_series_line(line_series[nline], linedata);
    line_mapper[nline]->setXColumn(linedata.xColumn()+1);
}

void QXYSeriesDecorator::updateAreaSeries(size_t nline, QAreaSeries *series)
{
    // extract data from QML
    data_from_chart_view(nline, series);

    // get chart settings
    size_t modelline;
    int nplot =  chart_data->plot(nline, &modelline);
    if(nplot < 0) {
        return;
    }
    auto* datamodel = chart_data->modelData(static_cast<size_t>(nplot));
    auto linedata = chart_data->lineData(nline);

    if(nline >= area_series.size()) {
        resize_area_series();
    }
    update_area_series(series, linedata);
    area_series[nline] = series;

    // set lower line
    QLineSeries *lower_series = nullptr;
    if(nline==0) {
        // must be loop from 0 line
        line_series.clear();
        line_mapper.clear();
    }
    else {
        lower_series = dynamic_cast<QLineSeries *>(line_series.back());
        series->setLowerSeries(lower_series);
    }

    // set upper line
    if(linedata.xColumn() < -1) {
        // empty area
        series->setUpperSeries(lower_series);
    }
    else {
        linedata.setLineChanges(1,1,0);
        line_series.push_back(new_series_line(linedata));
        line_mapper.push_back(nullptr);
        line_mapper.back().reset(map_series_line(line_series[nline], datamodel, modelline, linedata.xColumn()));
        series->setUpperSeries(dynamic_cast<QLineSeries *>(line_series.back()));
    }
}

void QXYSeriesDecorator::dropLabel(const QPointF& pointF, const QString& label)
{
    if(!series_chart) {
        return;
    }
    if(map_labels.find(label) != map_labels.end()) {
        series_chart->removeSeries(map_labels[label]);
    }
    QScatterSeries *series  = new_scatter_label(pointF, label);
    series_chart->addSeries(series);
    series->attachAxis(axisX);
    series->attachAxis(axisY);
    map_labels[label] = series;
}

void QXYSeriesDecorator::data_from_chart_view(size_t nline, QAbstractSeries *series)
{
    if(series_chart && series_chart!=series->chart()) {
        qDebug() << "Different chart line " << nline;
    }
    series_chart = series->chart();
    auto axises = series_chart->axes(Qt::Horizontal);
    if(axises.size() > 0) {
        axisX = dynamic_cast<QValueAxis*>(axises[0]);
    }
    axises = series_chart->axes(Qt::Vertical);
    if(axises.size() > 0) {
        axisY = dynamic_cast<QValueAxis*>(axises[0]);
    }
}

void QXYSeriesDecorator::generate_min_max_region()
{
    generated_region[0] = generated_region[2] = std::numeric_limits<double>::max();
    generated_region[1] = generated_region[3] = std::numeric_limits<double>::min();

    if(chart_data->graphType() == 1) {
        if(line_series.size()>0) {
            foreach(const auto& point, line_series[0]->points()) {
                if(point.x() < generated_region[0])  generated_region[0] = point.x();
                if(point.x() > generated_region[1])  generated_region[1] = point.x();
                if(point.y() < generated_region[2])  generated_region[2] = point.y();
                if(point.y() > generated_region[3])  generated_region[3] = point.y();
            }
            foreach(const auto& point, line_series.back()->points()) {
                if(point.x() < generated_region[0])  generated_region[0] = point.x();
                if(point.x() > generated_region[1])  generated_region[1] = point.x();
                if(point.y() < generated_region[2])  generated_region[2] = point.y();
                if(point.y() > generated_region[3])  generated_region[3] = point.y();
            }
        }
    }
    else {
        for(const auto& series: point_series) {
            foreach(const auto& point, series->points()) {
                if(point.x() < generated_region[0])  generated_region[0] = point.x();
                if(point.x() > generated_region[1])  generated_region[1] = point.x();
                if(point.y() < generated_region[2])  generated_region[2] = point.y();
                if(point.y() > generated_region[3])  generated_region[3] = point.y();
            }
        }
    }
}

void QXYSeriesDecorator::resize_series()
{
    point_series.resize(chart_data->seriesNumber());
    point_mapper.resize(chart_data->seriesNumber());
    line_series.resize(chart_data->seriesNumber());
    line_mapper.resize(chart_data->seriesNumber());
    area_series.clear();
    map_labels.clear();
}

void QXYSeriesDecorator::resize_area_series()
{
    area_series.resize(chart_data->seriesNumber());
    point_series.clear();
    point_mapper.clear();
    map_labels.clear();
}

void QXYSeriesDecorator::update_scatter_series(QScatterSeries* series, const SeriesLineData& linedata)
{
    series->setName(linedata.name());
    series->setPen( QPen(Qt::transparent));
    series->setMarkerShape(QScatterSeries::MarkerShapeRectangle);
    auto msize = linedata.markerSize()+2;
    series->setMarkerSize(msize);
    series->setLightMarker(markerShapeImage(linedata));
}

QVXYModelMapper* QXYSeriesDecorator::map_series_line(QXYSeries *series, ChartDataModel* datamodel,
                                                     int modelline, int xcolumn)
{
    if(!series) {
        return nullptr;
    }
    QVXYModelMapper *mapper = new QVXYModelMapper;
    mapper->setXColumn(datamodel->getXColumn(xcolumn)+1);
    mapper->setYColumn(datamodel->getYColumn(modelline)+1);
    mapper->setSeries(series);
    mapper->setModel(datamodel);
    return mapper;
}

QXYSeries* QXYSeriesDecorator::new_series_line(const SeriesLineData& linedata)
{
    QXYSeries* series = nullptr;
    if(linedata.penSize() <= 0) {
        return series;
    }
    if(linedata.spline()) {
        series = new QSplineSeries;
    }
    else {
        series = new QLineSeries;
    }
    series_chart->addSeries(series);
    series->attachAxis(axisX);
    series->attachAxis(axisY);

    update_series_line(series, linedata);
    return series;
}

void QXYSeriesDecorator::update_series_line(QXYSeries* series, const SeriesLineData& linedata)
{
    series->setName(linedata.name());
    QPen pen = series->pen();
    getLinePen(pen, linedata);
    series->setPen(pen);
}

void QXYSeriesDecorator::update_area_series(QAreaSeries* series, const SeriesLineData& linedata)
{
    series->setName(linedata.name());
    QPen pen = series->pen();
    getLinePen(pen, linedata);
    series->setPen(pen);
    series->setColor(pen.color());
}

QScatterSeries* QXYSeriesDecorator::new_scatter_label(const QPointF& pointF, const QString& label)
{
    QScatterSeries *series  =  new QScatterSeries;
    QFontMetrics fm(chart_data->axisFont());
    int size = std::max(fm.horizontalAdvance(label)+2, fm.height());

    series->setName(label);
    series->setPen(QPen(Qt::transparent));
    series->setMarkerShape(QScatterSeries::MarkerShapeRectangle);
    series->setMarkerSize(size);
    series->setBrush(textImage(chart_data->axisFont(), label));

    auto pointV = series_chart->mapToValue(pointF);
    series->append(pointV);
    return series;
}

void QXYSeriesDecorator::renderPdf(const QString &file_name)
{
    if(!series_chart) {
        return;
    }
    QPrinter printer(QPrinter::HighResolution);
    printer.setDocName(chart_data->graphTitle());
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(file_name);
    printer.setColorMode(QPrinter::Color);
    printer.setFullPage(true);
    printer.setPageSize(QPageSize(QPageSize::A4));
    QPainter painter;
    painter.begin(&printer);
    series_chart->paint(&painter, nullptr);
    painter.end();
}

void QXYSeriesDecorator::renderSvg(QSize size, const QString &file_name)
{
    if(!series_chart) {
        return;
    }
    QSvgGenerator generator;
    generator.setTitle(chart_data->graphTitle());
    generator.setFileName(file_name);
    generator.setSize(size);
    generator.setViewBox(series_chart->geometry());
    QPainter painter;
    painter.begin(&generator);
    series_chart->paint(&painter, nullptr);
    painter.end();
}

void QXYSeriesDecorator::setFragent(bool new_fragment)
{
    is_fragment = new_fragment;
    emit fragmentChanged();
}

} // namespace jsonqml
