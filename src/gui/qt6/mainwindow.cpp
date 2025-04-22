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

/*
 * Click tree item helper function
 */

void MainWindow::onTreeItemClicked(QTreeWidgetItem *item, int column) {
  int pageIndex = item->data(column, Qt::UserRole).toInt();
  if (pageIndex >= 0) {
    ui->stackedInputPage->setCurrentIndex(pageIndex);
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
  if (ui->elevFilePath->text() != "") {
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
    ui->solverPageSolveBtn->setEnabled(true);
    ui->solveButton->setToolTip("");
    ui->solverPageSolveBtn->setToolTip("");
  } else {
    ui->solveButton->setEnabled(false);
    ui->solverPageSolveBtn->setEnabled(false);
    ui->solveButton->setToolTip("Solver Methodology and Inputs must be passing to solve.");
    ui->solverPageSolveBtn->setToolTip("Solver Methodology and Inputs must be passing to solve.");
  }
}


/*
 * Signal and slot handlers
 */

// Use selects Conservation of Mass
void MainWindow::on_useCOM_clicked()
{
  AppState& state = AppState::instance();

  // Only allow CoM or CoMM to be toggled
  if (state.useCOMMtoggled) {
    ui->useCOMM->setChecked(false);
    state.useCOMMtoggled = ui->useCOMM->isChecked();
  }

  // Update app states
  state.useCOMtoggled = ui->useCOM->isChecked();
  refreshUI(ui);
}


// User selects Conservation of Mass and Momentum
void MainWindow::on_useCOMM_clicked()
{
  AppState& state = AppState::instance();

  // Only allow CoM or CoMM to be toggled
  if (state.useCOMtoggled) {
    ui->useCOM->setChecked(false);
    state.useCOMtoggled = ui->useCOM->isChecked();
  }

  // Update app states
  state.useCOMMtoggled = ui->useCOMM->isChecked();
  refreshUI(ui);
}


// User selects an elevation input file (by file)
void MainWindow::on_elevFilePath_textChanged(const QString &arg1)
{
  refreshUI(ui);
}


void MainWindow::on_openFileButton_clicked()
{
  QString downloadsPath = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
  QString filePath = QFileDialog::getOpenFileName(this,
                                                  "Select a file",                // Window title
                                                  downloadsPath,               // Starting directory
                                                  "(*.tif);;All Files (*)"  // Filter
                                                  );
  ui->elevFilePath->setText(filePath);
  ui->elevFilePath->setToolTip(filePath);
}


// User selects an elevation input file (by map import)
// TODO: get DEM file on button press
void MainWindow::on_getFromMapButton_clicked()
{
  // We have to use batching since the Javascript part is async
  struct JSFieldBatch {
    QMap<QString, QVariant> results;
    int expected = 7;
    std::function<void(QMap<QString, QVariant>)> onReady;
  };

  auto batch = new JSFieldBatch();
  batch->onReady = [this, batch](QMap<QString, QVariant> fields) {
    northLat = fields["north_lat"].toDouble();
    southLat = fields["south_lat"].toDouble();
    eastLon  = fields["east_lon"].toDouble();
    westLon  = fields["west_lon"].toDouble();
    centerLat = fields["center_lat"].toDouble();
    centerLon = fields["center_lon"].toDouble();
    radius = fields["radius"].toDouble();

    delete batch;  // clean up
  };

  auto run = [this, batch](QString fieldId) {
    webView->page()->runJavaScript(QString("document.getElementById('%1').value;").arg(fieldId),
                                   [fieldId, batch](const QVariant &result) {
                                     batch->results[fieldId] = result;
                                     if (batch->results.size() == batch->expected) {
                                       batch->onReady(batch->results);
                                     }
                                   });
  };

  // Kick off all the field reads
  run("north_lat");
  run("south_lat");
  run("east_lon");
  run("west_lon");
  run("center_lat");
  run("center_lon");
  run("radius");

  // Verify input validity
  if (northLat != 0 && southLat != 0 && eastLon != 0 && westLon != 0) {
    // AppState::instance().
  }
  qDebug() << northLat;
  qDebug() << southLat;
  qDebug() << eastLon;
  qDebug() << westLon;
  qDebug() << centerLat;
  qDebug() << centerLon;
  qDebug() << radius;
}

// User changes the mesh resolution spec for surface input
void MainWindow::on_meshResType_currentIndexChanged(int index)
{
  switch(index) {
  case 0:
    if (ui->meshResFeet->isChecked()) {
      ui->meshResValue->setValue(256.34);
    } else {
      ui->meshResValue->setValue(78.13);
    }
    ui->meshResValue->setEnabled(false);
    break;

  case 1:
    if (ui->meshResFeet->isChecked()) {
      ui->meshResValue->setValue(162.12);
    } else {
      ui->meshResValue->setValue(49.41);
    }
    ui->meshResValue->setEnabled(false);
    break;

  case 2:
    if (ui->meshResFeet->isChecked()) {
      ui->meshResValue->setValue(114.64);
    } else {
      ui->meshResValue->setValue(34.94);
    }
    ui->meshResValue->setEnabled(false);
    break;

  case 3:
    ui->meshResValue->setEnabled(true);
    break;

  }
}

// User selects new mesh resolution unit
void MainWindow::on_meshResFeet_clicked()
{
  MainWindow::on_meshResType_currentIndexChanged(ui->meshResType->currentIndex());
}


void MainWindow::on_meshResMeters_clicked()
{
  MainWindow::on_meshResType_currentIndexChanged(ui->meshResType->currentIndex());
}

// User selects a new time zone
void MainWindow::on_timeZoneSelector_currentIndexChanged(int index)
{
  emit timeZoneDetailsRequest();
}

// User toggles show all time zones
void MainWindow::on_showAllTimeZones_clicked()
{
  AppState& state = AppState::instance();

  // Update show all zones state
  state.showAllZones = ui->showAllTimeZones->isChecked();

  emit timeZoneDataRequest();
}

// User toggles show time zone details
void MainWindow::on_displayTimeZoneDetails_clicked()
{
  AppState& state = AppState::instance();

  // Update time zone details state
  state.displayTimeZoneDetails = ui->displayTimeZoneDetails->isChecked();

  // Update visibility of details pane
  ui->timeZoneDetails->setVisible(state.displayTimeZoneDetails);

}

// User selects Diurnal Input
void MainWindow::on_useDiurnalWind_clicked()
{
  AppState& state = AppState::instance();

  // Update UI state
  state.diurnalInputToggled = ui->useDiurnalWind->isChecked();

  // Change the domain average input table based on diurnal wind
  QTableWidget* table = ui->windTableData;
  if (!ui->useDiurnalWind->isChecked()) {
    table->hideColumn(2);
    table->hideColumn(3);
    table->hideColumn(4);
    table->hideColumn(5);
    ui->windTableData->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
  } else {
    table->showColumn(2);
    table->showColumn(3);
    table->showColumn(4);
    table->showColumn(5);
    ui->windTableData->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
  }

  refreshUI(ui);
}

// User selects Stability Input
void MainWindow::on_useStability_clicked()
{
  AppState& state = AppState::instance();

  // Update UI state
  state.stabilityInputToggled = ui->useStability->isChecked();
  refreshUI(ui);
}

/*
 * Wind Inputs
 */

// Domain Average Wind

// User selects Domain Average Wind
void MainWindow::on_useDomainAvgWind_clicked()
{
  AppState& state = AppState::instance();

  // Update the domain average wind state
  state.domainAverageWindToggled = ui->useDomainAvgWind->isChecked();

  // Only allow one wind methodology to be used
  if (state.domainAverageWindToggled) {
    ui->usePointInit->setChecked(false);
    ui->useWeatherModelInit->setChecked(false);
    state.pointInitializationToggled = ui->usePointInit->isChecked();
    state.weatherModelToggled = ui->useWeatherModelInit->isChecked();
  }

  // Update app state
  refreshUI(ui);
}

// User changes Domain Average Wind -> Input Wind Height
void MainWindow::on_domainAvgPicklist_currentIndexChanged(int index)
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
void MainWindow::on_clearDAWtable_clicked()
{
  ui->windTableData->clearContents();
  invalidDAWCells.clear();
  AppState::instance().domainAverageWindInputTableOk = true;
  refreshUI(ui);
}

// User changes a value in the domain average wind input table
void MainWindow::on_windTableData_cellChanged(int row, int column)
{
  QTableWidget* table = ui->windTableData;
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
void MainWindow::on_usePointInit_clicked()
{
  AppState& state = AppState::instance();

  // Update the domain average wind state
  state.pointInitializationToggled = ui->usePointInit->isChecked();

  // Only allow one wind methodology to be used
  if (state.pointInitializationToggled) {
    ui->useDomainAvgWind->setChecked(false);
    ui->useWeatherModelInit->setChecked(false);
    state.domainAverageWindToggled = ui->useDomainAvgWind->isChecked();
    state.weatherModelToggled = ui->useWeatherModelInit->isChecked();
  }

  // Update app state
  refreshUI(ui);
}

// User selects Weather Model Initialization model
void MainWindow::on_useWeatherModelInit_clicked()
{
  AppState& state = AppState::instance();

  // Update the domain average wind state
  state.weatherModelToggled = ui->useWeatherModelInit->isChecked();

  // Only allow one wind methodology to be used
  if (state.weatherModelToggled) {
    ui->useDomainAvgWind->setChecked(false);
    ui->usePointInit->setChecked(false);
    state.domainAverageWindToggled = ui->useDomainAvgWind->isChecked();
    state.pointInitializationToggled = ui->usePointInit->isChecked();
  }

  // Update app state
  refreshUI(ui);
}

// User selects a new output location
void MainWindow::on_outputSaveLocationBtn_clicked()
{
  QString currentPath = ui->outputDirectory->toPlainText();
  QString downloadsPath = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
  QString dirPath = QFileDialog::getExistingDirectory(this,
                                                      "Select a directory",  // Window title
                                                      currentPath,         // Starting location
                                                      QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

  if (!dirPath.isEmpty()) {
    ui->outputDirectory->setText(dirPath);
    ui->outputDirectory->setToolTip(dirPath);
  }
}

// User selects the solve button on the solver page
void MainWindow::on_solverPageSolveBtn_clicked()
{
  ui->solveButton->click();
}

// User selects the primary solve button
void MainWindow::on_solveButton_clicked()
{
  emit solveRequest();
}

// Enable double clicking on tree menu items
void MainWindow::on_treeWidget_itemDoubleClicked(QTreeWidgetItem *item, int column)
{
  if (item->text(0) == "Conservation of Mass") {
    ui->useCOM->click();
  } else if (item->text(0) == "Conservation of Mass and Momentum") {
    ui->useCOMM->click();
  } else if (item->text(0) == "Diurnal Input") {
    ui->useDiurnalWind->click();
  } else if (item->text(0) == "Stability Input") {
    ui->useStability->click();
  } else if (item->text(0) == "Domain Average Wind") {
    ui->useDomainAvgWind->click();
  } else if (item->text(0) == "Point Initialization") {
    ui->usePointInit->click();
  } else if (item->text(0) == "Weather Model") {
    ui->useWeatherModelInit->click();
  }
}


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow)
{
  ui->setupUi(this);

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
  ui->stackedInputPage->setCurrentIndex(0);
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

  /*
   * Downloaded Forecast explorer
   */

  populateForecastDownloads();

  /*
   * Basic initial setup steps
   */

  // Surface Input window
  // Set icons
  ui->openFileButton->setIcon(QIcon(":/folder.png"));
  ui->getFromMapButton->setIcon(QIcon(":/swoop_final.png"));

  // Solver window
  // Update processor count and set user input default value & upper bound
  int cpuCount = QThread::idealThreadCount();
  ui->availableProcessors->setPlainText("Available Processors:  " + QString::number(cpuCount));
  ui->numProcessorsSpinbox->setMaximum(cpuCount);
  ui->numProcessorsSpinbox->setValue(cpuCount);

  // Wind Input -> Point Init window
  ui->downloadPointInitData->setIcon(QIcon(":/application_get"));

  // Populate default location for output location
  ui->outputDirectory->setText(downloadsPath);
  ui->outputSaveLocationBtn->setIcon(QIcon(":/folder.png"));

  // Set initial visibility of time zone details
  ui->timeZoneDetails->setVisible(false);

  // Set initial formatting of domain average input table
    ui->windTableData->hideColumn(2);
    ui->windTableData->hideColumn(3);
    ui->windTableData->hideColumn(4);
    ui->windTableData->hideColumn(5);
    ui->windTableData->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

}

