#pragma once

#include <memory>
#include <array>
#include <QFont>
#include "jsonqml/charts/legend_data.h"
#include "jsonqml/charts/chart_model.h"

namespace jsonqml {

/// Description of 2D plotting widget
class ChartData : public QObject
{
    Q_OBJECT

    Q_PROPERTY(int graphType READ graphType WRITE setGraphType NOTIFY graphTypeChanged)
    Q_PROPERTY(QString title MEMBER title NOTIFY titleChanged)
    Q_PROPERTY(int axisX MEMBER axis_typeX NOTIFY axisXChanged)
    Q_PROPERTY(int axisY MEMBER axis_typeY NOTIFY axisYChanged)
    Q_PROPERTY(QString xName MEMBER xname NOTIFY xNameChanged)
    Q_PROPERTY(QString yName MEMBER yname NOTIFY yNameChanged)

    Q_PROPERTY(double xMin READ xMin WRITE setxMin NOTIFY xMinChanged)
    Q_PROPERTY(double xMax READ xMax WRITE setxMax NOTIFY xMaxChanged)
    Q_PROPERTY(double yMin READ yMin WRITE setyMin NOTIFY yMinChanged)
    Q_PROPERTY(double yMax READ yMax WRITE setyMax NOTIFY yMaxChanged)

    Q_PROPERTY(double fxMin READ fxMin WRITE setfxMin NOTIFY fxMinChanged)
    Q_PROPERTY(double fxMax READ fxMax WRITE setfxMax NOTIFY fxMaxChanged)
    Q_PROPERTY(double fyMin READ fyMin WRITE setfyMin NOTIFY fyMinChanged)
    Q_PROPERTY(double fyMax READ fyMax WRITE setfyMax NOTIFY fyMaxChanged)

    Q_PROPERTY(QColor backColor READ backgroundColor WRITE setBackgroundColor NOTIFY backColorChanged)
    Q_PROPERTY(QFont axisFont MEMBER axis_font NOTIFY axisFontChanged)
    Q_PROPERTY(QFont titleFont READ titleFont NOTIFY axisFontChanged)

public slots:
    void updateXSelections();
    void updateYSelections(bool update_names);

signals:
    void changedModelSelections();
    void dataChanged();

    void graphTypeChanged();
    void titleChanged();
    void axisXChanged();
    void axisYChanged();
    void xNameChanged();
    void yNameChanged();
    void xMinChanged();
    void xMaxChanged();
    void yMinChanged();
    void yMaxChanged();
    void fxMinChanged();
    void fxMaxChanged();
    void fyMinChanged();
    void fyMaxChanged();
    void backColorChanged();
    void axisFontChanged();

public:
    template <class T>
    ChartData(const std::vector<std::shared_ptr<T>>& aplots, const QString& atitle,
              const QString& aXname, const QString& aYname, int agraph_type = LineChart):
        graph_type(agraph_type),
        title(atitle),
        axis_typeX(5),
        axis_typeY(5),
        xname(aXname),
        yname(aYname),
        axis_font("Sans Serif", 12)
    {
        // Define background color
        setBackgroundColor(QColor(Qt::white));

        // Insert Plots and curves description
        modelsdata.clear();
        for(auto plot: aplots) {
            addPlot(plot);
        }

        // Graph&Fragment Min Max Region
        double regg[4] = {0., 0., 0., 0.};
        setMinMaxRegion(regg);
        connect_data_changed();
    }

    virtual ~ChartData();

    /// add new plot lines selection
    template <class T>
    void addPlot(const std::shared_ptr<T>& aplot)
    {
        int defined_lines = static_cast<int>(linesdata.size());
        int nlines = seriesNumber();

        aplot->setGraphType(static_cast<GRAPHTYPES>(graph_type));
        modelsdata.push_back(aplot);
        int nlin = aplot->getSeriesNumber();
        for(int jj=0; jj<nlin; jj++, nlines++) {
            if(nlines >= defined_lines) {
                linesdata.push_back(SeriesLineData(jj, nlin, aplot->getName(jj)));
            }
        }
        connect(modelsdata.back().get(), &ChartDataModel::changedXSelections,
                this,  &ChartData::updateXSelections);
        connect(modelsdata.back().get(), &ChartDataModel::changedYSelections,
                this,  &ChartData::updateYSelections);
    }

    /// get plot from index
    int plot(size_t line, size_t* modelline=nullptr) const;

    const QString& graphTitle() const
    {
        return title;
    }
    int graphType() const
    {
        return graph_type;
    }
    void setGraphType(int newtype);

    size_t seriesNumber() const;
    size_t modelsNumber() const
    {
        return modelsdata.size();
    }
    ChartDataModel* modelData(size_t ndx) const
    {
        return  modelsdata[ndx].get();
    }

    size_t linesNumber() const
    {
        return linesdata.size();
    }
    Q_INVOKABLE const std::vector<SeriesLineData>& lines() const
    {
        return linesdata;
    }
    Q_INVOKABLE void setLines(const std::vector<SeriesLineData>& new_lines);
    Q_INVOKABLE const SeriesLineData& lineData(size_t ndx) const
    {
        return  linesdata[ndx];
    }

    void setMinMaxRegion(double reg[4]);
    double xMin() const;
    void setxMin(double val);
    double xMax() const;
    void setxMax(double val);
    double yMin() const;
    void setyMin(double val);
    double yMax() const;
    void setyMax(double val);
    double fxMin() const;
    void setfxMin(double val);
    double fxMax() const;
    void setfxMax(double val);
    double fyMin() const;
    void setfyMin(double val);
    double fyMax() const;
    void setfyMax(double val);

    QColor backgroundColor() const;
    void setBackgroundColor(const QColor& acolor);

    QFont titleFont() const;
    QFont axisFont() const;

#ifndef NO_JSONIO
    void toJsonNode(jsonio::JsonBase& object) const;
    void fromJsonNode(const jsonio::JsonBase& object);
#endif
    void toJsonObject(QJsonObject& json) const;
    void fromJsonObject(const QJsonObject& json);

protected:
    /// GRAPHTYPES (0-line by line, 1- cumulative, 2 - isolines)
    int graph_type;
    /// Title of graphic
    QString title;

    /// Grid type for Abscissa
    int axis_typeX;
    /// Grid type for Ordinate
    int axis_typeY;
    /// Abscissa name
    QString xname;
    /// Ordinate name
    QString yname;
    /// Graph Min Max Region
    std::array<double, 4> region;
    /// Fragment Min Max Region
    std::array<double, 4> part;

    /// Define background color ( the Constructs a color with the RGB values)
    std::array<int, 3> b_color; // red, green, blue

    /// Define Axis Font
    QFont axis_font;

    /// Descriptions of model extracting data
    std::vector<std::shared_ptr<ChartDataModel>> modelsdata;
    /// Descriptions of all lines
    std::vector<SeriesLineData> linesdata;

    ChartData(const ChartData& data);   // not defined
    ChartData& operator=(const ChartData&); // not defined

    friend class PlotChartViewPrivate;
    friend class PlotChartView;  // to do functions
    friend class LegendDialog;

    void connect_data_changed();
    void model_update_y_xcolumns();
};

} // namespace jsonqml


