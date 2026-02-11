
#include "jsonqml/models/fields_model.h"
#include "jsonqml/models/schema_model.h"
#include "jsonqml/clients/settings_client.h"

namespace jsonqml {

static void struct2model(const jsonio::StructDef* struct_def, FieldLine* parent);
static void field2model(const jsonio::FieldDef* field_def, FieldLine* parent);
static void list2model(size_t level, const jsonio::FieldDef* field_def, FieldLine* parent);

/// \class SelectSchemaLine represents an item in a tree view, and contains several columns of data.
/// Each items data based on thrift field definition
class FieldLine
{
    friend class SelectFieldsModel;
    friend void list2model(size_t level, const jsonio::FieldDef* field_def, FieldLine* parent);

public:
    FieldLine(const jsonio::FieldDef* afld_def, FieldLine* the_parent);
    ~FieldLine();

    /// Set up default template of values for map or array
    void add_value(const QString& next);

    /// Add path to list of fields
    void add_link(std::vector<std::string>& fields_list, const std::string& path = "");

protected:
    /// Type of field ( 1- array, 2- map, 0 - other)
    int fld_type;
    /// Field description to line
    const jsonio::FieldDef* fld_thrift;

    /// Number of levels for array or map
    int arr_levels;
    /// Default value of field
    QString def_value;
    /// Values into columns (key, value, comment)
    QVector<QString> fld_value;

    size_t ndx_in_parent;
    FieldLine *parent;
    std::vector<std::unique_ptr<FieldLine>> children;
};

FieldLine::FieldLine(const jsonio::FieldDef* afld_def, FieldLine* theparent):
    fld_type(0), fld_thrift(afld_def), arr_levels(0), def_value(""), ndx_in_parent(0), parent(theparent)
{
    if(parent) {
        ndx_in_parent = parent->children.size();
        parent->children.push_back(std::unique_ptr<FieldLine>(this));
    }

    if(fld_thrift) {
        fld_value.append(QString::fromStdString(fld_thrift->name()));
        fld_value.append("");
        fld_value.append(QString::fromStdString(fld_thrift->description()));
    }
    else {
        fld_value.append("root");
        fld_value.append("");
        fld_value.append("");
    }
}

FieldLine::~FieldLine()
{
    children.clear();
}

void FieldLine::add_value(const QString& next)
{
    if(!def_value.isEmpty()) {
        def_value += QString(".");
    }
    def_value += next;
    arr_levels++;
    fld_value[1] = def_value;
}

void FieldLine::add_link(std::vector<std::string>& fields_list, const std::string& path)
{
    if(!parent) {
        fields_list.push_back(path);
        return;
    }

    std::string new_path = fld_value[0].toStdString();
    if(fld_type > 0  && (!fld_value[1].isEmpty() && children.size() > 0)){
        std::string fielndx =  fld_value[1].toStdString();
        std::queue<std::string> names = jsonio::split(fielndx, ";");
        std::string curname;
        while(!names.empty()) {
            if(!names.front().empty()) {
                curname = new_path + "." + names.front();
                if(!path.empty()) {
                    curname += "."+ path;
                }
                parent->add_link(fields_list, curname);
            }
            names.pop();
        }
    }
    else {
        if(!path.empty()) {
            new_path += "."+ path;
        }
        parent->add_link(fields_list, new_path);
    }
}

//--------------------------------------------------------------------------------------
// class SelectFieldsModel
// class for represents the data set and is responsible for fetching the data
// is needed for viewing and for selecting.
// Selecting data from ThriftSchema object
//---------------------------------------------------------------------------------------

SelectFieldsModel::SelectFieldsModel(const QString& aschema, QObject* parent):
    QAbstractItemModel(parent), schema_name(aschema)
{
    header_data << "key" << "indexes" << "comment";
    root_node = std::make_shared<FieldLine>(nullptr,  nullptr);

    const jsonio::StructDef* struct_def=nullptr;
    if(!schema_name.isEmpty()) {
        struct_def = jsonio::ioSettings().Schema().getStruct(schema_name.toStdString());
    }
    if(struct_def == nullptr) {
        ui_logger->warn("SelectFieldsModel: undefined struct definition  {}", schema_name.toStdString());
    }
    else {
        struct2model(struct_def, root_node.get());
    }
}

SelectFieldsModel::~SelectFieldsModel()
{}

QModelIndexList SelectFieldsModel::selectIndexes(const QModelIndex& parent_index,
                                                 const std::vector<std::string>& fields_list) const
{
    QModelIndexList lst;
    for(const auto& field: fields_list) {
        auto index = path2index(parent_index, field);
        lst.append(index);
    }
    return lst;
}

std::vector<std::string> SelectFieldsModel::selectedFields(const QModelIndexList& indexes) const
{
    std::vector<std::string> lst;
    for(const auto& index: indexes) {
        auto path = index2path(index);
        if(!path.empty()) {
            lst.push_back(path);
        }
    }
    return lst;
}

std::string SelectFieldsModel::index2path(const QModelIndex &index) const
{
    std::vector<std::string> selected;
    FieldLine* item = lineFromIndex(index);
    item->add_link(selected);
    if(!selected.empty()) {
        return selected[0];
    }
    return "";
}


QModelIndex SelectFieldsModel::path2index(const QModelIndex& parent_index, const std::string& field) const
{
    std::queue<std::string> names = jsonio::split(field, ".");
    return select_field(parent_index, names);
}

FieldLine *SelectFieldsModel::lineFromIndex(const QModelIndex &index) const
{
    if(index.isValid()) {
        return static_cast<FieldLine *>(index.internalPointer());
    } else {
        return root_node.get();
    }
}

QModelIndex SelectFieldsModel::index(int row, int column, const QModelIndex &parent) const
{
    if(!hasIndex(row, column, parent)) {
        return QModelIndex{};
    }
    auto* parent_item = lineFromIndex(parent);
    if(parent_item->children.size()>row) {
        return createIndex(row, column, parent_item->children[row].get());
    }
    else {
        return QModelIndex{};
    }
}

QModelIndex SelectFieldsModel::parent(const QModelIndex& child) const
{
    if(!child.isValid()) {
        return QModelIndex{};
    }
    auto* child_item = lineFromIndex(child);
    auto* parent_item = child_item->parent;
    return parent_item != root_node.get() ? createIndex(parent_item->ndx_in_parent, 0, parent_item) : QModelIndex{};
}

int SelectFieldsModel::rowCount( const QModelIndex& parent ) const
{
    if(!root_node) {
        return 0;
    }
    if(parent.column() > 0) {
        return 0;
    }
    FieldLine *parent_item = lineFromIndex(parent);
    return parent_item->children.size();
}

int SelectFieldsModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    if(JsonSchemaModel::showComments) {
        return 3;
    }
    return 2;
}

