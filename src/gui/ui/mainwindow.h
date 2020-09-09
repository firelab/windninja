#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <assert.h>

#include <QMainWindow>

#include <qtconcurrentrun.h>

#include <QByteArray>
#include <QDebug>
#include <QDesktopServices>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QLabel>
#include <QList>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QPair>
#include <QProgressBar>
#include <QString>
#include <QStringList>
#include <QThread>
#include <QTimeZone>
#include <QTreeWidgetItem>
#include <QUrl>
#include <QUrlQuery>

#include <cpl_string.h>
#include <gdal.h>
#include <ogr_srs_api.h>

#include <windninja.h>

namespace Ui {
  class MainWindow;
}

typedef struct {
  double minX;
  double maxX;
  double minY;
  double maxY;

  double dx;
  int nx;
  int ny;

  double coarseRes;
  double mediumRes;
  double fineRes;
} spatialInfo;

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
    spatialInfo elevInfo;
    QString elevPath;
    QString forecastPath;

    void init();

    void setIcons();
    void setConnections();
    void OGRFormats();

    int countRuns();

    int downloadUCAR(QString model, int hours, QString filename);

public slots:
    void updateStack(QTreeWidgetItem *, QTreeWidgetItem *);
    void openElevation();
    void downloadElev();
    void openForecast();
    void updateMesh(int index);
    void setProgress(int done, QString text="", int timeout=0);
    void openOutputPath();
    void downloadWx();
    void solve();
};

#endif // MAINWINDOW_H
