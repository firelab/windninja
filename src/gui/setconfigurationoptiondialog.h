#ifndef SETCONFIGURATIONOPTIONDIALOG_H
#define SETCONFIGURATIONOPTIONDIALOG_H

#include <QDialog>

namespace Ui {
class setConfigurationOptionDialog;
}

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

#endif // SETCONFIGURATIONOPTIONDIALOG_H
