#include <QKeyEvent>
#include <QMenu>
#include <QAction>
#include <QHeaderView>
#include <QMessageBox>

#include "jsonqml/clients/json_client.h"
#include "jsonqml/clients/settings_client.h"
#include "jsonqml/forms/json_view.h"
#include "jsonqml/forms/CalcDialog.h"


namespace jsonqml {

bool JsonView::expandedFields = true;

/// Internal structure to save expanded fields
class ExpandedLevelLine
{
public:
    QString keyname;           ///< Name of field
    ExpandedLevelLine *parent;
    std::vector<std::shared_ptr<ExpandedLevelLine>> children;

    ExpandedLevelLine(const QString& akey, ExpandedLevelLine* parentline):
        keyname(akey), parent(parentline)
    {}

    ~ExpandedLevelLine()
    {
        children.clear();
    }

    std::shared_ptr<ExpandedLevelLine> findKey(const QString& akey)
    {
        auto it = children.begin();
        while(it != children.end()) {
            if(it->get()->keyname == akey) {
                return *it;
            }
            it++;
        }
        return nullptr;
    }
};

//-------------------------------------------------------------------------------------
// class JsonView implements a tree view that displays items from a model to json data.
//-------------------------------------------------------------------------------------

JsonView::JsonView(JsonClient* client, QWidget* parent):
    QTreeView(parent),
    json_client(client)
{
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    header()->setSectionsClickable(true);

    setEditTriggers(QAbstractItemView::DoubleClicked|QAbstractItemView::AnyKeyPressed);
    setFocusPolicy(Qt::StrongFocus);
    setTabKeyNavigation(false);
    setContextMenuPolicy(Qt::CustomContextMenu);
    setSelectionMode(QAbstractItemView::/*MultiSelection*/ExtendedSelection);
    setSelectionBehavior(QAbstractItemView::SelectItems);
    setIndentation(30);

    setModel(json_client->jsonmodel());

    connect(this, &JsonView::customContextMenuRequested, this, &JsonView::slotPopupContextMenu);
    connect(header(), &QHeaderView::sectionClicked, this, &JsonView::changeCurrent);

    connect(model(), &QAbstractItemModel::modelAboutToBeReset, this, &JsonView::saveExpandedRows);
    connect(model(), &QAbstractItemModel::modelReset, this, &JsonView::getExpandedRows);
}

void JsonView::changeCurrent(int section)
{
    if(!currentIndex().isValid()) {
        QModelIndex index = model()->index(0, section, rootIndex());
        setCurrentIndex(index);
    }
}

void JsonView::slotPopupContextMenu(const QPoint &pos)
{
    QModelIndex index = indexAt(pos);
    auto themodel = current_index_model(index);

    QMenu *menu = new QMenu(this);

    QAction* act =  new QAction(tr("&Help"), this);
    act->setIcon(QIcon(":/menu/ShowHelpWindowIcon.png"));
    act->setShortcut(tr("F1"));
    act->setStatusTip(tr("Help for specified cell"));
    connect(act, &QAction::triggered, this, &JsonView::CmHelp);
    menu->addAction(act);

    menu->addSeparator();

    if(themodel && themodel->canBeAdd(index)) {
        act =  new QAction(tr("Add Structured Fields ..."), this);
        act->setShortcut(QKeySequence(Qt::Key_F7));
        act->setShortcutVisibleInContextMenu(true);
        act->setStatusTip(tr("Add new line(s) into object"));
        connect(act, &QAction::triggered, this, &JsonView::CmAddObjects);
        menu->addAction(act);
    }

    if(themodel && themodel->canBeAdd(index)) {
        act = new QAction(tr("Add &One Field ..."), this);
        act->setShortcut(tr("Ctr+O"));
        act->setStatusTip(tr("Add new line into object"));
        connect(act, &QAction::triggered, this, &JsonView::CmAddObject);
        menu->addAction(act);
    }

    if(themodel) {
        act =  new QAction(tr("Reset Data Object to &Defaults"), this);
        act->setShortcut(tr("Ctrl+D"));
        act->setStatusTip(tr("Setup default values to object"));
        connect(act, &QAction::triggered, this, &JsonView::CmResetObject);
        menu->addAction(act);
    }

    if(themodel && themodel->canBeRemoved(index)) {
        act =  new QAction(tr("Select and Remove Fields"), this);
        act->setShortcut(QKeySequence(Qt::Key_F8));
        act->setShortcutVisibleInContextMenu(true);
        act->setStatusTip(tr("Delete selected lines from object"));
        connect(act, &QAction::triggered, this, &JsonView::CmDelObjects);
        menu->addAction(act);
    }

    if(themodel && themodel->canBeRemoved(index)) {
        act =  new QAction(tr("&Remove Selected Field"), this);
        act->setShortcut(tr("Ctr+R"));
        act->setStatusTip(tr("Delete current line from object"));
        connect(act, &QAction::triggered, this, &JsonView::CmDelObject);
        menu->addAction(act);
    }

    if(themodel && themodel->isUnion(index)) {
        act =  new QAction(tr("Remove Alternatives (&Union)"), this);
        act->setShortcut(tr("Ctr+U"));
        act->setStatusTip(tr("Delete other of current line from object"));
        connect(act, &QAction::triggered, this, &JsonView::CmDelObjectsUnion);
        menu->addAction(act);
    }

    if(themodel && themodel->canBeResized(index)) {
        menu->addSeparator();
        act =  new QAction(tr("Resize Selected List/Map/Set ..."), this);
        act->setShortcut(QKeySequence(Qt::Key_F4));
        act->setShortcutVisibleInContextMenu(true);
        act->setStatusTip(tr("Change current array size"));
        connect(act, &QAction::triggered, this, &JsonView::CmResizeArray);
        menu->addAction(act);
    }

    if(themodel && themodel->canBeCloned(index)) {
        act =  new QAction(tr("Clone Selected Field"), this);
        act->setShortcut(tr("Ctr+C"));
        act->setStatusTip(tr("Add copy of field to end of list/map"));
        connect(act, &QAction::triggered, this, &JsonView::CmCloneObject);
        menu->addAction(act);
    }

    if(index.column()==1 && themodel && themodel->isNumber(index)) {
        menu->addSeparator();
        act =  new QAction(tr("Calculator"), this);
        act->setShortcut(QKeySequence(Qt::Key_F5));
        //act->setShortcutVisibleInContextMenu(true);
        act->setStatusTip(tr("Use Calculator for specified cells"));
        connect(act, &QAction::triggered, this, &JsonView::CmCalc);
        menu->addAction(act);
    }

    if(themodel) {
        menu->addSeparator();
        act =  new QAction(tr("Copy Field &Path"), this);
        act->setShortcut(tr("Ctrl+P"));
        act->setStatusTip(tr("Copy selected field path"));
        connect(act, &QAction::triggered, this, &JsonView::CopyFieldPath);
        menu->addAction(act);

        act =  new QAction(tr("Copy &Field"), this);
        act->setShortcut(tr("Ctrl+F"));
        act->setStatusTip(tr("Copy selected field data"));
        connect(act, &QAction::triggered, this, &JsonView::CopyField);
        menu->addAction(act);

        act =  new QAction(tr("Paste Field &Value"), this);
        act->setShortcut(tr("Ctrl+V"));
        act->setShortcutVisibleInContextMenu(true);
        act->setStatusTip(tr("Paste data to field"));
        connect(act, &QAction::triggered, this, &JsonView::PasteField);
        menu->addAction(act);
    }

    menu->exec(viewport()->mapToGlobal(pos));
    delete menu;
}

void JsonView::keyPressEvent(QKeyEvent* e)
{
    if(e->modifiers() & Qt::ControlModifier) {
        switch(e->key()) {
        case Qt::Key_R:
            CmDelObject();
            return;
        case Qt::Key_U:
            CmDelObjectsUnion();
            return;
        case Qt::Key_O:
            CmAddObject();
            return;
        case Qt::Key_D:
            CmResetObject();
            return;
        case Qt::Key_P:
            CopyFieldPath();
            return;
        case Qt::Key_F:
            CopyField();
            return;
        case Qt::Key_V:
            PasteField();
            return;
        case Qt::Key_C:
            CmCloneObject();
            return;
        }
    }

    switch(e->key()) {
    case Qt::Key_F1:
        CmHelp();
        return;
    case Qt::Key_F5:
        CmCalc();
        return;
    case Qt::Key_F7:
        CmAddObjects();
        return;
    case Qt::Key_F8:
        CmDelObjects();
        return;
    case Qt::Key_F4:
        CmResizeArray();
        return;
    }
    QTreeView::keyPressEvent(e);
}


void JsonView::CmCalc()
{
    QModelIndex index = currentIndex();
    auto themodel = json_client->jsonmodel();
    if(themodel && themodel->isNumber(index) && index.column()==1) {
        CalcDialog calc(window());
        if(calc.exec()) {
            QString res = calc.computeFunctionString(index.data(Qt::EditRole).toDouble());
            themodel->setData(index, res, Qt::EditRole);
        }
    }
}

void JsonView::CmAddObjects()
{
    QModelIndex index = currentIndex();
    if(!index.isValid()) {
        index = rootIndex();
    }
    JsonBaseModel* themodel = view_model();

    try {
        if(themodel && themodel->canBeAdd(index)) {
            // get list of names
            //const QModelIndex ndx = themodel->addObjects(window(), index);
            //selectionModel()->setCurrentIndex(ndx, QItemSelectionModel::ClearAndSelect);
        }
        else {
            QMessageBox::information(window(), "Add to Object", "Please, determine object to be expanded");
        }
    }
    catch(jsonio::jsonio_exception& e){
        QMessageBox::critical(window(), "jsonio_exception", e.what());
    }
    catch(std::exception& e) {
        QMessageBox::critical(window(), "std::exception", e.what());
    }
}

void JsonView::CmAddObject()
{
    QModelIndex index = currentIndex();
    if(!index.isValid()) {
        index = rootIndex();
    }
    JsonBaseModel* themodel = view_model();

    try {
        if(themodel && themodel->canBeAdd(index)) {
            QString field_type;
            QString field_name;
            //!!!!  need dialog to setup  field_type&field_name
            QModelIndex ndx = themodel->addObject(index, field_type, field_name);
            selectionModel()->setCurrentIndex(ndx, QItemSelectionModel::ClearAndSelect);
        }
        else {
            QMessageBox::information(window(), "Add to Object", "Please, determine object to be expanded");
        }
    }
    catch(jsonio::jsonio_exception& e) {
        QMessageBox::critical(window(), "jsonio_exception", e.what());
    }
    catch(std::exception& e)
    {
        QMessageBox::critical(window(), "std::exception", e.what());
    }
}


void JsonView::CmCloneObject()
{
    QModelIndex index = currentIndex();
    JsonBaseModel* themodel = view_model();

    try {
        if(index.isValid() && themodel && themodel->canBeCloned(index)) {
            QModelIndex ndx = themodel->cloneObject(index);
            selectionModel()->setCurrentIndex(ndx, QItemSelectionModel::ClearAndSelect);
        }
        else {
            QMessageBox::information(window(), "Clone Object", "Please, determine object to be expanded");
        }
    }
    catch(jsonio::jsonio_exception& e) {
        QMessageBox::critical(window(), "jsonio_exception", e.what());
    }
    catch(std::exception& e) {
        QMessageBox::critical(window(), "std::exception", e.what());
    }
}

void JsonView::CmResetObject()
{
    QModelIndex index = currentIndex();
    JsonBaseModel* themodel = view_model();

    try {
        if(index.isValid() && themodel) {
            // collapse(index);
            themodel->setFieldData(index, "");
            update();
            // selectionModel()->setCurrentIndex( index, QItemSelectionModel::ClearAndSelect );
        }
        else {
            QMessageBox::information(window(), "Reset Object", "Please, determine object to be reseted");
        }
    }
    catch(jsonio::jsonio_exception& e) {
        QMessageBox::critical(window(), "jsonio_exception", e.what());
    }
    catch(std::exception& e) {
        QMessageBox::critical(window(), "std::exception", e.what());
    }
}

void JsonView::CmDelObject()
{
    QModelIndex index = currentIndex();
    JsonBaseModel* themodel = view_model();

    try {
        if(index.isValid() && themodel && themodel->canBeRemoved(index)) {
            selectionModel()->clear();
            collapse(index);
            themodel->removeObject(index);
            //update();
        }
        else {
            QMessageBox::information(window(), "Delete from Object", "Please, determine the key-value pair to be deleted");
        }
    }
    catch(jsonio::jsonio_exception& e) {
        QMessageBox::critical(window(), "jsonio_exception", e.what());
    }
    catch(std::exception& e) {
        QMessageBox::critical(window(), "std::exception", e.what());
    }
    catch(...) {
        QMessageBox::critical(window(), "std::exception", "Undefined exception");
    }
}

void JsonView::CmDelObjects() //!!!!!!!!!!
{
    QModelIndex index = currentIndex();
    JsonBaseModel* themodel = view_model();

    try {
        if(index.isValid() && themodel && themodel->canBeRemoved(index)) {
            selectionModel()->clear();
            collapse(index);
            //themodel->delObjects(index);
            //update();
        }
        else {
            QMessageBox::information(window(), "Delete from Object", "Please, determine the key-value level to be deleted");
        }
    }
    catch(jsonio::jsonio_exception& e) {
        QMessageBox::critical(window(), "jsonio_exception", e.what());
    }
    catch(std::exception& e)  {
        QMessageBox::critical(window(), "std::exception", e.what());
    }
    catch(...) {
        QMessageBox::critical(window(), "std::exception", "Undefined exception");
    }
}

void JsonView::CmDelObjectsUnion()
{
    QModelIndex index = currentIndex();
    JsonBaseModel* themodel = view_model();

    try {
        if(index.isValid() && themodel && themodel->isUnion(index)) {
            collapse(index);
            //themodel->delObjectsUnion( window(), index);
            //update();
        }
        else {
            QMessageBox::information(window(), "Delete from Object", "Please, determine the objects to be deleted");
        }
    }
    catch(jsonio::jsonio_exception& e) {
        QMessageBox::critical(window(), "jsonio_exception", e.what());
    }
    catch(std::exception& e) {
        QMessageBox::critical(window(), "std::exception", e.what());
    }
    catch(...) {
        QMessageBox::critical(window(), "std::exception", "Undefined exception");
    }
}

void JsonView::CmResizeArray()
{
    QModelIndex index = currentIndex();
    JsonBaseModel* themodel = view_model();

    try {
        if(index.isValid() && themodel && themodel->canBeResized(index)) {
            // collapse(index);
            int new_size=5;
            // need dialog to set size
            themodel->resizeArray(index, new_size);
        }
        else {
            QMessageBox::information(window(), "Resize Array", "Please, determine arrays key to be resized");
        }
    }
    catch(jsonio::jsonio_exception& e) {
        QMessageBox::critical(window(), "jsonio_exception", e.what());
    }
    catch(std::exception& e) {
        QMessageBox::critical(window(), "std::exception", e.what());
    }
}

void JsonView::CopyFieldPath()
{
    QModelIndex index = currentIndex();
    auto themodel = current_index_model(index);

    try {
        if(themodel) {
            QString clip_text = themodel->getFieldPath(index);
            uiSettings().copy(clip_text);
        }
    }
    catch(jsonio::jsonio_exception& e) {
        QMessageBox::critical(this, "jsonio_exception", e.what());
    }
    catch(std::exception& e) {
        QMessageBox::critical(window(), "std::exception", e.what());
    }
    catch(...) {
        QMessageBox::critical( window(), "std::exception", "Undefined exception" );
    }
}


void JsonView::CopyField()
{
    QModelIndex index = currentIndex();
    auto themodel = current_index_model(index);

    try {
        if(themodel) {
            QString clip_text = themodel->getFieldData(index);
            uiSettings().copy(clip_text);
        }
    }
    catch(jsonio::jsonio_exception& e) {
        QMessageBox::critical(window(), "jsonio_exception", e.what());
    }
    catch(std::exception& e) {
        QMessageBox::critical(window(), "std::exception", e.what());
    }
    catch(...) {
        QMessageBox::critical(window(), "std::exception", "Undefined exception");
    }
}

void JsonView::PasteField()
{
    QModelIndex index = currentIndex();
    JsonBaseModel* themodel = view_model();
    auto clip_text = uiSettings().paste();

    if(clip_text.isEmpty() || !index.isValid() || !themodel) {
        return;
    }
    try {
        themodel->setFieldData(index, clip_text);
    }
    catch(jsonio::jsonio_exception& e) {
        QMessageBox::critical( this, "jsonio_exception", e.what() );
    }
    catch(std::exception& e) {
        QMessageBox::critical(window(), "std::exception", e.what());
    }
    catch(...) {
        QMessageBox::critical(window(), "std::exception", "Undefined exception");
    }
}

// Help on F1 pressed on data field
void JsonView::CmHelp()
{
    QModelIndex index = currentIndex();
    auto themodel = current_index_model( index );
    QString hlp= "", schema= "undefined";

    if(themodel) {
        hlp =  themodel->helpName(index);
        auto pos = hlp.indexOf('#');
        if(pos >=0) {
            schema = hlp.sliced(0, pos);
            hlp = hlp.sliced(pos+1);
        }
        ui_logger->info("help  {}#{}", schema.toStdString(), hlp.toStdString());
        //helpWin(schema, hlp);
    }
}

void JsonView::saveExpandedRows()
{
    section_0_size = header()->sectionSize(0);
    section_1_size = header()->sectionSize(1);
    expanded_rows = std::make_shared<ExpandedLevelLine>("root", nullptr);
    save_expanded_state(expanded_rows);
}

void JsonView::getExpandedRows()
{
    if(expandedFields) {
        restore_expanded_state(expanded_rows);
    }
    else {
        expandToDepth(1);
    }
    header()->resizeSection(0, section_0_size);
    header()->resizeSection(1, section_1_size);
}

// Save expand level  -----------------------------------------

void JsonView::updateModelData(const std::string& json_string, const QString& schema_name)
{
    JsonBaseModel* themodel = view_model();
    if(!themodel) {
        return;
    }
    themodel->setupModelData(json_string, schema_name);
}

void JsonView::save_expanded_state(std::shared_ptr<ExpandedLevelLine> topList)
{
    for(int row = 0; row < model()->rowCount(); ++row) {
        save_expanded_on_level( model()->index(row,0), topList);
    }
}

void JsonView::restore_expanded_state( std::shared_ptr<ExpandedLevelLine> topList)
{
    setUpdatesEnabled(false);
    for(int row = 0; row < model()->rowCount(); ++row) {
        restore_expanded_on_level(model()->index(row,0), topList);
    }
    setUpdatesEnabled(true);
}

void JsonView::save_expanded_on_level(const QModelIndex& index,  std::shared_ptr<ExpandedLevelLine> parent)
{
    if(index.isValid() && isExpanded(index)) {
        QString key = index.data(Qt::EditRole).toString();
        std::shared_ptr child = std::make_shared<ExpandedLevelLine>(key, parent.get());
        if(parent != nullptr) {
            parent->children.push_back(child);
        }
        for(int row = 0; row < model()->rowCount(index); ++row) {
            save_expanded_on_level( index.model()->index(row,0, index), child );
        }
    }
}

void JsonView::restore_expanded_on_level(const QModelIndex& index,  std::shared_ptr<ExpandedLevelLine> topData)
{
    if(index.isValid()) {
        QString key = index.data(Qt::EditRole).toString();
        auto child = topData->findKey(key);
        if(child != nullptr) {
            setExpanded(index, true);
            for(int row = 0; row < model()->rowCount(index); ++row) {
                restore_expanded_on_level(index.model()->index(row, 0, index), child);
            }
        }
    }
}

} // namespace jsonui

