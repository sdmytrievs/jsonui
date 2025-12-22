#pragma once

#include <QTreeView>
#include <QItemDelegate>
#include "jsonqml/models/base_model.h"

namespace jsonqml {

class JsonClient;
class ExpandedLevelLine;

// check error from settings

/// \class JsonView implements a tree view of json based structure
/// that displays items from a JsonBaseModel model.
class JsonView: public QTreeView
{
    Q_OBJECT

public slots:
    void CmHelp();
    void CmCalc();
    void CmAddObject();
    void CmDelObject();
    void CmDelObjectsUnion();
    void CmResizeArray();
    void CmCloneObject();
    void CmResetObject();

    void CopyFieldPath();
    void CopyField();
    void PasteField();

protected slots:
    void slotPopupContextMenu(const QPoint& pos);
    void changeCurrent(int section);

    void saveExpandedRows();
    void getExpandedRows();

public:
    /// Extern flag to keep data fields expanded
    static bool expandedFields;

    /// Constructor
    explicit JsonView(JsonClient* client, QWidget* parent = nullptr);

    /// Update model data with restore Expanded Level
    void updateModelData(const std::string& json_string, const QString& schema_name);

protected:
    JsonClient* json_client;
    std::shared_ptr<ExpandedLevelLine> expanded_rows;
    int section_0_size = 250;
    int section_1_size = 250;

    const JsonBaseModel* current_index_model(const QModelIndex& index) const
    {
        if(index.isValid()) {
            return dynamic_cast<const JsonBaseModel*>(index.model());
        }
        return nullptr;
    }

    JsonBaseModel* view_model()
    {
        return dynamic_cast<JsonBaseModel*>(model());
    }

    void keyPressEvent(QKeyEvent* e) override;

    void save_expanded_state(std::shared_ptr<ExpandedLevelLine> topList);
    void restore_expanded_state(std::shared_ptr<ExpandedLevelLine> topList);
    void save_expanded_on_level(const QModelIndex& index, std::shared_ptr<ExpandedLevelLine> parent);
    void restore_expanded_on_level(const QModelIndex& index, std::shared_ptr<ExpandedLevelLine> top_data);
};

/// \class JsonDelegate individual items in views are rendered and edited using delegates
class JsonDelegate: public QItemDelegate
{
    Q_OBJECT

public:

    JsonDelegate(QObject* parent = nullptr):
        QItemDelegate(parent)
    {}
    QWidget *createEditor(QWidget *parent,
                          const QStyleOptionViewItem &option,
                          const QModelIndex &index) const;
    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model,
                      const QModelIndex &index) const;
};


} // namespace jsonui

