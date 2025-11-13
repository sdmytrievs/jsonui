#pragma once

#include <QMainWindow>
#include "jsonqml/models/select_model.h"

namespace Ui {
class TableEditWidget;
}

namespace jsonqml {

class TableClient;
class MatrixTable;
class GraphDialog;

/// \class TableEditWidget  widget to work with table data.
class TableEditWidget : public QMainWindow
{
    Q_OBJECT

public slots:
    void CmImportCSV();
    void CmExportCSV();
    void CmPlotTable();
    void CmHelp() {}

protected slots:
    void slotPopupContextMenu(const QPoint &pos);
    void toggleX();
    void toggleY();

public:
    /// Constructor from csv file
    explicit TableEditWidget(const QString& title, int mode = TableFlag::TableIsEditable, QWidget *parent = nullptr);
    /// Destructor
    ~TableEditWidget();

private:
    QScopedPointer<Ui::TableEditWidget> ui;
    QScopedPointer<TableClient> table_client;
    QScopedPointer<MatrixTable> table_view;
    QScopedPointer<GraphDialog> graph_dlg;

    /// Title of window
    QString widow_title;
    /// Mode of using ( see TableFlag )
    int mode_using;

    void closeEvent(QCloseEvent* e) override;
    void set_actions();
    void close_graph();
    void show_graph_data();
};

} // namespace jsonui

