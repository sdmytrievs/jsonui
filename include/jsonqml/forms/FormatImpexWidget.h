#pragma once

#include <QMainWindow>
#include <QLineEdit>
#include "jsonqml/clients/vertex_client.h"

namespace Ui {
class FormatImpexWidget;
}

namespace jsonqml {

class JsonView;

using  ExecuteImpex_f = std::function<void( std::string impex_data_name,
                                          std::string impex_format_type, std::string impex_format_string,
                                          std::string data_file, bool overwrite )>;

/// \class FormatImpexWidget - Widget to work with Foreign Import/Export File Format
class FormatImpexWidget : public QMainWindow
{
    Q_OBJECT

public slots:
    void CmExportJSON();
    void CmImportJSON();
    void CmSelectDataFile();
    void CmRunScript();

    void CmNew();
    void CmClone();
    void CmRead();
    void CmUpdate();
    void CmDeleteList();

    void CmHelp();
    void CmHelpAbout();

protected slots:
    void updateViewMenu();
    void schema_changed(const QString &text);
    void editor_changed();

public:
    enum ImpexMode {
        editMode = 0,
        runModeImport = 1,
        runModeExport = 2,
        runModeUpdate = 3
    };

    /// Extern impex document schema name ( default "ImpexFormat" )
    static QString impex_shema_name;

    explicit FormatImpexWidget(ImpexMode mode, const QString& run_schema = "", QWidget *parent = nullptr);
    ~FormatImpexWidget();

    /// Set function to execute script
    void setExecuteFunction(ExecuteImpex_f afunc);

protected:
    QScopedPointer<Ui::FormatImpexWidget> ui;
    QScopedPointer<JsonView> json_tree;

    QScopedPointer<VertexClient> impex_client;
    QScopedPointer<JsonClient> recipe_client;
    QString schema_name;
    ExecuteImpex_f run_funct;

    FormatImpexWidget::ImpexMode use_mode = editMode;
    QString parent_schema_name = "";


    void set_actions();
    std::string to_json_node();
    void from_json_node(const jsonio::JsonBase &object);
};

} // namespace jsonui

