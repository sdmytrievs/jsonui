#ifndef CSVCLIENT_P_H
#define CSVCLIENT_P_H

#include "jsonqml/models/csv_model.h"
#include "table_client_p.h"

namespace jsonqml {

class CSVClientPrivate : public TableClientPrivate
{
    Q_DISABLE_COPY_MOVE(CSVClientPrivate)

public:
    explicit CSVClientPrivate(int mode):
        TableClientPrivate(nullptr, mode)
    {
        init_csv();
    }
    ~CSVClientPrivate() {}

    void read_files(const QString &path) override
    {
        read_CSV(path);
    }
    void save_files(const QString &path) override
    {
        save_CSV(path);
    }

    virtual bool is_number(int section)
    {
        return csv_model_data->is_number(section);
    }

protected:
    QSharedPointer<CSVModel> csv_model_data;

    friend class CSVClient;

    void init_csv();
    void read_CSV(const QString &path);
    void save_CSV(const QString &path);
};

}

#endif // CSVCLIENT_P_H
