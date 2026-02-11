#include "jsonio/jsonschema.h"
#include "jsonio/jsondump.h"
#include "jsonio/io_settings.h"
#include "jsonqml/models/query_model.h"

namespace jsonqml {

struct SchemaFieldData
{
    /// Thrift type of field
    jsonio::FieldDef::FieldType field_type = jsonio::FieldDef::T_STRING;
    /// Name of enum
    std::string class_name = "";
};

/// \class QueryLine represents a query item in a tree view.
/// Item data defines one query object
class AQLline
{
    friend class QueryModel;

public:
    enum fieldType
    { Top, Field, LogicalTwo, LogicalNot, Operator };

    AQLline(size_t andx, const std::string& akey, fieldType atype, AQLline* parentline);
    virtual ~AQLline();

    /// Return json string with template query
    std::string toTemplate();
    /// Return string with AQL query
    std::string toAQL(const std::string& collection);

protected:
    /// Type of field
    fieldType type;
    /// Name of field
    std::string keyname;
    /// Value of field
    std::string value;
    /// Internal data from thrift schema
    SchemaFieldData field_data;

    size_t ndx_in_parent;
    AQLline *parent;
    std::vector<std::shared_ptr<AQLline>> children;

    fieldType type_from_key(const std::string& akey);
    void query2dom(jsonio::JsonBase& object);
    void value2dom(jsonio::JsonBase& object);
    std::string query2AQL();
    std::string value2AQL(const std::string& the_operator);
    bool remove_child(AQLline *child);

    /// Return json object
    virtual void toDom(jsonio::JsonBase& object) const;
    /// Set json dom object
    virtual void fromDom(const jsonio::JsonBase& object);
    /// Json value type
    jsonio::JsonBase::Type value_type();
    int value_from_enum() const;

    /// Get Value from string
    /// If field is not type T, the false will be returned.
    template <class T>
    bool get_value(T& val)
    {
        if(jsonio::is<T>(val, value)) {
            return true;
        }
        return false;
    }

};

//--------------------------------------------------------------------------------------
//  class TQueryModel
//  class for represents the data set and is responsible for fetchin
//  the data is neaded for viewing and for writing back any changes.
//---------------------------------------------------------------------------------------

QueryModel::QueryModel(QObject* parent):
    QAbstractItemModel(parent), root_node(nullptr)
{
    header << "key" << "value";
    root_node.reset(new AQLline(0, "root", AQLline::Top, nullptr));
}

QueryModel::~QueryModel()
{}

void QueryModel::clear_all()
{
    beginResetModel();
    root_node.reset(new AQLline(0, "root", AQLline::Top, nullptr));
    endResetModel();
}

bool QueryModel::can_logical(const QModelIndex &index, bool AQL_mode) const
{
    if(!AQL_mode) {
        return false;
    }
    auto item = lineFromIndex(index);
    return( item && ((item->children.empty() && item->type==AQLline::Top)
                     || item->type==AQLline::LogicalTwo
                     || (item->children.empty() && item->type==AQLline::LogicalNot)));
}

bool QueryModel::can_operator(const QModelIndex &index, bool AQL_mode) const
{
    if(!AQL_mode) {
        return false;
    }
    auto item = lineFromIndex(index);
    return( item && ((item->children.empty() && item->type==AQLline::Top)
                     || item->type==AQLline::LogicalTwo
                     || (item->children.empty() && item->type==AQLline::LogicalNot)));
}

bool QueryModel::can_field(const QModelIndex &index, bool AQL_mode) const
{
    if(!AQL_mode) {
        return true;
    }
    auto item = lineFromIndex(index);
    return (item && item->children.empty() && item->type==AQLline::Operator);
}

AQLline *QueryModel::lineFromIndex(const QModelIndex &index) const
{
    if (index.isValid()) {
        return static_cast<AQLline *>(index.internalPointer());
    } else {
        return root_node.get();
    }
}

QModelIndex QueryModel::index(int row, int column, const QModelIndex &parent) const
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

QModelIndex QueryModel::parent(const QModelIndex& child) const
{
    if(!child.isValid()) {
        return QModelIndex{};
    }
    auto* child_item = lineFromIndex(child);
    auto* parent_item = child_item->parent;
    return parent_item != root_node.get() ? createIndex(parent_item->ndx_in_parent, 0, parent_item) : QModelIndex{};
}

int QueryModel::rowCount( const QModelIndex& parent ) const
{
   if(parent.column()> 0) {
        return 0;
    }
    return lineFromIndex(parent)->children.size();
}	

int QueryModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return 2;
}	

