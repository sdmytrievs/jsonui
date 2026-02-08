#pragma once

#include <QAbstractItemModel>
#include <QTreeView>
#include <QItemDelegate>
#include "jsonio/dbquerybase.h"
#include "jsonio/schema.h"

namespace jsonio {
class FieldDef;
}

namespace jsonqml {

class AQLline;

/// \class QueryModel
/// class for represents the data set and is responsible for fetching the data
/// is needed for viewing and for writing back any changes.
/// Reading/writing data from/to query object
class QueryModel: public QAbstractItemModel
{
    Q_OBJECT

public:
    QueryModel(QObject* parent = nullptr);
    ~QueryModel();

    /// Make query string from internal model data
    jsonio::DBQueryBase getQuery(jsonio::DBQueryBase::QType query_type, const std::string& collection);
    /// Make FILTER json object from internal model data
    std::string getFILTER();
    /// Set up internal model data from FILTER json string
    void setFILTER(const std::string& filter_generator);

    void clear_all();
    bool can_logical(const QModelIndex& index, bool AQL_mode) const;
    bool can_operator(const QModelIndex& index, bool AQL_mode) const;
    bool can_field(const QModelIndex& index, bool AQL_mode) const;
    const QModelIndex add_object(const std::string& akey, bool isoperator, const QModelIndex& index);
    const QModelIndex add_field(const std::string& flds_name, const jsonio::FieldDef* fld_data, const QModelIndex& index);
    void del_object(const QModelIndex& index);

    jsonio::FieldDef::FieldType field_type(const QModelIndex& index) const;
    std::string field_enum_class(const QModelIndex& index) const;

protected:
    QStringList header;
    std::unique_ptr<AQLline> root_node;

    QModelIndex index(int row, int column, const QModelIndex& parent) const override;
    QModelIndex parent(const QModelIndex& child) const override;
    int rowCount(const QModelIndex& parent) const override;
    int columnCount(const QModelIndex& parent) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role) override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    AQLline* lineFromIndex(const QModelIndex& index) const;

};

} // namespace jsonqml


