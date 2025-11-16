
#include <QHeaderView>
#include <QPushButton>
#include "ui_SelectDialog.h"
#include "jsonqml/forms/SelectDialog.h"
#include "jsonqml/forms/table_view.h"
#include "jsonqml/clients/table_client.h"
#include "jsonqml/clients/vertex_client.h"
#include "jsonqml/models/db_keys_model.h"

namespace jsonqml {


SelectDialog::SelectDialog(int mode, const QString &title,
                           const std::vector<std::string> &list,
                           const std::string &split_regexp,
                           model_line_t &&header_data,
                           QWidget *parent):
    QDialog(parent),
    ui(new Ui::SelectDialogData),
    table_client(nullptr),
    table_view(nullptr),
    internal_model(nullptr),
    mode_using(mode)
{
    internal_model.reset(new SelectModel(list, split_regexp, std::move(header_data), this));
    current_model = internal_model.get();

    init_dialog(title);
}

SelectDialog::SelectDialog(int mode, const QString &title,
                           model_table_t &&table_data,
                           model_line_t &&header_data,
                           QWidget *parent):
    QDialog(parent),
    ui(new Ui::SelectDialogData),
    table_client(nullptr),
    table_view(nullptr),
    internal_model(nullptr),
    mode_using(mode)
{
    internal_model.reset(new SelectModel(std::move(table_data), std::move(header_data), this));
    current_model = internal_model.get();

    init_dialog(title);
}

SelectDialog::SelectDialog(int mode, const QString &title,
                           SelectModel *keys_model, QWidget *parent):
    QDialog(parent),
    ui(new Ui::SelectDialogData),
    table_client(nullptr),
    table_view(nullptr),
    internal_model(nullptr),
    mode_using(mode)
{
    current_model = keys_model;

    init_dialog(title);
}

SelectDialog::~SelectDialog()
{ }

void SelectDialog::setSelection(int selrow)
{
    table_view->setCurrentRow(selrow);
}

void SelectDialog::setSelection(std::set<size_t> &selrows)
{
    table_view->selectRows(selrows);
}

int SelectDialog::selIndex() const
{
    if(!result()) {
        return -1;
    }
    return table_view->selectedRow();
}

std::set<size_t> SelectDialog::allSelected() const
{
    if(!result()) {
        return {};
    }
    return table_view->allSelectedRows();
}

// Returns single selection,
// Throw exeption if nothing selected
size_t SelectDialog::getSelectedIndex() const
{
    auto selndx = selIndex();
    jsonio::JSONIO_THROW_IF(selndx<0, "models", 11, "undefined single selection");
    return static_cast<size_t>(selndx);
}

void SelectDialog::CmHelp()
{
    // helpWin( "SelectJsonui", "" );
}

void SelectDialog::CmSelectAll()
{
    table_view->selectAll();
}

void SelectDialog::CmClearAll()
{
    table_view->clearSelection();
}

//-----------------------------------------------------

void SelectDialog::init_dialog(const QString& title)
{
    ui->setupUi(this);
    setWindowTitle(title);

    table_client.reset(new TableClient(current_model, mode_using, this));

    table_view.reset(new MatrixTable(mode_using, table_client.get(), this));
    ui->verticalLayout->insertWidget(1,table_view.get());
    table_view->setSelectionBehavior(QAbstractItemView::SelectRows);

    QObject::connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &SelectDialog::accept);
    QObject::connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &SelectDialog::reject);
    QObject::connect(ui->buttonBox, &QDialogButtonBox::helpRequested, this, &SelectDialog::CmHelp);

    if(mode_using & TableFlag::SingleSelection) {
        table_view->setSelectionMode(QAbstractItemView::SingleSelection);
        setSelection(0);
        QObject::connect(table_view.get(), SIGNAL(doubleClicked(const QModelIndex&)), this, SLOT(accept()) );
    }
    else {
        table_view->setSelectionMode(QAbstractItemView::MultiSelection);
        QPushButton* selectall =ui->buttonBox->addButton("&Select All", QDialogButtonBox::ActionRole );
        QPushButton* clearselection =ui->buttonBox->addButton("&Clear All", QDialogButtonBox::ActionRole );
        QObject::connect(selectall, &QPushButton::clicked, this, &SelectDialog::CmSelectAll);
        QObject::connect(clearselection, &QPushButton::clicked, this, &SelectDialog::CmClearAll);
    }
    table_view->setFocus();
}

// Select document key from current query result for collection
std::string SelectKeyFrom(const QString& title, VertexClient* vertex, int mode, QWidget* parent)
{
    qDebug() << "SelectKeyFrom rows " << vertex->tablemodel()->rowCount();
    SelectDialog sel_dlg(mode, title, vertex->tablemodel(), parent);
    if(sel_dlg.exec())  {
        auto selndx = sel_dlg.selIndex();
        if(selndx >= 0) {
            return vertex->tableId(selndx);
        }
    }
    return "";
}

// Select document keys from current query result for collection
std::vector<std::string> SelectKeysFrom(const QString& title, VertexClient* vertex, int mode, QWidget* parent)
{
    std::vector<std::string> keys;
    SelectDialog sel_dlg(mode, title, vertex->tablemodel(), parent);
    if(sel_dlg.exec())  {
        auto sel_ndx = sel_dlg.allSelected();
        for(auto sel: sel_ndx) {
            keys.push_back(vertex->tableId(sel));
        }
    }
    return keys;
}

} // namespace jsonui


