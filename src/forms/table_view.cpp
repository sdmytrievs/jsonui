#include <QMenu>
#include <QHeaderView>
#include <QKeyEvent>
#include <QLineEdit>

#include "jsonqml/clients/table_client.h"
#include "jsonqml/clients/settings_client.h"
#include "jsonqml/forms/table_view.h"
#include "jsonqml/forms/CalcDialog.h"

namespace jsonqml {

//-------------------------------------------------------------------------------------
// TMatrixDelegate -  individuak items in views are rendered and edited using delegates
//-------------------------------------------------------------------------------------

// Editing QTreeView for objects in System page
QWidget *MatrixDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &, const QModelIndex &index) const
{
    auto data_type = index.model()->data(index).typeId();
    QLineEdit *lineEdit = new QLineEdit(parent);

    if(data_type == QMetaType::Double) {
        lineEdit->setValidator(new QDoubleValidator);
    }
    else if(data_type == QMetaType::Int) {
        lineEdit->setValidator(new QIntValidator);
    }
    return lineEdit;
}

//-------------------------------------------------------------------------------------
// class TMatrixTable implements a table view that displays items from a model.
//-------------------------------------------------------------------------------------

MatrixTable::MatrixTable(int mode, TableClient* atable_client, QWidget* parent):
    QTableView(parent), mode_using(mode),table_client(atable_client)
{
    verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    //setFocusPolicy(Qt::WheelFocus/*Qt::StrongFocus*/);
    //setTabKeyNavigation( false );

    setVerticalScrollMode(QAbstractItemView::ScrollPerItem);
    setHorizontalScrollMode(QAbstractItemView::ScrollPerItem);
    setEditTriggers(QAbstractItemView::DoubleClicked|QAbstractItemView::AnyKeyPressed);

    if(mode_using & TableFlag::SingleSelection) {
        setSelectionMode(QAbstractItemView::SingleSelection);
    }
    else {
        setSelectionMode(QAbstractItemView::MultiSelection);
    }

    setSortingEnabled(table_client->sortingEnabled());
    setModel(table_client->tableModel());
    setItemDelegate(new MatrixDelegate(this));

    setContextMenuPolicy(Qt::CustomContextMenu);
    if( !(mode_using & TableFlag::NoMenuEnabled) ) {
        connect(this, &MatrixTable::customContextMenuRequested, this, &MatrixTable::slotPopupContextMenu);
    }
}

void MatrixTable::makePopupContextMenu(QMenu *menu, QModelIndex index)
{
    QAction* act;
    auto data_type = index.model()->data(index).typeId();

    if(mode_using & TableFlag::TableIsEditable) {
        if(data_type == QMetaType::Double || data_type == QMetaType::Int) {
            act =  new QAction(tr("&Calculator"), this);
            act->setShortcut(tr("F8"));
            act->setStatusTip(tr("Use calculator  to the specified cells"));
            connect(act, &QAction::triggered, this, &MatrixTable::runCalc);
            menu->addAction(act);
            menu->addSeparator();
        }
    }

    act =  new QAction(tr("Select &row"), this);
    act->setShortcut(tr("Ctrl+R"));
    act->setStatusTip(tr("Select current row"));
    connect(act, &QAction::triggered, this, &MatrixTable::selRow);
    menu->addAction(act);

    act =  new QAction(tr("Select co&lumn"), this);
    act->setShortcut(tr("Ctrl+L"));
    act->setStatusTip(tr("Select current column"));
    connect(act, &QAction::triggered, this, &MatrixTable::selColumn);
    menu->addAction(act);

    act =  new QAction(tr("Select &all"), this);
    act->setShortcut(tr("Ctrl+A"));
    act->setStatusTip(tr("Select all cells"));
    connect(act, &QAction::triggered, this, &MatrixTable::selectAll);
    menu->addAction(act);

    menu->addSeparator();

    act =  new QAction(tr("&Copy"), this);
    act->setShortcut(tr("Ctrl+C"));
    act->setStatusTip(tr("Copy selected cells"));
    connect(act, &QAction::triggered, this, &MatrixTable::copyData);
    menu->addAction(act);

    act =  new QAction(tr("Copy wit&h names"), this);
    act->setShortcut(tr("Ctrl+H"));
    act->setStatusTip(tr("Copy selected header&cells"));
    connect(act, &QAction::triggered, this, &MatrixTable::copyDataHeader);
    menu->addAction(act);

    if(mode_using & TableFlag::TableIsEditable) {
        menu->addSeparator();

        act =  new QAction(tr("C&ut"), this);
        act->setShortcut(tr("Ctrl+U"));
        act->setStatusTip(tr("Cut selected cells"));
        connect(act, &QAction::triggered, this, &MatrixTable::cutData);
        menu->addAction(act);

        act =  new QAction(tr("&Paste"), this);
        act->setShortcut(tr("Ctrl+V"));
        act->setStatusTip(tr("Paste to selected area"));
        connect(act, &QAction::triggered, this, &MatrixTable::pasteData);
        menu->addAction(act);

        act =  new QAction(tr("Paste &transposed"), this);
        act->setShortcut(tr("Ctrl+T"));
        act->setStatusTip(tr("Paste transposed to selected area"));
        connect(act, &QAction::triggered, this, &MatrixTable::pasteTransposedData);
        menu->addAction(act);

        act =  new QAction(tr("&Clear"), this);
        act->setShortcut(tr("Ctrl+Del"));
        act->setStatusTip(tr("Clear selected cells"));
        connect(act, &QAction::triggered, this, &MatrixTable::clearData);
        menu->addAction(act);
    }
}

