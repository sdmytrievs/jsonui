#pragma once

#include <QDialog>
#include <QButtonGroup>
#include "jsonqml/charts/legend_data.h"

namespace Ui {
class SymbolDialogData;
}

namespace jsonqml {

/// \class SymbolDialog - dialog to edit one plot line descriptions
class SymbolDialog: public QDialog
{
    Q_OBJECT

protected slots:
    virtual void CmSelectColor();

public:
    SymbolDialog(const SeriesLineData& aData, QWidget* parent = nullptr);
    virtual ~SymbolDialog();

    SeriesLineData& plotLine();

private:
    SeriesLineData linedata;
    QScopedPointer<Ui::SymbolDialogData> ui;
    QScopedPointer<QButtonGroup> smb_group;

};

} // namespace jsonui

