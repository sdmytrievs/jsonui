#pragma once

#include <QDialog>

namespace Ui {
class HelpEditDialog;
}

namespace jsonqml {

class HelpClient;

/// Declaration of HelpEditDialog widget - dialog for editing help pages in markdown format
class HelpEditDialog : public QDialog
{
    Q_OBJECT

signals:
    /// Reset current md text
    void updateMarkdown(const QString& new_mdtext);

protected slots:
    void applyChanges();
    void previewChanges();

    void insertLink();
    void addImage();
    void addDocPage();

    void showContextMenu(const QPoint &pt);
    void setLinkOid(std::string doc_id);

public:
    explicit HelpEditDialog(HelpClient* client, const QString& mdtext, QWidget *parent = nullptr);
    ~HelpEditDialog();

private:
    QScopedPointer<Ui::HelpEditDialog> ui;
    HelpClient* help_client;

};

} // namespace jsonui

