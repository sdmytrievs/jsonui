#ifndef CSVCLIENT_H
#define CSVCLIENT_H

#include <memory>
#include <QObject>
#include <QAbstractItemModel>
#include <QItemSelectionModel>
#include "jsonqml/clients/table_client.h"

namespace jsonqml {

class CSVClientPrivate;

class CSVClient : public TableClient
{
    Q_OBJECT

    Q_PROPERTY(QString csvfile READ csvfile WRITE setCsvFile NOTIFY csvFileChanged)

    Q_DISABLE_COPY(CSVClient)

signals:
    void csvFileChanged();

public:
    explicit CSVClient(int mode=default_table_settings, QObject *parent = nullptr);
    ~CSVClient();

    const QString &csvfile() {
        return csv_file;
    }
    void setCsvFile(const QString& file);

    Q_INVOKABLE void readCSV(const QString& path);
    Q_INVOKABLE void saveCSV(const QString& path);

protected:
    QString csv_file{"File"};

    CSVClientPrivate* impl_func()
    { return reinterpret_cast<CSVClientPrivate *>(impl_ptr.get()); }
    const CSVClientPrivate* impl_func() const
    { return reinterpret_cast<const CSVClientPrivate *>(impl_ptr.get()); }

    CSVClient(CSVClientPrivate *impl, QObject *parent);
};

}

#endif // CSVCLIENT_H
