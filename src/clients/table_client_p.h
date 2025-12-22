#ifndef TABLECLIENT_P_H
#define TABLECLIENT_P_H

#include "jsonqml/clients/table_client.h"

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
    explicit TableClientPrivate(SelectModel* table_model, int mode):
        keys_table_mode(mode),
        current_model(table_model)
    {
        init();
    }
    virtual ~TableClientPrivate() {}

    bool sorting_enabled() const;
    QAbstractItemModel* table_model() const;

    QModelIndex model_index_row(int row) const;
    int model_row_index(const QModelIndex& row) const;
    std::set<std::size_t> rows_selected(const QItemSelection& selection) const;
    QItemSelection select_rows(const std::set<std::size_t>& rows) const;

    void copy_selected(const QModelIndexList& selection);
    void copy_with_names(const QModelIndexList& selection);
    void paste_selected(const QModelIndexList& selection, bool transposed=false);

    virtual void read_files(const QString &) {}
    virtual void save_files(const QString &) {}

protected:
    int keys_table_mode = RowSortingEnabled;
    SelectModel *current_model = nullptr;
    //QSharedPointer<CSVModel> csv_model_data;
    QSharedPointer<SortFilterProxyModel> sort_proxy_model;

    friend class TableClient;

    void init();
    Selection get_selection_range(const QModelIndexList& selection, bool to_paste);
    QString create_header(const Selection& sel_box);
    QString create_string(const Selection& sel_box);
    void set_from_string(const QString& str, const Selection& sel);
    void set_from_string_transposed(const QString &str, const Selection &sel);
};

}

#endif // TABLECLIENT_P_H
