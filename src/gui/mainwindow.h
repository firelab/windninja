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
#include <vector>
#include <string>
#include "ninja_version.h"
#include "cpl_http.h"

#include "surfaceinput.h"
#include "menubar.h"


QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  Ui::MainWindow* getUi() const { return ui; }
  void populateForecastDownloads();
  void toggleExpandCollapse(const QModelIndex &index);
  void loadMapKMZ(const std::vector<std::string>& input);
  void refreshUI();

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
  void massSolverCheckBoxClicked();
  void momentumSolverCheckBoxClicked();
  void elevationInputFileDownloadButtonClicked();
  void elevationInputFileOpenButtonClicked();
  void elevationInputFileLineEditTextChanged(const QString &arg1);
  void meshResolutionComboBoxCurrentIndexChanged(int index);
  void diurnalCheckBoxClicked();
  void stabilityCheckBoxClicked();
  void windHeightComboBoxCurrentIndexChanged(int index);
  void domainAverageCheckBoxClicked();
  void treeWidgetItemDoubleClicked(QTreeWidgetItem *item, int column);
  void pointInitializationCheckBoxClicked();
  void useWeatherModelInitClicked();
  void clearTableButtonClicked();
  void solveButtonClicked();
  void outputDirectoryButtonClicked();
  void numberOfProcessorsSolveButtonClicked();
  void timeZoneAllZonesCheckBoxClicked();
  void timeZoneDetailsCheckBoxClicked();
  void timeZoneComboBoxCurrentIndexChanged(int index);
  void domainAverageTableCellChanged(int row, int column);
  void meshResolutionMetersRadioButtonToggled(bool checked);
  void meshResolutionFeetRadioButtonToggled(bool checked);
  void surfaceInputDownloadCancelButtonClicked();
  void surfaceInputDownloadButtonClicked();

signals:
  void solveRequest();
  void timeZoneDataRequest();
  void timeZoneDetailsRequest();
  void getDEMrequest(std::array<double, 4> boundsBox, QString outputFile);

private:
  void connectMenuActions();
  void treeItemClicked(QTreeWidgetItem *item, int column);
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
  SurfaceInput surfaceInput;
  MenuBar* menuBar;

  QString currentDemFilePath;

  bool NinjaCheckVersions(char * mostrecentversion, char * localversion);
  char * NinjaQueryServerMessages(bool checkAbort);
  void checkMessages(void);
};
#endif // MAINWINDOW_H
