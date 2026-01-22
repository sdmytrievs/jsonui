#ifndef VERTEXCLIENT_H
#define VERTEXCLIENT_H

#include <QAbstractItemModel>
#include "jsonqml/clients/json_client.h"
#include "jsonqml/models/db_keys_model.h"

namespace jsonqml {

class VertexClientPrivate;
class DBKeysModel;

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
    void executingChange(bool exec);
    void updatedKeyList(std::string doc_id);
    void editorChanged();

public slots:
    void setModelSchema() override;
    void setEditorData(std::string schema_name, std::string doc_json);
    void setEditorOid(std::string doc_id);
    void updateKeyList();

public:
    explicit VertexClient(DocumentType dtype, const QString& aschema, QObject *parent = nullptr);
    explicit VertexClient(const QString& aschema, const QString& doc_id, QObject *parent = nullptr);
    explicit VertexClient(DocumentType dtype, const QString& aschema,
                          const jsonio::DBQueryBase& query,
                          const model_line_t& query_fields,
                          QObject *parent = nullptr);


    QAbstractItemModel *keysmodel();
    DBKeysModel *tablemodel();
    bool sortingEnabled();
    Q_INVOKABLE bool queryExecuting();

    Q_INVOKABLE virtual void readEditorData(int row);
    Q_INVOKABLE virtual void readEditorId(QString vertex_id);
    Q_INVOKABLE QString editorId();
    std::string tableId(size_t row) const;

    Q_INVOKABLE virtual void deleteRecord(QString vertex_id);
    Q_INVOKABLE virtual void updateRecord();

    virtual void backupRecords(QString file, std::vector<std::string>&& keys);
    virtual void restoreRecords(QString file);
    virtual void backupGraph(QString file, std::vector<std::string>&& keys);
    virtual void restoreGraph(QString file);
    virtual void deleteRecords(std::vector<std::string>&& keys);

protected:
    std::string start_doc_id;

    VertexClientPrivate* impl_func()
    { return reinterpret_cast<VertexClientPrivate *>(impl_ptr.get()); }
    const VertexClientPrivate* impl_func() const
    { return reinterpret_cast<const VertexClientPrivate *>(impl_ptr.get()); }

    VertexClient(const QString& aschema, VertexClientPrivate *impl, QObject *parent);
};

}

#endif // VERTEXCLIENT_H
