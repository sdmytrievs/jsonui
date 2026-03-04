#include <QColorDialog>
#include <QRadioButton>
#include "ui_SymbolDialog.h"
#include "jsonqml/forms/SymbolDialog.h"
#include "../charts/markershapes.h"

namespace jsonqml {

SymbolDialog::SymbolDialog(const SeriesLineData& aData, QWidget* parent):
    QDialog(parent), linedata(aData),
    ui(new Ui::SymbolDialogData)
{
    ui->setupUi(this);

    smb_group.reset(new QButtonGroup(this));
    smb_group->setExclusive(true);

    // make symbol icons by program
    QRadioButton *pp;
    SeriesLineData plot_line( "Scale",  0, 40, 0, 0, 0, 60 );
    for(int ii=0; ii<20; ++ii) {
        pp = new QRadioButton(ui->pSymbols);
        plot_line.setMarkerShape(ii);
        pp->setIcon(markerShapeIcon(plot_line));
        smb_group->addButton(pp, ii);
        ui->gridLayout->addWidget(pp, ii/5, ii%5, 1, 1);
    }

    ui->pColor->setAutoFillBackground(true);
    QPalette pl = ui->pColor->palette();
    pl.setColor(QPalette::Window, linedata.color());
    ui->pColor->setPalette(pl);

    ui->pLineSize->setValue(linedata.penSize());
    ui->pLineStyle->setCurrentIndex(linedata.penStyle());
    ui->pSpline->setChecked(linedata.spline());
    ui->pSymbolSize->setValue(linedata.markerSize());
    smb_group->button(linedata.markerShape()%20)->setChecked(true);

    QString title  = "Customize plot curve: ";
    title += linedata.name();
    setWindowTitle(title);

    QObject::connect( ui->buttonBox, &QDialogButtonBox::accepted, this, &SymbolDialog::accept);
    QObject::connect( ui->buttonBox, &QDialogButtonBox::rejected, this, &SymbolDialog::reject);
    QObject::connect( ui->buttomColor, &QPushButton::clicked, this, &SymbolDialog::CmSelectColor);
}

SymbolDialog::~SymbolDialog()
{ }


SeriesLineData& SymbolDialog::plotLine()
{
    int type = smb_group->checkedId();
    QColor cl = ui->pColor->palette().color(QPalette::Window); //backgroundColor();
    linedata.setChanges(type, ui->pSymbolSize->value(), ui->pLineSize->value(),
                        ui->pLineStyle->currentIndex(), ui->pSpline->isChecked(), cl);
    return linedata;
}

void SymbolDialog::CmSelectColor()
{
    QPalette pl = ui->pColor->palette();
    QColor cl = QColorDialog::getColor(pl.color(QPalette::Window), this);

    if(cl.isValid()) {
        pl.setColor( QPalette::Window, cl);
        ui->pColor->setPalette(pl);
    }
}

} // namespace jsonui


