#ifndef SETCONFIGDIALOG_H
#define SETCONFIGDIALOG_H

#include <QDialog>

namespace Ui {
class SetConfigDialog;
}

class SetConfigDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SetConfigDialog(QWidget *parent = 0);
    ~SetConfigDialog();

    QString GetKey();
    QString GetVal();

private:
    Ui::SetConfigDialog *ui;
};

#endif // SETCONFIGDIALOG_H
