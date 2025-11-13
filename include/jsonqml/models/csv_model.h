
#ifndef CSVMODEL_H
#define CSVMODEL_H

#include "jsonqml/models/select_model.h"

namespace jsonqml {

/// \class CSVModel
/// \brief The CSVModel class provides a csv like result sets.
///
/// CSVModel is a high-level interface for executing selection statements and traversing the result set.
/// It can provide data to view classes such as QTableView.
class CSVModel: public SelectModel
{
    Q_OBJECT

public:
    explicit CSVModel(QObject *parent = nullptr);
    explicit CSVModel(std::string &&csv_data, QObject *parent = nullptr);
    virtual ~CSVModel();

    void resetTable(std::vector<std::vector<std::string>>&& table_data,
                const std::vector<std::string>& header_data) override;

    void setCsvString(std::string&& value_csv);
    std::string getCsvString();

    void setXColumns(const std::vector<int>& clmns);
    void setYColumns(const std::vector<int>& clmns);
    Q_INVOKABLE bool is_number(int section) {
        if(section>0 && section<is_number_clmn.size()) {
            return is_number_clmn[section];
        }
        return false;
    }

    QVariant data(const QModelIndex& item, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role) override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;


protected:
    /// Numerical column condition
    std::vector<bool> is_number_clmn;
    /// Abscissa columns list
    std::vector<int> x_clmns;
    /// Ordinate columns list
    std::vector<int> y_clmns;

    void build_table(std::string &&value_csv,
                     std::vector<std::vector<std::string>>& table_data,
                     std::vector<std::string>& header_data);
    void reset_clmn_type();
    void matrix_from_csv_string(std::string&& value_csv);
    std::string matrix_to_csv_string();
};


}

#endif // CSVMODEL_H
