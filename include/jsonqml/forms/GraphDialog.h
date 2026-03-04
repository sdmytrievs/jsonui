#pragma once

#include <QDialog>
#include <QTableWidget>
#include <QItemDelegate>
#include <QPushButton>

namespace Ui {
class GraphDialogData;
}

namespace jsonqml {

class DragTableWidget;
class LabelDelegate;
class PlotChartView;
class ChartClient;

/// \class GraphDialog plotting system main dialog
class GraphDialog: public QDialog
{
    Q_OBJECT

public slots:
    void UpdateAll(const QString& title = QString());
    void restoreRow();
    void updateFragment(QRectF rect);

protected slots:
    void changeIcon(int row, int column);
    void highlightRow(int row);

    void CmFragment();
    void CmLegend();
    void CmPrint();
    void CmSaveImage();
    void CmHelp();

public:
    GraphDialog(ChartClient *client, const QString& title= "Graphics Dialog", QWidget *parent=nullptr);
    ~GraphDialog();

    /// Update all graphic lines with new title
    void UpdatePlots(const QString& title);

private:
    QScopedPointer<Ui::GraphDialogData> ui;
    ChartClient* chart_client;
    QPushButton* button_fragment;

    QScopedPointer<DragTableWidget> legend_table;
    QScopedPointer<PlotChartView> plot;

    // work part
    bool is_fragment;
    int active_row = -1;
    void show_legend();

    friend class LabelDelegate;
    friend class LegendDialog;

};


//=========================================================================

/// Drag Table Widget - added for legend table
class DragTableWidget: public QTableWidget
{
    Q_OBJECT

public:
    DragTableWidget(GraphDialog* parent):
        QTableWidget(parent), top_dlg(parent)
    {}

    QSize sizeHint() const override
    {
        auto sizehint = QTableWidget::sizeHint();
        sizehint.setWidth(180);
        return sizehint;
    }

protected:
    QPoint start_pos;
    GraphDialog* top_dlg;

    void start_drag();
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void focusOutEvent(QFocusEvent* event)override;

};

/// \class LabelDelegate
/// Individual items in views are rendered and edited using delegates
class LabelDelegate: public QItemDelegate
{
    Q_OBJECT

public:
    LabelDelegate(GraphDialog* parent):
        QItemDelegate(parent), graph_dlg(parent)
    {}

    QWidget *createEditor(QWidget *parent,
                          const QStyleOptionViewItem &option,
                          const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model,
                      const QModelIndex &index) const;
private:
    GraphDialog* graph_dlg;

};

} // namespace jsonui

