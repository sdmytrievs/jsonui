#pragma once

#include <queue>
#include <QAbstractItemModel>

namespace jsonio {
class FieldDef;
}

namespace jsonqml {

class FieldLine;

/// \class SelectFieldsModel
/// class for represents the data set and is responsible for fetching the data
/// is needed for viewing and for selecting.
/// Each items data based on thrift field definition for current thrift schema
class SelectFieldsModel: public QAbstractItemModel
{
    Q_OBJECT

public:
    SelectFieldsModel(const QString& the_schema_name, QObject* parent = nullptr);
    ~SelectFieldsModel();

    QModelIndexList selectIndexes(const QModelIndex &parent_index, const std::vector<std::string> &fields_list) const;
    std::vector<std::string> selectedFields(const QModelIndexList &indexes) const;
    QModelIndex path2index(const QModelIndex &parent_index, const std::string &field) const;
    std::string index2path(const QModelIndex &index) const;
    const jsonio::FieldDef* fld_description(const QModelIndex &index) const;
    bool is_list(const QModelIndex &index) const;

private:
    QModelIndex index(int row, int column, const QModelIndex& parent) const;
    QModelIndex parent(const QModelIndex& child) const;
    int rowCount(const QModelIndex& parent) const;
    int columnCount(const QModelIndex& parent) const;
    QVariant data(const QModelIndex& index, int role) const;
    bool setData(const QModelIndex& index, const QVariant & value, int role);
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    Qt::ItemFlags flags(const QModelIndex& index) const;

    FieldLine* lineFromIndex(const QModelIndex& index) const;

protected:
    QStringList header_data;
    QString schema_name;
    std::shared_ptr<FieldLine> root_node;

    int levels(const QModelIndex& index) const;
    QModelIndex select_field(const QModelIndex &parent_index,
                             std::queue<std::string> names) const;
};

} // namespace jsonui

