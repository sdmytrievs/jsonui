#pragma once

#include <QMainWindow>

namespace Ui {
class VertexWidget;
}
class WaitingSpinnerWidget;


namespace jsonqml {

class VertexClient;
class JsonView;
class TableClient;
class DBKeysTable;

/// \class VertexWidget - window to work with Vertexes (TDBGraph)
class VertexWidget : public QMainWindow
{
    Q_OBJECT

public slots:
    void openDocument(const QString& key_document);

    // File
    void CmNew();
    void CmClone();
    void CmExportJSON();
    void CmImportJSON();

    // Record
    void CmRead();
    void CmUpdate();
    void CmDelete();
    void CmDeleteSelect();
    void CmDisplaySearchResult() {} /// !!!
    void CmSearchQuery() {} /// !!!!
    void CmBackupQueriedRecordstoFile();
    void CmRestoreMultipleRecordsfromFile();
    void CmBackupGraphtoFile();
    void CmRestoreGraphfromFile();

#ifndef IMPEX_OFF
    void CmImportFormat();
    void CmUpdateFormat();
    void CmExportFormat();
#endif

    // View
    void CmFollowInEdges();
    void CmFollowOutEdges();

protected slots:
    void setExecuting(bool exec);
    void updateViewMenu();
    void openRecordKey(const QModelIndex& index);

public:
    explicit VertexWidget(const QString& aschema, const QString& document_id="", QWidget *parent = nullptr);
    ~VertexWidget();

    /// Update query
    /// void setQueryEdited( QueryWidget* queryW  ) override;
    std::string currentKey() const;

private:
    QScopedPointer<Ui::VertexWidget> ui;
    QScopedPointer<VertexClient> vertex_client;
    QScopedPointer<JsonView> json_tree;
    QScopedPointer<TableClient> keys_table_client;
    QScopedPointer<DBKeysTable> keys_view;
    QString schema_name;

    /// Dialog when waiting db_rquest command ends
    WaitingSpinnerWidget* wait_dialog = nullptr;

    void set_actions();
    std::vector<std::string> select_keys(const QString &title);
};

} // namespace jsonui

