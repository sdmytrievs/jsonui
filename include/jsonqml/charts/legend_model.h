
#ifndef LEGENDMODEL_H
#define LEGENDMODEL_H

#include <QAbstractListModel>
#include "jsonqml/charts/legend_data.h"

namespace jsonqml {

class LegendModel : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(int size READ rowCount NOTIFY sizeChanged)

public:
    enum LegendRoles {
        IdRole = Qt::UserRole + 1,
        ShapeIconRole,
        AbscissaIndexRole,
        NameRole
    };
    Q_ENUM(LegendRoles)

    explicit LegendModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QHash<int, QByteArray> roleNames() const override;

    void updateLines(const std::vector<jsonqml::SeriesLineData>& in_lines);
    Q_INVOKABLE const std::vector<jsonqml::SeriesLineData>& lines() const;
    Q_INVOKABLE void setLineData(int line, const jsonqml::SeriesLineData& lines);
    Q_INVOKABLE jsonqml::SeriesLineData lineData(int line) const;

    /// Get Abscissa name from index
    Q_INVOKABLE static QString abscissaIndexName(int ndx);
    /// Get Abscissa index from name
    Q_INVOKABLE static int indexAbscissaName(const QString name);
    /// Get list of Abscissa indexes to QComboBox
    static QStringList abscissaIndexes(int abscissa_number);

signals:
    void sizeChanged();

private:
    /// Descriptions of all lines
    std::vector<jsonqml::SeriesLineData> lines_data;

};

}

#endif // LEGENDMODEL_H
