#pragma once

#include <QDialog>

namespace Ui {
class ModelLineDialog;
}

namespace jsonqml {

class JsonBaseModel;

/// \class ModelLineDialog provides a dialog that allow users to difine json object
class ModelLineDialog : public QDialog
{
    Q_OBJECT

protected slots:
    void setName(const QString &field_name);
    void setType(const QString &field_type);

public:
    explicit ModelLineDialog(const QModelIndex& index, QWidget *parentWidget = nullptr);
    explicit ModelLineDialog(bool, const QModelIndex& index, QWidget *parentWidget = nullptr);

    ~ModelLineDialog();

    /// Get object name
    QString objectName() const;

    /// Get object type
    QString objectType() const;

    /// Get object size (if object is array type)
    int arraySize() const;

private:
    Ui::ModelLineDialog *ui;

    /// Field schema name
    QString field_top_schema_name;
};

} // namespace jsonqml