Qt::ItemFlags QueryModel::flags(const QModelIndex& index) const
{
    Qt::ItemFlags flags = QAbstractItemModel::flags(index);
    AQLline *item = lineFromIndex(index);
    if( index.column() == 1 && item && item->type == AQLline::Field) {
        return (flags | Qt::ItemIsEditable);
    }
    return (flags & ~Qt::ItemIsEditable);
}

QVariant QueryModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(role == Qt::DisplayRole && orientation == Qt::Horizontal) {
        return header[section];\
    }
    return QVariant();
}

QVariant QueryModel::data(const QModelIndex& index, int role) const
{
    if(!index.isValid()) {
        return QVariant();
    }

    switch(role) {
    case Qt::DisplayRole:
    case Qt::EditRole:  {
        AQLline *item = lineFromIndex(index);
        if(index.column()== 0) {
            return QString::fromStdString(item->keyname);
        }
        else if(index.column()== 1) {
            return QString::fromStdString(item->value);
        }
    }
    break;
    default: break;
    }
    return QVariant();
}

bool QueryModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if(index.isValid() && role == Qt::EditRole) {
        if(index.column()== 1) {
            AQLline *line = lineFromIndex(index);
            if(line) {
                line->value = value.toString().toStdString();
            }
        }
        emit dataChanged(index, index);
        return true;
    }
    return false;
}


const QModelIndex QueryModel::add_field(const std::string& flds_name,
                                        const jsonio::FieldDef* fld_data,
                                        const QModelIndex& parent_index)
{
    AQLline *line = lineFromIndex(parent_index);
    if(!line) {
        return parent_index;
    }

    int row = static_cast<int>(line->children.size());
    beginInsertRows(parent_index, row, row);
    AQLline* newline = new AQLline(line->children.size(), flds_name, AQLline::Field, line);
    newline->field_data.field_type = fld_data->type();
    newline->field_data.class_name = fld_data->className();
    endInsertRows();
    return index(row, 0, parent_index);
}

const QModelIndex QueryModel::add_object(const std::string& akey,
                                         bool isoperator,
                                         const QModelIndex& parent_index)
{
    AQLline *line = lineFromIndex(parent_index);
    if(!line) {
        return parent_index;
    }
    int row = static_cast<int>(line->children.size());
    beginInsertRows(parent_index, row, row);
    auto type = (isoperator ? AQLline::Operator : (akey=="NOT" ? AQLline::LogicalNot: AQLline::LogicalTwo));
    new AQLline(line->children.size(), akey, type, line);
    endInsertRows();
    return index(row, 0, parent_index);
}

void QueryModel::del_object(const QModelIndex& index)
{
    AQLline *line = lineFromIndex(index);

    const QModelIndex parent_index = parent(index);
    AQLline *line_parent = line->parent;
    beginRemoveRows(parent_index, index.row(), index.row());
    line_parent->remove_child(line);
    endRemoveRows();
}

jsonio::FieldDef::FieldType QueryModel::field_type(const QModelIndex &index) const
{
    AQLline *line = lineFromIndex(index);
    return line->field_data.field_type;
}

std::string QueryModel::field_enum_class(const QModelIndex &index) const
{
    AQLline *line = lineFromIndex(index);
    return line->field_data.class_name;
}

