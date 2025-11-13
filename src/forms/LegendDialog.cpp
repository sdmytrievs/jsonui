#include <QColorDialog>
#include <QFontDialog>
#include "ui_LegendDialog.h"
#include "jsonqml/clients/chart_client.h"
#include "jsonqml/forms/LegendDialog.h"
#include "jsonqml/forms/GraphDialog.h"
#include "jsonqml/charts/graph_data.h"

namespace jsonqml {

LegendDialog::LegendDialog(GraphDialog* chart_dlg):
    QDialog(chart_dlg),
    ui(new Ui::LegendDialogData),
    graph_dlg(chart_dlg),
    client(chart_dlg->chart_client)
{
    ui->setupUi(this);

    QString title = "Customize Graph: ";
    title += client->chartData()->graphTitle();
    setWindowTitle(title);
    set_chart_data();

    connect( ui->comboBox1, &QComboBox::activated/*highlighted*/, this, &LegendDialog::CmChangeGraphType);
    connect( ui->pBtnChangeFont, &QPushButton::clicked, this, &LegendDialog::CmChangeLabelFont);
    connect( ui->buttomColor, &QPushButton::clicked, this, &LegendDialog::CmBackground);

    QObject::connect( ui->buttonBox, &QDialogButtonBox::accepted, this, &LegendDialog::CmOk);
    QObject::connect( ui->buttonBox, &QDialogButtonBox::rejected, this, &LegendDialog::reject);
    QObject::connect( ui->buttonBox->button(QDialogButtonBox::Apply), &QPushButton::clicked, this, &LegendDialog::CmApply);
    QObject::connect( ui->buttonBox, &QDialogButtonBox::helpRequested, this, &LegendDialog::CmHelp);
}


LegendDialog::~LegendDialog()
{
}


void  LegendDialog::CmChangeGraphType(int /*new_type*/)
{
}

void LegendDialog::CmOk()
{
    if(!apply2dialog()) {
        return;
    }
    accept();
}

void LegendDialog::CmApply()
{
    apply2dialog();
}

void LegendDialog::CmChangeLabelFont()
{
    bool ok;
    QFont selected_font = QFontDialog::getFont(&ok, label_font, this);
    if(ok)  {
        label_font = selected_font;
        ui->pLabelFont->setText(label_font.toString());
    }
}

void LegendDialog::CmBackground()
{
    QColor cl = QColorDialog::getColor(background_color, this);

    if(cl.isValid()) {
        QPalette pl = ui->pColor->palette();
        pl.setColor(QPalette::Window, cl);
        ui->pColor->setPalette(pl);
        background_color = cl;
    }
}

void LegendDialog::CmHelp()
{
    //    helpWin( "LegendDialogJsonui", "" );
}

//------------------------------------------------------------------------------

void LegendDialog::set_chart_data()
{
    auto chart_data = client->chartData();
    ui->pTitle->setText(chart_data->graphTitle());

    old_type = chart_data->graphType();
    ui->comboBox1->setCurrentIndex(old_type);

    ui->pAxisX->setValue(chart_data->axis_typeX);
    ui->pAxisY->setValue(chart_data->axis_typeY);

    ui->pXname->setText(chart_data->xname);
    ui->pYname->setText(chart_data->yname);

    QString str;
    ui->pX0->setValidator(new QDoubleValidator(ui->pX0));
    ui->pX0->setText(str.setNum( chart_data->xMin() ));

    ui->pXn->setValidator(new QDoubleValidator(ui->pXn));
    ui->pXn->setText(str.setNum( chart_data->xMax() ));

    ui->pY0->setValidator(new QDoubleValidator(ui->pY0));
    ui->pY0->setText(str.setNum( chart_data->yMin() ));

    ui->pYn->setValidator(new QDoubleValidator(ui->pYn));
    ui->pYn->setText(str.setNum( chart_data->yMax() ));

    ui->pfX0->setValidator(new QDoubleValidator(ui->pfX0));
    ui->pfX0->setText(str.setNum( chart_data->fxMin() ));

    ui->pfXn->setValidator(new QDoubleValidator(ui->pfXn));
    ui->pfXn->setText(str.setNum( chart_data->fxMax() ));

    ui->pfY0->setValidator(new QDoubleValidator(ui->pfY0));
    ui->pfY0->setText(str.setNum( chart_data->fyMin() ));

    ui->pfYn->setValidator(new QDoubleValidator(ui->pfYn));
    ui->pfYn->setText(str.setNum( chart_data->fyMax() ));

    background_color = chart_data->backgroundColor();
    ui->pColor->setAutoFillBackground(true);
    QPalette pl = ui->pColor->palette();
    pl.setColor(QPalette::Window, background_color);
    ui->pColor->setPalette(pl);

    label_font = chart_data->axisFont();
    ui->pLabelFont->setText(label_font.toString());
}


bool LegendDialog::apply2dialog()
{
    auto chart_data = client->chartData();

    double x0 =  ui->pX0->text().toDouble();
    double xn =  ui->pXn->text().toDouble();
    double y0 =  ui->pY0->text().toDouble();
    double yn =  ui->pYn->text().toDouble();

    double fx0 =  ui->pfX0->text().toDouble();
    double fxn =  ui->pfXn->text().toDouble();
    double fy0 =  ui->pfY0->text().toDouble();
    double fyn =  ui->pfYn->text().toDouble();

    chart_data->title = ui->pTitle->text();
    chart_data->setGraphType(ui->comboBox1->currentIndex());
    chart_data->axis_typeX = ui->pAxisX->value();
    chart_data->axis_typeY = ui->pAxisY->value();
    chart_data->xname = ui->pXname->text();
    chart_data->yname = ui->pYname->text();

    chart_data->setxMin(std::min(x0, xn));
    chart_data->setxMax(std::max(x0, xn));
    chart_data->setyMin(std::min(y0, yn));
    chart_data->setyMax(std::max(y0, yn));

    chart_data->setfxMin(std::min(fx0, fxn));
    chart_data->setfxMax(std::max(fx0, fxn));
    chart_data->setfyMin(std::min(fy0, fyn));
    chart_data->setfyMax(std::max(fy0, fyn));

    chart_data->axis_font = label_font;
    chart_data->setBackgroundColor(background_color);

    graph_dlg->UpdateAll();
    return true;
}

} // namespace jsonui


