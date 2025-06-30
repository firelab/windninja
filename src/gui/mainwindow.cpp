#include "mainwindow.h"
#include "appstate.h"
#include "./ui_mainwindow.h"
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
#include <vector>
#include <string>

// Menu filtering class
class DirectoryFilterModel : public QSortFilterProxyModel {
protected:
  bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override {
    QFileSystemModel *fsModel = qobject_cast<QFileSystemModel *>(sourceModel());
    if (!fsModel) return false;

    QModelIndex index = fsModel->index(source_row, 0, source_parent);
    if (!index.isValid()) return false;

    // Define the download path
    QFileInfo fileInfo = fsModel->fileInfo(index);
    QString downloadsPath = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);

    // Keep the Downloads root directory
    if (fileInfo.absoluteFilePath() == downloadsPath) {
      return true;
    }

    // Ensure filtering applies only inside Downloads
    if (!fileInfo.absoluteFilePath().startsWith(downloadsPath)) {
      return false;
    }

    // Allow `WXSTATIONS-*` directories
    if (fileInfo.isDir() && fileInfo.fileName().toLower().startsWith("wxstations")) {
      return true;
    }

    // Allow files **inside** `WXSTATIONS-*`
    QModelIndex parentIndex = index.parent();
    if (parentIndex.isValid()) {
      QFileInfo parentInfo = fsModel->fileInfo(parentIndex);
      if (parentInfo.isDir() && parentInfo.fileName().toLower().startsWith("wxstations")) {
        return true;
      }
    }

    return false;
  }
};

MainWindow::~MainWindow() { delete ui; }

//// functions for Menu actions

void MainWindow::newProject()
{
  printf("newProject clicked\n");
}
void MainWindow::openProject()
{
  printf("openProject clicked\n");
}
void MainWindow::exportSolution()
{
  printf("exportSolution clicked\n");
}
void MainWindow::closeProject()
{
  printf("closeProject clicked\n");
}

void MainWindow::enableConsoleOutput()
{
  printf("enableConsoleOutput clicked\n");
}
void MainWindow::writeConsoleOutput()
{
  printf("writeConsoleOutput clicked\n");
}

void MainWindow::resampleData()
{
  printf("resampleData clicked\n");
}
void MainWindow::writeBlankStationFile()
{
  printf("writeBlankStationFile clicked\n");
}
void MainWindow::setConfigurationOption()
{
  printf("setConfigurationOption clicked\n");
}

void MainWindow::displayArcGISProGuide()
{
  printf("displayArcGISProGuide clicked\n");
}

void MainWindow::displayTutorial1()
{
  printf("displayTutorial1 clicked\n");
}
void MainWindow::displayTutorial2()
{
  printf("displayTutorial2 clicked\n");
}
void MainWindow::displayTutorial3()
{
  printf("displayTutorial3 clicked\n");
}
void MainWindow::displayTutorial4()
{
  printf("displayTutorial4 clicked\n");
}

void MainWindow::displayDemDownloadInstructions()
{
  printf("displayDemDownloadInstructions clicked\n");
}
void MainWindow::displayFetchDemInstructions()
{
  printf("displayFetchDemInstructions clicked\n");
}
void MainWindow::displayCommandLineInterfaceInstructions()
{
  printf("displayCommandLineInterfaceInstructions clicked\n");
}

void MainWindow::aboutWindNinja()
{
  printf("aboutWindNinja clicked\n");
}
void MainWindow::citeWindNinja()
{
  printf("citeWindNinja clicked\n");
}
void MainWindow::supportEmail()
{
  printf("supportEmail clicked\n");
}
void MainWindow::submitBugReport()
{
  printf("submitBugReport clicked\n");
}

//// end functions for Menu actions

void MainWindow::connectMenuActions()
{
  // QMenu fileMenu "File" actions
  connect(ui->openElevationInputFileMenuAction, &QAction::triggered, this, &MainWindow::elevationInputFileOpenButtonClicked);
  connect(ui->newProjectAction, &QAction::triggered, this, &MainWindow::newProject);
  connect(ui->openProjectAction, &QAction::triggered, this, &MainWindow::openProject);
  connect(ui->exportSolutionAction, &QAction::triggered, this, &MainWindow::exportSolution);
  connect(ui->closeProjectAction, &QAction::triggered, this, &MainWindow::closeProject);
  //connect(ui->exitWindNinjaAction, &QAction::triggered, this, &QCoreApplication::quit);  // exit the entire app
  connect(ui->exitWindNinjaAction, &QAction::triggered, this, &QMainWindow::close);  // just close the mainWindow (behavior of the old qt4 code)

  // QMenu optionsMenu "Options" actions
  connect(ui->enableConsoleOutputAction, &QAction::triggered, this, &MainWindow::enableConsoleOutput);
  connect(ui->writeConsoleOutputAction, &QAction::triggered, this, &MainWindow::writeConsoleOutput);

  // QMenu toolsMenu "Tools" actions
  connect(ui->resampleDataAction, &QAction::triggered, this, &MainWindow::resampleData);
  connect(ui->writeBlankStationFileAction, &QAction::triggered, this, &MainWindow::writeBlankStationFile);
  connect(ui->setConfigurationOptionAction, &QAction::triggered, this, &MainWindow::setConfigurationOption);

  // QMenu helpMenu "Help" actions

  // sub QMenu displayingShapeFilesMenu "Displaying Shapefiles" actions
  connect(ui->displayArcGISProGuideAction, &QAction::triggered, this, &MainWindow::displayArcGISProGuide);

  // sub QMenu tutorialsMenu "Tutorials" actions
  connect(ui->displayTutorial1Action, &QAction::triggered, this, &MainWindow::displayTutorial1);
  connect(ui->displayTutorial2Action, &QAction::triggered, this, &MainWindow::displayTutorial2);
  connect(ui->displayTutorial3Action, &QAction::triggered, this, &MainWindow::displayTutorial3);
  connect(ui->displayTutorial4Action, &QAction::triggered, this, &MainWindow::displayTutorial4);

  // sub QMenu instructionsMenu "Instructions" actions
  connect(ui->displayDemDownloadInstructionsAction, &QAction::triggered, this, &MainWindow::displayDemDownloadInstructions);
  connect(ui->displayFetchDemInstructionsAction, &QAction::triggered, this, &MainWindow::displayFetchDemInstructions);
  connect(ui->displayCommandLineInterfaceInstructionsAction, &QAction::triggered, this, &MainWindow::displayCommandLineInterfaceInstructions);

  // remaining non-sub QMenu actions
  connect(ui->aboutWindNinjaAction, &QAction::triggered, this, &MainWindow::aboutWindNinja);
  connect(ui->citeWindNinjaAction, &QAction::triggered, this, &MainWindow::citeWindNinja);
  connect(ui->supportEmailAction, &QAction::triggered, this, &MainWindow::supportEmail);
  connect(ui->submitBugReportAction, &QAction::triggered, this, &MainWindow::submitBugReport);
  connect(ui->aboutQtAction, &QAction::triggered, this, &QApplication::aboutQt);
}

