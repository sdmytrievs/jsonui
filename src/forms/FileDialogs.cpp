#include <QFileInfo>
#include <QFileDialog>
#include "jsonqml/clients/settings_client.h"
#include "jsonqml/forms/FileDialogs.h"

namespace jsonqml {

const QString jsonFilters = "JSON files (*.json);;All files (*.*)";
const QString textFilters = "All files (*);;Text files (*.txt)";

#ifndef IMPEX_OFF
const QString domFilters =  "JSON files (*.json);;YAML files (*.yaml);;XML files (*.xml);;All files (*.*)";
const QString yamlFilters = "YAML files (*.yaml);;All files (*.*)";
const QString xmlFilters =  "XML files (*.xml);;All files (*.*)";
#else
const QString domFilters =  jsonFilters;
#endif

const QString csvFilters = "CSV files (*.csv);;All files (*)";

bool ChooseFileOpen(QWidget* par, QString& file_path, const QString& title, const QString& filter)
{
    QString path = uiSettings().addWorkDir(file_path);
    QString filt;
    if(filter.isEmpty()) {
        filt = "All files (*)";
    }
    else {
        filt = filter;
    }

    QString fn = QFileDialog::getOpenFileName(par, title, path, filt, nullptr, QFileDialog::DontConfirmOverwrite);
    if(!fn.isEmpty()) {
        file_path = uiSettings().handleFileChosen(fn);
        return true;
    }
    else  {
        file_path = "";
        return false;
    }

}

bool ChooseFileSave(QWidget* par, QString& file_path, const QString& title, const QString& filter, const QString& schemafilter)
{
    QString path = uiSettings().addWorkDir(file_path);
    QString filt;
    if(!schemafilter.isEmpty()) {
        filt += QString("Schema (%1)").arg(schemafilter)+";;";
    }
    if(filter.isEmpty()) {
        filt += "All files (*.*)";
    }
    else {
        filt += filter;
    }
    QString selectedFilter;
    QString fn = QFileDialog::getSaveFileName( par, title, path, filt, &selectedFilter, QFileDialog::DontConfirmOverwrite);
    if(!fn.isEmpty())  {
        QFileInfo flinfo(fn);
        if(flinfo.suffix().isEmpty()) { // ??? add default filter solwing for linux
            int posb = selectedFilter.lastIndexOf(".");
            if(!schemafilter.isEmpty()) {
                posb = selectedFilter.indexOf(".");
            }
            int pose = selectedFilter.indexOf(")", posb);
            if( posb > 0 && pose > posb ) {
                QString ext = selectedFilter.mid(posb+1, pose-posb-1);
                fn += "."+ext;
            }
        }
        file_path = uiSettings().handleFileChosen(fn);
        return true;
    }
    else  {
        file_path  = "";
        return false;
    }
}

} // namespace jsonui


