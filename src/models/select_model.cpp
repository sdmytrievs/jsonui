
#include <QColor>
#include "jsonqml/models/select_model.h"
#include "jsonio/service.h"

namespace jsonqml {


SelectModel::SelectModel(QObject *parent)
    : QAbstractTableModel(parent)
{
}

SelectModel::SelectModel(const std::vector<std::string>& list,
                         const std::string& split_regexp,
                         model_line_t &&header_data,
                         QObject *parent)
    : QAbstractTableModel(parent),
      header(std::move(header_data))
{
    for(const auto& item: list) {
        table.push_back(jsonio::regexp_split(item, split_regexp));
    }
    set_default_header();
}

SelectModel::SelectModel(model_table_t &&table_data,
                         model_line_t &&header_data,
                         QObject *parent)
    : QAbstractTableModel(parent),
      header(std::move(header_data)),
      table(std::move(table_data))
{
    set_default_header();
}

SelectModel::~SelectModel()
{
}

void SelectModel::resetTable(model_table_t&& table_data,
                             const model_line_t& header_data)
{
    beginResetModel();
    table = std::move(table_data);
    header = header_data;
    set_default_header();
    endResetModel();
}

void SelectModel::setColorFunction(GetColor_f func)
{
    use_color_function = true;
    color_function = func;
}

/*!
    Returns the model's role names.
*/
QHash<int, QByteArray> SelectModel::roleNames() const
{
    return QHash<int, QByteArray> {
        { Qt::DisplayRole, QByteArrayLiteral("display") }
    };
}

int SelectModel::rowCount(const QModelIndex &index) const
{
    return index.isValid() ? 0 : table.size();
}

int SelectModel::columnCount(const QModelIndex &index) const
{
    return index.isValid() ? 0 : header.size();
}

/*!
    Returns the value for the specified \a item and \a role.
    If \a item is out of bounds an invalid QVariant is returned.
*/
QVariant SelectModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    switch( role )
    {
    case Qt::DisplayRole:
    case Qt::EditRole:
        if(index.row()<table.size() && index.column()<table[index.row()].size()) {
            return QString::fromStdString(table[index.row()][index.column()]);
        }
        return QString("");
    case Qt::ToolTipRole:
    case Qt::StatusTipRole:
        return QString("%1[%2]").arg(QString::fromStdString(header[index.column()])).arg(index.row());
    case Qt::TextAlignmentRole:
        return int(Qt::AlignLeft | Qt::AlignVCenter);
    case  Qt::ForegroundRole:
        if(use_color_function) {
            return color_function(index.row(), index.column());
        }
        break;
    case  Qt::SizeHintRole:
    default: break;
    }
    return QVariant();
}

/*!
    Returns the header data for the given \a role in the \a section
    of the header with the specified \a orientation.
*/
QVariant SelectModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    switch( role ) {
    case Qt::DisplayRole:
        if( orientation == Qt::Horizontal )
            return QString::fromStdString(header[section]);
        else
            return QVariant(section);
    case Qt::TextAlignmentRole:
        return int(Qt::AlignRight | Qt::AlignVCenter);
    default:
        break;
    }
    return QAbstractItemModel::headerData(section, orientation, role);
}

void SelectModel::set_default_header()
{
    if(table.size()>0 && header.empty()) {
        for(size_t jj=0; jj<table.size(); ++jj) {
            header.push_back(std::to_string(jj));
        }
    }
}

//----------------------------------------------------

bool SortFilterProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    // need test compare double values
    return QSortFilterProxyModel::lessThan(left, right);
}


}