/*
 * Click tree item helper function
 */

void MainWindow::onTreeItemClicked(QTreeWidgetItem *item, int column) {
  int pageIndex = item->data(column, Qt::UserRole).toInt();
  if (pageIndex >= 0) {
    if(pageIndex >= 6) {
      ui->inputsStackedWidget->setCurrentIndex(pageIndex+1);
    }
    else {
      ui->inputsStackedWidget->setCurrentIndex(pageIndex);
    }
  }
}

// Recursive function to add files and directories correctly with Name and Date columns
void addFilesRecursively(QStandardItem *parentItem, const QString &dirPath) {
  QDir dir(dirPath);
  QFileInfoList entries = dir.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
  for (const QFileInfo &entry : entries) {
    QStandardItem *nameItem = new QStandardItem(entry.fileName());
    QStandardItem *dateItem = new QStandardItem(entry.lastModified().toString("yyyy-MM-dd HH:mm:ss"));
    nameItem->setEditable(false);
    dateItem->setEditable(false);
    parentItem->appendRow({nameItem, dateItem});
    if (entry.isDir()) {
      addFilesRecursively(nameItem, entry.absoluteFilePath());
    }
  }
}

// Function to populate forecastDownloads with .tif parent directories and all nested contents
void MainWindow::populateForecastDownloads() {
  QString downloadsPath = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
  QDir downloadsDir(downloadsPath);

  if (!downloadsDir.exists()) return;

  QStandardItemModel *model = new QStandardItemModel(this);
  model->setHorizontalHeaderLabels({"Name", "Date Modified"});

  QDirIterator it(downloadsPath, QDir::Dirs | QDir::NoDotAndDotDot);
  while (it.hasNext()) {
    QString dirPath = it.next();
    if (dirPath.endsWith(".tif", Qt::CaseInsensitive)) {
      QStandardItem *parentItem = new QStandardItem(QFileInfo(dirPath).fileName());
      parentItem->setEditable(false);
      addFilesRecursively(parentItem, dirPath);
      model->appendRow(parentItem);
    }
  }

  ui->forecastDownloads->setModel(model);
  ui->forecastDownloads->header()->setSectionResizeMode(QHeaderView::Stretch);

  // Disable editing and enable double-click expansion
  ui->forecastDownloads->setEditTriggers(QAbstractItemView::NoEditTriggers);
  ui->forecastDownloads->setExpandsOnDoubleClick(true);
}

/*
 * Dynamic UI handling
 */

/*
 * Helper function to refresh the ui state of the app
 * Called on every user input action
 */
