#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <assert.h>

#include <QMainWindow>

#include <QDebug>
#include <QFileDialog>
#include <QFileInfo>
#include <QString>
#include <QTreeWidgetItem>

#include <cpl_string.h>
#include <gdal.h>

namespace Ui {
class MainWindow;
}

const int defaultMsgLength = 2500;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;

    QStringList ogrFormats;

    void init();

    void setIcons();
    void setConnections();
    void OGRFormats();

public slots:
    void updateStack(QTreeWidgetItem *, QTreeWidgetItem *);
    void openElevation();
    void updateMesh(int index);
};

#endif // MAINWINDOW_H
