#include <QKeyEvent>
#include <QComboBox>
#include <QMenu>
#include <QAction>
#include <QHeaderView>
#include <QLineEdit>
#include <QMessageBox>
#include "jsonio/jsonschema.h"
#include "jsonqml/models/schema_model.h"
#include "jsonqml/forms/query_view.h"
#include "jsonqml/forms/FieldsSelectDialog.h"

namespace jsonqml {


//-------------------------------------------------------------------------------------
// class TQueryView implements a tree view that displays items from a model to query.
//-------------------------------------------------------------------------------------

QueryView::QueryView(const QString& schema, QWidget * parent):
    QTreeView(parent),
    schema_name(schema)
{
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    header()->setSectionsClickable(true);
    header()->setSectionResizeMode(/*QHeaderView::ResizeToContents*/QHeaderView::Interactive);

    setEditTriggers(QAbstractItemView::DoubleClicked|QAbstractItemView::AnyKeyPressed);
    setFocusPolicy(Qt::StrongFocus);
    setTabKeyNavigation(false);
    setContextMenuPolicy(Qt::CustomContextMenu);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setIndentation(30);

    connect(this, &QueryView::customContextMenuRequested,
            this, &QueryView::slotPopupContextMenu);
    connect(header(), &QHeaderView::sectionClicked,
            this, &QueryView::changeCurrent);
}

bool QueryView::AQLmode(bool checked)
{
    if(checked != AQL_mode) {
        ClearAll();
        AQL_mode = checked;
        return true;
    }
    return false;
}

void QueryView::updateSchema(const QString &schema)
{
    schema_name = schema;
}

void QueryView::ClearAll()
{
    QueryModel* themodel = view_model();
    if(themodel) {
        themodel->clear_all();
    }
}

void QueryView::CmAddField()
{
    QueryModel* themodel = view_model();

    QModelIndex index = currentIndex();
    if(!index.isValid() || !AQL_mode) {
        index = rootIndex();
    }
    if(themodel && themodel->can_field(index, AQL_mode))  {
        SelectFieldsDialog dlg("Please, mark addition data field for the query",
                               schema_name, window());
        if(dlg.exec()) {
            const jsonio::FieldDef* flddata=nullptr;
            auto fldpath = dlg.selIndex(&flddata);
            QModelIndex ndx = themodel->add_field(fldpath, flddata, index);
            selectionModel()->setCurrentIndex( ndx, QItemSelectionModel::ClearAndSelect );
        }
    }
    else {
        QMessageBox::information( window(), "Add to FILTER",
                                 "Could not execute action." );
    }
}

void QueryView::CmDelLine()
{
    QModelIndex index = currentIndex();
    auto themodel = dynamic_cast<const QueryModel*>(index.model());
    if(themodel) {
        selectionModel()->clear();
        collapse(index);
        const_cast<QueryModel*>(themodel)->del_object(index);
    }
    else {
        QMessageBox::information(window(), "Delete from Object",
                                 "Please, determine the key-value pair to be deleted" );
    }
}

void QueryView::changeCurrent(int section)
{
    if(!currentIndex().isValid() ) {
        QModelIndex index = model()->index(0, section, rootIndex());
        setCurrentIndex(index);
    }
}

void QueryView::slotPopupContextMenu(const QPoint &pos)
{
    QModelIndex index = indexAt(pos);
    QMenu *menu = context_menu(index);
    if(menu) {
        menu->exec(viewport()->mapToGlobal(pos));
        delete menu;
    }
}

QMenu* QueryView::context_menu(const QModelIndex &index)
{
    const QueryModel* themodel = view_model();

    if(!themodel) {
        return nullptr;
    }

    QMenu *menu = new QMenu(this);
    QAction* act;

    if(themodel->can_field(index, AQL_mode)) {
        act =  new QAction(tr("Add &Fieldpath"), this);
        act->setShortcut(tr("Ctr+F"));
        act->setStatusTip(tr("Add the Next Fieldpath"));
        connect(act, &QAction::triggered, this, &QueryView::CmAddField);
        menu->addAction(act);
        menu->addSeparator();
    }

    if(themodel->can_logical(index, AQL_mode)) {
        act =  new QAction(tr("Add '&AND'"), this);
        act->setShortcut(tr("Ctr+A"));
        act->setStatusTip(tr("Add '&&' joined condition"));
        connect(act, &QAction::triggered, this, &QueryView::CmAddAnd);
        menu->addAction(act);

        act =  new QAction(tr("Add '&OR'"), this);
        act->setShortcut(tr("Ctr+O"));
        act->setStatusTip(tr("Add '||' joined condition"));
        connect(act, &QAction::triggered, this, &QueryView::CmAddOr);
        menu->addAction(act);

        act =  new QAction(tr("Add '&NOT'"), this);
        act->setShortcut(tr("Ctr+N"));
        act->setStatusTip(tr("Add '!' condition"));
        connect(act, &QAction::triggered, this, &QueryView::CmAddNot);
        menu->addAction(act);

        menu->addSeparator();
    }

    if(themodel->can_operator(index, AQL_mode)) {
        act =  new QAction(tr("Add '==' operator"), this);
        act->setShortcut(tr("="));
        act->setStatusTip(tr("Value equality comparison operator"));
        connect(act, &QAction::triggered, this, &QueryView::CmAddEq);
        menu->addAction(act);

        act =  new QAction(tr("Add '!=' operator"), this);
        act->setShortcut(tr("!"));
        act->setStatusTip(tr("Value inequality comparison operator"));
        connect(act, &QAction::triggered, this, &QueryView::CmAddNotEq);
        menu->addAction(act);

        act =  new QAction(tr("Add '>' operator"), this);
        act->setShortcut(tr(">"));
        act->setStatusTip(tr("Value greater than"));
        connect(act, &QAction::triggered, this, &QueryView::CmAddGt);
        menu->addAction(act);

        act =  new QAction(tr("Add '>=' operator"), this);
        act->setShortcut(tr("Ctr+G"));
        act->setStatusTip(tr("Value greater than or equal to"));
        connect(act, &QAction::triggered, this, &QueryView::CmAddGte);
        menu->addAction(act);

        act =  new QAction(tr("Add '<' operator"), this);
        act->setShortcut(tr("<"));
        act->setStatusTip(tr("Value lesser than "));
        connect(act, &QAction::triggered, this, &QueryView::CmAddLt);
        menu->addAction(act);

        act =  new QAction(tr("Add '<=' operator"), this);
        act->setShortcut(tr("Ctr+E"));
        act->setStatusTip(tr(" Value lesser than or equal to"));
        connect(act, &QAction::triggered, this, &QueryView::CmAddLte);
        menu->addAction(act);

        menu->addSeparator();

        act =  new QAction(tr("Add 'IN'"), this);
        act->setShortcut(tr("Ctr+I"));
        act->setStatusTip(tr("Test if a value is contained in an array"));
        connect(act, &QAction::triggered, this, &QueryView::CmAddIn);
        menu->addAction(act);

        act =  new QAction(tr("Add 'NOT IN'"), this);
        act->setStatusTip(tr("Test if a value is not contained in an array"));
        connect(act, &QAction::triggered, this, &QueryView::CmAddNin);
        menu->addAction(act);

        act =  new QAction(tr("Add '&LIKE'"), this);
        act->setShortcut(tr("Ctr+L"));
        act->setStatusTip(tr("Tests if a string value matches a pattern"));
        connect(act, &QAction::triggered, this, &QueryView::CmAddLike);
        menu->addAction(act);

        act =  new QAction(tr("Add '=~'"), this);
        act->setShortcut(tr("Ctr+R"));
        act->setStatusTip(tr("Tests if a string value matches a regular expression"));
        connect(act, &QAction::triggered, this, &QueryView::CmAddRegexp);
        menu->addAction(act);

        menu->addSeparator();
        act =  new QAction(tr("Add '!~'"), this);
        act->setStatusTip(tr("Tests if a string value does not match a regular expression"));
        connect(act, &QAction::triggered, this, &QueryView::CmAddNotRegexp);
        menu->addAction(act);

        menu->addSeparator();
    }

    act =  new QAction(tr("Remove current line"), this);
    act->setShortcut(tr("F8"));
    act->setStatusTip(tr("Delete current line from object"));
    connect(act, &QAction::triggered, this, &QueryView::CmDelLine);
    menu->addAction(act);

    return menu;
}

void QueryView::keyPressEvent(QKeyEvent* e)
{
    if(e->modifiers() & Qt::ControlModifier) {
        switch(e->key()) {
        case Qt::Key_F:
            CmAddField();
            return;
        case Qt::Key_A:
            CmAddAnd();
            return;
        case Qt::Key_O:
            CmAddOr();
            return;
        case Qt::Key_N:
            CmAddNot();
            return;

        case Qt::Key_Equal:
            CmAddEq();
            return;
        case Qt::Key_Exclam:
            CmAddNotEq();
            return;
        case Qt::Key_Greater:
            CmAddGt();
            return;
        case Qt::Key_G:
            CmAddGte();
            return;
        case Qt::Key_Less:
            CmAddLt();
            return;
        case Qt::Key_E:
            CmAddLte();
            return;

        case Qt::Key_I:
            CmAddIn();
            return;
        case Qt::Key_L:
            CmAddLike();
            return;
        case Qt::Key_R:
            CmAddRegexp();
            return;
        }
    }

    switch(e->key()) {
    case Qt::Key_F8:
        CmDelLine();
        return;
    }
    QTreeView::keyPressEvent(e);
}

void QueryView::add_object(const std::string& akey, bool isoperator)
{
    QueryModel* themodel = view_model();

    QModelIndex index = currentIndex();
    if(!index.isValid()) {
        index = rootIndex();
    }
    if(themodel && ((!isoperator  && themodel->can_logical(index, AQL_mode) )
                     || (isoperator  && themodel->can_operator(index, AQL_mode)))) {
        QModelIndex ndx = themodel->add_object(akey, isoperator, index);
        selectionModel()->setCurrentIndex( ndx, QItemSelectionModel::ClearAndSelect );
    }
    else {
        QMessageBox::information(window(), "Add to FILTER", "Could not execute action." );
    }
}

//-------------------------------------------------------------------------------------
// TQueryDelegate -  individual items in views are rendered and edited using delegates
//-------------------------------------------------------------------------------------

QWidget *QueryDelegate::createEditor(QWidget *parent,
                                     const QStyleOptionViewItem &option,
                                     const QModelIndex &index) const
{
    const QueryModel *themodel = dynamic_cast<const QueryModel *>(index.model());
    if(themodel) {
        auto fld_type = themodel->field_type(index);
        auto fld_enum = themodel->field_enum_class(index);
        auto combo_data = editor_combobox(fld_type, fld_enum);

        if(combo_data.size()>0) {
            QComboBox *combo_box = new QComboBox(parent);
            foreach (const QVariantMap& map, combo_data) {
                combo_box->addItem(map["text"].toString(), map["value"]);
            }
            return combo_box;
        }
        else {
            QLineEdit *line_edit = new QLineEdit(parent);
            jsonio::JsonBase::Type json_type =  jsonio::JsonSchema::fieldtype2basetype(fld_type);
            if(json_type==jsonio::JsonBase::Double) {
                QDoubleValidator *dvalid = new QDoubleValidator(line_edit);
                line_edit->setValidator(dvalid);
            }
            if(json_type==jsonio::JsonBase::Int) {
                QIntValidator *dvalid = new QIntValidator(line_edit);
                line_edit->setValidator(dvalid);
            }
            return line_edit;
        }
    }
    else {
        return QItemDelegate::createEditor(parent, option, index);
    }
}

} // namespace jsonui

