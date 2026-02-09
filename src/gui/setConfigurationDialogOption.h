#ifndef SETCONFIGURATIONDIALOGOPTION_H
#define SETCONFIGURATIONDIALOGOPTION_H

#include "ui_setConfigurationDialogOption.h"
#include <QDialog>

class setConfigurationOptionDialog : public QDialog
{
    Q_OBJECT

public:
    explicit setConfigurationOptionDialog(QWidget *parent = nullptr);
    ~setConfigurationOptionDialog();

    QString getKey();
    QString getValue();

private:
    Ui::setConfigurationOptionDialog *ui;
};

#endif // SETCONFIGURATIONDIALOGOPTION_H
