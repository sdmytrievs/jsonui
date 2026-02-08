#include <QPushButton>
#include <QLineEdit>
#include "ui_FieldsSelectDialog.h"
#include "jsonqml/forms/FieldsSelectDialog.h"
#include "jsonqml/models/fields_model.h"

namespace jsonqml {

SelectFieldsDialog::SelectFieldsDialog(const QString &title,
                                       const QString &aschema,
                                       const std::vector<std::string> &def_fields_list,
                                       QWidget *parent):
    QDialog(parent),
    ui(new Ui::FieldsSelectDialog),
    multi_selection(true), schema_name(aschema), def_fields(def_fields_list)
{
    setWindowTitle(title);
    setup_window();
    updateSelection();
}

SelectFieldsDialog::SelectFieldsDialog(const QString &title,
                                       const QString &aschema,
                                       QWidget *parent):
    QDialog(parent),
    ui(new Ui::FieldsSelectDialog),
    multi_selection(false), schema_name(aschema), def_fields()
{
    setWindowTitle(title);
    setup_window();
}

SelectFieldsDialog::~SelectFieldsDialog()
{}

void SelectFieldsDialog::CmHelp()
{
    ///    helpWin( "SchemaSelectJsonui", "" );
}

std::vector<std::string> SelectFieldsDialog::allSelected()
{
    QModelIndexList selection = ui->schemaView->selectionModel()->selectedRows();
    if(model_schema) {
        return model_schema->selectedFields(selection);
    }
    return {};
}

std::string SelectFieldsDialog::selIndex(const jsonio::FieldDef** flddata)
{
    QModelIndex index = ui->schemaView->currentIndex();
    if(index.isValid()) {
        if(model_schema) {
            *flddata = model_schema->fld_description(index);
            return model_schema->index2path(index);
        }
    }
    return "";
}

void SelectFieldsDialog::setup_window()
{
    ui->setupUi(this);

    // set up model
    model_schema = new SelectFieldsModel(schema_name, this);
    FieldsDelegate* deleg = new FieldsDelegate();
    ui->schemaView->setModel(model_schema);
    ui->schemaView->setItemDelegate(deleg);
    ui->schemaView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->schemaView->setEditTriggers(QAbstractItemView::AnyKeyPressed);
    ui->schemaView->header()->resizeSection(0, 250);
    ui->schemaView->expandToDepth(0);

    if(multi_selection) {
        ui->schemaView->setSelectionMode(QAbstractItemView::MultiSelection);
        QPushButton* btn = ui->buttonBox->addButton(QDialogButtonBox::Reset);
        QObject::connect(btn, &QPushButton::clicked, ui->schemaView, &QTreeView::clearSelection);
        btn = ui->buttonBox->addButton(QDialogButtonBox::RestoreDefaults);
        QObject::connect(btn, &QPushButton::clicked, this, &SelectFieldsDialog::updateSelection);
    }
    else {
        ui->schemaView->setSelectionMode(QAbstractItemView::SingleSelection);
        QObject::connect(ui->schemaView, SIGNAL(doubleClicked(const QModelIndex&)), this, SLOT(accept()));
    }

    ui->schemaView->setFocus();
    QObject::connect(ui->buttonBox, &QDialogButtonBox::helpRequested, this, &SelectFieldsDialog::CmHelp);
}

void SelectFieldsDialog::updateSelection()
{
    ui->schemaView->clearSelection();
    if(model_schema) {
        const QModelIndexList sel_lst = model_schema->selectIndexes(ui->schemaView->rootIndex(), def_fields);
        for(const auto& index: sel_lst) {
            ui->schemaView->selectionModel()->select(index,
                                                     QItemSelectionModel::Rows|QItemSelectionModel::Select);
        }
    }
}

QWidget *FieldsDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                                      const QModelIndex &index) const
{
    const SelectFieldsModel *themodel = dynamic_cast<const SelectFieldsModel *>(index.model());
    if(themodel) {
        QLineEdit *line_edit = new QLineEdit(parent);
        if(themodel->is_list(index)) {
            QIntValidator *dvalid = new QIntValidator(line_edit);
            line_edit->setValidator(dvalid);
        }
        return line_edit;
    }
    else {
        return QItemDelegate::createEditor(parent, option, index);
    }
}

} // namespace jsonui