jsonio::DBQueryBase QueryModel::getQuery(jsonio::DBQueryBase::QType query_type,
                                         const std::string& collection)
{
    std::string query_string;

    if(root_node->children.size()<1) {
        return jsonio::DBQueryBase(); // all
    }
    if(query_type == jsonio::DBQueryBase::qTemplate) {
        query_string = root_node->toTemplate();
    }
    else if(query_type == jsonio::DBQueryBase::qAQL) {
        query_string = root_node->toAQL(collection);
    }
    else {
        query_type = jsonio::DBQueryBase::qUndef;
    }

    return jsonio::DBQueryBase(query_string, query_type);
}

std::string QueryModel::getFILTER()
{
    auto query_object = jsonio::JsonFree::object();
    root_node->toDom(query_object);
    return query_object.dump(true);
}

void QueryModel::setFILTER(const std::string &filter)
{
    if(!filter.empty()) {
        auto  query_object = jsonio::json::loads(filter);
        root_node->fromDom(query_object);
    }
}

//-----------------------------------------------------------------

AQLline::AQLline(size_t andx, const std::string& akey, AQLline::fieldType atype, AQLline* parentline):
    type(atype), keyname(akey), value(""), ndx_in_parent(andx)
{
    if(!parentline) {
        type = Top;
    }
    parent = parentline;
    if(parent) {
        parent->children.push_back(std::shared_ptr<AQLline>(this));
        field_data = parent->field_data;
    }
}

AQLline::~AQLline()
{
    children.clear();
}

jsonio::JsonBase::Type AQLline::value_type()
{
    auto jstype = jsonio::JsonSchema::fieldtype2basetype(field_data.field_type);
    if(parent && parent->type == AQLline::Operator &&
        ( parent->keyname == "IN" || parent->keyname == "NOT IN" )) {
        jstype = jsonio::JsonBase::Array;
    }

    if( parent && parent->type == AQLline::Operator &&
        (parent->keyname == "LIKE" || parent->keyname == "=~" || parent->keyname == "!~")) {
        jstype = jsonio::JsonBase::String;
    }
    return jstype;
}

int AQLline::value_from_enum() const
{
    int ivalue=0;
    const jsonio::EnumDef* enumdef = jsonio::ioSettings().Schema().getEnum(field_data.class_name);
    if(enumdef != nullptr) {
        ivalue = enumdef->name2value(value);
    }
    return ivalue;
}

void AQLline::value2dom(jsonio::JsonBase& object)
{
    std::string add_key = keyname;
    auto jstype = value_type();

    switch(jstype) {
    case jsonio::JsonBase::Null:
        object.set_value_via_path(keyname, "");
        object.getChild(keyname)->set_null();
        break;
    case jsonio::JsonBase::Bool:
        object.set_value_via_path(keyname, (value == "true"? true : false));
        break;
    case jsonio::JsonBase::Int: {
        int ivalue=0;
        if(!field_data.class_name.empty()) {
            ivalue = value_from_enum();
            object.set_value_via_path(keyname, ivalue);
        }
        else if(!get_value(ivalue)) {
            object.set_value_via_path(keyname, ivalue);
        }
    }
    break;
    case jsonio::JsonBase::Double: {
        int dvalue;
        if(!get_value(dvalue)) {
            object.set_value_via_path(keyname, dvalue);
        }
    }
    break;
    case jsonio::JsonBase::String:
        object.set_value_via_path(keyname, value);
        break;
    case jsonio::JsonBase::Object:
        object.add_object_via_path(keyname).loads(value);
        break;
    case jsonio::JsonBase::Array:
        object.add_array_via_path(keyname).loads(value);
        break;
    }
}

void AQLline::query2dom(jsonio::JsonBase& object)
{
    switch(type) {
    // important datatypes
    case Top:
        object.clear();
        for(size_t ii1=0; ii1< children.size(); ii1++) {
            children[ii1]->query2dom(object);
        }
        break;
    case Field:
        if(!value.empty()) {
            value2dom(object);
        }
        break;
    default: break;
    }
}

std::string AQLline::toTemplate()
{
    auto  dom_data = jsonio::JsonFree::object();
    query2dom(dom_data);
    return dom_data.dump();
}