void MatrixTable::slotPopupContextMenu(const QPoint &pos)
{
    if(mode_using & TableFlag::NoMenuEnabled) {
        return;
    }

    QModelIndex index = indexAt( pos );
    std::shared_ptr<QMenu> menu(new QMenu(this));
    makePopupContextMenu(menu.get(), index);
    menu->exec(viewport()->mapToGlobal(pos));
}

void MatrixTable::keyPressEvent(QKeyEvent* e)
{
    if(mode_using & TableFlag::NoMenuEnabled) {
        QTableView::keyPressEvent(e);
        return;
    }

    if(e->modifiers() & Qt::ControlModifier) {
        switch (e->key()) {
        case Qt::Key_R:
            selRow();
            return;
        case Qt::Key_L:
            selColumn();
            return;
        case Qt::Key_A:
            selectAll();
            return;
        case Qt::Key_U:
            cutData();
            return;
        case Qt::Key_C:
            copyData();
            return;
        case Qt::Key_H:
            copyDataHeader();
            return;
        case Qt::Key_V:
            pasteData();
            return;
        case Qt::Key_T:
            pasteTransposedData();
            return;
        case Qt::Key_Delete:
            clearData();
            return;
        }
    }
    switch(e->key()) {
    case Qt::Key_F8:
        runCalc();
        return;
    }
    QTableView::keyPressEvent(e);
}


// Calculator on F8 pressed on data field
void MatrixTable::runCalc()
{
    if( !(mode_using & TableFlag::TableIsEditable) ) {
        return;
    }

    QString res;
    CalcDialog calc(window());
    if(calc.exec()) {
        foreach(QModelIndex ndx, selectedIndexes()) {
            res = calc.computeFunctionString(ndx.data(Qt::EditRole).toDouble());
            model()->setData(ndx, res, Qt::EditRole);
        }
    }
}

void MatrixTable::selRow()
{
    selectRow(currentIndex().row());
}

void MatrixTable::selColumn()
{
    //selectionModel()->select( currentIndex(), QItemSelectionModel::Columns|QItemSelectionModel::ClearAndSelect );
    selectColumn(currentIndex().column());
}

void MatrixTable::cutData()
{
    if(mode_using & TableFlag::TableIsEditable) {
        copyData();
        clearData();
    }
}

void MatrixTable::clearData()
{
    if(mode_using & TableFlag::TableIsEditable) {
        foreach(QModelIndex ndx, selectedIndexes()) {
            model()->setData(ndx, ""/*emptiness.c_str()*/, Qt::EditRole);
        }
    }
}

void MatrixTable::copyData()
{
    table_client->copySelected(selectedIndexes());
}

