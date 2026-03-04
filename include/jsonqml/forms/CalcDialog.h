#pragma once

#include <QDialog>
#include <QButtonGroup>

namespace Ui {
class CalcDialogData;
}

namespace jsonqml {

class CalcDialog : public QDialog
{
    Q_OBJECT
    
protected slots:
    void setMode(int);
    void ok();
    void unaryMode();
    void help();
    
public:
    explicit CalcDialog(QWidget* parent);
    virtual ~CalcDialog();

    /// The function returns a result of calculating
    /// the selected function from a given value
    double computeFunction(double val);

    /// The function returns a string representation of the result of calculating
    /// the selected function from a given value
    QString computeFunctionString(double val);

    /// Get the selected function and an additional parameter
    int extructInternal(double& val);

    /// The function returns a string representation of expression of
    /// the selected function with a given parameter text
    QString expression(const QString& val_text);

private:
    Ui::CalcDialogData *ui;
    QButtonGroup *all_buttons;

    double extruct_value();
};

} // namespace jsonui

