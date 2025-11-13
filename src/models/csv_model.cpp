
#include <QFile>
#include "jsonqml/models/csv_model.h"
#include "jsonio/service.h"
#include "jsonio/jsondetail.h"

namespace jsonqml {

CSVModel::CSVModel(QObject *parent)
    : SelectModel(parent)
{
}

CSVModel::CSVModel(std::string &&csv_data, QObject *parent)
    : SelectModel(parent)
{
    build_table(std::move(csv_data), table, header);
    reset_clmn_type();
}

CSVModel::~CSVModel()
{
}

void CSVModel::resetTable(std::vector<std::vector<std::string> >&& table_data,
                          const std::vector<std::string>& header_data)
{
    beginResetModel();
    table = std::move(table_data);
    header = header_data;
    reset_clmn_type();
    x_clmns.clear();
    y_clmns.clear();
    endResetModel();
}

void CSVModel::setCsvString(std::string&& value_csv)
{
    matrix_from_csv_string(std::move(value_csv));
}

std::string CSVModel::getCsvString()
{
    return matrix_to_csv_string();
}

void CSVModel::setXColumns(const std::vector<int> &clmns)
{
    x_clmns = clmns;
    emit headerDataChanged(Qt::Horizontal, 0, columnCount()-1);
}

void CSVModel::setYColumns(const std::vector<int> &clmns)
{
    y_clmns = clmns;
    emit headerDataChanged(Qt::Horizontal, 0, columnCount()-1);
}

QVariant CSVModel::data(const QModelIndex &index, int role) const
{
    if(!index.isValid()) {
        return QVariant();
    }
    if(role==Qt::DisplayRole || role==Qt::EditRole) {
        if(index.row()<table.size() && index.column()<table[index.row()].size()) {
            if(is_number_clmn[index.column()]) {
                double val=0;
                if(jsonio::is(val, table[index.row()][index.column()])) {
                    return val;
                }
            }
            return QString::fromStdString(table[index.row()][index.column()]);
        }
        return QString("");
    }
    return SelectModel::data(index, role);
}

QVariant CSVModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    switch(role) {
    case Qt::DisplayRole:
        if( orientation == Qt::Horizontal ) {
            QString head;
            if( std::find(x_clmns.begin(), x_clmns.end(), section) != x_clmns.end()) {
                head = "(x)";
            }
            if( std::find(y_clmns.begin(), y_clmns.end(), section) != y_clmns.end()) {
                head = "(y)";
            }
            return QString::fromStdString(header[section])+head;
        }
    default:
        break;
    }
    return SelectModel::headerData(section, orientation, role);
}

bool CSVModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    Q_UNUSED( role );
    if(index.isValid()) {
        if(index.row()<table.size() && index.column()<table[index.row()].size()) {
            table[index.row()][index.column()]=value.toString().toStdString();
        }
        emit dataChanged(index, index);
        return true;
    }
    return false;
}

Qt::ItemFlags CSVModel::flags(const QModelIndex& index) const
{
    if(!index.isValid()) {
       return Qt::NoItemFlags;
    }
    //return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable;
    return (QAbstractItemModel::flags(index) | Qt::ItemIsEditable);
}

void CSVModel::matrix_from_csv_string(std::string&& value_csv)
{
    std::vector<std::string> new_header;
    std::vector<std::vector<std::string>> new_table;

    build_table(std::move(value_csv), new_table, new_header);
    resetTable(std::move(new_table), std::move(new_header));
}

std::string CSVModel::matrix_to_csv_string()
{
    std::string csv_str;
    if(header.size()==0) {
        return csv_str;
    }
    for(const auto &item : header) csv_str += item+",";
    csv_str += "\n";
    for(const auto &row : table) {
        for(const auto &item : row) csv_str += item+",";
        csv_str += "\n";
    }
    return csv_str;
}

void CSVModel::build_table(std::string &&value_csv,
                           std::vector<std::vector<std::string>>& new_table,
                           std::vector<std::string>& new_header)
{
    auto rows = jsonio::regexp_split(value_csv, "\n");
    if(rows.size()>0) {
        new_header = jsonio::regexp_split(rows[0], ",");
        rows.erase(rows.begin());
    }
    for(const auto& item: rows) {
        new_table.push_back(jsonio::regexp_split(item, ","));
    }
}

void CSVModel::reset_clmn_type()
{
    double val;
    size_t count_no_number=0;
    is_number_clmn.clear();
    is_number_clmn.resize(header.size(), true);

    for(const auto &row : table) {
        for(size_t ii=0; ii<row.size(); ++ii) {
            if(is_number_clmn[ii] && !jsonio::is(val,row[ii])) {
                is_number_clmn[ii]=false;
                count_no_number++;
            }
        }
        if(count_no_number>=header.size()) {
            break; // all strings
        }
    }
}

}
