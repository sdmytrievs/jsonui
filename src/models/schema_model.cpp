#include <QColor>
#include "jsonqml/models/schema_model.h"
#include "jsonqml/clients/settings_client.h"
//#include "jsonio/io_settings.h"

namespace jsonqml {
extern std::shared_ptr<spdlog::logger> ui_logger;

JsonSchemaModel::JsonSchemaModel(const QString& schema_name,
                                 const QStringList& header_names,
                                 QObject* parent):
    JsonBaseModel(parent),
    header_data(header_names),
    current_schema(schema_name),
    root_node(jsonio::JsonSchema::object(current_schema.toStdString()))
{
    setupModelData("", schema_name);
}

JsonSchemaModel::~JsonSchemaModel() {}

jsonio::JsonBase* JsonSchemaModel::lineFromIndex(const QModelIndex &index ) const
{
    if(index.isValid()) {
        return static_cast<jsonio::JsonSchema*>(index.internalPointer());
    }
    else {
        return const_cast<jsonio::JsonSchema*>(&root_node);
    }
}

void JsonSchemaModel::setupModelData(const std::string& json_string, const QString& schema)
{
    try {
        beginResetModel();
        if(current_schema != schema) {
            current_schema = schema;
            root_node = jsonio::JsonSchema::object(current_schema.toStdString());
        }
        if(json_string.empty()) {
            root_node.clear();
        }
        else {
            root_node.loads(json_string);
        }
        endResetModel();
    }
    catch(...) {
        endResetModel();
        throw;
    }
    emit modelExpand();
}

QModelIndex JsonSchemaModel::index(int row, int column, const QModelIndex &parent) const
{
    if(!hasIndex(row, column, parent)) {
        return QModelIndex{};
    }
    auto * parent_item = lineFromIndex(parent);
    if(parent_item->size()>row) {
        return createIndex(row, column, parent_item->getChild(row));
    }
    else {
        return QModelIndex{};
    }
}

QModelIndex JsonSchemaModel::parent(const QModelIndex& child) const
{
    if(!child.isValid()) {
        return QModelIndex{};
    }
    auto* child_item = lineFromIndex(child);
    auto* parent_item = child_item->getParent();
    return parent_item != &root_node ? createIndex(parent_item->getNdx(), 0, parent_item) : QModelIndex{};
 }

int JsonSchemaModel::rowCount(const QModelIndex& parent) const
{
    if(parent.column()> 0) {
        return 0;
    }
    return lineFromIndex(parent)->size();
}

int JsonSchemaModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED( parent );
    if(showComments) {
        return 3;
    }
    return 2;
}

Qt::ItemFlags JsonSchemaModel::flags(const QModelIndex& index) const
{
    Qt::ItemFlags flags = QAbstractItemModel::flags(index);
    auto item = lineFromIndex( index );
    if(!editID && item->getKey()[0]=='_') {
        return (flags & ~Qt::ItemIsEditable);
    }
    if(index.column()==1 && !(item->isObject()||item->isArray()||get_map_enumdef(index)!= nullptr)) {
        return (flags | Qt::ItemIsEditable);
    }
    if(index.column()==0 && !item->isTop() && schemajs(item->getParent())->isMap()) {
        return (flags | Qt::ItemIsEditable);
    }
    return (flags & ~Qt::ItemIsEditable);
}

QVariant JsonSchemaModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(role==Qt::DisplayRole && orientation==Qt::Horizontal && section<header_data.size()) {
        return header_data[section];
    }
    return QVariant();
}

QVariant JsonSchemaModel::data(const QModelIndex& index, int role) const
{
    if(!index.isValid()) {
        return QVariant();
    }

    auto item = lineFromIndex(index);
    switch(role)  {
    case Qt::DisplayRole:
        if(useEnumNames) {
            auto enumdef = get_i32_enumdef(schemajs(item));
            if(enumdef != nullptr) {
                int idx;
                item->get_to(idx);
                std::string name = enumdef->value2name(idx);
                if(index.column()==1) {
                    return  QString::fromStdString(name);
                }
                if(index.column()==2) {
                    return  QString::fromStdString(enumdef->name2description(name));
                }
            }
        }
        if(index.column()==2) {
            auto mapenum = get_map_enumdef( index );
            if(mapenum != nullptr) {
                std::string name;
                item->get_to(name);
                return  QString::fromStdString(mapenum->name2description(name));
            }
        }
        return get_value(index.column(), item);
    case Qt::EditRole:
        return get_value(index.column(), item);
    case Qt::ToolTipRole:
    case Qt::StatusTipRole:
        return  QString::fromStdString(schemajs(item)->getFullDescription());
    case Qt::ForegroundRole:  {
        if(index.column()==0 && !item->isTop() && !schemajs(item->getParent())->isMap()) {
            return QVariant(QColor(Qt::darkCyan));
        }
    }
        break;
    default: break;
    }
    return QVariant();
}

