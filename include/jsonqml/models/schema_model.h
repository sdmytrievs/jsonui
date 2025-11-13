#pragma once

#include <memory>
#include "jsonqml/models/base_model.h"
#include "jsonio/jsonschema.h"

namespace jsonqml {

/// \class JsonSchemaModel
/// Class for represents the data set and is responsible for fetching the data
/// is needed for viewing and for writing back any changes.
/// Reading/writing data from/to schema based json objects
class JsonSchemaModel: public JsonBaseModel
{
    Q_OBJECT

public:
    /// Extern flag to show schema comments into editor
    inline static bool showComments = false;
    /// Extern flag to show enum names into editor
    inline static bool useEnumNames = false;
    /// Extern flag to edit system data
    inline static bool editID = false;

    JsonSchemaModel(const QString& schema_name,
                    const QStringList& header_names,
                    QObject* parent = nullptr);
    ~JsonSchemaModel();

    /// Return internal data to const link
    const jsonio::JsonBase& current_object() const override
    {
        return  root_node;
    }

    /// Extern update data
    void setupModelData(const std::string& json_string, const QString& schema) override;

    bool useSchema() const override
    {
       return true;
    }
    Q_INVOKABLE QStringList fieldNames(const QModelIndex& index) const;

    Q_INVOKABLE bool isUnion(const QModelIndex& index) const override;
    Q_INVOKABLE bool canBeAdd(const QModelIndex& index) const override;
    Q_INVOKABLE bool canBeResized(const QModelIndex& index) const override;
    Q_INVOKABLE bool canBeCloned(const QModelIndex& index) const override;
    Q_INVOKABLE bool canBeRemoved(const QModelIndex& index) const override;

    Q_INVOKABLE const QModelIndex addObject(const QModelIndex& index,
                          const QString &field_type,  const QString &field_name) override;
    Q_INVOKABLE const QModelIndex cloneObject(const QModelIndex& index) override;
    Q_INVOKABLE void removeObject(const QModelIndex& index) override;
    Q_INVOKABLE void setFieldData(const QModelIndex& index, const QString& data) override;

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
    /// Current document schema
    QString current_schema;
    /// Current document object
    jsonio::JsonSchema root_node;

    jsonio::JsonSchema& current_data() const override
    {
        return  const_cast<jsonio::JsonSchema&>(root_node);
    }
    jsonio::JsonBase* lineFromIndex(const QModelIndex& index) const override;

    QString get_value(int column, const jsonio::JsonBase *object) const;
    const jsonio::EnumDef *get_map_enumdef(const QModelIndex& index) const;
    const jsonio::EnumDef *get_i32_enumdef(jsonio::JsonSchema* item) const;
    jsonio::JsonSchema* schemajs(jsonio::JsonBase* base_ptr) const
    {
        return dynamic_cast<jsonio::JsonSchema*>(base_ptr);
    }
    void enums_to_combobox(const jsonio::EnumDef* enumdef);
    void check_editor_type(const QModelIndex &index) override;

};

} // namespace jsonqml

