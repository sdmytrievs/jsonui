
#pragma once

#include <QtCharts/QAreaSeries>
#include <QtCharts/QLineSeries>
#include <QtCharts/QSplineSeries>
#include <QtCharts/QScatterSeries>
#include <QtCharts/QVXYModelMapper>
#include <QtCharts/QValueAxis>
#include "jsonqml/charts/graph_data.h"

namespace jsonqml {

class QXYSeriesDecorator: public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool fragment MEMBER is_fragment NOTIFY fragmentChanged)
    Q_PROPERTY(int size READ size NOTIFY sizeChanged)

signals:
    void fragmentChanged();
    void sizeChanged();

public slots:
    void updateMinMax();

    void updateSeriesLine(size_t nline);
public:
    explicit QXYSeriesDecorator(const ChartData *graphdata, QObject *parent = nullptr);
    ~QXYSeriesDecorator();

    int size() const
    {
        return chart_data->seriesNumber();
    }

    Q_INVOKABLE void updateSeries(size_t nline, QScatterSeries *series);
    Q_INVOKABLE void updateAreaSeries(size_t nline, QAreaSeries *series);

    Q_INVOKABLE void highlightSeries(size_t line, bool enable);

    Q_INVOKABLE void dropLabel(const QPointF &pointF, const QString &label);

    Q_INVOKABLE void renderPdf(const QString &file_name);
    Q_INVOKABLE void renderSvg(QSize size, const QString &file_name);

    void setFragent(bool new_fragment);

protected:
    bool is_fragment = false;
    double generated_region[4];
    const ChartData *chart_data;

    QChart* series_chart = nullptr;
    QValueAxis *axisX = nullptr;
    QValueAxis *axisY = nullptr;

    std::vector<QScatterSeries *>  point_series;
    std::vector<std::shared_ptr<QVXYModelMapper>> point_mapper;

    std::vector<QXYSeries *> line_series;
    std::vector<std::shared_ptr<QVXYModelMapper>> line_mapper;

    std::vector<QAreaSeries *> area_series;

    std::map<QString,QScatterSeries*> map_labels;

    void update_scatter_series(QScatterSeries *series, const SeriesLineData &linedata);
    void update_area_series(QAreaSeries *series, const SeriesLineData &linedata);
    void update_series_line(QXYSeries *series, const SeriesLineData &linedata);

    QXYSeries *new_series_line(const SeriesLineData &linedata);
    QVXYModelMapper *map_series_line(QXYSeries *series, ChartDataModel *datamodel, int modelline, int xcolumn);
    void resize_series();
    void resize_area_series();
    void data_from_chart_view(size_t nline, QAbstractSeries *series);
    void generate_min_max_region();
    QScatterSeries *new_scatter_label(const QPointF &pointF, const QString &label);
};

} // namespace jsonqml