bool JsonSchemaModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if(index.isValid() && role==Qt::EditRole) {
        auto item = lineFromIndex(index);
        switch(index.column()) {
        case 0: {
            auto lineschema = schemajs(item);
            lineschema->setMapKey(value.toString().toStdString());
        }
            break;
        case 1:
            set_value_via_type(item, "", item->type(), value.toString().toStdString());
            break;
        default:
            break;
        }
        emit dataChanged(index, index);
        return true;
    }
    return false;
}

bool JsonSchemaModel::canBeResized(const QModelIndex &index) const
{
    auto item = schemajs(lineFromIndex(index));
    return item->isArray() || item->isMap();
}

bool JsonSchemaModel::canBeAdd(const QModelIndex &index) const
{
    auto item = schemajs(lineFromIndex(index));
    if(item->isTop() || (item->isStruct() && item->size()<1)) {
        return true;
    }
    auto parent_item = schemajs(lineFromIndex(parent(index)));
    return parent_item->isStruct();
}

bool JsonSchemaModel::isUnion(const QModelIndex &index) const
{
    auto item = schemajs(lineFromIndex(index));
    return item->isUnion();
}

bool JsonSchemaModel::canBeCloned(const QModelIndex &index) const
{
    auto item = lineFromIndex(index);
    if(item->isTop()) {
        return false;
    }
    auto parent_index = parent(index);
    auto parent_item = schemajs(lineFromIndex(parent_index));
    return parent_item->isArray() || parent_item->isMap();
}

bool JsonSchemaModel::canBeRemoved(const QModelIndex &index) const
{
    auto item = lineFromIndex(index);
    return !item->isTop() && (item->getParent()->isObject()||item->getParent()->isArray());
}

const QModelIndex JsonSchemaModel::addObject(const QModelIndex &cindex,
                    const QString &field_type, const QString &field_name)
{
    Q_UNUSED( field_type );
    if(field_name.isEmpty()) {
         uiSettings().setError(" can't add empty key ");
         return cindex;
    }
    int row=0;
    QModelIndex parent_index;
    auto item = lineFromIndex(cindex);
    if(item->isObject() && item->size()<1) {
        parent_index = cindex.siblingAtColumn(0);
        row = 0;
    }
    else {
        parent_index = parent(cindex).siblingAtColumn(0);
        row = rowCount(parent_index);
    }

    auto parent_item = schemajs(lineFromIndex(parent_index));
    auto field_top_schema_name = parent_item->getStructName();
    std::string new_object_key = field_name.toStdString();
    auto schema_def = jsonio::ioSettings().Schema().getStruct(field_top_schema_name);
    if(!schema_def) {
        uiSettings().setError(" can't add undefined schema "+ QString::fromStdString(field_top_schema_name));
        return cindex;
    }
    auto field_def = schema_def->getField(new_object_key);
    if(!field_def) {
        uiSettings().setError(" can't add undefined field into "+ QString::fromStdString(field_top_schema_name));
        return cindex;
    }
    std::string def_value = item->checked_value(item->type(), field_def->defaultValue());
    jsonio::JsonBase::Type new_object_type = jsonio::JsonSchema::fieldtype2basetype(field_def->type(0));

    try {
        beginResetModel();
        set_value_via_type(parent_item, new_object_key, new_object_type, def_value);
        endResetModel();
    }
    catch(std::exception& e) {
        endResetModel();
        uiSettings().setError(e.what());
    }
    return index(row, 0, parent_index);
}

const QModelIndex JsonSchemaModel::cloneObject(const QModelIndex &cindex)
{
    auto item = lineFromIndex(cindex);
    QModelIndex parent_index = parent(cindex);
    auto parent_item = lineFromIndex(parent_index);
    int row = rowCount(parent_index);
    auto data = item->dump();
    auto defval = item->checked_value(item->type(), "");

    try  {
        // fixed existent keys in map
        auto new_object_key = std::to_string(row);
        auto used_names = parent_item->getUsedKeys();
        while(std::find(used_names.begin(), used_names.end(), new_object_key) != used_names.end()) {
             new_object_key += "0";
        }
        beginInsertRows(parent_index, row, row);
        if(set_value_via_type(parent_item, new_object_key, item->type(), defval) && !data.empty()) {
            parent_item->getChild(row)->loads(data);
        }
        endInsertRows();
    }
    catch(std::exception& e) {
        endInsertRows();
        uiSettings().setError(e.what());
    }
    return index( row, 0, parent_index);
}

