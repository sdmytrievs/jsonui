#ifndef TABLECLIENT_P_H
#define TABLECLIENT_P_H

#include "jsonqml/clients/table_client.h"
#include "jsonqml/models/csv_model.h"

namespace jsonqml {

#ifdef __APPLE__
const char  splitRow = '\r';
const char  splitCol = '\t';
#else
const char  splitRow = '\n';
const char  splitCol = '\t';
#endif

/// Internal data for selection
struct Selection
{
    int fromCol;
    int toCol;
    int fromRow;
    int toRow;

    Selection():
        fromCol(-1), toCol(-1), fromRow(-1), toRow(-1)
    {}
    Selection(int n1, int n2, int m1, int m2 ):
        fromCol(n1), toCol(n2), fromRow(m1), toRow(m2)
    {}
};


class TableClientPrivate
{
    Q_DISABLE_COPY_MOVE(TableClientPrivate)

public:
    explicit TableClientPrivate()
    {
        init();
    }
    virtual ~TableClientPrivate() {}

    bool sorting_enabled() const;
    QAbstractItemModel* csv_model() const;

    void copy_selected(const QModelIndexList& selection);
    void copy_with_names(const QModelIndexList& selection);
    void paste_selected(const QModelIndexList& selection);

    virtual void read_files(const QString &path)
    {
        read_CSV(path);
    }
    virtual void save_files(const QString &path)
    {
        save_CSV(path);
    }

    virtual bool is_number(int section)
    {
        return csv_model_data->is_number(section) ;
    }

protected:
    int keys_table_mode = RowSortingEnabled;
    QSharedPointer<CSVModel> csv_model_data;
    QSharedPointer<SortFilterProxyModel> sort_proxy_model;

    friend class TableClient;

    void init();
    Selection get_selection_range(const QModelIndexList& selection, bool to_paste);
    QString create_header(const Selection& sel_box);
    QString create_string(const Selection& sel_box);
    void set_from_string(const QString& str, const Selection& sel);

    void read_CSV(const QString &path);
    void save_CSV(const QString &path);
};

}

#endif // TABLECLIENT_P_H
