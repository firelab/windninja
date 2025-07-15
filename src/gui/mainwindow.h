#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "ninja_version.h"
#include "surfaceinput.h"
#include "surfaceinputview.h"
#include "menubarview.h"
#include "mapbridge.h"
#include "ui_mainwindow.h"
#include "cpl_http.h"
#include "appstate.h"
#include <QWebChannel>
#include <QFutureWatcher>
#include <QtConcurrent/QtConcurrentRun>
#include <QProgressDialog>
#include <QMainWindow>
#include <QMessageBox>
#include <QTreeWidgetItem>
#include <QtWebEngineWidgets/qwebengineview.h>
#include <QDir>
#include <QDirIterator>
#include <QDateTime>
#include <QDebug>
#include <QFileDialog>
#include <QFileInfo>
#include <QFileSystemModel>
#include <QSortFilterProxyModel>
#include <QSplitter>
#include <QStandardItemModel>
#include <QStandardPaths>
#include <QTextEdit>
#include <QTextStream>
#include <QThread>
#include <QTimer>
#include <QTreeWidget>
#include <QtWebEngineWidgets/qwebengineview.h>
#include <QWebEngineProfile>
#include <QWebEngineSettings>
#include <QListView>
#include <vector>
#include <string>



QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  void populateForecastDownloads();
  void toggleExpandCollapse(const QModelIndex &index);
  void loadMapKMZ(const std::vector<std::string>& input);

  MainWindow(QWidget *parent = nullptr);
  ~MainWindow();

signals:
  void openElevationFile();

private slots:
  void massSolverCheckBoxClicked();
  void momentumSolverCheckBoxClicked();
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
  void refreshUI();


private:
  void connectSignals();
  void treeItemClicked(QTreeWidgetItem *item, int column);
  QSet<QPair<int, int>> invalidDAWCells;

  Ui::MainWindow *ui;
  QWebEngineView *webView;
  QWebChannel *channel;
  MapBridge *mapBridge;
  SurfaceInput *surfaceInput;
  SurfaceInputView *surfaceInputView;
  MenuBarView *menuBarView;

  bool NinjaCheckVersions(char * mostrecentversion, char * localversion);
  char * NinjaQueryServerMessages(bool checkAbort);
  void checkMessages(void);
};
#endif // MAINWINDOW_H
