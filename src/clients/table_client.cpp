#include <QFile>
#include <QFileInfo>
#include "jsonqml/clients/settings_client.h"
#include "table_client_p.h"

namespace jsonqml {

bool TableClientPrivate::sorting_enabled() const
{
    return keys_table_mode&RowSortingEnabled;
}

void TableClientPrivate::init()
{
    //qDebug() << "TableClientPrivate::init() rows " << current_model->rowCount();
    if(sorting_enabled()) {
        sort_proxy_model.reset(new SortFilterProxyModel());
        sort_proxy_model->setSourceModel(current_model);
        qDebug() << "TableClientPrivate::init() rows " << sort_proxy_model->rowCount();
    }
}

QAbstractItemModel *TableClientPrivate::table_model() const
{
    if(sorting_enabled()) {
        return sort_proxy_model.get();
    }
    return current_model;
}

QModelIndex TableClientPrivate::model_index_row(int row) const
{
    if(sorting_enabled()) {
        return sort_proxy_model->mapFromSource(sort_proxy_model->sourceModel()->index(row,0)) ;
    }
    return current_model->index(row,0);
}

int TableClientPrivate::model_row_index(const QModelIndex &index) const
{
    if(sorting_enabled()) {
        return sort_proxy_model->mapToSource(index).row();
    }
    return index.row();
}

std::set<std::size_t> TableClientPrivate::rows_selected(const QItemSelection& itemselection) const
{
    std::set<std::size_t> rows;
    QItemSelection selitems = itemselection;
    if(sorting_enabled()) {
        selitems = sort_proxy_model->mapSelectionToSource(selitems);
    }
    QModelIndexList selection = selitems.indexes();

    // Multiple rows can be selected
    for(int i=0; i< selection.count(); ++i) {
        QModelIndex index = selection.at(i);
        if( index.column() == 0 ) {
            rows.insert( index.row() );
        }
    }
    return rows;
}

QItemSelection TableClientPrivate::select_rows(const std::set<std::size_t> &rows) const
{
    QItemSelection selitems;
    int start_row=-1, end_row=-1;
    // make groups for save time
    for(int row: rows ) {
        if(row >= current_model->rowCount()) {
            break;
        }
        if(start_row == -1) {
            start_row = end_row = row;
        }
        else if(end_row+1 == row) {
            end_row++;
        }
        else {
            selitems.merge(QItemSelection(current_model->index(start_row, 0), current_model->index(end_row, 0)),
                           QItemSelectionModel::Select);
            start_row = end_row = row;
        }
    }
    if(start_row != -1) {
        selitems.merge(QItemSelection(current_model->index(start_row, 0), current_model->index(end_row, 0)),
                       QItemSelectionModel::Select);
    }

    if(sorting_enabled()) {
        selitems = sort_proxy_model->mapSelectionFromSource(selitems);
    }
    return selitems;
}

Selection TableClientPrivate::get_selection_range(const QModelIndexList& selection, bool to_paste)
{
    Selection sel_box;
    foreach(QModelIndex ndx,  selection) {
        if(sel_box.fromCol==-1) {
            sel_box.fromCol = ndx.column();
        }
        if(sel_box.fromRow==-1) {
            sel_box.fromRow = ndx.row();
        }
        if(sel_box.fromCol > ndx.column()) sel_box.fromCol = ndx.column();
        if(sel_box.toCol < ndx.column()) sel_box.toCol = ndx.column();
        if(sel_box.fromRow > ndx.row()) sel_box.fromRow = ndx.row();
        if(sel_box.toRow < ndx.row()) sel_box.toRow = ndx.row();
    }

    // only one selected => all for end of table
    if(to_paste && sel_box.fromCol==sel_box.toCol && sel_box.fromRow==sel_box.toRow) {
        sel_box.toCol = table_model()->columnCount()-1;
        sel_box.toRow = table_model()->rowCount()-1;
    }
    return sel_box;
}

QString TableClientPrivate::create_header(const Selection& sel_box)
{
    QString text, clip_text;
    for(int col=sel_box.fromCol; col<=sel_box.toCol; col++) {
        if(col > sel_box.fromCol) {
            clip_text += splitCol;
        }
        text = table_model()->headerData(col, Qt::Horizontal, Qt::DisplayRole).toString();
        clip_text += (text.isEmpty() ? " " : text);;
    }
    return clip_text;
}

QString TableClientPrivate::create_string(const Selection& sel_box)
{
    QString text, clip_text;
    for(int row=sel_box.fromRow; row<=sel_box.toRow; row++) {
        for(int col=sel_box.fromCol; col<=sel_box.toCol; col++) {
            if(col > sel_box.fromCol) {
                clip_text += splitCol;
            }
            text = table_model()->index(row,col).data(Qt::DisplayRole).toString();
            clip_text += (text.isEmpty() ? " " : text);
        }
        clip_text += splitRow;
    }
    return clip_text;
}

void TableClientPrivate::set_from_string(const QString& str, const Selection& sel)
{
    if(str.isEmpty()) {
        return;
    }
    const QStringList rows = str.split(splitRow, Qt::KeepEmptyParts);
    for(int it=0, row=sel.fromRow; it<rows.count()&&row<=sel.toRow; it++, row++) {
        const QStringList cells = rows[it].split(splitCol, Qt::KeepEmptyParts);
        for(int cell=0, column=sel.fromCol; cell<cells.count()&&column<=sel.toCol; cell++,column++)  {
            table_model()->setData(table_model()->index(row,column), cells[cell].trimmed(), Qt::EditRole);
        }
    }
}

void TableClientPrivate::set_from_string_transposed(const QString& str, const Selection& sel)
{
    if(str.isEmpty()) {
        return;
    }
    const QStringList rows = str.split(splitRow, Qt::KeepEmptyParts);
    for(int it=0, row=sel.fromCol; it<rows.count()&&row<=sel.toCol; it++, row++) {
        const QStringList cells = rows[it].split(splitCol, Qt::KeepEmptyParts);
        for(int cell=0, column=sel.fromRow; cell<cells.count()&&column<=sel.toRow; cell++,column++)  {
            table_model()->setData(table_model()->index(row,column), cells[cell].trimmed(), Qt::EditRole);
        }
    }
}

void TableClientPrivate::copy_selected(const QModelIndexList &selection)
{
    Selection sel = get_selection_range(selection, false);
    QString clip_text = create_string(sel);
    uiSettings().copy(clip_text);
}

void TableClientPrivate::copy_with_names(const QModelIndexList &selection)
{
    Selection sel = get_selection_range(selection, false);
    QString clip_text = create_header(sel);
    clip_text += splitRow;
    clip_text += create_string(sel);
    uiSettings().copy(clip_text);
}

void TableClientPrivate::paste_selected(const QModelIndexList &selection, bool transposed)
{
    Selection sel = get_selection_range(selection, true);
    auto clip_text = uiSettings().paste();
    if(transposed) {
        set_from_string_transposed(clip_text, sel);
    }
    else {
        set_from_string(clip_text, sel);
    }
}

//--------------------------------------------------------------------------

TableClient::TableClient(TableClientPrivate* impl, QObject *parent):
    QObject(parent),
    impl_ptr(impl)
{
}

TableClient::TableClient(SelectModel* table_model, int mode, QObject *parent):
    TableClient(new TableClientPrivate(table_model, mode), parent)
{
}

TableClient::~TableClient() {}

QAbstractItemModel *TableClient::tableModel()
{
    return impl_func()->table_model();
}

bool TableClient::sortingEnabled()
{
    return impl_func()->sorting_enabled();
}

QItemSelection TableClient::selectAll()
{
    auto dmodel = tableModel();
    QItemSelection all_sel;
    if(dmodel->rowCount()>0 && dmodel->columnCount()>0) {
        all_sel.select(dmodel->index(0,0),
                       dmodel->index(dmodel->rowCount()-1,dmodel->columnCount()-1));
    }
    return all_sel;
}

QModelIndex TableClient::indexRow(int row) const
{
    return impl_func()->model_index_row(row);
}

int TableClient::rowIndex(const QModelIndex &index) const
{
    return impl_func()->model_row_index(index);
}

std::set<std::size_t> TableClient::rowsSelection(const QItemSelection &selection)
{
    return impl_func()->rows_selected(selection);
}

QItemSelection TableClient::selectionRows(const std::set<std::size_t> &rows)
{
    return impl_func()->select_rows(rows);
}

void TableClient::copySelected(const QModelIndexList& selection)
{
    if(!selection.isEmpty()) {
        impl_func()->copy_selected(selection);
    }
}

void TableClient::copyWithNames(const QModelIndexList& selection)
{
    if(!selection.isEmpty()) {
        impl_func()->copy_with_names(selection);
    }
}

void TableClient::pasteSelected(const QModelIndexList& selection)
{
    if(!selection.isEmpty()) {
        impl_func()->paste_selected(selection);
    }
}

void TableClient::pasteSelectedTransposed(const QModelIndexList& selection)
{
    if(!selection.isEmpty()) {
        impl_func()->paste_selected(selection, true);
    }
}

}
