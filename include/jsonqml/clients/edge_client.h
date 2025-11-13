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

public:
    explicit EdgeClient(QObject *parent = nullptr);

    QAbstractItemModel* inmodel();
    QAbstractItemModel* outmodel();

    Q_INVOKABLE void setIncomingEdges(QString vertex_id);
    Q_INVOKABLE void setOutgoingEdges(QString vertex_id);

    Q_INVOKABLE QString incomingVertex() const;
    Q_INVOKABLE QString outgoingVertex() const;

protected:
    EdgeClientPrivate* impl_func()
    { return reinterpret_cast<EdgeClientPrivate *>(impl_ptr.get()); }
    const EdgeClientPrivate* impl_func() const
    { return reinterpret_cast<const EdgeClientPrivate *>(impl_ptr.get()); }

    EdgeClient(EdgeClientPrivate *impl, QObject *parent);
};

}

#endif // EDGECLIENT_H
