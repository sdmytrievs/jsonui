#pragma once

#include <memory>
#include "jsonqml/models/base_model.h"
#include "jsonio/jsonfree.h"

namespace jsonqml {

/// \class JsonFreeModel
/// Class for represents the data set and is responsible for fetching the data
/// is needed for viewing and for writing back any changes.
/// Reading/writing data from/to json free format objects
class JsonFreeModel: public JsonBaseModel
{
    Q_OBJECT

public:
    explicit JsonFreeModel(const QStringList& header_names, QObject* parent = nullptr);
    ~JsonFreeModel();

    /// Return internal data to const link
    const jsonio::JsonBase& current_object() const override
    {
        return  root_node;
    }

    /// Extern update data
    void setupModelData(const std::string& json_string, const QString& schema) override;

    Q_INVOKABLE bool canBeAdd(const QModelIndex& index) const override
    {
        auto item = lineFromIndex(index);
        return (item->isTop() || item->getParent()->isObject() ||
                (item->isObject() && item->size()<1));
    }
    Q_INVOKABLE bool canBeResized(const QModelIndex& index) const override
    {
        return lineFromIndex(index)->isArray();
    }
    Q_INVOKABLE bool canBeCloned(const QModelIndex& index) const override
    {
        auto item = lineFromIndex(index);
        return  !item->isTop() && item->getParent()->isArray();
    }
    Q_INVOKABLE bool canBeRemoved(const QModelIndex& index) const override
    {
        auto item = lineFromIndex(index);
        return !item->isTop() && item->getParent()->isStructured();
    }

private:
    QModelIndex index(int row, int column, const QModelIndex& parent) const override;
    QModelIndex parent(const QModelIndex& child) const override;
    int rowCount(const QModelIndex& parent) const override;
    int columnCount(const QModelIndex& parent) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role) override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;

protected:
    /// Names of columns
    QStringList header_data;
    /// Current document object
    jsonio::JsonFree root_node;

    jsonio::JsonFree& current_data() const override
    {
        return  const_cast<jsonio::JsonFree&>(root_node);
    }
    jsonio::JsonBase* lineFromIndex(const QModelIndex& index) const override;
};

} // namespace jsonqml

