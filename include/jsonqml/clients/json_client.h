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
    Q_PROPERTY(QStringList headerNames READ headerNames)

    Q_DISABLE_COPY(JsonClient)

signals:
    void listChanged();
    void schemaChanged();
    void jsonModelChanged();
    void jsonModelAboutChanged();

public slots:
    virtual void setModelSchema();
    void updateSchemaList();

public:
    explicit JsonClient(QObject *parent = nullptr);
    ~JsonClient();

    const QString &schema();
    const QStringList &schemasList();
    JsonBaseModel *jsonmodel();
    const QStringList &headerNames();

    Q_INVOKABLE void setSchema(const QString &new_schema);
    void setSchemaList(const QStringList &new_list);

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
