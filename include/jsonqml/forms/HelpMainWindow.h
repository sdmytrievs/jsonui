#pragma once

#include <QLineEdit>
#include <QMainWindow>
#include <QTextBrowser>

// Not implemented scroll to anchor

namespace Ui {
class HelpWindowData;
}

namespace jsonqml {

void helpWin(const QString &name, const QString &item);

class VertexClient;
class HelpClient;

class HelpTextBrowser: public QTextBrowser
{
    Q_OBJECT

public slots:
    void helpPrint();

public:
    HelpTextBrowser(HelpClient* client, QWidget *parent = nullptr):
        QTextBrowser(parent),
        help_client(client)
    {}

    QVariant loadResource(int type, const QUrl &name) override;

protected:
    HelpClient* help_client;

    void doSetSource(const QUrl &url, QTextDocument::ResourceType type = QTextDocument::UnknownResource) override;
};

/// Declaration of classes HelpMainWindow - Help system main dialog
class HelpMainWindow : public QMainWindow
{
    Q_OBJECT

public slots:
    void helpAbout();
    void helpVersion();
    void helpAuthors();
    void helpLicense();
    void helpOnHelp();

protected slots:
    void CmEdit();
    void CmRead();
    void CmUpdate();
    void CmDeleteList();

    void action_find_next();
    void action_find_previous();
    void updateDocPage(const QString& mdtext);

public:
    static HelpMainWindow* pDia;

    HelpMainWindow(QWidget* parent = nullptr);
    virtual ~HelpMainWindow();

    void showHelp(const QString &name, const QString &item);

protected:
    QScopedPointer<Ui::HelpWindowData> ui;
    QScopedPointer<HelpTextBrowser> help_view;
    QScopedPointer<QLineEdit> adress_line;
    QScopedPointer<QLineEdit> find_line;
    QScopedPointer<HelpClient> help_client;

    void set_actions();
};

} // namespace jsonui


