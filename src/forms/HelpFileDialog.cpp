#include <QFileInfo>
#include "ui_HelpFileDialog.h"
#include "HelpFileDialog.h"
#include "jsonqml/clients/help_client.h"
#include "jsonqml/clients/chart_client.h"
#include "jsonqml/forms/FileDialogs.h"

namespace jsonqml {

HelpFileDialog::HelpFileDialog(int type, QWidget *parent):
    QDialog(parent),
    ui(new Ui::HelpFileDialog)
{
    ui->setupUi(this);

    switch(type) {
    case DocPagesData::Markdown:
        setWindowTitle("Insert new Markdown File");
        filters = QString("Markdown Files (*.markdown *.md *.mdown);;All Files (*)");
        break;
    case DocPagesData::Image:
        setWindowTitle("Select Image File");
        filters = ChartClient::imageFilters().join(";;")+";;All Files (*)";
        break;
    }
}

HelpFileDialog::~HelpFileDialog()
{}


void HelpFileDialog::SelectFile()
{
    auto copyfile= ui->file->text();
    if(ChooseFileOpen(this, copyfile, "Select Original File ", filters)) {
        ui->file->setText(copyfile);
    }
}

QString HelpFileDialog::linkName()
{
    QString new_file = ui->name->text();
    if(new_file.isEmpty()) {
        QString source = sourceFile();
        QFileInfo finfo(source);
        new_file = finfo.baseName();
    }
    return new_file;
}

QString HelpFileDialog::sourceFile()
{
    return ui->file->text();
}

} // namespace jsonui
