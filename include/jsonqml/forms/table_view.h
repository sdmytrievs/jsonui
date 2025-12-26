#pragma once

#include <set>
#include <QItemDelegate>
#include <QTableView>

namespace jsonqml {

class TableClient;

using  ShowRecord_f = std::function<void( const QModelIndex& index)>;

///  \class MatrixDelegate
/// individual items in views are rendered and edited using delegates
class MatrixDelegate: public QItemDelegate
{
    Q_OBJECT

public:
    explicit MatrixDelegate(QObject* parent = nullptr):
        QItemDelegate(parent)
    {}

    QWidget *createEditor(QWidget* parent, const QStyleOptionViewItem& option,
                          const QModelIndex& index) const override;
};


///  \class MatrixTable implements a table view
/// that displays items from a MatrixModel model.
class MatrixTable: public QTableView
{
    Q_OBJECT

public slots:
    void slotPopupContextMenu(const QPoint& pos);
    void selRow();
    void selColumn();
    void copyData();
    void copyDataHeader();

    // only if editable
    void runCalc();
    void cutData();
    void clearData();
    void pasteData();
    void pasteTransposedData();

public:
    explicit MatrixTable(int mode, TableClient* atable_client, QWidget* parent = nullptr);

    void makePopupContextMenu(QMenu *menu, QModelIndex index);

    void setCurrentRow(int row);
    int selectedRow() const;
    void selectRows(const std::set<std::size_t> &rows);
    std::set<std::size_t> allSelectedRows() const;

protected:
    int mode_using;
    TableClient* table_client;

    void keyPressEvent(QKeyEvent* e) override;
};

///  \class MatrixUniqueSelection implements a table view
/// that displays items from a MatrixModel model.
/// Can be selected only unique values into defined column.
class MatrixUniqueSelection: public MatrixTable
{
    Q_OBJECT

protected slots:
    void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected) override
    {
        MatrixTable::selectionChanged(selected, deselected);
        slot_selection_change(selected, deselected);
    }

public:
    explicit MatrixUniqueSelection(int mode, TableClient* atable_client,
                                   const std::vector<int>& col_uniq, QWidget* parent = nullptr):
        MatrixTable(mode, atable_client, parent),
        column_to_be_unique(col_uniq)
    {}

protected:
    std::vector<int> column_to_be_unique;

    void slot_selection_change(const QItemSelection &selected, const QItemSelection &deselected);
};

///  \class DBKeyTable implements a table view that displays
/// items from database keys/fields table.
class DBKeysTable: public MatrixTable
{
    Q_OBJECT

public slots:
    void CmNext();
    void CmPrevious();
    /// Move to current id
    void move_to_key(std::string id_key);

public:
    DBKeysTable(int mode, TableClient* atable_client,
                ShowRecord_f show_fn, QWidget* parent = nullptr):
        MatrixTable(mode, atable_client, parent),
        open_function(show_fn)
    {}


protected:
    ShowRecord_f open_function;

    void keyPressEvent(QKeyEvent* e);
};

} // namespace jsonu

