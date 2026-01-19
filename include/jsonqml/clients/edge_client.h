#ifndef EDGECLIENT_H
#define EDGECLIENT_H

#include "jsonqml/clients/vertex_client.h"


namespace jsonqml {

class EdgeClientPrivate;

class EdgeClient : public VertexClient
{
    Q_OBJECT

    Q_PROPERTY(QAbstractItemModel* inmodel READ inmodel NOTIFY jsonModelChanged)
    Q_PROPERTY(QAbstractItemModel* outmodel READ outmodel NOTIFY jsonModelChanged)

    Q_DISABLE_COPY(EdgeClient)

signals:
    void updatedInList(std::string doc_id);
    void updatedOutList(std::string doc_id);

public slots:
    void updateInList();
    void updateOutList();

public:
    explicit EdgeClient(const QString& aschema, QObject *parent = nullptr);
    explicit EdgeClient(const QString& aschema,
                        const jsonio::DBQueryBase& query,
                        const model_line_t& query_fields,
                        QObject *parent = nullptr);
    explicit EdgeClient(const QString& aschema,
                        bool in_edges,
                        const QString& vertex_id,
                        QObject *parent = nullptr);

    QAbstractItemModel* inmodel();
    QAbstractItemModel* outmodel();

    DBQueryModel *intablemodel();
    DBQueryModel *outtablemodel();

    Q_INVOKABLE void setIncomingEdges(QString vertex_id);
    Q_INVOKABLE void setOutgoingEdges(QString vertex_id);

    Q_INVOKABLE QString incomingVertex() const;
    Q_INVOKABLE QString outgoingVertex() const;

    QString vertex_schema_from_id(const QString &key_id);
protected:
    EdgeClientPrivate* impl_func()
    { return reinterpret_cast<EdgeClientPrivate *>(impl_ptr.get()); }
    const EdgeClientPrivate* impl_func() const
    { return reinterpret_cast<const EdgeClientPrivate *>(impl_ptr.get()); }

    EdgeClient(const QString& aschema, EdgeClientPrivate *impl, QObject *parent);
};

}

#endif // EDGECLIENT_H
