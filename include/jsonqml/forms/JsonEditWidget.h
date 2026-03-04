#pragma once

#include <QMainWindow>

namespace Ui {
class JsonEditWidget;
}

namespace jsonqml {

class JsonClient;
class VertexClient;
class JsonView;

/// \class JsonEditWidget - window to edit/create free format json or
/// schema based json
class JsonEditWidget : public QMainWindow
{
    Q_OBJECT

public slots:
    void CmNew();
    void CmClone();
    void CmExportJSON();
    void CmImportJSON();

    void CmRead();
    void CmUpdate();
    void CmDeleteList();
    void CmBackup();

    void CmHelpContens();
    void CmHelpAbout();

protected slots:
    void updateViewMenu();

public:
    explicit JsonEditWidget(const QString& aschema_name = "",
                            QWidget *parent = nullptr );
    ~JsonEditWidget();

private:
    QScopedPointer<Ui::JsonEditWidget> ui;
    QScopedPointer<JsonClient> json_client;
    QScopedPointer<JsonView> json_tree;
    QString schema_name;
    QString collection_name;

    void set_actions();
    std::vector<std::string> select_keys(VertexClient *vclient, const QString &title);
};

} // namespace jsonui
