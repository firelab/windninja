/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja Qt GUI
 * Purpose:  Main window and parent to all other widgets
 * Author:   Kyle Shannon <ksshannon@gmail.com>
 *
 ******************************************************************************
 *
 * THIS SOFTWARE WAS DEVELOPED AT THE ROCKY MOUNTAIN RESEARCH STATION (RMRS)
 * MISSOULA FIRE SCIENCES LABORATORY BY EMPLOYEES OF THE FEDERAL GOVERNMENT
 * IN THE COURSE OF THEIR OFFICIAL DUTIES. PURSUANT TO TITLE 17 SECTION 105
 * OF THE UNITED STATES CODE, THIS SOFTWARE IS NOT SUBJECT TO COPYRIGHT
 * PROTECTION AND IS IN THE PUBLIC DOMAIN. RMRS MISSOULA FIRE SCIENCES
 * LABORATORY ASSUMES NO RESPONSIBILITY WHATSOEVER FOR ITS USE BY OTHER
 * PARTIES,  AND MAKES NO GUARANTEES, EXPRESSED OR IMPLIED, ABOUT ITS QUALITY,
 * RELIABILITY, OR ANY OTHER CHARACTERISTIC.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 *****************************************************************************/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui>
#include <QMainWindow>
#include <QMenu>
#include <QLabel>
#include <QAction>
#include <QTime>
#include <QTextEdit>
#include <QColor>
#include <QRgb>
#include <QProcess>
#include <QThread>
#include <QToolBar>
#include <QString>
#include <QProgressDialog>
#include <QMutex>
#include <QDir>
#include <QDesktopServices>

#include "gdal_priv.h"
#include "ogr_srs_api.h"
#include "cpl_string.h"
#include "gdal_util.h"
#include "startRuns.h"
#include "ninja.h"
#include "version.h"
#include "WindNinjaTree.h"
#include "consoleDockWidget.h"
#include "solveThread.h"
#include "ninjaException.h"
#include "ninjaUnits.h"
#include "wxStation.h"
#include "ninjaUnits.h"
#include "ninjaArmy.h"
#include "ninja_conv.h"

#include "setconfigdialog.h"

class mainWindow : public QMainWindow
{
  Q_OBJECT

 public:
  mainWindow(QWidget *parent = 0);
  
  //ninja *inputNinja;
  
  ninjaArmy *army;

  QProgressDialog *progressDialog;
  int nRuns;

  QDir cwd;
  QDir pwd;

  QTimer *timer;

  int *runProgress;
  int totalProgress;

  bool okToContinueCheck;

  public slots:
   void updateProgress(int run, int progress);
   void updateTimer();
   void openDEMDownloader();

 public:
  WindNinjaTree *tree;
  ConsoleDockWidget *console;
  QString prompt;
  QColor orange;

  enum eInputFileType{
    ASC, LCP, GTIFF, IMG};
  QString inputFileName;
  QDir inputFileDir;
  QString shortInputFileName;
  QString outputPath;
  int inputFileType;
  bool hasPrj;
  QString prjFileName;
  double meshCellSize;

  enum eInputStatus{
    blue, green, amber, red};

  int lineNumber;

  //GDAL values
  QString GDALDriverName, GDALDriverLongName;
  std::string GDALProjRef;
  bool hasGDALCenter;
  double GDALCenterLat;
  double GDALCenterLon;
  int GDALXSize, GDALYSize;
  double GDALCellSize, GDALNoData;

  //threshold for no-googling = 400000

  static const int noGoogleNumCells = 400000;
  double noGoogleCellSize;
  //threads
  solveThread *sThread;

 signals:
  void inputFileChanged(QString newFile);

 public slots:
  void openInputFile();
  void updateFileInput(const char* file);
  void inputFileDeleted();
  void openMainWindow();
  double computeCellSize(int index);
  int checkInputFile(QString fileName);
  void checkMeshCombo();
  void checkMeshUnits(bool checked);
  void updateOutRes();
  void setPrompt(QString p);
  void writeToConsole(QString message, QColor color = Qt::black);

  void writeConsoleOutput();
  void resampleData();
  void writeBlankStationFile();
  void windNinjaHelp();
  void displayArcView();
  void displayArcMap();
  void tutorial1();
  void tutorial2();
  void tutorial3();
  void tutorial4();
  void demDownload();
  void fetchDem();
  void cliInstructions();
  void aboutWindNinja();
  void supportEmail();
  void bugReport();
  int openHelp(int target = 0);

  void treeDoubleClick(QTreeWidgetItem *item, int column);

  bool getLatLon();

  void test();

  int solve();
  void cancelSolve();
  int countRuns();

  void openOutputPath();

  //functions for checking inputItems
  int checkInputItem();
  int checkSurfaceItem();
  int checkDiurnalItem();
#ifdef STABILITY
  int checkStabilityItem();
#endif
#ifdef NINJAFOAM
  int checkNativeSolverItem();
  int checkNinjafoamItem();
  int checkSolverMethodItem();
  void selectNativeSolver( bool pick );
  void selectNinjafoamSolver( bool pick );
#endif
  int checkWindItem();
  int checkSpdDirItem();
  int checkPointItem();
  int checkWeatherItem();
  int checkOutputItem();
  int checkGoogleItem();
  int checkFbItem();
  int checkShapeItem();
  int checkVtkItem();
  int checkSolveItem();
  int checkAllItems();

  int checkKmlLimit(double);

  bool okToContinue();

  //initialization mutual exclusion
  void selectWindInitialization( bool pick );
  void selectPointInitialization( bool pick );
  void selectWeatherInitialization( bool pick );
  void enablePointDate(bool enable);
#ifdef NINJAFOAM
  void enableNinjafoamOptions(bool enable);
#endif

  void SetConfigOption();

 protected:
  void closeEvent(QCloseEvent *event);

 private:
  QAction *openInputFileAction;
  QAction *exitAction;
  QAction *editPromptAction;
  QAction *writeConsoleOutputAction;
  QAction *rddsAction;
  QAction *writeBlankStationFileAction;
  QAction *setConfigAction;
  QAction *rddsInstructAction;
  QAction *resampleAction;
  QAction *windNinjaHelpAction;
  QAction *tutorial1Action, *tutorial2Action;
  QAction *tutorial3Action, *tutorial4Action;
  QAction *downloadDemAction;
  QAction *fetchDemAction;
  QAction *displayShapeFileViewAction;
  QAction *displayShapeFileMapAction;
  QAction *cliInstructionsAction;
  QAction *aboutWindNinjaAction;
  QAction *aboutQtAction;
  QAction *supportEmailAction;
  QAction *submitBugReportAction;

  //test action connected to test slot
  QAction *testAction;

  //Menus for the interface main window
  QMenu *fileMenu;
  QMenu *optionsMenu;
  QMenu *toolsMenu;
  QMenu *helpMenu;
  QMenu *tutorialSubMenu;
  QMenu *shapeSubMenu;

  QLabel *statusLabel;

  QTime *runTime;

  double elapsedRunTime;

  //create various entities in there own functions.

  void createMenus();
  void createActions();
  void createTimers();
  void createConnections();
  void createConsole();
  void createTreeView();
  void readSettings();
  void writeSettings();
  QString checkForNoData( QString fileName );

  QVBoxLayout *mainLayout;

  QFileSystemWatcher fileWatcher;
  WidgetDownloadDEM *demWidget;
};

#endif /* MAINWINDOW_H */

