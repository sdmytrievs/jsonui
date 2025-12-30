
#include "jsonqml/models/base_model.h"
#include "jsonqml/clients/settings_client.h"
#include "jsonio/service.h"

namespace jsonqml {


const QStringList JsonBaseModel::type_names= {"string", "bool", "int", "double", "object", "array", "null"};

void JsonBaseModel::updateModel()
{
    beginResetModel();
    endResetModel();
}

JsonBaseModel::JsonBaseModel(QObject *parent):
    QAbstractItemModel(parent)
{}

JsonBaseModel::~JsonBaseModel() {}

// Set up editor data if ComboBox input
bool JsonBaseModel::isEditable(const QModelIndex &index)
{
    check_editor_type(index);
    return flags(index)&Qt::ItemIsEditable;
}

const QModelIndex JsonBaseModel::addObject(const QModelIndex &cindex,
                                           const QString &field_type,
                                           const QString &field_name)
{
    if(field_name.isEmpty()) {
        uiSettings().setError(" can't add empty key ");
        return cindex;
    }

    int row;
    auto item = lineFromIndex(cindex);
    QModelIndex pindex = parent_add(cindex, row);
    auto parent_item = lineFromIndex(pindex);

    std::string new_object_key = field_name.toStdString();
    jsonio::trim(new_object_key);
    jsonio::JsonBase::Type new_object_type = type_from(field_type);
    auto defval = item->checked_value(new_object_type, "");

    auto used_names = parent_item->getUsedKeys();
    if(std::find(used_names.begin(), used_names.end(), new_object_key) != used_names.end()) {
        uiSettings().setError(" can't add existing key "+field_name);
        return cindex;
    }

    try {
        beginInsertRows(pindex, row, row);
        set_value_via_type(parent_item, new_object_key, new_object_type, defval);
        endInsertRows();
    }
    catch(std::exception& e) {
        endInsertRows();
        uiSettings().setError(e.what());
    }
    return index(row, 0, pindex);
}

void JsonBaseModel::resizeArray(const QModelIndex &cindex, int new_size)
{
    auto item = lineFromIndex(cindex);
    auto old_size = rowCount(cindex);
    if(old_size>new_size) {
        try {
            beginRemoveRows(cindex, new_size, old_size-1);
            item->array_resize(new_size, "");
            endRemoveRows();
        }
        catch(std::exception& e) {
            endRemoveRows();
            uiSettings().setError(e.what());
        }
    }
    else if(old_size<new_size) {
        try {
            beginInsertRows(cindex, old_size, new_size-1);
            item->array_resize(new_size, "");
            endInsertRows();
        }
        catch(std::exception& e) {
            endInsertRows();
            uiSettings().setError(e.what());
        }
    }
}

const QModelIndex JsonBaseModel::cloneObject(const QModelIndex &cindex)
{
    auto item = lineFromIndex(cindex);
    QModelIndex parent_index = parent(cindex);
    auto parent_item = lineFromIndex(parent_index);
    if(!parent_item->isArray()) {
        return cindex;
    }

    int row = rowCount(parent_index);
    auto data = item->dump();
    auto defval = item->checked_value(item->type(), "");

    try {
        beginInsertRows(parent_index, row, row);
        if(set_value_via_type(parent_item, std::to_string(row), item->type(), defval) && !data.empty()) {
            parent_item->getChild(row)->loads(data);
        }
        endInsertRows();
    }
    catch(std::exception& e) {
        endInsertRows();
        uiSettings().setError(e.what());
    }
    return index(row, 0, parent_index);
}

void JsonBaseModel::removeObject(const QModelIndex &index)
{
    auto item = lineFromIndex(index);
    beginRemoveRows(parent(index), index.row(), index.row());
    item->remove();
    endRemoveRows();
}

QString JsonBaseModel::getFieldPath(const QModelIndex &index) const
{
    return QString::fromStdString(lineFromIndex(index)->get_path());
}

QString JsonBaseModel::getFieldData(const QModelIndex &index) const
{
    return QString::fromStdString(lineFromIndex(index)->dump());
}

void  JsonBaseModel::setFieldData(const QModelIndex& index, const QString& data)
{
    auto line = lineFromIndex(index);
    try {
        beginResetModel();
        if(data.isEmpty()) {
            line->clear();
        }
        else {
            line->loads(data.toStdString());
        }
        endResetModel();
    }
    catch(std::exception& e) {
        endResetModel();
        uiSettings().setError(e.what());
    }
}

bool JsonBaseModel::dataIndex(const QModelIndex &index, QString &data, QString &type, int &size) const
{
    auto item = lineFromIndex(index);
    data = QString::fromStdString(item->getKey());
    type = item->typeName();
    size = item->size();
    return true;
}

void JsonBaseModel::setOid(const std::string &doc_id)
{
    beginResetModel();
    current_data().set_oid(doc_id);
    endResetModel();
}

const jsonio::JsonBase* JsonBaseModel::set_value_via_type(jsonio::JsonBase* object, const std::string& add_key,
                                       jsonio::JsonBase::Type add_type, const std::string& add_value)
{
    const jsonio::JsonBase* new_object = nullptr;
    if(!object) {
        return new_object;
    }

    switch(add_type) {
    case jsonio::JsonBase::Null:
    case jsonio::JsonBase::Bool:
    case jsonio::JsonBase::Int:
    case jsonio::JsonBase::Double:
    case jsonio::JsonBase::String:
        new_object = object->set_scalar_via_path(add_key, add_value);
        break;
    case jsonio::JsonBase::Object:
        new_object = &object->add_object_via_path(add_key);
        break;
    case jsonio::JsonBase::Array:
        new_object = &object->add_array_via_path(add_key);
        //if( !add_value.toString().isEmpty() )
        //    new_object.loads(add_value.toString().toStdString());
        break;
    }
    return new_object;
}

jsonio::JsonBase::Type JsonBaseModel::type_from(const QString &field_type) const
{
    jsonio::JsonBase::Type type = jsonio::JsonBase::Null;

    auto index = type_names.indexOf(field_type, 0, Qt::CaseInsensitive);
    switch(index) {
    case -1: type = jsonio::JsonBase::Null; break;
    case 1:  type = jsonio::JsonBase::Bool; break;
    case 2: type = jsonio::JsonBase::Int;   break;
    case 3: type = jsonio::JsonBase::Double; break;
    case 0: type = jsonio::JsonBase::String; break;
    case 4: type = jsonio::JsonBase::Object; break;
    case 5: type = jsonio::JsonBase::Array; break;
    }
    return type;
}

void JsonBaseModel::check_editor_type(const QModelIndex &index)
{
    use_combo_box = false;
    editor_fields_values.clear();
    if(lineFromIndex(index)->isBool()) {
        use_combo_box = true;
        editor_fields_values.push_back(QVariantMap({{QString("value"), true},
                                                    {QString("text"), "true"}}));
        editor_fields_values.push_back(QVariantMap({{QString("value"), false},
                                                    {QString("text"), "false"}}));
    }
    emit editorChange();
}

QModelIndex JsonBaseModel::parent_add(const QModelIndex &index, int &row) const
{
    QModelIndex pindex;
    auto item = lineFromIndex(index);
    if(item->isObject() && item->size()<1) {
        pindex = index.siblingAtColumn(0);
        row = 0;
    }
    else {
        pindex = parent(index).siblingAtColumn(0);
        row = rowCount(pindex);
    }
    return pindex;
}

} // namespace jsonqml
