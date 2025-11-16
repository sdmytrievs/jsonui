#pragma once

#include <QMainWindow>

namespace Ui {
class JsonEditWidget;
}

namespace jsonqml {

class JsonClient;
class JsonView;

/// \class JsonEditWidget - window to edit/create free format json or
/// schema based json
class JsonEditWidget : public QMainWindow
{
    Q_OBJECT

public slots:
    /// Set empty or default json document
    void CmNew();
    /// Clone json document (clear _id, _rev ... )
    void CmClone();

    void CmExportJSON();
    void CmImportJSON();
#ifndef IMPEX_OFF
    void CmExportYAML();
    void CmImportYAML();
    void CmExportXML();
    void CmImportXML();
#endif

    /// Save current document to database as new
    void CmCreateInsert();
    /// Read new document from database
    void CmRead();
    /// Save current document to database
    void CmUpdate();
    /// Delete current document from database
    void CmDelete();

public:
    explicit JsonEditWidget(const QString& aschema_name = "",
                            const QString& collect_name = "",
                            QWidget *parent = nullptr );
    ~JsonEditWidget();

//    void setQueryEdited(QueryWidget* ) override {}

private:
    QScopedPointer<Ui::JsonEditWidget> ui;
    QScopedPointer<JsonClient> json_client;
    QScopedPointer<JsonView> json_tree;
    QString schema_name;
    QString collection_name;

    void set_actions();
};

} // namespace jsonui

