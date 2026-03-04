#pragma once

#include <QtCharts/QChartView>
#include <QtCharts/QValueAxis>

namespace jsonqml {

class ChartClient;
class QXYSeriesDecorator;

/// Manages the graphical representation of the chart's series and axes
class PlotChartView : public QChartView
{
    Q_OBJECT

public slots:
    void updateAll();
    void updateLines();
    void highlightLine(size_t line, bool enable);
    void updateFragment(bool is_fragment);
    void modelDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QList<int> &roles);

signals:
    void fragmentChanged(QRectF rect);

public:
    explicit PlotChartView(ChartClient *client, QWidget *parent = nullptr);
    ~PlotChartView();

    /**
      Render graphical representation of the chart's series and axes to a file

      Supported formats are:

      - pdf\n
        Portable Document Format PDF
      - ps\n
        Postcript
      - svg\n
        Scalable Vector Graphics SVG
      - all image formats supported by Qt\n
        see QImageWriter::supportedImageFormats()

      \param document title
      \param file_name Path of the file, where the document will be stored

    */
    void renderDocument(const QString &file_name);

protected:
    QRubberBand* rubberBand;
    ChartClient* chart_client;
    QXYSeriesDecorator* chart_decorator;

    QValueAxis *axisX = nullptr;
    QValueAxis *axisY = nullptr;

    void dragEnterEvent(QDragEnterEvent* event);
    void dragMoveEvent(QDragMoveEvent *event);
    void dropEvent(QDropEvent* event);
    void mouseReleaseEvent(QMouseEvent *event);

    void show_plot_lines();
    void show_area_chart();
    void show_plot();
    void grid_decorate();
    void show_plot_internal();
};

} // namespace jsonui

