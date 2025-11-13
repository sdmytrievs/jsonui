#pragma once

#include <QAbstractItemModel>
#include "jsonio/jsonbase.h"

namespace jsonqml {

/// \class JsonBaseModel
/// Abstract class for represents the data set and is responsible for fetching the data
/// is needed for viewing and for writing back any changes.
/// Reading/writing data from/to json type objects
class JsonBaseModel: public QAbstractItemModel
{
    Q_OBJECT

    Q_PROPERTY(QStringList typeNames READ typeNames CONSTANT)
    Q_PROPERTY(bool useSchema READ useSchema CONSTANT)
    Q_PROPERTY(bool useComboBox READ useComboBox NOTIFY editorChange)
    Q_PROPERTY(QList<QVariantMap> editorValues READ editorValues NOTIFY editorChange)

signals:
    void modelExpand();
    void editorChange();

public:
    JsonBaseModel(QObject* parent = nullptr);
    ~JsonBaseModel();

    /// Return internal data as const link
    virtual const jsonio::JsonBase& current_object() const = 0;

    /// Extern update data
    virtual void setupModelData(const std::string& json_string, const QString& schema) = 0;

    virtual bool useSchema() const
    {
       return false;
    }

    Q_INVOKABLE bool isEditable(const QModelIndex &index);

    Q_INVOKABLE virtual QString helpName(const QModelIndex& index) const
    {
        return QString::fromStdString(lineFromIndex(index)->getHelpName());
    }

    Q_INVOKABLE virtual bool isNumber(const QModelIndex& index) const
    {
        return lineFromIndex(index)->isNumber();
    }
    Q_INVOKABLE virtual bool isUnion(const QModelIndex&) const
    {
        return false;
    }
    Q_INVOKABLE virtual bool canBeAdd(const QModelIndex&) const
    {
        return false;
    }
    Q_INVOKABLE virtual bool canBeResized(const QModelIndex&) const
    {
        return false;
    }
    Q_INVOKABLE virtual bool canBeCloned(const QModelIndex&) const
    {
        return false;
    }
    Q_INVOKABLE virtual bool canBeRemoved(const QModelIndex&) const
    {
        return false;
    }

    Q_INVOKABLE virtual const QModelIndex addObject(const QModelIndex& index,
                          const QString &field_type,  const QString &field_name);
    Q_INVOKABLE virtual void resizeArray(const QModelIndex& index, int new_size);
    Q_INVOKABLE virtual const QModelIndex cloneObject(const QModelIndex& index);
    Q_INVOKABLE virtual void removeObject(const QModelIndex& index);

    Q_INVOKABLE virtual QString getFieldPath(const QModelIndex& index) const;
    Q_INVOKABLE virtual QString getFieldData(const QModelIndex& index) const;
    Q_INVOKABLE virtual void setFieldData(const QModelIndex& index, const QString& data);

    QStringList typeNames() const
    {
        return type_names;
    }
    bool useComboBox() const
    {
        return use_combo_box;
    }
    QList<QVariantMap> editorValues() const
    {
        return editor_fields_values;
    }

protected:
    static const QStringList type_names;
    bool use_combo_box;
    QList<QVariantMap> editor_fields_values;

    virtual jsonio::JsonBase& current_data() const = 0;
    virtual jsonio::JsonBase* lineFromIndex(const QModelIndex& index) const = 0;

    virtual bool set_value_via_type(jsonio::JsonBase* object, const std::string& add_key,
                                    jsonio::JsonBase::Type add_type, const std::string& add_value);

    jsonio::JsonBase::Type type_from(const QString &field_type);

    virtual void check_editor_type(const QModelIndex &index);
};


} // namespace jsonqml

