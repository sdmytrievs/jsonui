#pragma once

#include <QMainWindow>
#include <QLineEdit>
#include <QStringListModel>
#include "jsonio/dbquerybase.h"

namespace Ui {
class QueryWidget;
}

namespace jsonqml {

class VertexClient;
class QueryView;
class QueryModel;

/// \class QueryWidget window to insert/edit  database query
class QueryWidget : public QMainWindow
{
    Q_OBJECT

signals:
    void queryChanged(jsonio::DBQueryDef query);

public slots:
    // Files
    void CmSaveQuery();
    void CmLoadQuery();

    // Record
    void CmClone();
    void CmRead();
    void CmUpdate();
    void CmDeleteList();

    // Where part
    void CmWhereApply();
    void CmWhereRestore();

    // Select part
    void CmSelectFields();
    void CmAddSelectFields();
    void CmExecute();
    void CmApplay();

    // Help
    void CmHelpContens();
    void CmHelpAbout();

protected slots:
    void AQL_toggled(bool checked);
    void schema_changed(const QString& text);
    void select_restore();
    void editor_changed();

public:
    /// Extern query document schema name ( default "Query" )
    static QString query_shema_name;

    explicit QueryWidget(const QString& title,
                         const QStringList& schemas,
                         const jsonio::DBQueryBase& query,
                         const jsonio::values_t& query_fields,
                         QWidget *parent);
    ~QueryWidget();

    void updateQuery(const QStringList& schemas,
                     const jsonio::DBQueryBase& query,
                     const jsonio::values_t& query_fields);
    const jsonio::DBQueryDef& getQuery();
    QString getSchema() const;

protected:
    QScopedPointer<Ui::QueryWidget> ui;
    QScopedPointer<QLineEdit> qId;

    QScopedPointer<VertexClient> query_client;
    QScopedPointer<QueryView> query_edit;
    QueryModel* query_model;

    QStringListModel* lstmodel = nullptr;

    QString schema_name;
    jsonio::DBQueryBase default_query;
    jsonio::values_t   default_query_fields;
    jsonio::DBQueryDef query_description;

    void set_actions();
    void show_query();
    std::string to_json_node();
    void from_json_node(const jsonio::JsonBase &object);
    const jsonio::DBQueryDef &get_query();
    jsonio::DBQueryBase::QType get_query_type() const;
    jsonio::DBQueryBase new_schema_query();
};

} // namespace jsonui

