#pragma once

#include <QDialog>
#include <QItemDelegate>

namespace jsonio {
class FieldDef;
}

namespace Ui {
class FieldsSelectDialog;
}

namespace jsonqml {

class SelectFieldsModel;

/// \class SchemaSelectDialog class provides a dialog that allow users to select
/// items from thrift schema fields list
class SelectFieldsDialog : public QDialog
{
    Q_OBJECT

protected slots:
    void CmHelp();
    void updateSelection();

public:
    /// Multiple select constructor
    /// Construct tree view of fields thrift schema
    /// \param schema_name.
    /// \param def_fieldsList - default selection
    SelectFieldsDialog(const QString& title,
                       const QString& schema_name,
                       const std::vector<std::string>& def_fields_list,
                       QWidget* parent=nullptr);

    /// Single select constructor.
    /// Construct tree view of fields thrift schema schema_name.
    SelectFieldsDialog(const QString& title,
                       const QString& schema_name,
                       QWidget* parent=nullptr);

    /// Destructor
    virtual ~SelectFieldsDialog();

    /// Returns single selection
    /// \returns empty string if nothing selected
    std::string selIndex(const jsonio::FieldDef** flddata);

    /// Returns selection array
    /// array is empty if nothing is selected
    std::vector<std::string> allSelected();

private:
    QScopedPointer<Ui::FieldsSelectDialog> ui;
    bool multi_selection;
    QString schema_name;
    std::vector<std::string> def_fields;
    SelectFieldsModel *model_schema = nullptr;

    void setup_window();
};

/// \class FieldsDelegate
/// individual items in views are rendered and edited using delegates
class FieldsDelegate: public QItemDelegate
{
    Q_OBJECT

public:
    FieldsDelegate(QObject* parent = nullptr):
        QItemDelegate(parent)
    {}
    QWidget *createEditor(QWidget *parent,
                          const QStyleOptionViewItem &option,
                          const QModelIndex &index) const override;
};

} // namespace jsonui

