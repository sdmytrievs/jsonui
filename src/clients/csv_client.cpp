#include <QFile>
#include <QFileInfo>
#include "jsonqml/clients/settings_client.h"
#include "jsonqml/clients/csv_client.h"
#include "csv_client_p.h"

namespace jsonqml {


void CSVClientPrivate::init_csv()
{
    csv_model_data.reset(new CSVModel());
    current_model = csv_model_data.get();

    if(sorting_enabled()) {
        sort_proxy_model.reset(new SortFilterProxyModel());
        sort_proxy_model->setSourceModel(csv_model_data.get());
    }
}

void CSVClientPrivate::read_CSV(const QString &path)
{
    QFile file(path);
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        uiSettings().setError("could not open file");
        return;
    }
    QByteArray ba = file.readAll();
    std::string csv_srtring =ba.toStdString();
    csv_model_data->setCsvString(std::move(csv_srtring));
    file.close();
}

void CSVClientPrivate::save_CSV(const QString &path)
{
    auto csv_string = csv_model_data->getCsvString();
    QFile file(path);
    if(!file.open(QIODevice::WriteOnly)) {
        uiSettings().setError("could not open file");
        return;
    }
    QTextStream stream(&file);
    stream << csv_string.c_str();
    file.close();
}

//--------------------------------------------------------------------------

CSVClient::CSVClient(CSVClientPrivate* impl, QObject *parent):
    TableClient(impl, parent)
{
}

CSVClient::CSVClient(int mode, QObject *parent):
    CSVClient(new CSVClientPrivate(mode), parent)
{
}

CSVClient::~CSVClient() {}

void CSVClient::setCsvFile(const QString& path)
{
    QFileInfo file(path);
    csv_file = file.fileName();
    emit csvFileChanged();
}

void CSVClient::readCSV(const QString &url)
{
    if(url.isEmpty()) {
        return;
    }
    uiSettings().setError(QString());
    try {
        // save work path
        auto path = uiSettings().handleFileChosen(url);
        impl_func()->read_files(path);
        setCsvFile(path);
    }
    catch(std::exception& e) {
        uiSettings().setError(e.what());
    }
}

void CSVClient::saveCSV(const QString &url)
{
    if(url.isEmpty()) {
        return;
    }
    uiSettings().setError(QString());
    try {
        auto path = uiSettings().handleFileChosen(url);
        impl_func()->save_files(path);
        setCsvFile(path);
    }
    catch(std::exception& e) {
        uiSettings().setError(e.what());
    }
}

}
