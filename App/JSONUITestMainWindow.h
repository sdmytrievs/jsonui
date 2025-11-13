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

    // File
    void CmSetWorkDir() {}
    void CmNewVertex() {}
    void CmNewEdge() {}
    void CmNewBson() {}
    void CmNewCollection() {}
    void CmNewCsv();
    void CmNewImpexFormat() {}

    // Help
    void CmHelpAbout() {}
    void CmHelpAuthors() {}
    void CmHelpLicense() {}
    void CmHelpContens() {}

    // update after change preferences
    void updateViewMenu();
    void updateModel() {}
    void updateTable() {}
    void updateDB() {}
    void closeAll() {}

    void onDeleteVertex() {}
    void onDeleteEdge() {}
    void onLoadGraph() {}

    void CmSettingth();
public:
    explicit TJSONUITestWin(QWidget *parent = nullptr);
    ~TJSONUITestWin();

    /// Open new EdgesWidget  or JSONUIWidget  window
    //void OpenNewWidget( bool isVertex, const std::string& testschema,
    //                     const std::string& recordkey, const jsonio17::DBQueryBase& query  );

private:

    Ui::TJSONUITestWin *ui;
    QScopedPointer<jsonqml::TableEditWidget> csv_window;
    // Internal data
    //std::list<jsonui17::BaseWidget*> jsonuiWindows;
    //jsonui17::OnCloseEvent_f onCloseEvent;
    //jsonui17::ShowVertexEdge_f showWidget;

    // Work functions
    void set_actions();
    //void closeEvent(QCloseEvent* e);

};

#endif // TMAINWINDOW_H
