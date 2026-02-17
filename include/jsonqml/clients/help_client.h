#ifndef HELPCLIENT_H
#define HELPCLIENT_H

#include "jsonqml/clients/vertex_client.h"

namespace jsonqml {

/// Declarations of DocPagesData class description of  Help record
/// ( Markdown or image file )
struct DocPagesData {

    /// Types of Help files
    enum DocFileTypes {
        /// Markdown text file
        Markdown,
        /// Image binary file
        Image
    };

    /// Name of file (used as key field)
    std::string name;
    /// Extension of file (used as key field)
    std::string ext;
    /// Type of content (used as key field)
    int type;
    /// markdown content
    QString markdown;
    /// image content
    QByteArray image;

    /// Last modified
    QDateTime last_modif;

    int docType(const std::string& ext) const;
    void setRecordKey(const std::string& akey);
    void setMarkdownRecord(const std::string& akey, const QString& mdtext);

    QString fileName() const;

    std::string to_json_node(const std::string& doc_id);
    void from_json_node(const jsonio::JsonBase &object);

    void setFileInfo(const QString& file, bool test_modif = true);
    void readFile(int atype, const QString& file);
    void saveFile(const QString& help_dir);
    void deleteFile(const QString& help_dir);
};

class HelpClient : public VertexClient
{
    Q_OBJECT

    Q_DISABLE_COPY(HelpClient)

    Q_PROPERTY(bool canEdit READ canEdit CONSTANT)
    Q_PROPERTY(QString currentId READ currentId WRITE setCurrentId NOTIFY currentIdChanged)

signals:
    void currentIdChanged(QString id);

public:
    /// Extern flag to edit docpages
    static bool edit_help;
    /// Extern help schema name (default  "DocPages")
    static QString help_schema_name;

    explicit HelpClient(QObject *parent = nullptr);

    bool canEdit() const;
    QString currentId() const;
    void setCurrentId(QString doc_key);

    Q_INVOKABLE std::string recordKey(const std::string& name, const std::string& ext) const;
    Q_INVOKABLE QString urlfromOid(const std::string &doc_id, int& type);
    Q_INVOKABLE QString linktoOid(const std::string &doc_id);

    Q_INVOKABLE void addFileDocument(int type, const QString &name, const QString &path);
    Q_INVOKABLE void saveMarkdownDocument(const QString& akey, const QString& mdtext);

    Q_INVOKABLE QVariant readResource(int type, const QUrl& url);

protected:
    std::string help_collection_name;
    QString last_readed_md_id;
};

}

#endif // HELPCLIENT_H