Qt::ItemFlags SelectFieldsModel::flags(const QModelIndex& index) const
{
    Qt::ItemFlags flags = QAbstractItemModel::flags(index);
    FieldLine *item = lineFromIndex(index);
    if(index.column() == 1 && (item->fld_type > 0)) {
        return (flags | Qt::ItemIsEditable);
    }
    else {
        return (flags & ~Qt::ItemIsEditable);
    }
}

QVariant SelectFieldsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(role == Qt::DisplayRole && orientation == Qt::Horizontal && section<header_data.size()) {
        return header_data[section];
    }
    return QVariant();
}

QVariant SelectFieldsModel::data(const QModelIndex& index, int role) const
{
    if(!index.isValid()) {
        return QVariant();
    }

    switch(role) {
    case Qt::DisplayRole:
    case Qt::EditRole: {
        FieldLine *item = lineFromIndex(index);
        if(index.column() < item->fld_value.size()) {
            return item->fld_value[index.column()];
        }
    }
    break;
    case Qt::ToolTipRole:
    case Qt::StatusTipRole: {
        FieldLine *item = lineFromIndex(index);
        return item->fld_value[2];
    }
    default: break;
    }
    return QVariant();
}

bool SelectFieldsModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if(index.isValid() && (role == Qt::EditRole)) {
        FieldLine *line =lineFromIndex(index);
        if(index.column()<line->fld_value.size()) {
            line->fld_value[index.column()] = value.toString();
        }
        emit dataChanged(index, index);
        return true;
    }
    return false;
}

int SelectFieldsModel::levels(const QModelIndex &index) const
{
    FieldLine* item = lineFromIndex(index);
    return item->arr_levels;
}

const jsonio::FieldDef *SelectFieldsModel::fld_description(const QModelIndex &index) const
{
    FieldLine* item = lineFromIndex(index);
    return item->fld_thrift;
}

bool SelectFieldsModel::is_list(const QModelIndex &index) const
{
    FieldLine* item = lineFromIndex(index);
    return item->fld_type==1;
}

QModelIndex SelectFieldsModel::select_field(const QModelIndex& parent_index,
                                            std::queue<std::string> names) const
{
    for(int ii=0; ii<levels(parent_index); ii++) {  //skip array and map names
        if(!names.empty()) {
            names.pop();
        }
    }
    if(names.empty()) { // select current line
        return parent_index;
    }
    // test children
    std::string fname = names.front();
    names.pop();

    QModelIndex item_index;
    for(int rw = 0; rw < rowCount(parent_index); rw++) {
        item_index = index(rw, 0, parent_index);
        std::string fldname = data(item_index, Qt::DisplayRole).toString().toStdString();
        if(fldname == fname) {
            return select_field(item_index, names);
        }
    }
    return QModelIndex();  // not found
}

static void struct2model(const jsonio::StructDef* struct_def, FieldLine* parent)
{
    auto field_it = struct_def->cbegin();
    while(field_it != struct_def->cend()) {
        field2model(field_it->get(), parent);
        field_it++;
    }
}

static void field2model(const jsonio::FieldDef* field_def, FieldLine* parent)
{
    FieldLine *line = new FieldLine(field_def, parent);
    // add levels for objects and arrays
    list2model(0, field_def, line);
}

static void list2model(size_t level, const jsonio::FieldDef* field_def, FieldLine* parent)
{
    switch(field_def->type(level)) {
    case jsonio::FieldDef::T_STRUCT: {
        if(field_def->className().empty()) {
            ui_logger->warn("SelectFieldsModel: undefined struct name into schema  {}", field_def->name());
        }
        else {
            auto struct_def2 = jsonio::ioSettings().Schema().getStruct(field_def->className());
            if(struct_def2 == nullptr) {
                ui_logger->warn("SelectFieldsModel: undefined struct definition  {}", field_def->className());
            }
            else {
                struct2model(struct_def2, parent);
            }
        }
    }
    break;
    case jsonio::FieldDef::T_MAP:  {  // map
        parent->fld_type = 2;
        parent->add_value("key");
        list2model(level+2, field_def, parent);
    }
    break;
    case jsonio::FieldDef::T_LIST:
    case jsonio::FieldDef::T_SET:  {   // !! next level
        if(parent->fld_type == 0) {
            parent->fld_type = 1;
        }
        parent->add_value("0");
        list2model(level+1, field_def, parent);
    }
    break;
    default:     break;
    }
}

} // namespace jsonui