static void refreshUI(const Ui::MainWindow* ui)
{
  // Alias the AppState
  AppState& state = AppState::instance();

  // Define state icons
  QIcon tickIcon(":/tick.png");
  QIcon xIcon(":/cross.png");
  QIcon bulletIcon(":/bullet_blue.png");

  // Enable mouse tracking on tree
  ui->treeWidget->setMouseTracking(true);

  // Update Solver Methodology UI
  if (state.useCOMtoggled != state.useCOMMtoggled) {
    state.solverMethodologyOk = true;
    ui->treeWidget->topLevelItem(0)->setIcon(0, tickIcon);
    ui->treeWidget->topLevelItem(0)->setToolTip(0, "");
  } else if (state.useCOMtoggled && state.useCOMMtoggled) {
    state.solverMethodologyOk = false;
    ui->treeWidget->topLevelItem(0)->setIcon(0, xIcon);
    ui->treeWidget->topLevelItem(0)->setToolTip(0,"Requires exactly one selection: currently too many selections.");
  } else {
    state.solverMethodologyOk = false;
    ui->treeWidget->topLevelItem(0)->setIcon(0, xIcon);
    ui->treeWidget->topLevelItem(0)->setToolTip(0,"Requires exactly one selection: currently no selections.");
  }

  if (state.useCOMtoggled) {
    ui->treeWidget->topLevelItem(0)->child(0)->setIcon(0, tickIcon);
  } else {
    ui->treeWidget->topLevelItem(0)->child(0)->setIcon(0, bulletIcon);
  }

  if (state.useCOMMtoggled) {
    ui->treeWidget->topLevelItem(0)->child(1)->setIcon(0, tickIcon);
  } else {
    ui->treeWidget->topLevelItem(0)->child(1)->setIcon(0, bulletIcon);
  }

  /*
   * Primary state machine for inputs (surface, wind, etc.)
   */

  // Update surface input state
  if (ui->elevationInputFileLineEdit->text() != "") {
    state.surfaceInputOk = true;
    ui->treeWidget->topLevelItem(1)->child(0)->setIcon(0, tickIcon);
    ui->treeWidget->topLevelItem(1)->child(0)->setToolTip(0, "");
  } else {
    state.surfaceInputOk = false;
    ui->treeWidget->topLevelItem(1)->child(0)->setIcon(0, xIcon);
    ui->treeWidget->topLevelItem(1)->child(0)->setToolTip(0, "No DEM file detected.");
  }

  // Update diurnal input state
  if (state.diurnalInputToggled) {
    ui->treeWidget->topLevelItem(1)->child(1)->setIcon(0, tickIcon);
  } else {
    ui->treeWidget->topLevelItem(1)->child(1)->setIcon(0, bulletIcon);
  }

  // Update stability input state
  if (state.stabilityInputToggled) {
    ui->treeWidget->topLevelItem(1)->child(2)->setIcon(0, tickIcon);
  } else {
    ui->treeWidget->topLevelItem(1)->child(2)->setIcon(0, bulletIcon);
  }

  // Update domain average wind state
  if (state.domainAverageWindToggled && state.domainAverageWindInputTableOk) {
    ui->treeWidget->topLevelItem(1)->child(3)->child(0)->setIcon(0, tickIcon);
    ui->treeWidget->topLevelItem(1)->child(3)->child(0)->setToolTip(0, "");
    state.domainAverageWindOk = true;
  } else if (state.domainAverageWindToggled && !state.domainAverageWindInputTableOk){
    ui->treeWidget->topLevelItem(1)->child(3)->child(0)->setIcon(0, xIcon);
    ui->treeWidget->topLevelItem(1)->child(3)->child(0)->setToolTip(0, "Bad wind inputs; hover over red cells for explanation.");
    state.domainAverageWindOk = false;
  } else {
    ui->treeWidget->topLevelItem(1)->child(3)->child(0)->setIcon(0, bulletIcon);
    ui->treeWidget->topLevelItem(1)->child(3)->child(0)->setToolTip(0, "");
    state.domainAverageWindOk = false;
  }

  // Update point initialization state
  if (state.pointInitializationToggled) {
    ui->treeWidget->topLevelItem(1)->child(3)->child(1)->setIcon(0, tickIcon);
    state.pointInitializationOk = true;
  } else {
    ui->treeWidget->topLevelItem(1)->child(3)->child(1)->setIcon(0, bulletIcon);
    state.pointInitializationOk = false;
  }

  // Update weather model state
  if (state.weatherModelToggled) {
    ui->treeWidget->topLevelItem(1)->child(3)->child(2)->setIcon(0, tickIcon);
    state.weatherModelOk = true;
  } else {
    ui->treeWidget->topLevelItem(1)->child(3)->child(2)->setIcon(0, bulletIcon);
    state.weatherModelOk = false;
  }

  //  Update wind input
  if (state.domainAverageWindOk || state.pointInitializationOk || state.weatherModelOk) {
    ui->treeWidget->topLevelItem(1)->child(3)->setIcon(0, tickIcon);
    state.windInputOk = true;
  } else {
    ui->treeWidget->topLevelItem(1)->child(3)->setIcon(0, xIcon);
    state.windInputOk = false;
  }

  // Update overall input UI state
  if (state.surfaceInputOk && state.windInputOk) {
    state.inputsOk = true;
    ui->treeWidget->topLevelItem(1)->setIcon(0, tickIcon);
    ui->treeWidget->topLevelItem(1)->setToolTip(0, "");
  } else if (!state.surfaceInputOk && !state.windInputOk) {
    state.inputsOk = false;
    ui->treeWidget->topLevelItem(1)->setIcon(0, xIcon);
    ui->treeWidget->topLevelItem(1)->setToolTip(0, "Bad surface and wind inputs.");
  } else if (!state.surfaceInputOk) {
    state.inputsOk = false;
    ui->treeWidget->topLevelItem(1)->setIcon(0, xIcon);
    ui->treeWidget->topLevelItem(1)->setToolTip(0, "Bad surface input.");
  } else if (!state.windInputOk) {
    state.inputsOk = false;
    ui->treeWidget->topLevelItem(1)->setIcon(0, xIcon);
    ui->treeWidget->topLevelItem(1)->setToolTip(0, "Bad wind input.");
  }

  // Update solve state
  if (state.solverMethodologyOk && state.inputsOk) {
    ui->solveButton->setEnabled(true);
    ui->numberOfProcessorsSolveButton->setEnabled(true);
    ui->solveButton->setToolTip("");
    ui->numberOfProcessorsSolveButton->setToolTip("");
  } else {
    ui->solveButton->setEnabled(false);
    ui->numberOfProcessorsSolveButton->setEnabled(false);
    ui->solveButton->setToolTip("Solver Methodology and Inputs must be passing to solve.");
    ui->numberOfProcessorsSolveButton->setToolTip("Solver Methodology and Inputs must be passing to solve.");
  }
}


/*
 * Signal and slot handlers
 */

// Use selects Conservation of Mass
void MainWindow::massSolverCheckBoxClicked()
{
  AppState& state = AppState::instance();

  // Only allow CoM or CoMM to be toggledGithub requies
  if (state.useCOMMtoggled) {
    ui->massAndMomentumSolverCheckBox->setChecked(false);
    state.useCOMMtoggled = ui->massAndMomentumSolverCheckBox->isChecked();
  }

  // Update app states
  state.useCOMtoggled = ui->massSolverCheckBox->isChecked();

  // Run mesh calculator
  MainWindow::meshResolutionComboBoxCurrentIndexChanged(ui->meshResolutionComboBox->currentIndex());

  refreshUI(ui);
}


// User selects Conservation of Mass and Momentum
void MainWindow::massAndMomentumSolverCheckBoxClicked()
{
  AppState& state = AppState::instance();

  // Only allow CoM or CoMM to be toggled
  if (state.useCOMtoggled) {
    ui->massSolverCheckBox->setChecked(false);
    state.useCOMtoggled = ui->massSolverCheckBox->isChecked();
  }

  // Update app states
  state.useCOMMtoggled = ui->massAndMomentumSolverCheckBox->isChecked();

  // Run mesh calculator
  MainWindow::meshResolutionComboBoxCurrentIndexChanged(ui->meshResolutionComboBox->currentIndex());

  refreshUI(ui);
}


