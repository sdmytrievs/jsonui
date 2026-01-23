#pragma once

#include <QTreeView>
#include <QItemDelegate>
#include "jsonqml/models/query_model.h"

namespace jsonio {
class EnumDef;
}

namespace jsonqml {

/// \class QueryDelegate
/// individual items in views are rendered and edited using delegates
class QueryDelegate: public QItemDelegate
{
    Q_OBJECT

public:
    QueryDelegate(QObject * parent = nullptr):
        QItemDelegate(parent)
    {}
    QWidget *createEditor(QWidget *parent,
                          const QStyleOptionViewItem &option,
                          const QModelIndex &index) const override;
protected:
    QWidget *enums_to_combobox(QWidget *parent, const jsonio::EnumDef *enumdef) const;
};

///  \class QueryView implements a tree view of query structure
/// that displays items from a TQueryModel model.
class QueryView: public QTreeView
{
    Q_OBJECT

public slots:
    bool AQLmode(bool checked);
    void ClearAll();
    void CmAddObjects(const std::vector<std::string>& fields_list,
                      const std::vector<const jsonio::FieldDef*>& fields_types);
    void CmAddField();
    void CmDelLine();

    void CmAddAnd()
    {  add_object("AND", false);  }
    void CmAddOr()
    {  add_object("OR", false);  }
    void CmAddNot()
    {  add_object("NOT", false);  }
    void CmAddIn()
    {  add_object("IN");  }
    void CmAddNin()
    {  add_object("NOT IN");  }
    void CmAddLike()
    {  add_object("LIKE");  }
    void CmAddRegexp()
    {  add_object("=~");  }
    void CmAddNotRegexp()
    {  add_object("!~");  }
    void CmAddGt()
    {  add_object(">");  }
    void CmAddGte()
    {  add_object(">=");  }
    void CmAddLt()
    {  add_object("<");  }
    void CmAddLte()
    {  add_object("<=");  }
    void CmAddEq()
    {  add_object("==");  }
    void CmAddNotEq()
    {  add_object("!=");  }

protected slots:
    void slotPopupContextMenu(const QPoint& pos);
    void changeCurrent(int section);

public:
    QueryView(QWidget * parent = nullptr);

private:
    bool AQL_mode;
    void keyPressEvent(QKeyEvent* e) override;

    void add_object(const std::string& akey, bool isoperator = true);
    QMenu* context_menu(const QModelIndex &index );

    QueryModel* view_model()
    {
        return dynamic_cast<QueryModel*>(model());
    }
};

} // namespace jsonui


