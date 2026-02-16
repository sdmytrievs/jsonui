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

    QString fileName() const;

    void setRecordKey(const std::string& akey);
    void setMarkdownRecord(const std::string& akey, const QString& mdtext);

    void toJsonNode(jsonio::JsonBase& object) const;
    void fromJsonNode(const jsonio::JsonBase& object);

    void setFileInfo(const QString& file, bool test_modif = true);
    void readFile(int atype, const QString& fileName);
    void saveFile(const QString& help_dir);
    void deleteFile(const QString& help_dir);
};

class HelpClient : public VertexClient
{
    Q_OBJECT

    Q_DISABLE_COPY(HelpClient)

    Q_PROPERTY(bool canEdit READ canEdit CONSTANT)
    Q_PROPERTY(QString markdown READ markdown WRITE setMarkdown NOTIFY markdownChanged)

signals:
    void markdownChanged();
    void updatedMarkdown(const QString &makdown, const QString &anchor);

public slots:
    void setMarkdown(const QString &md_doc);
    void updateHelpData();

public:
    /// Extern flag to edit docpages
    static bool edit_help;
    /// Extern help schema name (default  "DocPages")
    static QString help_schema_name;

    explicit HelpClient(QObject *parent = nullptr);

    bool canEdit() const;
    const QString &markdown() const;

    Q_INVOKABLE std::string recordKey(const std::string& name, const std::string& ext) const;
    Q_INVOKABLE void showHelp(const QString &name, const QString &item);
    Q_INVOKABLE void showDocument(int ndx);

    Q_INVOKABLE QVariant readResource(const QUrl& url);

protected:
    std::string help_collection_name;
    QString anchor_item = "";
    DocPagesData doc_page;

};

}

#endif // HELPCLIENT_H
