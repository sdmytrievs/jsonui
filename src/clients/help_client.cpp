#include <QFileInfo>
#include "jsonio/jsondump.h"
#include "jsonqml/clients/help_client.h"
#include "jsonqml/clients/settings_client.h"

namespace jsonqml {

bool HelpClient::edit_help = true;
QString HelpClient::help_schema_name = "DocPages";

HelpClient::HelpClient(QObject *parent):
    VertexClient(Resource, help_schema_name, parent)
{
    help_collection_name = ArangoDatabase::collectionFromSchema(help_schema_name.toStdString());
}

bool HelpClient::canEdit() const
{
    return edit_help;
}

QString HelpClient::currentId() const
{
    return last_readed_md_id;
}

void HelpClient::setCurrentId(QString doc_key)
{
    last_readed_md_id = doc_key;
    emit currentIdChanged(last_readed_md_id);
}

std::string HelpClient::recordKey(const std::string &name, const std::string &ext) const
{
    return help_collection_name+"/"+name+";"+ext;
}

QString HelpClient::urlfromOid(const std::string &doc_id, int& type)
{
    DocPagesData data;
    data.setRecordKey(doc_id);
    type = data.type;
    return data.fileName();
}

QString HelpClient::linktoOid(const std::string& doc_id)
{
    QString link;
    DocPagesData data;
    data.setRecordKey(doc_id);
    if(data.type==0) {
        link = QString(" [%1](%1.%2) ").arg(data.name.c_str(),data.ext.c_str());
    }
    else {
        link = QString(" ![%1](images/%1.%2) ").arg(data.name.c_str(), data.ext.c_str());
    }
    return link;
}

QVariant HelpClient::readResource(int type, const QUrl &url)
{
    DocPagesData help_data;
    help_data.setFileInfo(url.toString());

    std::string doc_key = recordKey(help_data.name, help_data.ext);
    if(help_data.type==0 && type !=2) {
        setCurrentId(QString::fromStdString(doc_key));
    }
    std::string doc_json;
    ui_logger->info("Read help resource {}  {}", url.toString().toStdString(), doc_key);
    if(tablemodel()) {
      doc_json =  tablemodel()->read_query_doc(doc_key);
    }
    if(!doc_json.empty()) {
        auto js_free = jsonio::json::loads(doc_json);
        help_data.from_json_node(js_free);
        if(help_data.type==0) {
            return help_data.markdown;
        }
        else {
            return QByteArray::fromBase64(help_data.image);
        }
    }
    return QVariant{};
}

void HelpClient::addFileDocument(int type, const QString& name, const QString& path)
{
    DocPagesData data;
    if(!path.isEmpty()) {
        data.setFileInfo(path);
        data.readFile(type, path);
    }
    else  {
        data.ext = "md";
        data.type = DocPagesData::Markdown;
    }
    data.name = name.toStdString();

    auto json_str = data.to_json_node("");
    setEditorData(help_schema_name.toStdString(), json_str);
    updateRecord();
}

void HelpClient::saveMarkdownDocument(const QString &akey, const QString &mdtext)
{
    DocPagesData data;
    data.setMarkdownRecord(akey.toStdString(), mdtext);
    std::string json_str = data.to_json_node(akey.toStdString());
    setEditorData(help_schema_name.toStdString(), json_str);
    updateRecord();
}

//------------------------------------------------------------

QString DocPagesData::fileName() const
{
    QString fname;
    switch(type) {
    case Image : fname += "images/";
        break;
    case Markdown:
    default:     break;
    }
    fname += name+"."+ext;
    return fname;
}

// for all markdown must be md
//(*.markdown *.md *.mdown)
int DocPagesData::docType(const std::string &aext) const
{
    int atype;
    if(aext.find("md") != std::string::npos ||
        aext == "markdown" || aext == "mdown" || aext.empty()) {
        atype = 0;
    }
    else {
        atype = 1;    // image file
    }
    return atype;
}

// must be key: "docpages/<name>;<ext>"
void DocPagesData::setRecordKey(const std::string& akey)
{
    auto pos_ext = akey.find_last_of(';');
    auto name_start = akey.find_first_of('/');

    jsonio::JSONIO_THROW_IF(name_start == std::string::npos,
                            "HelpView", 10, " illegal record key: " + akey);

    if(pos_ext != std::string::npos) {
        name = akey.substr(name_start+1, pos_ext-name_start-1);
        ext = akey.substr(pos_ext+1);
    }
    else  {
        name = akey.substr(name_start+1);
        ext = "";
    }
    type = docType(ext);
    markdown = "";
    image = "";
    //lastModif =  QDateTime::currentDateTime();
}

void DocPagesData::setMarkdownRecord(const std::string &akey, const QString &mdtext)
{
    setRecordKey(akey);
    markdown = mdtext;
}

std::string DocPagesData::to_json_node(const std::string& doc_id)
{
    auto object = jsonio::JsonFree::object();
    object.set_value_via_path("name", name);
    object.set_value_via_path("ext", ext);
    object.set_value_via_path("type", type);
    object.set_value_via_path("markdown", markdown.toStdString());
    object.set_value_via_path("image", image.toStdString());
    if(!doc_id.empty()) {
        object.set_oid(doc_id);
    }
    return object.dump();
}

void DocPagesData::from_json_node(const jsonio::JsonBase& object)
{
    object.get_value_via_path<std::string>("name", name, "undefined");
    object.get_value_via_path<std::string>("ext", ext, "md");
    object.get_value_via_path<int>("type", type, Markdown);
    std::string buf;
    object.get_value_via_path<std::string>("markdown", buf, "");
    markdown = QString::fromStdString(buf);
    object.get_value_via_path<std::string>("image", buf, "");
    image = QByteArray::fromStdString(buf);
}

void DocPagesData::setFileInfo(const QString& file, bool test_modif)
{
    QFileInfo finfo(file);

    name = finfo.baseName().toStdString(); // baseName
    ext = finfo.suffix().toStdString();
    type = docType(ext);
    markdown = "";
    image = "";
    if(test_modif) {
        last_modif = finfo.lastModified();
    }
}

void DocPagesData::readFile(int atype, const QString& file)
{
    QFile infile(file);
    type = atype;

    switch(type) {
    case Image:     // read binary image
        if(infile.open( QFile::ReadOnly)) {
            QByteArray barr = infile.readAll();
            image = barr.toBase64();
        }
        break;
    case Markdown:  // read md text file
    default:
        if(infile.open(QFile::ReadOnly | QFile::Text)) {
            markdown = infile.readAll();
        }
        break;
    }
    ui_logger->debug("DocPages read file {}.{} {}", name, ext, image.size());
}

void DocPagesData::saveFile(const QString& help_dir)
{
    QString fname = help_dir+"/"+fileName();
    QFile file(fname);

    switch(type) {
    case Image:     // write binary image
        if(!image.isEmpty()) {
            file.open( QIODevice::WriteOnly );
            QByteArray imgbase64 = image;
            file.write(QByteArray::fromBase64(imgbase64));
        }
        break;
    case Markdown:  // read md text file
    default:
        if(!markdown.isEmpty()) {
            file.open(QIODevice::WriteOnly | QIODevice::Text);
            file.write(markdown.toUtf8());
        }
        break;
    }
}

void DocPagesData::deleteFile(const QString& help_dir)
{
    QString fname = help_dir+"/"+fileName();
    QFile file(fname);
    file.remove();
}

}
