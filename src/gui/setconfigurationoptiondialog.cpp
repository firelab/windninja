#include "setconfigurationoptiondialog.h"
#include "ui_setconfigurationoptiondialog.h"

setConfigurationOptionDialog::setConfigurationOptionDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::setConfigurationOptionDialog)
{
    ui->setupUi(this);
}

setConfigurationOptionDialog::~setConfigurationOptionDialog()
{
    delete ui;
}

QString setConfigurationOptionDialog::getKey()
{
    return ui->keyLineEdit->text();
}

QString setConfigurationOptionDialog::getValue()
{
    return ui->valueLineEdit->text();
}
