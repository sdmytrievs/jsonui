
#ifndef SELECTMODEL_H
#define SELECTMODEL_H

#include <string>
#include <vector>
#include <functional>
#include <QAbstractTableModel>
#include <QSortFilterProxyModel>

namespace jsonqml {

/// Function fetching document from a collection that match the specified condition
using  GetColor_f = std::function<QColor(int line, int column)>;

/// Record values type
using model_line_t = std::vector<std::string>;

/// Lines of colums values table
using model_table_t = std::vector<model_line_t>;

enum TableFlag {
    SingleSelection =   0x0020,///< Multiple items can be selected
    RowSortingEnabled =   0x0010,///< Added sorting into colums
    GraphDataEnabled =  0x0004,  ///< Connect 2d graphic for columns
    TableIsEditable =   0x0002,  ///< Enable editing
    NoMenuEnabled = 0x0001,        ///< Disable context menu
    NoTableFlags = 0             ///< Use only show mode
};
Q_DECLARE_FLAGS(TableFlags, TableFlag)
//Q_DECLARE_OPERATORS_FOR_FLAGS(TableFlags)
//Q_FLAGS(TableFlags)

///  \class SelectModel
///  \brief The SelectModel class provides a read-only data model for table data sets.
///
///  SelectModel is a high-level interface for executing selection statements and traversing the table data set.
///  It can provide data to view classes such as QTableView.
class SelectModel: public QAbstractTableModel
{
    Q_OBJECT

public:

    ///  Creates an empty SelectModel with the given \a parent
    explicit SelectModel(QObject *parent = nullptr);

    /// Creates an SelectModel from list of lines with the given \a parent.
    /// Split each line into a list of columns using regexp.
    explicit SelectModel(const std::vector<std::string>& list,
                         const std::string& split_regexp="\t",
                         model_line_t&& header_data={},
                         QObject *parent = nullptr);

    ///  Creates a SelectModel for 2D vector data with the given \a parent.
    explicit SelectModel(model_table_t&& table_data,
                         model_line_t&& header_data={},
                         QObject *parent = nullptr);

    /// Destroys the object and frees any allocated resources.
    virtual ~SelectModel();

    virtual void resetTable(model_table_t&& table_data, const model_line_t& header_data);

    void setColorFunction(GetColor_f func);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &item, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

protected:
    model_line_t header;
    model_table_t table;

private:
    bool use_color_function = false;
    GetColor_f color_function;

    void set_default_header();
};

/// The SortFilterProxyModel class provides support for sorting
/// and filtering data passed between another model and a view.
class SortFilterProxyModel : public QSortFilterProxyModel
 {
     Q_OBJECT

 public:

     SortFilterProxyModel(QObject *parent = nullptr):
         QSortFilterProxyModel(parent)
     { }

 protected:
     bool lessThan(const QModelIndex &left, const QModelIndex &right) const;

 };


}

#endif // SELECTMODEL_H