// User selects an elevation input file (by file)
void MainWindow::elevationInputFileLineEditTextChanged(const QString &arg1)
{
  // Get GDAL data information on DEM input
  QString fileName = ui->elevationInputFileLineEdit->text();
  double adfGeoTransform[6];
  GDALDataset *poInputDS;
  poInputDS = (GDALDataset*)GDALOpen(fileName.toStdString().c_str(), GA_ReadOnly);

  // Set driver info
  GDALDriverName = poInputDS->GetDriver()->GetDescription();
  GDALDriverLongName = poInputDS->GetDriver()->GetMetadataItem(GDAL_DMD_LONGNAME);

  // get x and y dimensions
  GDALXSize = poInputDS->GetRasterXSize();
  GDALYSize = poInputDS->GetRasterYSize();

  // Calculate cell size
  if (poInputDS->GetGeoTransform(adfGeoTransform) == CE_None) {
    double c1, c2;
    c1 = adfGeoTransform[1];
    c2 = adfGeoTransform[5];
    if (abs(c1) == abs(c2)) {
      GDALCellSize = abs(c1);
    } else {
      GDALClose((GDALDatasetH)poInputDS);
    }
  }

  // Get GDAL min/max values
  GDALRasterBand* band = poInputDS->GetRasterBand(1);
  int gotMin = 0, gotMax = 0;
  double minVal = band->GetMinimum(&gotMin);
  double maxVal = band->GetMaximum(&gotMax);

  if (!gotMin || !gotMax) {
    band->ComputeStatistics(false, &minVal, &maxVal, nullptr, nullptr, nullptr, nullptr);
  }

  GDALMinValue = minVal;
  GDALMaxValue = maxVal;

  // Close
  GDALClose((GDALDatasetH)poInputDS);

  // Run mesh calculator
  MainWindow::meshResolutionComboBoxCurrentIndexChanged(ui->meshResolutionComboBox->currentIndex());

  refreshUI(ui);
}


void MainWindow::elevationInputFileOpenButtonClicked()
{
  QString downloadsPath = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
  QString filePath = QFileDialog::getOpenFileName(this,
                                                  "Select a file",                // Window title
                                                  downloadsPath,               // Starting directory
                                                  "(*.tif);;All Files (*)"  // Filter
                                                  );
  ui->elevationInputFileLineEdit->setText(filePath);
  ui->elevationInputFileLineEdit->setToolTip(filePath);
}


// User selects an elevation input file (by map import)
void MainWindow::elevationInputFileDownloadButtonClicked()
{
  int currentIndex = ui->inputsStackedWidget->currentIndex();
  ui->inputsStackedWidget->setCurrentIndex(currentIndex+1);
}

  // User changes the mesh resolution spec for surface input
void MainWindow::meshResolutionComboBoxCurrentIndexChanged(int index)
{
  // Set value box enable for custom/other
  if (index == 3) {
    ui->meshResolutionSpinBox->setEnabled(true);
  } else {
    ui->meshResolutionSpinBox->setEnabled(false);
  }

  // default values are native mesh values

  int coarse = 4000;
  int medium = 10000;
  int fine = 20000;
  double meshResolution = 200.0;

  // initial run values, a dem file has not yet been selected
  if( GDALCellSize == 0.0 || GDALXSize == 0 || GDALYSize == 0 )
  {
    ui->meshResolutionSpinBox->setValue(meshResolution);
    return;
  }

#ifdef NINJAFOAM
  if (ui->massAndMomentumSolverCheckBox->isChecked()) {
    coarse = 25000;
    medium = 50000;
    fine = 100000;
  }
#endif //NINJAFOAM

  int targetNumHorizCells = fine;
  switch (index) {
  case 0:
    targetNumHorizCells = coarse;
    break;
  case 1:
    targetNumHorizCells = medium;
    break;
  case 2:
    targetNumHorizCells = fine;
    break;
  case 3:
    ui->meshResolutionSpinBox->setValue(ui->meshResolutionSpinBox->value());
    return;
  default:
    ui->meshResolutionSpinBox->setValue(ui->meshResolutionSpinBox->value());
    return;
  }

  // default values are native mesh values

  double XLength = GDALXSize * GDALCellSize;
  double YLength = GDALYSize * GDALCellSize;
  double nXcells = 2 * std::sqrt((double)targetNumHorizCells) * (XLength / (XLength + YLength));
  double nYcells = 2 * std::sqrt((double)targetNumHorizCells) * (YLength / (XLength + YLength));

  double XCellSize = XLength / nXcells;
  double YCellSize = YLength / nYcells;

  meshResolution = (XCellSize + YCellSize) / 2;

#ifdef NINJAFOAM
  if (ui->massAndMomentumSolverCheckBox->isChecked()) {
    XLength = GDALXSize * GDALCellSize;
    YLength = GDALYSize * GDALCellSize;

    double dz = GDALMaxValue - GDALMinValue;
    double ZLength = std::max((0.1 * std::max(XLength, YLength)), (dz + 0.1 * dz));
    double zmin, zmax;
    zmin = GDALMaxValue + 0.05 * ZLength; //zmin (above highest point in DEM for MDM)
    zmax = GDALMaxValue + ZLength; //zmax

    double volume;
    double cellCount;
    double cellVolume;

    volume = XLength * YLength * (zmax-zmin); //volume of blockMesh
    cellCount = targetNumHorizCells * 0.5; // cell count in volume 1
    cellVolume = volume/cellCount; // volume of 1 cell in blockMesh
    double side = std::pow(cellVolume, (1.0/3.0)); // length of side of cell in blockMesh

    //determine number of rounds of refinement
    int nCellsToAdd = 0;
    int refinedCellCount = 0;
    int nCellsInLowestLayer = int(XLength/side) * int(YLength/side);
    int nRoundsRefinement = 0;
    while(refinedCellCount < (0.5 * targetNumHorizCells)){
      nCellsToAdd = nCellsInLowestLayer * 8; //each cell is divided into 8 cells
      refinedCellCount += nCellsToAdd - nCellsInLowestLayer; //subtract the parent cells
      nCellsInLowestLayer = nCellsToAdd/2; //only half of the added cells are in the lowest layer
      nRoundsRefinement += 1;
    }

    meshResolution = side/(nRoundsRefinement*2.0);
  }
#endif //NINJAFOAM

  ui->meshResolutionSpinBox->setValue(meshResolution);
}

