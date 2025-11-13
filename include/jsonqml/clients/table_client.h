#ifndef TABLECLIENT_H
#define TABLECLIENT_H

#include <memory>
#include <QObject>
#include <QAbstractItemModel>
#include <QItemSelectionModel>

namespace jsonqml {

class TableClientPrivate;

class TableClient : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QAbstractItemModel* csvmodel READ csvmodel NOTIFY csvModelChanged)
    Q_PROPERTY(bool sortingEnabled READ sortingEnabled NOTIFY tablePropertiesChanged)
    Q_PROPERTY(QString csvfile READ csvfile WRITE setCsvFile NOTIFY csvFileChanged)

    Q_DISABLE_COPY(TableClient)

signals:
    void csvModelChanged();
    void tablePropertiesChanged();
    void csvFileChanged();

public slots:
    void copySelected(const QModelIndexList& selection);
    void copyWithNames(const QModelIndexList& selection);
    void pasteSelected(const QModelIndexList& selection);

public:
    explicit TableClient(QObject *parent = nullptr);
    ~TableClient();

    QAbstractItemModel *csvmodel();
    bool sortingEnabled();
    const QString &csvfile() {
        return csv_file;
    }
    void setCsvFile(const QString& file);

    Q_INVOKABLE void readCSV(const QString& path);
    Q_INVOKABLE void saveCSV(const QString& path);
    Q_INVOKABLE QItemSelection selectAll();

protected:
    QString csv_file{"File"};

    std::unique_ptr<TableClientPrivate> impl_ptr;
    TableClientPrivate* impl_func()
    { return reinterpret_cast<TableClientPrivate *>(impl_ptr.get()); }
    const TableClientPrivate* impl_func() const
    { return reinterpret_cast<const TableClientPrivate *>(impl_ptr.get()); }

    TableClient(TableClientPrivate* impl, QObject *parent);
};

}

#endif // TABLECLIENT_H
