#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QFutureWatcher>
#include <QtConcurrent/QtConcurrentRun>
#include <QProgressDialog>
#include <QMainWindow>
#include <QMessageBox>
#include <QTreeWidgetItem>
#include <QtWebEngineWidgets/qwebengineview.h>
#include "ui_mainwindow.h"
#include "gdal_utils.h"
#include "gdal_priv.h"
#include <vector>
#include <string>
#include "ninja_version.h"
#include "cpl_http.h"



QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  Ui::MainWindow* getUi() const { return ui; }
  void populateForecastDownloads();
  void toggleExpandCollapse(const QModelIndex &index);
  void loadMapKMZ(const std::vector<std::string>& input);

  // GDAL Values
  QString GDALDriverName, GDALDriverLongName;
  std::string GDALProjRef;
  bool hasGDALCenter;
  double GDALCenterLat;
  double GDALCenterLon;
  int GDALXSize, GDALYSize;
  double GDALCellSize, GDALNoData;
  double GDALMaxValue, GDALMinValue;


  MainWindow(QWidget *parent = nullptr);
  ~MainWindow();

private slots:
  void on_useCOM_clicked();
  void on_useCOMM_clicked();

  void on_getFromMapButton_clicked();

  void on_openFileButton_clicked();

  void on_elevFilePath_textChanged(const QString &arg1);

  void on_meshResType_currentIndexChanged(int index);

  void on_useDiurnalWind_clicked();

  void on_useStability_clicked();

  void on_domainAvgPicklist_currentIndexChanged(int index);

  void on_useDomainAvgWind_clicked();

  void on_treeWidget_itemDoubleClicked(QTreeWidgetItem *item, int column);

  void on_usePointInit_clicked();

  void on_useWeatherModelInit_clicked();

  void on_clearDAWtable_clicked();

  void on_solveButton_clicked();

  void on_outputSaveLocationBtn_clicked();

  void on_solverPageSolveBtn_clicked();

  void on_showAllTimeZones_clicked();

  void on_displayTimeZoneDetails_clicked();

  void on_timeZoneSelector_currentIndexChanged(int index);

  void on_windTableData_cellChanged(int row, int column);

  void on_meshResMeters_toggled(bool checked);

  void on_meshResFeet_toggled(bool checked);

signals:
  void solveRequest();
  void timeZoneDataRequest();
  void timeZoneDetailsRequest();
  void getDEMrequest(std::array<double, 4> boundsBox, QString outputFile);

private:
  void onTreeItemClicked(QTreeWidgetItem *item, int column);
  QSet<QPair<int, int>> invalidDAWCells;

  // DEM inputs
  double northLat;
  double southLat;
  double eastLon;
  double westLon;
  double centerLat;
  double centerLon;
  double radius;

  Ui::MainWindow *ui;
  QWebEngineView *webView;

  bool NinjaCheckVersions(char * mostrecentversion, char * localversion);
  char * NinjaQueryServerMessages(bool checkAbort);
  void checkMessages(void);
};
#endif // MAINWINDOW_H
