#pragma once

#include <QMainWindow>
#include <QLineEdit>
#include "jsonio/dbschemadoc.h"

namespace Ui {
class QueryWidget;
}

namespace jsonqml {

class VertexClient;
class QueryView;
class QueryModel;
class StringVectorModel;

/// \class QueryWidget window to insert/edit  database query
class QueryWidget : public QMainWindow
{
    Q_OBJECT

public slots:
    // Files
    void CmSaveQuery();
    void CmLoadQuery();
    //void CmExecute();
    //void CmApplay();

    // Select part
    void CmSelectFields();
    void CmAddSelectFields();

    // Where part
    void CmWhereApply();
    void CmWhereRestore();
    void CmAddWhereFields();

    // Record
    void CmClone();
    void CmRead();
    void CmUpdate();
    void CmDeleteList();

    // Help
    void CmHelp();
    void CmHelpAbout();

protected slots:
    void AQL_toggled(bool checked);
    void schema_changed(const QString& text);
    void select_restore();

public:
    /// Extern impex collection name ( default "queries" )
    static std::string query_collection_name;
    std::string query_schema;

    explicit QueryWidget(const QString& title, const QStringList& schemas,
                         const jsonio::DBQueryDef& query, QWidget *parent );
    ~QueryWidget();

    const jsonio::DBQueryDef& getQuery();
    std::string getSchema() const;

protected:
    QScopedPointer<Ui::QueryWidget> ui;
    QScopedPointer<VertexClient> query_client;
    QScopedPointer<QueryView> query_edit;
    QScopedPointer<QLineEdit> qId;
    StringVectorModel* lstmodel = nullptr;
    QueryModel* query_model;

    QString schema_name;
    jsonio::DBQueryDef default_query;
    jsonio::DBQueryDef query_description;

    bool select_line(std::string& fldpath, const jsonio::FieldDef** flddata);
    void set_actions();
    void show_query();
    std::string to_json_node();
    void from_json_node(const jsonio::JsonBase &object);
    const jsonio::DBQueryDef &get_query();
    jsonio::DBQueryBase::QType get_query_type() const;
    jsonio::DBQueryBase new_schema_query();
};

} // namespace jsonui

