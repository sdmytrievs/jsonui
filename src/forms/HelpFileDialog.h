#pragma once

#include <QDialog>
#include <QString>

namespace Ui {
class HelpFileDialog;
}

namespace jsonqml {

/// Declaration of HelpFileDialog widget - dialog for add existing help pages
/// in markdown format or images to database
class HelpFileDialog : public QDialog
{
    Q_OBJECT

public slots:
    void SelectFile();

public:
    explicit HelpFileDialog(int type, QWidget *parent = nullptr);
    ~HelpFileDialog();

    QString linkName();
    QString sourceFile();

private:
    QScopedPointer<Ui::HelpFileDialog> ui;
    QString filters;
};

} // namespace jsonui


