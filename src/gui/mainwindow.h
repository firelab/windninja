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
};
#endif // MAINWINDOW_H