void MainWindow::meshResolutionMetersRadioButtonToggled(bool checked)
{
  if (checked) {
//    ui->meshResolutionSpinBox->setValue(ui->meshResolutionSpinBox->value() * 0.3048);
    ui->meshResolutionSpinBox->setValue(ui->meshResolutionSpinBox->value());
  }
}

void MainWindow::meshResolutionFeetRadioButtonToggled(bool checked)
{
  if (checked) {
    ui->meshResolutionSpinBox->setValue(ui->meshResolutionSpinBox->value() * 3.28084);
  }
}

// User selects a new time zone
void MainWindow::timeZoneComboBoxCurrentIndexChanged(int index)
{
  emit timeZoneDetailsRequest();
}

// User toggles show all time zones
void MainWindow::timeZoneAllZonesCheckBoxClicked()
{
  AppState& state = AppState::instance();

  // Update show all zones state
  state.showAllZones = ui->timeZoneAllZonesCheckBox->isChecked();

  emit timeZoneDataRequest();
}

// User toggles show time zone details
void MainWindow::timeZoneDetailsCheckBoxClicked()
{
  AppState& state = AppState::instance();

  // Update time zone details state
  state.displayTimeZoneDetails = ui->timeZoneDetailsCheckBox->isChecked();

  // Update visibility of details pane
  ui->timeZoneDetailsTextEdit->setVisible(state.displayTimeZoneDetails);

}

// User selects Diurnal Input
void MainWindow::diurnalCheckBoxClicked()
{
  AppState& state = AppState::instance();

  // Update UI state
  state.diurnalInputToggled = ui->diurnalCheckBox->isChecked();

  // Change the domain average input table based on diurnal wind
  QTableWidget* table = ui->domainAverageTable;
  if (!ui->diurnalCheckBox->isChecked()) {
    table->hideColumn(2);
    table->hideColumn(3);
    table->hideColumn(4);
    table->hideColumn(5);
    ui->domainAverageTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
  } else {
    table->showColumn(2);
    table->showColumn(3);
    table->showColumn(4);
    table->showColumn(5);
    ui->domainAverageTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
  }

  refreshUI(ui);
}

// User selects Stability Input
void MainWindow::stabilityCheckBoxClicked()
{
  AppState& state = AppState::instance();

  // Update UI state
  state.stabilityInputToggled = ui->stabilityCheckBox->isChecked();
  refreshUI(ui);
}

/*
 * Wind Inputs
 */

// Domain Average Wind

// User selects Domain Average Wind
void MainWindow::domainAverageCheckBoxClicked()
{
  AppState& state = AppState::instance();

  // Update the domain average wind state
  state.domainAverageWindToggled = ui->domainAverageCheckBox->isChecked();

  // Only allow one wind methodology to be used
  if (state.domainAverageWindToggled) {
    ui->pointInitializationCheckBox->setChecked(false);
    ui->useWeatherModelInit->setChecked(false);
    state.pointInitializationToggled = ui->pointInitializationCheckBox->isChecked();
    state.weatherModelToggled = ui->useWeatherModelInit->isChecked();
  }

  // Update app state
  refreshUI(ui);
}

// User changes Domain Average Wind -> Input Wind Height
void MainWindow::windHeightComboBoxCurrentIndexChanged(int index)
{
  switch(index) {
  case 0:
    ui->windHeightValue->setValue(20.00);
    ui->windHeightValue->setEnabled(false);
    ui->windHeightFeet->setChecked(true);
    ui->windHeightFeet->setEnabled(false);
    ui->windHeightMeters->setEnabled(false);
    break;

  case 1:
    ui->windHeightValue->setValue(10.00);
    ui->windHeightValue->setEnabled(false);
    ui->windHeightMeters->setChecked(true);
    ui->windHeightFeet->setEnabled(false);
    ui->windHeightMeters->setEnabled(false);
    break;

  case 2:
    ui->windHeightValue->setEnabled(true);
    ui->windHeightFeet->setEnabled(true);
    ui->windHeightMeters->setEnabled(true);
    break;
  }
}

// User clears the domain average wind input table
void MainWindow::clearTableButtonClicked()
{
  ui->domainAverageTable->clearContents();
  invalidDAWCells.clear();
  AppState::instance().domainAverageWindInputTableOk = true;
  refreshUI(ui);
}

// User changes a value in the domain average wind input table
void MainWindow::domainAverageTableCellChanged(int row, int column)
{
  QTableWidget* table = ui->domainAverageTable;
  QTableWidgetItem* item = table->item(row, column);
  if (!item) return;

  QString value = item->text().trimmed();
  bool valid = false;
  QString errorMessage;

  // Allow empty input
  if (value.isEmpty()) {
    valid = true;
  } else {
    switch (column) {
    case 0: {
      double d = value.toDouble(&valid);
      if (!valid || d <= 0)
        valid = false;
        errorMessage = "Must be a positive number";
      break;
    }
    case 1: {
      int i = value.toDouble(&valid);
      if (!valid || i < 0 || i > 359.9) {
        valid = false;
        errorMessage = "Must be a number between 0 and 359";
      }
      break;
    }
    case 2: {
      QTime t = QTime::fromString(value, "hh:mm");
      valid = t.isValid();
      if (!valid) errorMessage = "Must be a valid 24h time (hh:mm)";
      break;
    }
    case 3: {
      QDate d = QDate::fromString(value, "MM/dd/yyyy");
      valid = d.isValid();
      if (!valid) errorMessage = "Must be a valid date (MM/DD/YYYY)";
      break;
    }
    case 4: {
      int i = value.toDouble(&valid);
      if (!valid || i < 0 || i > 100) {
        valid = false;
        errorMessage = "Must be a number between 0 and 100";
      }
      break;
    }
    case 5: {
      value.toInt(&valid);
      if (!valid) errorMessage = "Must be an integer";
      break;
    }
    default:
      valid = true;
    }
  }

  QPair<int, int> cell(row, column);
  if (!valid) {
    invalidDAWCells.insert(cell);
    item->setBackground(Qt::red);
    item->setToolTip(errorMessage);
  } else {
    invalidDAWCells.remove(cell);
    item->setBackground(QBrush());  // Reset to default
    item->setToolTip("");
  }

  AppState::instance().domainAverageWindInputTableOk = invalidDAWCells.isEmpty();
  refreshUI(ui);
}

