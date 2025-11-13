#include "JSONUITestMainWindow.h"
#include "jsonio/io_settings.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    jsonio::JsonioSettings::settingsFileName = "jsonui-config.json";

    QApplication a(argc, argv);
    a.setWindowIcon(QIcon(":/menu/Icons/jsonui-logo-icon.png"));
    TJSONUITestWin w;
    w.show();

    return a.exec();
}
