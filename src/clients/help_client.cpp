#include <QFileInfo>
#include "jsonio/jsondump.h"
#include "jsonqml/clients/help_client.h"
#include "jsonqml/clients/settings_client.h"

namespace jsonqml {

bool HelpClient::edit_help = false;
QString HelpClient::help_schema_name = "DocPages";

HelpClient::HelpClient(QObject *parent):
    VertexClient(Resource, help_schema_name, parent)
{
    help_collection_name = ArangoDatabase::collectionFromSchema(help_schema_name.toStdString());

    connect(this, &HelpClient::editorChanged,
            this, &HelpClient::updateHelpData, Qt::UniqueConnection);

}

bool HelpClient::canEdit() const
{
    return edit_help;
}

const QString &HelpClient::markdown() const
{
    return doc_page.markdown;
}

void HelpClient::setMarkdown(const QString &md_doc)
{
    doc_page.markdown = md_doc;
    emit markdownChanged();
}

void HelpClient::updateHelpData()
{
    doc_page.fromJsonNode(jsonmodel()->current_object());
    emit markdownChanged();
    emit updatedMarkdown(markdown(), anchor_item);
}

std::string HelpClient::recordKey(const std::string &name, const std::string &ext) const
{
    return help_collection_name+"/"+name+";"+ext;
}

void HelpClient::showHelp(const QString &name, const QString &item)
{
    QString keywd =  QString::fromStdString(recordKey(name.toStdString(), "md"));
    ui_logger->info("show help  '{}' '{}' keywd {}", name.toStdString(), item.toStdString(), keywd.toStdString());
    anchor_item = item;
    readEditorId(keywd);
}

void HelpClient::showDocument(int ndx)
{
    ui_logger->info(" show help {}", tableId(ndx));
    anchor_item = "";
    readEditorData(ndx);
}

QVariant HelpClient::readResource(const QUrl &url)
{
    DocPagesData help_data;
    help_data.setFileInfo(url.toString());

    std::string doc_key = recordKey(help_data.name, help_data.ext);
    std::string doc_json;
    ui_logger->info("Read help resource {}  {}", url.toString().toStdString(), doc_key);
    if(tablemodel()) {
      doc_json =  tablemodel()->read_query_doc(doc_key);
    }
    if(!doc_json.empty()) {
        auto js_free = jsonio::json::loads(doc_json);
        help_data.fromJsonNode(js_free);
        if(help_data.type==0) {
            return help_data.markdown;
        }
        else {
            return QByteArray::fromBase64(help_data.image);
        }
    }
    return QVariant{};
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

// must be key: "docpages/<name>;<ext>"
// for all markdown must be md
//(*.markdown *.md *.mdown)
void DocPagesData::setRecordKey(const std::string& akey)
{
    auto pos_ext = akey.find_last_of(';');
    auto name_start = akey.find_first_of('/');

    jsonio::JSONIO_THROW_IF(name_start == std::string::npos, "HelpView", 10, " illegal record key: " + akey);

    if(pos_ext != std::string::npos) {
        name = akey.substr(name_start+1, pos_ext-name_start-1);
        ext = akey.substr(pos_ext+1);
    }
    else  {
        name = akey.substr(name_start+1);
        ext = "";
    }

    if(ext == "md" || ext == "markdown" || ext == "mdown" || ext.empty()) {
        type = 0;
    }
    else {
        type = 1;    // image file
    }
    markdown = "";
    image = "";
    //lastModif =  QDateTime::currentDateTime();
}

void DocPagesData::setMarkdownRecord(const std::string &akey, const QString &mdtext)
{
    setRecordKey(akey);
    markdown = mdtext;
}

void DocPagesData::toJsonNode(jsonio::JsonBase& object) const
{
    object.clear();
    object.set_value_via_path("name", name);
    object.set_value_via_path("ext", ext);
    object.set_value_via_path("type", type);
    object.set_value_via_path("markdown", markdown.toStdString());
    object.set_value_via_path("image", image.toStdString());
}

void DocPagesData::fromJsonNode(const jsonio::JsonBase& object)
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
    if(file.contains("images/")) {
        type = Image;
    }
    else {
        type = Markdown;
    }
    markdown = "";
    image = "";
    if(test_modif) {
        last_modif =  finfo.lastModified();
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
