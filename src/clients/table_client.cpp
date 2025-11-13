#include <QFile>
#include <QFileInfo>
#include "jsonqml/clients/settings_client.h"
#include "table_client_p.h"

namespace jsonqml {


QAbstractItemModel *TableClientPrivate::csv_model() const
{
    if(sorting_enabled()) {
        return sort_proxy_model.get();
    }
    return csv_model_data.get();
}

bool TableClientPrivate::sorting_enabled() const
{
    return keys_table_mode&RowSortingEnabled;
}

void TableClientPrivate::init()
{
    csv_model_data.reset(new CSVModel());
    if(sorting_enabled()) {
        sort_proxy_model.reset(new SortFilterProxyModel());
        sort_proxy_model->setSourceModel(csv_model_data.get());
    }
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
        sel_box.toCol = csv_model()->columnCount()-1;
        sel_box.toRow = csv_model()->rowCount()-1;
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
        text = csv_model()->headerData(col, Qt::Horizontal, Qt::DisplayRole).toString();
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
            text = csv_model()->index(row,col).data(Qt::DisplayRole).toString();
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
            csv_model()->setData(csv_model()->index(row,column), cells[cell].trimmed(), Qt::EditRole);
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

void TableClientPrivate::paste_selected(const QModelIndexList &selection)
{
    Selection sel = get_selection_range(selection, true);
    auto clip_text = uiSettings().paste();
    set_from_string(clip_text, sel);
}

void TableClientPrivate::read_CSV(const QString &path)
{
    QFile file(path);
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        uiSettings().setError("could not open file");
        return;
    }
    QByteArray ba = file.readAll();
    std::string csv_srtring =ba.toStdString();
    csv_model_data->setCsvString(std::move(csv_srtring));
    file.close();
}

void TableClientPrivate::save_CSV(const QString &path)
{
    auto csv_string = csv_model_data->getCsvString();
    QFile file(path);
    if(!file.open(QIODevice::WriteOnly)) {
        uiSettings().setError("could not open file");
        return;
    }
    QTextStream stream(&file);
    stream << csv_string.c_str();
    file.close();
}

//--------------------------------------------------------------------------

TableClient::TableClient(TableClientPrivate* impl, QObject *parent):
    QObject(parent),
    impl_ptr(impl)
{
}

TableClient::TableClient(QObject *parent):
    TableClient(new TableClientPrivate, parent)
{
}

TableClient::~TableClient() {}

QAbstractItemModel *TableClient::csvmodel()
{
    return impl_func()->csv_model();
}

bool TableClient::sortingEnabled()
{
    return impl_func()->sorting_enabled();
}

void TableClient::setCsvFile(const QString& path)
{
    QFileInfo file(path);
    csv_file = file.fileName();
    emit csvFileChanged();
}

void TableClient::readCSV(const QString &url)
{
    if(url.isEmpty()) {
        return;
    }
    uiSettings().setError(QString());
    try {
        // save work path
        auto path = uiSettings().handleFileChosen(url);
        impl_func()->read_files(path);
        setCsvFile(path);
    }
    catch(std::exception& e) {
        uiSettings().setError(e.what());
    }
}

void TableClient::saveCSV(const QString &url)
{
    if(url.isEmpty()) {
        return;
    }
    uiSettings().setError(QString());
    try {
        auto path = uiSettings().handleFileChosen(url);
        impl_func()->save_files(path);
        setCsvFile(path);
    }
    catch(std::exception& e) {
        uiSettings().setError(e.what());
    }
}

QItemSelection TableClient::selectAll()
{
    auto dmodel=csvmodel();
    QItemSelection all_sel;
    if(dmodel->rowCount()>0 && dmodel->columnCount()>0) {
        all_sel.select(dmodel->index(0,0),
                       dmodel->index(dmodel->rowCount()-1,dmodel->columnCount()-1));
    }
    return all_sel;
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

}
