#ifndef VERTEXCLIENT_H
#define VERTEXCLIENT_H

#include <QAbstractItemModel>
#include "jsonqml/clients/json_client.h"

namespace jsonqml {

class VertexClientPrivate;

class VertexClient : public JsonClient
{
    Q_OBJECT

    Q_PROPERTY(QAbstractItemModel* keysmodel READ keysmodel NOTIFY keysModelChanged)
    Q_PROPERTY(bool sortingEnabled READ sortingEnabled NOTIFY tablePropertiesChanged)
    Q_PROPERTY(bool queryExecuting READ queryExecuting NOTIFY executingChange)

    Q_DISABLE_COPY(VertexClient)

signals:
    void keysModelChanged();
    void tablePropertiesChanged();
    void executingChange();

public slots:
    void setModelSchema() override;

protected slots:
    void setEditorData(std::string schema_name, std::string doc_json);
    void setEditorOid(std::string doc_id);

public:
    explicit VertexClient(QObject *parent = nullptr);

    QAbstractItemModel *keysmodel();
    bool sortingEnabled();
    Q_INVOKABLE bool queryExecuting();

    Q_INVOKABLE virtual void readEditorData(int row);
    Q_INVOKABLE virtual void readEditorId(QString vertex_id);
    Q_INVOKABLE QString editorId();

protected:
    VertexClientPrivate* impl_func()
    { return reinterpret_cast<VertexClientPrivate *>(impl_ptr.get()); }
    const VertexClientPrivate* impl_func() const
    { return reinterpret_cast<const VertexClientPrivate *>(impl_ptr.get()); }

    VertexClient(VertexClientPrivate *impl, QObject *parent);
};

}

#endif // VERTEXCLIENT_H
