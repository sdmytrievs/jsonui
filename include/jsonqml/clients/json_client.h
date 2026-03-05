#ifndef JSONCLIENT_H
#define JSONCLIENT_H

#include <QObject>
#include "jsonqml/models/json_model.h"

namespace jsonqml {

class JsonClientPrivate;

class JsonClient : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString schema READ schema WRITE setSchema NOTIFY schemaChanged)
    Q_PROPERTY(QStringList schemasList READ schemasList WRITE setSchemaList NOTIFY listChanged)

    Q_PROPERTY(JsonBaseModel* jsonmodel READ jsonmodel NOTIFY jsonModelChanged)
    Q_PROPERTY(QStringList headerNames READ headerNames CONSTANT)
    Q_PROPERTY(bool expandedFields READ expandedFields CONSTANT)

    Q_DISABLE_COPY(JsonClient)

signals:
    void listChanged();
    void schemaChanged();
    void jsonModelChanged();
    void jsonModelAboutChanged();
    void editorChanged();

public slots:
    void setSchema(const QString &new_schema);
    virtual void setModelSchema();
    void updateSchemaList();
    void setEditorData(std::string schema_name, std::string doc_json);

public:
    /// Extern flag to keep data fields expanded
    static bool expanded_fields;
    static bool expandedFields();

    explicit JsonClient(QObject *parent = nullptr);
    ~JsonClient();

    const QString &schema();
    const QStringList &schemasList();
    JsonBaseModel *jsonmodel();
    const QStringList &headerNames();

    void setSchemaList(const QStringList &new_list);

    template <typename T>
    bool setValue(const std::string& jsonpath, const T& value) {
        if(jsonmodel()) {
            return jsonmodel()->setValue(jsonpath, value);
        }
        return false;
    }

    template <typename T>
    bool getValue(const std::string& jsonpath, T& value, const T& def_value)
    {
        if(jsonmodel()) {
            return jsonmodel()->current_object().get_value_via_path(jsonpath, value, def_value);
        }
        return false;
    }

    Q_INVOKABLE void readJson(const QString& path);
    Q_INVOKABLE void saveJson(const QString& path);

    Q_INVOKABLE static QString fileSchemaExt(const QString& schema_name, const QString& ext);
    Q_INVOKABLE static QString schemaFromPath(const QString& file_path);

protected:
    std::unique_ptr<JsonClientPrivate> impl_ptr;
    JsonClientPrivate* impl_func()
    { return reinterpret_cast<JsonClientPrivate *>(impl_ptr.get()); }
    const JsonClientPrivate* impl_func() const
    { return reinterpret_cast<const JsonClientPrivate *>(impl_ptr.get()); }

    JsonClient(JsonClientPrivate* impl, QObject *parent);
};

}

#endif // JSONCLIENT_H
