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
  void on_massSolverCheckBox_clicked();
  void on_massAndMomentumSolverCheckBox_clicked();

  void on_elevationInputFileDownloadButton_clicked();

  void on_elevationInputFileOpenButton_clicked();

  void on_elevationInputFileLineEdit_textChanged(const QString &arg1);

  void on_meshResolutionComboBox_currentIndexChanged(int index);

  void on_diurnalCheckBox_clicked();

  void on_stabilityCheckBox_clicked();

  void on_windHeightComboBox_currentIndexChanged(int index);

  void on_domainAverageCheckBox_clicked();

  void on_treeWidget_itemDoubleClicked(QTreeWidgetItem *item, int column);

  void on_pointInitializationCheckBox_clicked();

  void on_useWeatherModelInit_clicked();

  void on_clearTableButton_clicked();

  void on_solveButton_clicked();

  void on_outputDirectoryButton_clicked();

  void on_numberOfProcessorsSolveButton_clicked();

  void on_timeZoneAllZonesCheckBox_clicked();

  void on_timeZoneDetailsCheckBox_clicked();

  void on_timeZoneComboBox_currentIndexChanged(int index);

  void on_domainAverageTable_cellChanged(int row, int column);

  void on_meshResolutionMetersRadioButton_toggled(bool checked);

  void on_meshResolutionFeetRadioButton_toggled(bool checked);

  void on_surfaceInputDownloadCancelButton_clicked();

signals:
  void solveRequest();
  void timeZoneDataRequest();
  void timeZoneDetailsRequest();
  void getDEMrequest(std::array<double, 4> boundsBox, QString outputFile);

private:
  // functions for Menu actions
  // functions for QMenu fileMenu "File" actions
  void newProject();
  void openProject();
  void exportSolution();
  void closeProject();
  // functions for QMenu optionsMenu "Options" actions
  void enableConsoleOutput();
  void writeConsoleOutput();
  // functions for QMenu toolsMenu "Tools" actions
  void resampleData();
  void writeBlankStationFile();
  void setConfigurationOption();
  // functions for QMenu helpMenu "Help" actions
  // functions for sub QMenu displayingShapeFilesMenu "Displaying Shapefiles" actions
  void displayArcGISProGuide();
  // functions for sub QMenu tutorialsMenu "Tutorials" actions
  void displayTutorial1();
  void displayTutorial2();
  void displayTutorial3();
  void displayTutorial4();
  // functions for sub QMenu instructionsMenu "Instructions" actions
  void displayDemDownloadInstructions();
  void displayFetchDemInstructions();
  void displayCommandLineInterfaceInstructions();
  // functions for remaining non-sub QMenu actions
  void aboutWindNinja();
  void citeWindNinja();
  void supportEmail();
  void submitBugReport();

  void connectMenuActions();
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
