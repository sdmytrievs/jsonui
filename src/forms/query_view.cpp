#include <QKeyEvent>
#include <QComboBox>
#include <QMenu>
#include <QAction>
#include <QHeaderView>
#include <QLineEdit>
#include <QMessageBox>
#include "jsonio/jsonschema.h"
// #include "jsonio17/jsondump.h"
// #include "jsonui17/preferences.h"
// #include "jsonui17/QueryWidget.h"
#include "jsonqml/forms/query_view.h"

namespace jsonqml {


//-------------------------------------------------------------------------------------
// class TQueryView implements a tree view that displays items from a model to query.
//-------------------------------------------------------------------------------------

QueryView::QueryView(QWidget * parent):
    QTreeView(parent)
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

void QueryView::ClearAll()
{
    QueryModel* themodel =  view_model();
    if(themodel) {
        themodel->clear_all();
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

    try {
        QModelIndex index = currentIndex();
        if(!index.isValid()) {
            index = rootIndex();
        }
        if(themodel && ((!isoperator  && themodel->can_logical(index, AQL_mode) )
                         || (isoperator  && themodel->can_operator(index, AQL_mode)))) {
            QModelIndex ndx = themodel->add_object(akey, isoperator, index);
            selectionModel()->setCurrentIndex( ndx, QItemSelectionModel::ClearAndSelect );
        }
        else
            QMessageBox::information(window(), "Add to FILTER", "Could not execute action." );
    }
    catch(jsonio::jsonio_exception& e) {
        QMessageBox::critical( window(), "jsonio_exception", e.what() );
    }
    catch(std::exception& e) {
        QMessageBox::critical( window(), "std::exception", e.what() );
    }
}

void QueryView::CmAddField()
{
    QueryModel* themodel = view_model();

    try {
        QModelIndex index = currentIndex();
        if(!index.isValid() || !AQL_mode) {
            index = rootIndex();
        }
        if(themodel && themodel->can_field(index, AQL_mode))  {
            /// QueryWidget* parent_dlg = dynamic_cast<QueryWidget*>(window());
            std::string fldpath;
            const jsonio::FieldDef* flddata=nullptr;
            /// parent_dlg->select_line(fldpath, &flddata);

            QModelIndex ndx = themodel->add_field(fldpath, flddata, index);
            selectionModel()->setCurrentIndex( ndx, QItemSelectionModel::ClearAndSelect );
        }
        else
            QMessageBox::information( window(), "Add to FILTER",
                                     "Could not execute action." );
    }
    catch(jsonio::jsonio_exception& e) {
        QMessageBox::critical(window(), "jsonio_exception", e.what());
    }
    catch(std::exception& e) {
        QMessageBox::critical(window(), "std::exception", e.what());
    }
}


void QueryView::CmAddObjects(const std::vector<std::string>& fields_list,
                             const std::vector<const jsonio::FieldDef*>& fields_types )
{
    QueryModel* themodel = view_model();

    try {
        QModelIndex index = rootIndex();
        if(themodel && themodel->can_field(index, AQL_mode)) {
            QModelIndex ndx = index;
            for(size_t ii=0; ii<fields_list.size(); ii++)
                ndx = themodel->add_field(fields_list[ii], fields_types[ii], index);
            selectionModel()->setCurrentIndex(ndx, QItemSelectionModel::ClearAndSelect);
        }
        else
            QMessageBox::information( window(), "Add to FILTER", "Could not execute action." );
    }
    catch(jsonio::jsonio_exception& e) {
        QMessageBox::critical(window(), "jsonio_exception", e.what());
    }
    catch(std::exception& e) {
        QMessageBox::critical(window(), "std::exception", e.what());
    }
}

void QueryView::CmDelLine()
{
    QModelIndex index = currentIndex();
    auto themodel = dynamic_cast<const QueryModel*>(index.model());
    try {
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
    catch(jsonio::jsonio_exception& e) {
        QMessageBox::critical(window(), "jsonio_exception", e.what());
    }
    catch(std::exception& e) {
        QMessageBox::critical( window(), "std::exception", e.what() );
    }
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

//-------------------------------------------------------------------------------------
// TQueryDelegate -  individual items in views are rendered and edited using delegates
//-------------------------------------------------------------------------------------

QWidget* QueryDelegate::enums_to_combobox(QWidget *parent, const jsonio::EnumDef *enumdef) const
{
    auto lst = enumdef->all_names();
    QComboBox *accessComboBox = new QComboBox(parent);
    for(size_t ii=0; ii<lst.size(); ii++) {
        std::string itname = lst[ii];
        accessComboBox->addItem(tr(itname.c_str()), enumdef->name2value(itname) );
        accessComboBox->setItemData( static_cast<int>(ii), enumdef->name2description(itname).c_str(), Qt::ToolTipRole);
    }
    return accessComboBox;
}

// Editing QTreeView for objects in System page
QWidget *QueryDelegate::createEditor( QWidget *parent,
                                     const QStyleOptionViewItem &option,
                                     const QModelIndex &index) const
{
    const QueryModel *themodel = dynamic_cast< const QueryModel *>(index.model() );
    if( !themodel )
        return QAbstractItemDelegate::createEditor( parent, option,  index );
/*
    AQLline *line =  themodel->lineFromIndex(index);
    if( index.column() > 0 )
    {
        switch( line->value_type() )
        {
        case jsonio17::JsonBase::Bool:
        {
            QComboBox *accessComboBox = new QComboBox(parent);
            accessComboBox->addItem(tr("false"), tr("false"));
            accessComboBox->addItem(tr("true"), tr("true"));
            return accessComboBox;
        }
        case jsonio17::JsonBase::Int:
        {
            std::string enumName = line->field_data.class_name;
            if( !enumName.empty() )
            {
                const jsonio17::EnumDef* enumdef = jsonio17::ioSettings().Schema().getEnum( enumName );
                if( enumdef != nullptr )
                    return enums_to_combobox( parent, enumdef );
            }
            QLineEdit *lineEdit = new QLineEdit(parent);
            QIntValidator *ivalid = new QIntValidator(lineEdit);
            if( line->field_data.field_type == jsonio17::FieldDef::T_U64 )
                ivalid->setBottom(0);
            lineEdit->setValidator(ivalid);
            return lineEdit;
        }
        case jsonio17::JsonBase::Double:
        {
            QLineEdit *lineEdit = new QLineEdit(parent);
            QDoubleValidator *dvalid = new QDoubleValidator(lineEdit);
            lineEdit->setValidator(dvalid);
            return lineEdit;
        }
        case jsonio17::JsonBase::String:
        default:
        {
            QLineEdit *lineEdit = new QLineEdit(parent);
            return lineEdit;
        }
        }
    }
*/
    return QAbstractItemDelegate::createEditor( parent, option,  index );
}


} // namespace jsonui

