#pragma once

#include <QMainWindow>
#include <QLineEdit>

namespace Ui {
class EdgesWidget;
}
namespace jsonio {
class DBQueryBase;
}
class WaitingSpinnerWidget;

namespace jsonqml {

class EdgeClient;
class JsonView;
class TableClient;
class DBKeysTable;

/// \class EdgesWidget - window to insert/update  Edges (TDBGraph)
class EdgesWidget : public QMainWindow
{
    Q_OBJECT

signals:
    /// Emit signal, when delete recod(s)
    //void edgeDeleted();
    /// Emit signal, when loaded graph
    //void graphLoaded();

public slots:
    void openRecordKey(const QModelIndex& index);
    void setExecuting(bool exec);
    void updateViewMenu();
    void scrollInOut();
    void typeChanged(const QString &text);
    /// Add Incoming key to edited document
    void addIncoming(const QModelIndex& index) {}
    /// Add Outgoing key to edited document
    void addOutgoing(const QModelIndex& index) {}

    void openIncoming(const QString& vertex_id);
    void openOutgoing(const QString& vertex_id);
    //void openRecordKey(const QModelIndex& index, bool reset_in_out_query = false);
    //void changeKeyList();
    //void updateQuery();

    // File
    void CmNew();
    void CmClone();
    void CmExportJSON();
    void CmImportJSON();
    void CmIncomingQuery() {}
    void CmOutgoingQuery() {}

    // Record
    void CmRead();
    void CmUpdate();
    void CmDelete();
    void CmDeleteSelect() {}
    void CmDisplaySearchResult() {}
    void CmSearchQuery() {}
    void CmBackupQueriedRecordstoFile() {}
    void CmRestoreMultipleRecordsfromFile(){}
    void CmBackupGraphtoFile() {}
    void CmRestoreGraphfromFile(){}

    // View
    void CmFollowInVertex();
    void CmFollowOutVertex();
    void CmHelpContens();

public:
    explicit EdgesWidget(const QString& aschema, QWidget *parent = nullptr);
    explicit EdgesWidget(bool in_edges, const QString& vertex_id, QWidget *parent = nullptr);
    ~EdgesWidget();

    /// Update query
    //void setQueryEdited(QueryWidget* queryW);

    std::string currentKey() const;

private:
    QScopedPointer<Ui::EdgesWidget> ui;
    QScopedPointer<EdgeClient> edge_client;
    QScopedPointer<JsonView> json_tree;
    QScopedPointer<TableClient> keys_table_client;
    QScopedPointer<DBKeysTable> keys_view;

    QScopedPointer<TableClient> in_keys_table_client;
    QScopedPointer<DBKeysTable> in_keys_view;
    QScopedPointer<TableClient> out_keys_table_client;
    QScopedPointer<DBKeysTable> out_keys_view;

    QString schema_name;
    bool is_default_query;
    QLineEdit* pLineTask = nullptr;

    /// Dialog when waiting db_rquest command ends
    WaitingSpinnerWidget* wait_dialog = nullptr;

    void set_actions();
    void define_window();

    //void closeEvent(QCloseEvent* e) override;
    //bool event(QEvent * e) override;
};

} // namespace jsonui