// User selects Point Initialization wind model
void MainWindow::pointInitializationCheckBoxClicked()
{
  AppState& state = AppState::instance();

  // Update the domain average wind state
  state.pointInitializationToggled = ui->pointInitializationCheckBox->isChecked();

  // Only allow one wind methodology to be used
  if (state.pointInitializationToggled) {
    ui->domainAverageCheckBox->setChecked(false);
    ui->useWeatherModelInit->setChecked(false);
    state.domainAverageWindToggled = ui->domainAverageCheckBox->isChecked();
    state.weatherModelToggled = ui->useWeatherModelInit->isChecked();
  }

  // Update app state
  refreshUI(ui);
}

// User selects Weather Model Initialization model
void MainWindow::useWeatherModelInitClicked()
{
  AppState& state = AppState::instance();

  // Update the domain average wind state
  state.weatherModelToggled = ui->useWeatherModelInit->isChecked();

  // Only allow one wind methodology to be used
  if (state.weatherModelToggled) {
    ui->domainAverageCheckBox->setChecked(false);
    ui->pointInitializationCheckBox->setChecked(false);
    state.domainAverageWindToggled = ui->domainAverageCheckBox->isChecked();
    state.pointInitializationToggled = ui->pointInitializationCheckBox->isChecked();
  }

  // Update app state
  refreshUI(ui);
}

