
#include <QColor>
#include "jsonqml/models/json_model.h"

namespace jsonqml {


JsonFreeModel::JsonFreeModel(const QStringList& header_names, QObject* parent):
    JsonBaseModel(parent),
    header_data(header_names),
    root_node(jsonio::JsonFree::object())
{
    setupModelData("", "");
}

JsonFreeModel::~JsonFreeModel() {}

jsonio::JsonBase* JsonFreeModel::lineFromIndex(const QModelIndex &index) const
{
    if(index.isValid())  {
        return static_cast<jsonio::JsonFree*>(index.internalPointer());
    }
    else {
        return const_cast<jsonio::JsonFree*>(&root_node);
    }
}

void JsonFreeModel::setupModelData(const std::string& json_string, const QString&)
{
    try {
        beginResetModel();
        if(json_string.empty()) {
            root_node.clear();
        }
        else {
            root_node.loads(json_string);
        }
        endResetModel();
    }
    catch(...)
    {
        endResetModel();
        throw;
    }
    emit modelExpand();
}

QModelIndex JsonFreeModel::index(int row, int column, const QModelIndex &parent) const
{
    if(!hasIndex(row, column, parent)) {
        return QModelIndex{};
    }
    auto* parent_item = lineFromIndex(parent);
    if(parent_item->size()>row) {
        return createIndex(row, column, parent_item->getChild(row));
    }
    else {
        return QModelIndex{};
    }
}

QModelIndex JsonFreeModel::parent(const QModelIndex& child) const
{
    if(!child.isValid()) {
        return QModelIndex{};
    }
    auto* child_item = lineFromIndex(child);
    auto* parent_item = child_item->getParent();
    return parent_item != &root_node ? createIndex(parent_item->getNdx(), 0, parent_item) : QModelIndex{};
}

int JsonFreeModel::rowCount(const QModelIndex& parent) const
{
    if(parent.column()> 0) {
        return 0;
    }
    return lineFromIndex(parent)->size();
}

int JsonFreeModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED( parent );
    return 2;
}

Qt::ItemFlags JsonFreeModel::flags(const QModelIndex& index) const
{
    Qt::ItemFlags flags = QAbstractItemModel::flags(index);
    auto item = lineFromIndex(index);
    if(index.column() == 1 && (!item->isObject() && !item->isArray())) {
        return (flags | Qt::ItemIsEditable);
    }
    else {
        return (flags & ~Qt::ItemIsEditable);
    }
}

QVariant JsonFreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(role == Qt::DisplayRole && orientation == Qt::Horizontal && section<header_data.size()) {
        return header_data[section];
    }
    return QVariant();
}

QVariant JsonFreeModel::data(const QModelIndex& index, int role) const
{
    if(!index.isValid()) {
        return QVariant();
    }

    switch(role) {
    case Qt::DisplayRole:
    case Qt::EditRole:  {
        auto item = lineFromIndex(index);
        if(index.column() == 0) {
            return QString::fromStdString(item->getKey());
        }
        else if(index.column() == 1) {
            return QString::fromStdString(item->getFieldValue());
        }
    }
        break;
    case Qt::ToolTipRole:
    case Qt::StatusTipRole:
        return QString::fromStdString(lineFromIndex(index)->getDescription());
    case Qt::ForegroundRole:
        if(index.column() == 0) {
            return QVariant(QColor(Qt::darkCyan));
        }
        break;
    default: break;
    }
    return QVariant();
}

bool JsonFreeModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if(index.isValid() && (role == Qt::EditRole)) {
        if(index.column() == 1) {
            auto item = lineFromIndex( index );
            set_value_via_type(item, "", item->type(), value.toString().toStdString());
        }
        emit dataChanged(index, index);
        return true;
    }
    return false;
}

} // namespace jsonqml
