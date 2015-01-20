#include "setconfigdialog.h"
#include "ui_setconfigdialog.h"

SetConfigDialog::SetConfigDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SetConfigDialog)
{
    ui->setupUi(this);
}

SetConfigDialog::~SetConfigDialog()
{
    delete ui;
}

QString SetConfigDialog::GetKey()
{
    return ui->keyEdit->text();
}
QString SetConfigDialog::GetVal()
{
    return ui->valEdit->text();
}