std::string AQLline::value2AQL(const std::string& the_operator)
{
    auto jstype = value_type();
    std::string filter("u." + keyname + " " + the_operator + " ");

    switch (jstype) {
    case jsonio::JsonBase::String:  {
        auto str = jsonio::json::dump(value);
        //filter += "'"+str+"'";
        filter += str;
    }
    break;
    case jsonio::JsonBase::Int:  {
        if(!field_data.class_name.empty()) {
            auto val = value_from_enum();
            filter += std::to_string(val);
        }
        else {
            filter += value;
        }
    }
    break;
    default:
        filter += value;
        break;
    }
    filter += " ";
    return filter;
}

std::string AQLline::query2AQL()
{
    std::string filter;

    switch(type) {
    case Top:
        for(size_t ii=0; ii< children.size(); ii++) {
            filter += children[ii]->query2AQL();
        }
        break;
    case LogicalTwo: {
        filter += "( ";
        for(size_t ii=0; ii< children.size(); ii++) {
            if(ii > 0) {
                filter += " "+ keyname +" ";
            }
            filter += children[ii]->query2AQL();
        }
        filter += " )";
    }
    break;
    case LogicalNot:
        if(!children.empty()) {
            filter += " "+ keyname + "  ";
            filter += children[0]->query2AQL();
            filter += " ";
        }
        break;
    case Operator:
        if(!children.empty() && children[0]->type == Field) {
            filter += " ";
            filter += children[0]->value2AQL(keyname);
            filter += " ";
        }
        break;
    case Field: {
        filter += value2AQL("==");
    }
    break;
    }
    return filter;
}

std::string AQLline::toAQL(const std::string& collection)
{
    std::string aql_query("FOR u IN " );
    aql_query += collection;
    aql_query += "\nFILTER ";
    aql_query += query2AQL();
    aql_query += "\nRETURN u ";
    return aql_query;
}

void AQLline::toDom(jsonio::JsonBase& object) const
{
    object.set_value_via_path<int>("type", type);
    object.set_value_via_path("keyname", keyname);
    object.set_value_via_path("value", value);
    if(type == AQLline::Field) {
        object.set_value_via_path<int>("thrifttype", field_data.field_type);
        object.set_value_via_path("enumname", field_data.class_name);
    }
    auto& arr = object.add_array_via_path("children");
    for(size_t ii=0; ii< children.size(); ii++) {
        auto& obj = arr.add_object_via_path(std::to_string(ii) );
        children[ii]->toDom(obj);
    }
}

void AQLline::fromDom(const jsonio::JsonBase& object)
{
    children.clear();
    int atype = Operator;
    object.get_value_via_path<int>("type", atype, atype);
    object.get_value_via_path<std::string>("keyname", keyname, "");
    object.get_value_via_path<std::string>("value", value, "");
    type = static_cast<AQLline::fieldType>(atype);
    if(type == AQLline::Field) {
        object.get_value_via_path<int>("thrifttype", atype, 11);
        field_data.field_type = static_cast<jsonio::FieldDef::FieldType>(atype);
        object.get_value_via_path<std::string>("enumname", field_data.class_name, "");
    }
    auto arr = object.field("children");
    if(arr == nullptr) {
        return;
    }
    for(size_t ii=0; ii<arr->size(); ii++) {
        auto newline = new AQLline(children.size(), "undef", AQLline::Field, this);
        newline->fromDom( arr->child(ii) );
    }
}

bool AQLline::remove_child(AQLline* child)
{
    int thisndx = -1;
    for(std::size_t ii=0; ii< children.size(); ii++) {
        if(children[ii].get() == child) {
            thisndx = static_cast<int>(ii);
        }
        if(thisndx >= 0) {
            children[ii]->ndx_in_parent--;
        }
    }
    if(thisndx >= 0) {
        children.erase(children.begin() + thisndx);
        return true;
    }
    return false;
}

} // namespace jsonqml

