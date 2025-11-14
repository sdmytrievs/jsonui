#pragma once

#include <set>
#include <QDialog>
#include "jsonqml/models/select_model.h"

namespace jsonio {
class DBDocumentBase;
}

namespace Ui {
class SelectDialogData;
}

namespace jsonqml {

class TableClient;
class MatrixTable;
class VertexClient;

const int single_selection = TableFlag::SingleSelection|TableFlag::NoMenuEnabled|TableFlag::RowSortingEnabled;
const int multi_selection = TableFlag::NoMenuEnabled|TableFlag::RowSortingEnabled;

/// Select document key from current query result for collection
std::string SelectKeyFrom(const QString& title, VertexClient* vertex, int mode=single_selection, QWidget* parent=nullptr);

/// Select document keys from current query result for collection
std::vector<std::string> SelectKeysFrom(const QString& title, VertexClient* vertex, int mode=multi_selection, QWidget* parent=nullptr);

/// \class SelectDialog - dialog for single or multiple
/// selection from list/table;
class SelectDialog : public QDialog
{
    Q_OBJECT

public slots:
    void objectChanged() {}
    void CmHelp();
    void CmSelectAll();
    void CmClearAll();

public:
    /// Creates an SelectDialog from list of lines with the given \a parent.
    /// Split each line into a list of columns using regexp.
    explicit SelectDialog(int mode, const QString& title,
                         const std::vector<std::string>& list,
                         const std::string& split_regexp="\t",
                         model_line_t&& header_data={},
                         QWidget *parent = nullptr);

    ///  Creates a SelectDialog for 2D vector data with the given \a parent.
    explicit SelectDialog(int mode, const QString& title,
                         model_table_t&& table_data,
                         model_line_t&& header_data={},
                         QWidget *parent = nullptr);

    ///  Creates a SelectModel for 2D vector data with the given \a parent.
    explicit SelectDialog(int mode, const QString& title,
                          SelectModel* keys_model,
                          QWidget *parent = nullptr);

    /// Destructor
    virtual ~SelectDialog();

    /// Set Singler selection
    void setSelection(int selrow);

    /// Set Multiple selection
    void setSelection(std::set<size_t>& selrows);

    /// Set Multiple selection
    template<typename T,
             template <typename, typename > class Container>
    void setSelection(const Container<T, std::allocator<T>>& selrows)
    {
        std::set<size_t> sellst;
        for(auto it = selrows.begin(); it != selrows.end(); ++it) {
            sellst.insert(static_cast<size_t>(*it));
        }
        setSelection(sellst);
    }

    /// Returns single selection, \returns '-1' if nothing selected
    int selIndex() const;

    /// Returns single selection, \throw exeption if nothing selected
    std::size_t getSelectedIndex() const;

    /// Returns selection array, array is empty if nothing is selected
    std::set<size_t> allSelected() const;

    /// Get multiple selection
    template<typename T,
             template <typename, typename > class Container>
    void getSelection( Container<T, std::allocator<T>>& selrows )
    {
        selrows.clear();
        auto allrows = allSelected();
        for(const auto& row: allrows) {
            selrows.push_back(static_cast<T>(row));
        }
    }

protected:
    QScopedPointer<Ui::SelectDialogData> ui;
    QScopedPointer<TableClient> table_client;
    QScopedPointer<MatrixTable> table_view;
    QScopedPointer<SelectModel> internal_model;
    SelectModel *current_model = nullptr;
    /// Mode of using ( see TableFlag )
    int mode_using;

    void init_dialog(const QString& title);
};

} // namespace jsonui

