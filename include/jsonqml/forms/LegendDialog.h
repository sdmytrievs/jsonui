#pragma once

#include <QDialog>

namespace Ui {
class LegendDialogData;
}

namespace jsonqml {

class ChartClient;
class GraphDialog;

/// \class LegendDialog dialog to edit plotting system description
class LegendDialog :public QDialog
{
    Q_OBJECT

protected slots:
    void CmBackground();
    void CmChangeLabelFont();
    void CmChangeGraphType(int);

    void CmApply();
    void CmHelp();
    void CmOk();

public:
    LegendDialog(GraphDialog* chart_dlg);
    ~LegendDialog();

private:
    QScopedPointer<Ui::LegendDialogData> ui;
    GraphDialog* graph_dlg;
    ChartClient* client;

    QColor background_color;
    QFont label_font;
    int old_type;

    bool apply2dialog();
    void set_chart_data();
};

} // namespace jsonui

