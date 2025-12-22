#include "ModelLineDialog.h"
#include "ui_ModelLineDialog.h"
#include "jsonqml/models/schema_model.h"

namespace jsonqml {

ModelLineDialog::ModelLineDialog(const QModelIndex &index, QWidget *parentWidget):
    QDialog(parentWidget),
    ui(new Ui::ModelLineDialog)
{
    ui->setupUi(this);

    if(index.isValid()) {
        QString field_key, field_type;
        int field_size;
        const JsonBaseModel* field_model = dynamic_cast<const JsonBaseModel*>(index.model());
        field_model->dataIndex(index, field_key, field_type, field_size);

        ui->nameBox->addItems({field_key});
        ui->nameBox->setEditText(field_key);
        ui->typeBox->addItems(field_model->typeNames());
        ui->typeBox->setCurrentText(field_type);
        ui->sizeBox->setValue(field_size);
    }
    ui->docLine->setText("Change size for");
    ui->nameBox->setEnabled(false);
    ui->typeBox->setEnabled(false);
}

ModelLineDialog::ModelLineDialog(bool, const QModelIndex &index, QWidget *parentWidget):
    QDialog(parentWidget),
    ui(new Ui::ModelLineDialog)
{
    ui->setupUi(this);
    ui->sizeBox->hide();
    ui->labelSize->hide();

    const JsonBaseModel* field_model = dynamic_cast<const JsonBaseModel*>(index.model());

    ui->nameBox->addItems(field_model->fieldNames(index, field_top_schema_name));
    ui->typeBox->addItems(field_model->typeNames());
    ui->sizeBox->setValue(1);

    if(!field_model->useSchema()) {
        ui->nameBox->setEditable(true);
        QObject::connect(ui->typeBox, &QComboBox::currentTextChanged, this, &ModelLineDialog::setType);
    }
    else {
        ui->typeBox->setEnabled(false);
        QObject::connect(ui->nameBox, &QComboBox::currentTextChanged, this, &ModelLineDialog::setName);
        if(ui->nameBox->count()>0) {
            setName(ui->nameBox->currentText());
        }
    }
}

ModelLineDialog::~ModelLineDialog()
{
    delete ui;
}

QString ModelLineDialog::objectType() const
{
    return ui->typeBox->currentText();
}

int ModelLineDialog::arraySize() const
{
    return ui->sizeBox->value();
}

QString ModelLineDialog::objectName() const
{
    return ui->nameBox->currentText();
}

void ModelLineDialog::setType(const QString &field_type)
{
    ui->sizeBox->setVisible(field_type.contains("array"));
    ui->labelSize->setVisible(field_type.contains("array"));
}

void ModelLineDialog::setName(const QString &field_name)
{
    auto afield_def = JsonSchemaModel::field_def(field_top_schema_name.toStdString(),
                                                 field_name.toStdString());
    if(afield_def) {
        ui->docLine->setText(QString::fromStdString(afield_def->description()));
        auto afield_type = afield_def->type(0);
        auto ajson_type = jsonio::JsonSchema::fieldtype2basetype(afield_type);
        ui->typeBox->setCurrentText(jsonio::JsonBase::typeName(ajson_type));

        bool is_resized = (afield_type == jsonio::FieldDef::T_SET  ||
                           afield_type == jsonio::FieldDef::T_LIST ||
                           afield_type == jsonio::FieldDef::T_MAP);
        ui->sizeBox->setVisible(is_resized);
        ui->labelSize->setVisible(is_resized);
    }
}

} // namespace jsonqml

