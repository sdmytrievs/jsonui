
#include "jsonqml/charts/legend_model.h"
#include "markershapes.h"

namespace jsonqml {

QString LegendModel::abscissaIndexName(int ndx)
{
    if(ndx == -2) {
        return QString("Off");
    }
    if(ndx == -1) {
        return QString("#");
    }
    return QString("%1").arg(ndx);
}

int LegendModel::indexAbscissaName(const QString name)
{
    if(name == QString("Off")) {
        return -2;
    }
    if(name == QString("#")) {
        return -1;
    }
    return name.toInt();
}

QStringList LegendModel::abscissaIndexes(int abscissa_number)
{
    QStringList lst;
    lst << abscissaIndexName(-2);
    lst << abscissaIndexName(-1);
    for(int ii=0; ii<abscissa_number; ii++) {
        lst <<  abscissaIndexName(ii);
    }
    return lst;
}

LegendModel::LegendModel(QObject *parent)
    : QAbstractListModel{parent}
{
}

int LegendModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return static_cast<int>(lines_data.size());
}

QVariant LegendModel::data(const QModelIndex &index, int role) const
{
    if(!index.isValid() || index.row() >= rowCount()) {
        return QVariant();
    }

    const jsonqml::SeriesLineData& info = lines_data[index.row()];
    switch (role) {
    case IdRole:
        return index.row();
    case ShapeIconRole: {
        QString info_str = QString("%1/%2/%3").arg(info.markerShape())
        .arg(info.penSize()).arg(info.color().name());
        return info_str;
        //return markerShapeIcon(info);
    }
    case AbscissaIndexRole:
        return info.xColumn()+2; //abscissaIndexName(info.xColumn());
    case NameRole:
        return info.name();
    }
    return QVariant();
}

bool LegendModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if(!index.isValid() || index.row() >= rowCount()) {
        return false;
    }

    jsonqml::SeriesLineData& info = lines_data[index.row()];
    switch (role) {
    case AbscissaIndexRole:
        info.setXColumn(value.toInt()-2);
        //info.setXColumn(indexAbscissaName(value.toString()));
        break;
    case NameRole:
        info.setName(value.toString());
        break;
    default: return false;
    }

    emit dataChanged(index, index, { role });
    return true ;
}

Qt::ItemFlags LegendModel::flags(const QModelIndex& index) const
{
    if(!index.isValid()) {
        return Qt::NoItemFlags;
    }
    return (QAbstractItemModel::flags(index) | Qt::ItemIsEditable);
}

QHash<int, QByteArray> LegendModel::roleNames() const
{
    return {
        {IdRole, "id"},
        {ShapeIconRole, "icon"},
        {AbscissaIndexRole, "abscissa"},
        {NameRole, "name"}
    };
}

void LegendModel::updateLines(const std::vector<jsonqml::SeriesLineData>& in_lines)
{
    beginResetModel();
    lines_data = in_lines;
    endResetModel();
}

const std::vector<SeriesLineData> &LegendModel::lines() const
{
    return lines_data;
}

void LegendModel::setLineData(int line, const jsonqml::SeriesLineData &data)
{
    if(line<rowCount() || line>0) {
        lines_data[line] = data;
        emit dataChanged(index(line), index(line));
    }
}

jsonqml::SeriesLineData LegendModel::lineData(int line) const
{
    if(line>=rowCount() || line<0) {
        return jsonqml::SeriesLineData{};
    }
    return  lines_data[line];
}

}
