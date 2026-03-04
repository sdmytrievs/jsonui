#ifndef TMAINWINDOW_H
#define TMAINWINDOW_H

//#include <string>
#include <QMainWindow>
//#include <QSettings>



//#include "jsonui17/BaseWidget.h"

namespace Ui {
class TJSONUITestWin;
}

namespace jsonqml {
class  TableEditWidget;
}


class TJSONUITestWin : public QMainWindow
{
    Q_OBJECT

public slots:
    void showErrorMessage();
    void updateViewMenu();

    void CmSettingth();
    void CmSetWorkDir() {}
    void CmNewVertex();
    void CmNewEdge();
    void CmNewJson();
    void CmNewCollection();
    void CmNewCsv();
    void CmNewImpexFormat();

    // Help
    void CmHelpAbout();
    void CmHelpAuthors();
    void CmHelpLicense();
    void CmHelpContens();


public:
    explicit TJSONUITestWin(QWidget *parent = nullptr);
    ~TJSONUITestWin();

private:
    Ui::TJSONUITestWin *ui;
    QScopedPointer<jsonqml::TableEditWidget> csv_window;

    void set_actions();
    void closeEvent(QCloseEvent* e);
};

#endif // TMAINWINDOW_H