void MatrixTable::copyDataHeader()
{
    table_client->copyWithNames(selectedIndexes());
}

void MatrixTable::pasteData()
{
    table_client->pasteSelected(selectedIndexes());
}

void MatrixTable::pasteTransposedData()
{
    table_client->pasteSelectedTransposed(selectedIndexes());
}

void MatrixTable::setCurrentRow(int row)
{
    if(model()->rowCount() <= 0) {
        return;
    }
    row = std::min(row, model()->rowCount()-1);
    row = std::max(row, 0);
    setCurrentIndex(table_client->indexRow(row));
}

int MatrixTable::selectedRow() const
{
    if(model()->rowCount() <= 0 || !currentIndex().isValid()) {
        return -1;
    }
    return table_client->rowIndex(currentIndex());
}

void MatrixTable::selectRows(const std::set<std::size_t> &rows)
{
    QItemSelection selitems = table_client->selectionRows(rows);
    selectionModel()->select(selitems, QItemSelectionModel::Rows|QItemSelectionModel::ClearAndSelect);
}

std::set<std::size_t> MatrixTable::allSelectedRows() const
{
    return table_client->rowsSelection(selectionModel()->selection());
}

//---------------------------------------------------------------------------

void MatrixUniqueSelection::slot_selection_change(const QItemSelection& selected, const QItemSelection& /*deselected*/)
{
    bool equal;
    std::set<int> testedrows, rows;

    auto indexes = selected.indexes();
    foreach(QModelIndex ndx, indexes) {
        rows.insert(ndx.row());
    }

    QModelIndexList selection = selectionModel()->selectedRows();
    for(const auto& therow: rows) {
        for(int i=0; i< selection.count(); i++)  {
            QModelIndex index = selection.at(i);
            if(index.row() == therow || testedrows.find(index.row())!= testedrows.end()) {
                continue;
            }
            equal = true;
            for(auto colmn: column_to_be_unique) {
                if(index.sibling(index.row(),colmn).data() != index.sibling( therow, colmn).data()) {
                    equal = false;
                    break;
                }
            }
            if(equal) {
                selectionModel()->select(index, QItemSelectionModel::Rows|QItemSelectionModel::Deselect);
            }
        }
        testedrows.insert(therow);
    }
}

//-----------------------------------------------------------------

void DBKeysTable::keyPressEvent(QKeyEvent* e)
{
    MatrixTable::keyPressEvent(e);
    switch(e->key()) {
    case Qt::Key_Up:
    case Qt::Key_Down:
    case Qt::Key_PageUp:
    case Qt::Key_PageDown: {
        QModelIndex index = currentIndex();
        if(index.isValid())  {
            open_function(index);
            setCurrentIndex(index);
        }
    }
    break;
    }
}

void DBKeysTable::CmNext()
{
    QModelIndex index = currentIndex();
    if(!index.isValid()) {
        if(model()->rowCount() > 0) {
            index = model()->index(0,0);
        }
    }
    else {
        int row = index.row();
        row = std::min( row+1, model()->rowCount()-1 );
        index = index.sibling( row, index.column() );
    }
    if(index.isValid())  {
        open_function(index);
        setCurrentIndex(index);
    }
}

void DBKeysTable::CmPrevious()
{
    QModelIndex index = currentIndex();
    if(index.isValid()) {
        int row = index.row();
        row = std::max(row-1, 0);
        index = index.sibling(row, index.column());
        open_function( index );
        setCurrentIndex(index);
    }
}

void DBKeysTable::move_to_key(std::string id_key)
{
    if(id_key.empty()) {
        return;
    }

    setColumnHidden(0, false);  // ??? not hide to edges
    // search `item` text in model
    QModelIndexList items = model()->match(model()->index(0, 0), Qt::DisplayRole, QString::fromStdString(id_key));
    if( !items.isEmpty() ) {
        selectionModel()->select(items.first(), QItemSelectionModel::ClearAndSelect|QItemSelectionModel::Rows);
        scrollTo(items.first(), QAbstractItemView::EnsureVisible);
    }
    setColumnHidden(0, true); // ??? not hide to edges
}

} // namespace jsonui