void JsonSchemaModel::removeObject(const QModelIndex &index)
{
    auto item = schemajs(lineFromIndex(index));
    size_t level = 0;
    auto field_data = item->fieldDescription(level);
    if(level==0 && field_data->required()==jsonio::FieldDef::fld_required ) {
        ui_logger->error("Required data object cannot be deleted {}", field_data->name());
        return;
    }
    beginRemoveRows(parent(index), index.row(), index.row());
    item->remove();
    endRemoveRows();
}

void  JsonSchemaModel::setFieldData(const QModelIndex& index, const QString& data)
{
    auto item = lineFromIndex(index);
    if(!editID && item->getKey()[0] == '_') {
        return;
    }
    try {
        beginResetModel();
        if(data.isEmpty()) {
            item->clear();
        }
        else {
            item->loads(data.toStdString());
        }
        endResetModel();
    }
    catch(std::exception& e) {
        endResetModel();
        uiSettings().setError(e.what());
    }
}

QStringList JsonSchemaModel::fieldNames(const QModelIndex &index) const
{
    QModelIndex parent_index;
    auto item = schemajs(lineFromIndex(index));
    if(item->isObject() && item->size()<1) {
        parent_index = index.siblingAtColumn(0);
    }
    else  {
        parent_index = parent(index).siblingAtColumn(0);
    }
    auto parent_item = schemajs(lineFromIndex(parent_index));
    auto no_used_list = parent_item->getNoUsedKeys();
    QStringList list;
    std::transform(no_used_list.begin(), no_used_list.end(),
                   std::back_inserter(list),
                   [](const std::string &v){ return QString::fromStdString(v); });
    return list;
}

//-------------------------------------------------------------------------------------

QString JsonSchemaModel::get_value(int column, const jsonio::JsonBase *object) const
{
    switch(column) {
    case 0: return  QString::fromStdString(object->getKey());
    case 2: return  QString::fromStdString(object->getDescription());
    case 1: return  QString::fromStdString(object->getFieldValue());
    }
    return QString();
}

const jsonio::EnumDef* JsonSchemaModel::get_map_enumdef(const QModelIndex& index) const
{
    if(!index.isValid()) {
        return nullptr;
    }
    auto parent_index = parent(index);
    auto parent_item = schemajs(lineFromIndex(parent_index));

    if(parent_item->isMap()) {
        size_t level = 0;
        auto field_data = parent_item->fieldDescription(level);
        std::string enumName = field_data->className();
        if(!enumName.empty()) {
            return jsonio::ioSettings().Schema().getEnum(enumName);
        }
    }
    return nullptr;
}

const jsonio::EnumDef* JsonSchemaModel::get_i32_enumdef(jsonio::JsonSchema* item) const
{
    if(item->fieldType() != jsonio::FieldDef::T_I32) {
        return nullptr;
    }
    size_t level = 0;
    auto field_data = item->fieldDescription(level);
    std::string enumName = field_data->className();
    if(!enumName.empty()) {
        return jsonio::ioSettings().Schema().getEnum(enumName);
    }
    return nullptr;
}

void JsonSchemaModel::enums_to_combobox(const jsonio::EnumDef* enumdef)
{
    for(const auto& itname: enumdef->all_names()) {
        editor_fields_values.push_back(QVariantMap({{QString("value"), enumdef->name2value(itname)},
                                                    {QString("text"),  QString::fromStdString(itname)}}));
    }
}

void JsonSchemaModel::check_editor_type(const QModelIndex &index)
{
    use_combo_box = false;
    editor_fields_values.clear();

    auto item = schemajs(lineFromIndex(index));
    int type;
    // map key
    if(index.column() == 0) {
        type = item->fieldKeyType();
        auto enumdef = get_map_enumdef(index);
        if(enumdef) {
            enums_to_combobox(enumdef);
        }
    }
    else {
        type = item->fieldType();
        if(type == jsonio::FieldDef::T_BOOL) {
            editor_fields_values.push_back(QVariantMap({{QString("value"), true},
                                                        {QString("text"), "true"}}));
            editor_fields_values.push_back(QVariantMap({{QString("value"), false},
                                                        {QString("text"), "false"}}));
        }
        else if(type == jsonio::FieldDef::T_I08 ||
                type == jsonio::FieldDef::T_I16 ||
                type == jsonio::FieldDef::T_I32) {

            auto enumdef = get_i32_enumdef(item);
            if(enumdef) {
                enums_to_combobox(enumdef);
            }
        }
    }
    use_combo_box = editor_fields_values.size()>0;
    emit editorChange();
}

} // namespace jsonqml
