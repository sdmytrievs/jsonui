TARGET = jsonui-app
TEMPLATE = app

CONFIG += thread
CONFIG += c++20

#internal help configuraion
#DEFINES  +=  NO_QWEBENGINE
#DEFINES  +=  USE_MARKDOWN
#win32:DEFINES += IMPEX_OFF
DEFINES += IMPEX_OFF
#DEFINES += USE_THRIFT

QT   += core gui widgets
QT   += svg printsupport concurrent
QT   += charts
!contains(DEFINES, NO_QWEBENGINE) {
    QT   += webenginewidgets
}

# Define the directory where jsonio source code is located
JSONIO_DIR =  $$PWD/../jsonio/src
JSONIO_HEADERS_DIR =  $$PWD/../jsonio/include

JSONUI_DIR =  $$PWD/src
JSONUI_HEADERS_DIR =  $$PWD/include

DEPENDPATH   += $$JSONIO_HEADERS_DIR
DEPENDPATH   += $$JSONUI_HEADERS_DIR

INCLUDEPATH   += $$JSONIO_HEADERS_DIR
INCLUDEPATH   += $$JSONUI_HEADERS_DIR

HEADERS += \
    App/JSONUITestMainWindow.h \
    include/jsonqml/arango_database.h \
    include/jsonqml/arango_document.h \
    include/jsonqml/charts/chart_model.h \
    include/jsonqml/charts/graph_data.h \
    include/jsonqml/charts/legend_data.h \
    include/jsonqml/charts/legend_model.h \
    include/jsonqml/charts/xyseries_decorator.h \
    include/jsonqml/clients/csv_client.h \
    include/jsonqml/clients/csv_client_p.h \
    include/jsonqml/forms/CalcDialog.h \
    include/jsonqml/forms/EdgesWidget.h \
    include/jsonqml/forms/FileDialogs.h \
    include/jsonqml/forms/GraphDialog.h \
    include/jsonqml/forms/JsonEditWidget.h \
    include/jsonqml/forms/LegendDialog.h \
    include/jsonqml/forms/PreferencesUI.h \
    include/jsonqml/forms/QueryWidget.h \
    include/jsonqml/forms/SelectDialog.h \
    include/jsonqml/forms/SymbolDialog.h \
    include/jsonqml/forms/TableEditWindow.h \
    include/jsonqml/forms/VertexWidget.h \
    include/jsonqml/forms/chart_view.h \
    include/jsonqml/forms/json_view.h \
    include/jsonqml/forms/query_view.h \
    include/jsonqml/forms/table_view.h \
    include/jsonqml/forms/waitingspinnerwidget.h \
    include/jsonqml/models/base_model.h \
    include/jsonqml/models/json_model.h \
    include/jsonqml/models/query_model.h \
    include/jsonqml/models/schema_model.h \
    include/jsonqml/models/select_model.h \
    include/jsonqml/models/db_keys_model.h \
    include/jsonqml/models/db_query_model.h \
    include/jsonqml/models/csv_model.h \
    include/jsonqml/clients/settings_client.h \
    include/jsonqml/clients/json_client.h \
    include/jsonqml/clients/vertex_client.h \
    include/jsonqml/clients/edge_client.h \
    include/jsonqml/clients/table_client.h \
    include/jsonqml/clients/chart_client.h \
    src/arango_database_p.h \
    src/arango_document_p.h \
    src/charts/markershapes.h \
    src/clients/csv_client_p.h \
    src/clients/json_client_p.h \
    src/clients/vertex_client_p.h \
    src/clients/edge_client_p.h \
    src/clients/table_client_p.h \
    src/forms/ModelLineDialog.h

SOURCES += \
    #main.cpp \
    App/JSONUITestMainWindow.cpp \
    App/main.cpp \
    src/arango_database.cpp \
    src/arango_document.cpp \
    src/charts/chart_model.cpp \
    src/charts/graph_data.cpp \
    src/charts/legend_data.cpp \
    src/charts/legend_model.cpp \
    src/charts/markershapes.cpp \
    src/charts/xyseries_decorator.cpp \
    src/clients/csv_client.cpp \
    src/forms/CalcDialog.cpp \
    src/forms/EdgesWidget.cpp \
    src/forms/FileDialogs.cpp \
    src/forms/GraphDialog.cpp \
    src/forms/JsonEditWidget.cpp \
    src/forms/LegendDialog.cpp \
    src/forms/ModelLineDialog.cpp \
    src/forms/PreferencesUI.cpp \
    src/forms/QueryWidget.cpp \
    src/forms/SelectDialog.cpp \
    src/forms/SymbolDialog.cpp \
    src/forms/TableEditWindow.cpp \
    src/forms/VertexWidget.cpp \
    src/forms/chart_view.cpp \
    src/forms/json_view.cpp \
    src/forms/query_view.cpp \
    src/forms/table_view.cpp \
    src/forms/waitingspinnerwidget.cpp \
    src/models/base_model.cpp \
    src/models/json_model.cpp \
    src/models/query_model.cpp \
    src/models/schema_model.cpp \
    src/models/select_model.cpp \
    src/models/db_query_model.cpp \
    src/models/db_keys_model.cpp \
    src/models/csv_model.cpp \
    src/clients/edge_client.cpp \
    src/clients/settings_client.cpp \
    src/clients/vertex_client.cpp \
    src/clients/json_client.cpp \
    src/clients/table_client.cpp \
    src/clients/chart_client.cpp \

FORMS += \
    App/JSONUITestMainWindow.ui \
    src/forms/CalcDialog.ui \
    src/forms/EdgesWidget.ui \
    src/forms/GraphDialog.ui \
    src/forms/JsonEditWidget.ui \
    src/forms/LegendDialog.ui \
    src/forms/ModelLineDialog.ui \
    src/forms/PreferencesUI.ui \
    src/forms/QueryWidget.ui \
    src/forms/SelectDialog.ui \
    src/forms/SymbolDialog.ui \
    src/forms/TableEditWindow.ui \
    src/forms/VertexWidget.ui

resources.prefix = /$${TARGET}

RESOURCES += src/jsonqml.qrc \
             tools/tools.qrc

win32:LIBS +=  -larango-cpp-static
!win32:LIBS += -larango-cpp

OBJECTS_DIR   = obj
MOC_DIR = tmp
UI_DIR  = $$MOC_DIR

RESOURCES += jsonui.qrc


include($$JSONIO_DIR/jsonio.pri)


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target