// User selects a new output location
void MainWindow::outputDirectoryButtonClicked()
{
  QString currentPath = ui->outputDirectoryTextEdit->toPlainText();
  QString downloadsPath = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
  QString dirPath = QFileDialog::getExistingDirectory(this,
                                                      "Select a directory",  // Window title
                                                      currentPath,         // Starting location
                                                      QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

  if (!dirPath.isEmpty()) {
    ui->outputDirectoryTextEdit->setText(dirPath);
    ui->outputDirectoryTextEdit->setToolTip(dirPath);
  }
}

// User selects the solve Button on the solver page
void MainWindow::numberOfProcessorsSolveButtonClicked()
{
  ui->solveButton->click();
}

// User selects the primary solve Button
void MainWindow::solveButtonClicked()
{
  emit solveRequest();
}

// Enable double clicking on tree menu items
void MainWindow::treeWidgetItemDoubleClicked(QTreeWidgetItem *item, int column)
{
  if (item->text(0) == "Conservation of Mass") {
    ui->massSolverCheckBox->click();
  } else if (item->text(0) == "Conservation of Mass and Momentum") {
    ui->massAndMomentumSolverCheckBox->click();
  } else if (item->text(0) == "Diurnal Input") {
    ui->diurnalCheckBox->click();
  } else if (item->text(0) == "Stability Input") {
    ui->stabilityCheckBox->click();
  } else if (item->text(0) == "Domain Average Wind") {
    ui->domainAverageCheckBox->click();
  } else if (item->text(0) == "Point Initialization") {
    ui->pointInitializationCheckBox->click();
  } else if (item->text(0) == "Weather Model") {
    ui->useWeatherModelInit->click();
  }
}

void MainWindow::loadMapKMZ(const std::vector<std::string>&  input){

  for (const auto& dir : input) {
    QString qDir = QString::fromStdString(dir);

    QFile f(qDir);
    f.open(QIODevice::ReadOnly);
    QByteArray data = f.readAll();
    QString base64 = data.toBase64();

    webView->page()->runJavaScript("loadKmzFromBase64('"+base64+"')");
  }

}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow)
{
  ui->setupUi(this);

  checkMessages();

  // Set default window size
  resize(1200, 700);

  // Immediately call a UI refresh to set initial states
  refreshUI(ui);
  // Expand tree UI
  ui->treeWidget->expandAll();

  /*
   * Create file handler window for point init screen
   */

  // Get the correct Downloads folder path
  QString downloadsPath = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);

  // Enable QFileSystemModel to process directories and files
  QFileSystemModel *model = new QFileSystemModel(this);
  model->setFilter(QDir::NoDotAndDotDot | QDir::AllDirs | QDir::AllEntries);  // Ensure files appear
  model->setRootPath(downloadsPath);

  // Enable file watching so contents refresh properly
  model->setReadOnly(false);
  model->setResolveSymlinks(true);

  // Create a filtering model
  DirectoryFilterModel *filterModel = new DirectoryFilterModel();
  filterModel->setSourceModel(model);

  // Set the correct root index inside Downloads
  QModelIndex rootIndex = model->index(downloadsPath);
  ui->treeFileExplorer->setModel(filterModel);
  ui->treeFileExplorer->setRootIndex(filterModel->mapFromSource(rootIndex));

  // Ensure folders expand and collapse correctly
  ui->treeFileExplorer->setExpandsOnDoubleClick(true);
  ui->treeFileExplorer->setAnimated(true);
  ui->treeFileExplorer->setIndentation(15);
  ui->treeFileExplorer->setSortingEnabled(true);
  ui->treeFileExplorer->setItemsExpandable(true);
  ui->treeFileExplorer->setUniformRowHeights(true);

  // Show only "Name" and "Date Modified" columns
  ui->treeFileExplorer->hideColumn(1);  // Hide Size column
  ui->treeFileExplorer->hideColumn(2);  // Hide Type column

  // Optional: Set column headers
  QHeaderView *header = ui->treeFileExplorer->header();
  header->setSectionResizeMode(0, QHeaderView::Interactive);  // Name fits content
  header->setSectionResizeMode(3, QHeaderView::Stretch);           // Date Modified stretches
  model->setHeaderData(0, Qt::Horizontal, "Name");
  model->setHeaderData(3, Qt::Horizontal, "Date Modified");

  // Force model to reload children
  ui->treeFileExplorer->expandAll();  // Force expand all to check visibility

  /*
   * Functionality for the map widget
   */

  // Enable remote content
  QWebEngineProfile::defaultProfile()->settings()->setAttribute(QWebEngineSettings::LocalContentCanAccessRemoteUrls, true);
  QWebEngineProfile::defaultProfile()->settings()->setAttribute(QWebEngineSettings::LocalContentCanAccessFileUrls, true);

  // Resolve the map file path
  QString filePath = QString(MAP_PATH);

  //Load HTML file with Leaflet
  webView = new QWebEngineView(ui->mapPanelWidget);
  QUrl url = QUrl::fromLocalFile(filePath);
  webView->setUrl(url);

  // Set up layout
  QVBoxLayout *layout = new QVBoxLayout();
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(webView);

  // Apply
  ui->mapPanelWidget->setLayout(layout);

  /*
   * Connect tree items to stacked tab window
   */

  // Top-level items
  ui->inputsStackedWidget->setCurrentIndex(0);
  ui->treeWidget->topLevelItem(0)->setData(0, Qt::UserRole, 1);  // Solver Methodology (Page 0)
  ui->treeWidget->topLevelItem(1)->setData(0, Qt::UserRole, 4);  // Inputs (Page 5)
  ui->treeWidget->topLevelItem(2)->setData(0, Qt::UserRole, 12); // Inputs (Page 13)

  // Sub-items for Solver Methodology
  ui->treeWidget->topLevelItem(0)->child(0)->setData(0, Qt::UserRole, 2);  // Conservation of Mass (Page 1)
  ui->treeWidget->topLevelItem(0)->child(1)->setData(0, Qt::UserRole, 3);  // Conservation of Mass and Momentum (Page 2)

  // Sub-items for Inputs
  ui->treeWidget->topLevelItem(1)->child(0)->setData(0, Qt::UserRole, 5);  // Surface Input (Page 6)
  ui->treeWidget->topLevelItem(1)->child(1)->setData(0, Qt::UserRole, 6);  // Dirunal Input (Page 7)
  ui->treeWidget->topLevelItem(1)->child(2)->setData(0, Qt::UserRole, 7);  // Stability Input (Page 8)
  ui->treeWidget->topLevelItem(1)->child(3)->setData(0, Qt::UserRole, 8);  // Wind Input (Page 9)

  // Sub-sub-items for Wind Input
  QTreeWidgetItem *windInputItem = ui->treeWidget->topLevelItem(1)->child(3);
  windInputItem->child(0)->setData(0, Qt::UserRole, 9);  // Domain Average Wind (Page 9)
  windInputItem->child(1)->setData(0, Qt::UserRole, 10); // Point Init (Page 10)
  windInputItem->child(2)->setData(0, Qt::UserRole, 11); // Weather Model (Page 11)

  connect(ui->treeWidget, &QTreeWidget::itemClicked, this, &MainWindow::onTreeItemClicked);

  connectMenuActions();

  /*
   * Downloaded Forecast explorer
   */

  populateForecastDownloads();

  /*
   * Basic initial setup steps
   */

  // Surface Input window
  // Set icons
  ui->elevationInputFileOpenButton->setIcon(QIcon(":/folder.png"));
  ui->elevationInputFileDownloadButton->setIcon(QIcon(":/swoop_final.png"));

  // Solver window
  // Update processor count and set user input default value & upper bound
  int nCPUs = QThread::idealThreadCount();
  ui->availableProcessorsTextEdit->setPlainText("Available Processors:  " + QString::number(nCPUs));
  ui->numberOfProcessorsSpinBox->setMaximum(nCPUs);
  ui->numberOfProcessorsSpinBox->setValue(nCPUs);

  // Wind Input -> Point Init window
  ui->downloadPointInitData->setIcon(QIcon(":/application_get"));

  // Populate default location for output location
  ui->outputDirectoryTextEdit->setText(downloadsPath);
  ui->outputDirectoryButton->setIcon(QIcon(":/folder.png"));

  // Set initial visibility of time zone details
  ui->timeZoneDetailsTextEdit->setVisible(false);

  // Set initial formatting of domain average input table
  ui->domainAverageTable->hideColumn(2);
  ui->domainAverageTable->hideColumn(3);
  ui->domainAverageTable->hideColumn(4);
  ui->domainAverageTable->hideColumn(5);
  ui->domainAverageTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

  connect(ui->elevationInputTypeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
          ui->elevationInputTypeStackedWidget, &QStackedWidget::setCurrentIndex);

  connect(ui->massSolverCheckBox, &QCheckBox::clicked, this, &MainWindow::massSolverCheckBoxClicked);
  connect(ui->massAndMomentumSolverCheckBox, &QCheckBox::clicked, this, &MainWindow::massAndMomentumSolverCheckBoxClicked);

  connect(ui->elevationInputFileDownloadButton, &QPushButton::clicked, this, &MainWindow::elevationInputFileDownloadButtonClicked);
  connect(ui->elevationInputFileOpenButton, &QPushButton::clicked, this, &MainWindow::elevationInputFileOpenButtonClicked);
  connect(ui->elevationInputFileLineEdit, &QLineEdit::textChanged, this, &MainWindow::elevationInputFileLineEditTextChanged);

  connect(ui->meshResolutionComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::meshResolutionComboBoxCurrentIndexChanged);

  connect(ui->diurnalCheckBox, &QCheckBox::clicked, this, &MainWindow::diurnalCheckBoxClicked);
  connect(ui->stabilityCheckBox, &QCheckBox::clicked, this, &MainWindow::stabilityCheckBoxClicked);
  connect(ui->windHeightComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::windHeightComboBoxCurrentIndexChanged);
  connect(ui->domainAverageCheckBox, &QCheckBox::clicked, this, &MainWindow::domainAverageCheckBoxClicked);

  connect(ui->treeWidget, &QTreeWidget::itemDoubleClicked, this, &MainWindow::treeWidgetItemDoubleClicked);

  connect(ui->pointInitializationCheckBox, &QCheckBox::clicked, this, &MainWindow::pointInitializationCheckBoxClicked);
  connect(ui->useWeatherModelInit, &QCheckBox::clicked, this, &MainWindow::useWeatherModelInitClicked);

  connect(ui->clearTableButton, &QPushButton::clicked, this, &MainWindow::clearTableButtonClicked);
  connect(ui->solveButton, &QPushButton::clicked, this, &MainWindow::solveButtonClicked);
  connect(ui->outputDirectoryButton, &QPushButton::clicked, this, &MainWindow::outputDirectoryButtonClicked);
  connect(ui->numberOfProcessorsSolveButton, &QPushButton::clicked, this, &MainWindow::numberOfProcessorsSolveButtonClicked);

  connect(ui->timeZoneAllZonesCheckBox, &QCheckBox::clicked, this, &MainWindow::timeZoneAllZonesCheckBoxClicked);
  connect(ui->timeZoneDetailsCheckBox, &QCheckBox::clicked, this, &MainWindow::timeZoneDetailsCheckBoxClicked);
  connect(ui->timeZoneComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::timeZoneComboBoxCurrentIndexChanged);

  connect(ui->domainAverageTable, &QTableWidget::cellChanged, this, &MainWindow::domainAverageTableCellChanged);

  connect(ui->meshResolutionMetersRadioButton, &QRadioButton::toggled, this, &MainWindow::meshResolutionMetersRadioButtonToggled);
  connect(ui->meshResolutionFeetRadioButton, &QRadioButton::toggled, this, &MainWindow::meshResolutionFeetRadioButtonToggled);

  connect(ui->surfaceInputDownloadCancelButton, &QPushButton::clicked, this, &MainWindow::surfaceInputDownloadCancelButtonClicked);


}

