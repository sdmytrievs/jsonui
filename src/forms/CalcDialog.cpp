
#include "ui_CalcDialog.h"
#include "jsonqml/forms/CalcDialog.h"

namespace jsonqml {

const double IPNC_DBL_MAX = 1e37;
const double IPNC_DBL_MIN = 1e-37;
const double IPNC_DBL_MAX_10_EXP = 37.;
const double IPNC_DBL_MIN_10_EXP = -37.;
const double IPNC_DBL_MAX_EXP = 85.195648;
const double IPNC_DBL_MIN_EXP = -85.195648;
const std::string emptiness("---");
const double DOUBLE_EMPTY = 2.2250738585072014e-308;
bool IsDoubleEmpty( const double v );
bool IsDoubleEmpty( const double v )
{
    return (v>0. && v <= DOUBLE_EMPTY);
}

CalcDialog::CalcDialog(QWidget* parent):
    QDialog(parent),
    ui(new Ui::CalcDialogData)
{
    ui->setupUi(this);
    
    all_buttons = new QButtonGroup( ui->pButtonBox );
    all_buttons->addButton(ui->pAssignBtn, 0);
    all_buttons->addButton(ui->PushButton2, 1);
    all_buttons->addButton(ui->PushButton3, 2);
    all_buttons->addButton(ui->PushButton4, 3);
    all_buttons->addButton(ui->PushButton5, 4);
    all_buttons->addButton(ui->PushButton6, 5);
    all_buttons->addButton(ui->PushButton7, 6);
    all_buttons->addButton(ui->PushButton8, 7);
    all_buttons->addButton(ui->PushButton9, 8);
    all_buttons->addButton(ui->PushButton10, 9);
    all_buttons->addButton(ui->PushButton11, 10);
    all_buttons->addButton(ui->PushButton12, 11);
    all_buttons->addButton(ui->PushButton13, 12);
    all_buttons->addButton(ui->PushButton14, 13);
    all_buttons->addButton(ui->PushButton15, 14);
    all_buttons->addButton(ui->pClearBtn, 15);
    ui->pAssignBtn->setChecked( true);

    QObject::connect(all_buttons, &QButtonGroup::idClicked, this, &CalcDialog::setMode);
    QObject::connect(ui->PushButton1_3, &QPushButton::clicked, this, &CalcDialog::ok);
    QObject::connect(ui->PushButton1_2, &QPushButton::clicked, this, &QDialog::reject);
    //  QObject::connect(ui->PushButton1_2, &QPushButton::clicked, this, &CalcDialog::unaryMode);
    QObject::connect(ui->PushButton42, &QPushButton::clicked, this, &CalcDialog::help );
}

CalcDialog::~CalcDialog()
{
    delete all_buttons;
    delete ui;
}

void CalcDialog::unaryMode()
{
    ui->pValue->setEnabled(false);
}

void CalcDialog::setMode(int id)
{
    if(id <= 5) {
        ui->pValue->setEnabled(true);
    }
    else {
        ui->pValue->setEnabled(false);
    }
}

void CalcDialog::ok()
{
    accept();
}

void CalcDialog::help()
{
    /// helpWin( "CalcDialogJsonui", "" );
}


double CalcDialog::computeFunction(double val)
{
    double ret = val;
    int ii = all_buttons->checkedId();
    double val2 = extruct_value();

    switch(ii) {
    case 0:
        ret = val2;
        break;
    case 1:
        ret = val + val2;
        break;
    case 2:
        ret = val - val2;
        break;
    case 3:
        ret = val * val2;
        break;
    case 4:
        if(fabs(val2) >=  IPNC_DBL_MIN) { // Attempt of zerodivide!
            ret = val / val2;
        }
        break;
    case 5:
        if(!(fabs(val)<IPNC_DBL_MIN || fabs(val)>IPNC_DBL_MAX
              || fabs(val2) < IPNC_DBL_MIN_10_EXP
              || fabs(val2) > IPNC_DBL_MAX_10_EXP)) { // Attempt of pow() argument out of range
            ret = pow(val, val2);
        }
        break;
    case 6:
        if(fabs(val) >= IPNC_DBL_MIN) { // Attempt of zerodivide!
            ret = 1/val;
        }
        break;
    case 7:
        if(val >= IPNC_DBL_MIN) { // Attempt of sqrt() argument <= 0
            ret = sqrt(val);
        }
        break;
    case 8:
        if(val >= IPNC_DBL_MIN) {
            ret = log10(val);
        }
        break;
    case 9:
        if(val >= IPNC_DBL_MIN) {
            ret = log(val);
        }
        break;
    case 10:
        if( !( fabs(val) < IPNC_DBL_MIN_10_EXP
              || fabs(val) > IPNC_DBL_MAX_10_EXP) ) { // Attempt of pow() argument out of range
            ret = pow(10.,val);
        }
        break;
    case 11:
        if( !(val < IPNC_DBL_MIN_EXP || val > IPNC_DBL_MAX_EXP) ) { // Attempt of exp() argument out of range
            ret =  exp(val);
        }
        break;
    case 12:
        ret = sin(val);
        break;
    case 13:
        ret = cos(val);
        break;
    case 14:
        ret = tan(val);
        break;
    case 15:
        ret = DOUBLE_EMPTY; //No Data value
    }

    return ret;
}

QString CalcDialog::computeFunctionString(double val)
{
    double ret = computeFunction(val);
    QString retstr;
    if( IsDoubleEmpty( ret ) ) {
        retstr = emptiness.c_str();
    }
    else {
        retstr = QString::number( ret, 'g', 6 );//QVariant(ret).toString();
    }
    return retstr;
}

int CalcDialog::extructInternal(double& val)
{
    val = extruct_value();
    return all_buttons->checkedId();
}


QString CalcDialog::expression(const char * val_text)
{
    int ii, pos;
    QString res;

    ii = all_buttons->checkedId();
    QString str = "";
    if(ii <= 5)  {
        str = ui->pValue->currentText();
        pos = str.indexOf('(');
        if( pos >= 0 ) {
            str.truncate(pos);
        }
        if( str.isEmpty() || (str.indexOf(emptiness.c_str()) != -1) || str[0] == '`') {
            str = "empty()";
        }
    }

    switch(ii)
    {
    case 0:
        res = val_text; break;
    case 1:
        res = QString("%1+%2").arg(val_text, str); break;
    case 2:
        res = QString("%1-%2").arg(val_text, str); break;
    case 3:
        res = QString("%1*%2").arg(val_text, str);; break;
    case 4:
        res = QString("%1/%2").arg(val_text, str);; break;
    case 5:
        res = QString("%1^%2").arg(val_text, str);; break;
    case 6:
        res = QString( "( %1 ? (1/%1) : empty() )" ).arg(val_text) ; break;
    case 7:
        res = QString( "( %1 > 0. ? sqrt(%1) : empty() )" ).arg(val_text) ; break;
    case 8:
        res = QString( "( %1 > 0. ? lg(%1) : empty() )" ).arg(val_text) ; break;
    case 9:
        res = QString( "( %1 > 0. ? ln(%1) : empty() )" ).arg(val_text) ; break;
    case 10:
        res = QString("10.^%1").arg(val_text); break;
    case 11:
        res = QString("exp(%1)").arg(val_text); break;
    case 12:
        res = QString("sin(%1)").arg(val_text); break;
    case 13:
        res = QString("cos(%1)").arg(val_text); break;
    case 14:
        res = QString("tan(%1)").arg(val_text); break;
    case 15:
        res = "empty()"; //No Data value
    }
    return res;
}

double CalcDialog::extruct_value()
{
    int ii, pos;

    ii = all_buttons->checkedId();
    double val=0.;
    if(ii <= 5) {
        QString str = ui->pValue->currentText();
        pos = str.indexOf('(');
        if(pos >= 0) {
            str.truncate(pos);
        }
        if( str.isEmpty() || (str.indexOf(emptiness.c_str()) != -1) || str[0] == '`' ) {
            val = DOUBLE_EMPTY;
        }
        else {
            val = str.toDouble();  /// check for error
        }
    }
    return val;
}


} // namespace jsonui

