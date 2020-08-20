#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <assert.h>

#include <QMainWindow>

#include <qtconcurrentrun.h>

#include <QDebug>
#include <QDesktopServices>
#include <QFileDialog>
#include <QFileInfo>
#include <QLabel>
#include <QProgressBar>
#include <QString>
#include <QThread>
#include <QTreeWidgetItem>

#include <cpl_string.h>
#include <gdal.h>

#include <windninja.h>

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

    QProgressBar *progress;
    QLabel *progressLabel;

    QString outputPath;

    void init();

    void setIcons();
    void setConnections();
    void OGRFormats();

    int countRuns();

public slots:
    void updateStack(QTreeWidgetItem *, QTreeWidgetItem *);
    void openElevation();
    void updateMesh(int index);
    void setProgress(int done, QString text="", int timeout=0);
    void openOutputPath();
    void solve();

};

#endif // MAINWINDOW_H
