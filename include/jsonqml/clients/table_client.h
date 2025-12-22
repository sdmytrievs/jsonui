#ifndef TABLECLIENT_H
#define TABLECLIENT_H

#include <set>
#include <memory>
#include <QObject>
#include <QAbstractItemModel>
#include <QItemSelectionModel>
#include "jsonqml/models/select_model.h"

namespace jsonqml {

class TableClientPrivate;

const int default_table_settings = TableFlag::TableIsEditable|TableFlag::RowSortingEnabled;

class TableClient : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QAbstractItemModel* csvmodel READ tableModel NOTIFY tableModelChanged)
    Q_PROPERTY(bool sortingEnabled READ sortingEnabled NOTIFY tablePropertiesChanged)

    Q_DISABLE_COPY(TableClient)

signals:
    void tableModelChanged();
    void tablePropertiesChanged();

public slots:
    void copySelected(const QModelIndexList& selection);
    void copyWithNames(const QModelIndexList& selection);
    void pasteSelected(const QModelIndexList& selection);
    void pasteSelectedTransposed(const QModelIndexList &selection);

public:
    explicit TableClient(SelectModel* table_model, int mode=default_table_settings, QObject *parent = nullptr);
    ~TableClient();

    QAbstractItemModel *tableModel();
    bool sortingEnabled();
    Q_INVOKABLE QItemSelection selectAll();

    QModelIndex indexRow(int row) const;
    int rowIndex(const QModelIndex& index) const;
    std::set<std::size_t> rowsSelection(const QItemSelection& selection);
    QItemSelection selectionRows(const std::set<std::size_t>& rows);

protected:
    std::unique_ptr<TableClientPrivate> impl_ptr;
    TableClientPrivate* impl_func()
    { return reinterpret_cast<TableClientPrivate *>(impl_ptr.get()); }
    const TableClientPrivate* impl_func() const
    { return reinterpret_cast<const TableClientPrivate *>(impl_ptr.get()); }

    TableClient(TableClientPrivate* impl, QObject *parent);
};

}

#endif // TABLECLIENT_H
