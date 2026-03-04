#include <QPushButton>
#include <QMenu>
#include "ui_HelpEditDialog.h"
#include "HelpEditDialog.h"
#include "HelpFileDialog.h"
#include "jsonqml/clients/help_client.h"
#include "jsonqml/forms/SelectDialog.h"

namespace jsonqml {

HelpEditDialog::HelpEditDialog(HelpClient* client, const QString &mdtext, QWidget *parent):
    QDialog(parent),
    ui(new Ui::HelpEditDialog),
    help_client(client)
{
    ui->setupUi(this);
    ui->editor->setPlainText(mdtext);

    QPushButton* button =ui->buttons->addButton("&Preview", QDialogButtonBox::ActionRole);
    QObject::connect(button, &QPushButton::clicked, this, &HelpEditDialog::previewChanges);

    button =ui->buttons->addButton("&Undo", QDialogButtonBox::ActionRole);
    button->setEnabled(false);
    QObject::connect(button, &QPushButton::clicked, ui->editor, &QPlainTextEdit::undo);
    QObject::connect(ui->editor, &QPlainTextEdit::undoAvailable, button, &QPushButton::setEnabled);

    button =ui->buttons->addButton("&Redo", QDialogButtonBox::ActionRole);
    button->setEnabled(false);
    QObject::connect(button, &QPushButton::clicked, ui->editor, &QPlainTextEdit::redo);
    QObject::connect(ui->editor, &QPlainTextEdit::redoAvailable, button, &QPushButton::setEnabled);

    button =ui->buttons->addButton("&Insert Link", QDialogButtonBox::ActionRole);
    QIcon icon1;
    icon1.addFile(QString::fromUtf8(":/menu/Icons/load_db.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
    button->setIcon(icon1);
    QObject::connect(button, &QPushButton::clicked, this, &HelpEditDialog::insertLink);

    button =ui->buttons->addButton("Add &Docpage", QDialogButtonBox::ActionRole);
    QIcon icon;
    icon.addFile(QString::fromUtf8(":/menu/Icons/ShowFilesIcon24.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
    button->setIcon(icon);
    QObject::connect(button, &QPushButton::clicked, this, &HelpEditDialog::addDocPage);

    button =ui->buttons->addButton("&Add Image", QDialogButtonBox::ActionRole);
    button->setIcon(icon);
    QObject::connect(button, &QPushButton::clicked, this, &HelpEditDialog::addImage);

    connect(ui->buttons->button(QDialogButtonBox::Apply), &QPushButton::clicked, this, &HelpEditDialog::applyChanges);
    connect(ui->buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    ui->editor->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->editor, &QPlainTextEdit::customContextMenuRequested,
            this, &HelpEditDialog::showContextMenu);

    connect(client->tablemodel(), &DBKeysModel::updatedOid,
            this, &HelpEditDialog::setLinkOid, Qt::UniqueConnection);
}

HelpEditDialog::~HelpEditDialog()
{}

void HelpEditDialog::setLinkOid(std::string doc_id)
{
    ui->editor->insertPlainText(help_client->linktoOid(doc_id));
}

void HelpEditDialog::previewChanges()
{
    emit updateMarkdown(ui->editor->toPlainText());
}

void HelpEditDialog::applyChanges()
{
    previewChanges();
    accept();
}

void HelpEditDialog::insertLink()
{
    // md or image
    SelectDialog sel_dlg(single_selection, "Please, select an record to link",
                         help_client->tablemodel(), this);
    if(sel_dlg.exec())  {
        auto selndx = sel_dlg.selIndex();
        if(selndx>=0) {
            setLinkOid(help_client->tableId(selndx));
        }
    }
}

void HelpEditDialog::addImage()
{
    HelpFileDialog dlg(DocPagesData::Image, this);
    if(dlg.exec())  {
        QString file = dlg.sourceFile();
        QString name = dlg.linkName();
        if(file.isEmpty() || name.isEmpty()) {
            return;
        }
        help_client->addFileDocument(DocPagesData::Image, name, file);
    }
}

void HelpEditDialog::addDocPage()
{
    HelpFileDialog dlg(DocPagesData::Markdown, this);
    if(dlg.exec()) {
        QString file = dlg.sourceFile();
        QString name = dlg.linkName();
        if(name.isEmpty()) {
            return;
        }
        help_client->addFileDocument(DocPagesData::Markdown, name, file);
    }
}

void HelpEditDialog::showContextMenu(const QPoint &pt)
{
    QMenu *menu = ui->editor->createStandardContextMenu();
    menu->addSeparator();

    auto act =  new QAction(tr("&Insert Link"), this);
    act->setShortcut(tr("Ctrl+I"));
    act->setStatusTip(tr("Insert existing link to document."));
    connect(act, &QAction::triggered, this, &HelpEditDialog::insertLink);
    menu->addAction(act);

    act =  new QAction(tr("&Add Image"), this);
    act->setShortcut(tr("Ctrl+A"));
    act->setStatusTip(tr("Add new image to database and insert link to document."));
    connect(act, &QAction::triggered, this, &HelpEditDialog::addImage);
    menu->addAction(act);

    act =  new QAction(tr("Add &Docpage"), this);
    act->setShortcut(tr("Ctrl+D"));
    act->setStatusTip(tr("Add new docpage to DB and insert link to document."));
    connect(act, &QAction::triggered, this, &HelpEditDialog::addDocPage);
    menu->addAction(act);

    menu->exec(ui->editor->mapToGlobal(pt));
    delete menu;
}

} // namespace jsonui
