#pragma once

#include <QJsonObject>
#include <QAbstractTableModel>

#ifndef NO_JSONIO
namespace jsonio {
class JsonBase;
}
#endif

namespace jsonqml {

enum GRAPHTYPES {
    LineChart = 0,
    AreaChart = 1,
    BarChart  = 2,
    Isolines  = 3,   // under construction
    lines_3D  = 4    // for future using
};

/// \class ChartDataModel
/// \brief The ChartDataModel class is a vertical model for line,
///  spline, and scatter series.
///
/// Model enable using a data model derived from the QAbstractTableModel
/// class as a data source for a chart. First column of model used to iterate
/// by index
class ChartDataModel : public QAbstractTableModel
{
    Q_OBJECT

public Q_SLOTS:
    void modelUpdated(QModelIndex topLeft, QModelIndex bottomRight);
    void modelRowsAdded(QModelIndex parent, int start, int end);
    void modelRowsRemoved(QModelIndex parent, int start, int end);
    void modelColumnsAdded(QModelIndex parent, int start, int end);
    void modelColumnsRemoved(QModelIndex parent, int start, int end);

signals:
    void changedXSelections();
    void changedYSelections(bool update_names);

public:
    explicit ChartDataModel(QAbstractTableModel* table_model, QObject* parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
    Qt::ItemFlags flags(const QModelIndex &index) const;

    /// Update GRAPHTYPES ( LineChart,  AreaChart,  BarChart, Isolines ... )
    void setGraphType(GRAPHTYPES type);
    /// Set abscissa columns
    void setXColumns(const std::vector<int>& axcolumns);
    /// Set ordinate columns
    void setYColumns(const std::vector<int>& aycolumns, bool update_names);

    /// Get number of series
    size_t getSeriesNumber() const
    {
        return ycolumns.size();
    }
    /// Get ordinate column index for line
    int getYColumn(size_t line) const
    {
        return (line<ycolumns.size() ? ycolumns[line] : 0);
    }
    /// Get line name
    QString getName(size_t line) const
    {
        auto name = m_model->headerData(getYColumn(line), Qt::Horizontal, Qt::DisplayRole).toString();
        name.replace("(y)", "", Qt::CaseInsensitive);
        return name;
    }

    /// Get number of Abscissa lines
    int getAbscissaNumber() const
    {
        return xcolumns.size();
    }
    /// Get Abscissa colunm index for line
    int getXColumn(int line) const
    {
        if(line == -2) {
            return line;
        }
        else if(line>=0 && line<static_cast<int>(xcolumns.size())) {
            return xcolumns[static_cast<size_t>(line)];
        }
        return -1;
    }

    const std::vector<int> xColumns() const
    {
        return xcolumns;
    }
    const std::vector<int> yColumns() const
    {
        return ycolumns;
    }

#ifndef NO_JSONIO
    void toJsonNode(jsonio::JsonBase& object) const;
    void fromJsonNode(const jsonio::JsonBase& object);
#endif
    void toJsonObject(QJsonObject& json) const;
    void fromJsonObject(const QJsonObject& json);

protected:
    /// Extern table model
    QAbstractTableModel *m_model;

    /// GRAPHTYPES ( LineChart,  AreaChart,  BarChart, Isolines ... )
    GRAPHTYPES graph_type = LineChart;

    /// List of the column of the model that contains the x-coordinates of data points
    std::vector<int> xcolumns;
    /// List of the column of the model that contains the y-coordinates of data points
    std::vector<int> ycolumns;
    /// List that contains the x-coordinates of every chart line (y-coordinate).
    /// Must be the same size as ycolumns, internal for showAreaChart
    std::vector<int> y_xcolumns;

    QModelIndex mIndex(int row, int column) const
    {
        if(column == 0) {
            return QModelIndex();
        }
        else {
            return m_model->index(row, column-1);
        }
    }
    QModelIndex mIndex(const QModelIndex &index) const
    {
        return mIndex(index.row(), index.column());
    }
    QModelIndex indexM(const QModelIndex &m_index) const
    {
        if(m_index.isValid()) {
            return index(m_index.row(), m_index.column()+1);
        }
        return QModelIndex();
    }

    bool append_x(int xclm);
    bool append_y(int yclm);
    ///  Clear list that contains the x-coordinates of every chart line (y-coordinate).
    void clearXColumn()
    {
        y_xcolumns.clear();
    }
    ///  Add line to list that contains the x-coordinates of every chart line (y-coordinate).
    void addXColumn(int line)
    {
        y_xcolumns.push_back(line);
    }

    friend class ChartData;
    friend class PlotChartViewPrivate;
};

} // namespace jsonqml