void MainWindow::surfaceInputDownloadCancelButtonClicked()
{
  int currentIndex = ui->inputsStackedWidget->currentIndex();
  ui->inputsStackedWidget->setCurrentIndex(currentIndex-1);
}

/*
** Check for version updates, or messages from the server.
*/
void MainWindow::checkMessages(void) {
  QMessageBox mbox;
  char *papszMsg = NinjaQueryServerMessages(true);
  if (papszMsg != NULL) {
    if (strcmp(papszMsg, "TRUE\n") == 0) {
      mbox.setText("There is a fatal flaw in Windninja, it must close.");
      mbox.exec();
      delete[] papszMsg;
      abort();
    }

    else {
      char *papszMsg = NinjaQueryServerMessages(false);
      if (papszMsg != NULL) {
        mbox.setText(papszMsg);

        mbox.exec();
        delete[] papszMsg;
      }
    }
  }
}

/*
** Query the ninjastorm.firelab.org/sqlitetest/messages.txt and ask for the most up to date version.
** There are three current values:
**
** VERSION -> a semantic version string, comparable with strcmp()
** MESSAGE -> One or more messages to display to the user
** ABORT   -> There is a fundamental problem with windninja, and it should call
**            abort() after displaying a message.
*/

bool MainWindow::NinjaCheckVersions(char * mostrecentversion, char * localversion) {
  char comparemostrecentversion[256];
  char comparelocalversion[256];
  int mostrecentversionIndex = 0;
  int localversionIndex = 0;
  while (*mostrecentversion) {
    if (*mostrecentversion != '.') {
      comparemostrecentversion[mostrecentversionIndex++] = *mostrecentversion;
    }
    mostrecentversion++;
  }
  comparemostrecentversion[mostrecentversionIndex] = '\0';

  while (*localversion) {
    if (*localversion != '.') {
      comparelocalversion[localversionIndex++] = *localversion;
    }
    localversion++;
  }

  comparelocalversion[localversionIndex] = '\0';
  return strcmp(comparemostrecentversion, comparelocalversion) == 0;

}

char * MainWindow::NinjaQueryServerMessages(bool checkAbort)
{
  CPLSetConfigOption("GDAL_HTTP_UNSAFESSL", "YES");
  const char* url = "https://ninjastorm.firelab.org/sqlitetest/messages.txt";
  CPLHTTPResult *poResult = CPLHTTPFetch(url, NULL);
  CPLSetConfigOption( "GDAL_HTTP_TIMEOUT", NULL );
  if( !poResult || poResult->nStatus != 0 || poResult->nDataLen == 0 )
  {
    CPLDebug( "NINJA", "Failed to reach the ninjastorm server." );
    return NULL;
  }

  const char* pszTextContent = reinterpret_cast<const char*>(poResult->pabyData);
  std::vector<std::string> messages;
  std::istringstream iss(pszTextContent);
  std::string message;

         // Read all lines into the vector
  while (std::getline(iss, message)) {
    messages.push_back(message);
  }

         // Process all lines except the last two
  std::ostringstream oss;
  if (checkAbort) {
    for (size_t i = 0; i < messages.size(); ++i) {
      if (i == messages.size()-1) { // check final line
        oss << messages[i] << "\n";
      }
    }
  }
  else {
    bool versionisuptodate = NinjaCheckVersions(const_cast<char*>(messages[1].c_str()), const_cast<char*>(NINJA_VERSION_STRING));
    if (!versionisuptodate) {
      oss << messages[0] << "\n";
      oss << "You are using an outdated WindNinja version, please update to version: " << messages[1] << "\n" << "\n";
    }
    if (messages[4].empty() == false) {
      for (size_t i = 3; i < messages.size() - 2; ++i) {
        if (!messages[i].empty()) {
          oss << messages[i] << "\n";
        }
      }
    }
    if (messages[4].empty() && versionisuptodate) {
      return NULL;
    }
  }

  std::string resultingmessage = oss.str();
  char* returnString = new char[resultingmessage.length() + 1];
  std::strcpy(returnString, resultingmessage.c_str());
  CPLHTTPDestroyResult(poResult);
  return returnString;

  return NULL;
}
