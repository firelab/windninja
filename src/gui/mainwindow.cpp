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
#include "../ninja/windninja.h"


/*
 * Helper function to refresh the ui state of the app
 * Called on every user input action
 */
void MainWindow::refreshUI()
{
  AppState& state = AppState::instance();

  QIcon tickIcon(":/tick.png");
  QIcon xIcon(":/cross.png");
  QIcon bulletIcon(":/bullet_blue.png");

  ui->treeWidget->setMouseTracking(true);

  // Update Solver Methodology UI
  if (state.isMassSolverToggled != state.isMomentumSolverToggled) {
    state.isSolverMethodologyValid = true;
    ui->treeWidget->topLevelItem(0)->setIcon(0, tickIcon);
    ui->treeWidget->topLevelItem(0)->setToolTip(0, "");
  } else if (state.isMassSolverToggled && state.isMomentumSolverToggled) {
    state.isSolverMethodologyValid = false;
    ui->treeWidget->topLevelItem(0)->setIcon(0, xIcon);
    ui->treeWidget->topLevelItem(0)->setToolTip(0,"Requires exactly one selection: currently too many selections.");
  } else {
    state.isSolverMethodologyValid = false;
    ui->treeWidget->topLevelItem(0)->setIcon(0, xIcon);
    ui->treeWidget->topLevelItem(0)->setToolTip(0,"Requires exactly one selection: currently no selections.");
  }

  if (state.isMassSolverToggled) {
    ui->treeWidget->topLevelItem(0)->child(0)->setIcon(0, tickIcon);
  } else {
    ui->treeWidget->topLevelItem(0)->child(0)->setIcon(0, bulletIcon);
  }

  if (state.isMomentumSolverToggled) {
    ui->treeWidget->topLevelItem(0)->child(1)->setIcon(0, tickIcon);
  } else {
    ui->treeWidget->topLevelItem(0)->child(1)->setIcon(0, bulletIcon);
  }

  // Update surface input state
  if (ui->elevationInputFileLineEdit->text() != "") {
    state.isSurfaceInputValid = true;
    ui->treeWidget->topLevelItem(1)->child(0)->setIcon(0, tickIcon);
    ui->treeWidget->topLevelItem(1)->child(0)->setToolTip(0, "");
  } else {
    state.isSurfaceInputValid = false;
    ui->treeWidget->topLevelItem(1)->child(0)->setIcon(0, xIcon);
    ui->treeWidget->topLevelItem(1)->child(0)->setToolTip(0, "No DEM file detected.");
  }

  // Update diurnal input state
  if (state.isDiurnalInputToggled) {
    ui->treeWidget->topLevelItem(1)->child(1)->setIcon(0, tickIcon);
  } else {
    ui->treeWidget->topLevelItem(1)->child(1)->setIcon(0, bulletIcon);
  }

  // Update stability input state
  if (state.isStabilityInputToggled) {
    ui->treeWidget->topLevelItem(1)->child(2)->setIcon(0, tickIcon);
  } else {
    ui->treeWidget->topLevelItem(1)->child(2)->setIcon(0, bulletIcon);
  }

  // Update domain average initialization
  if (state.isDomainAverageInitializationToggled && state.isDomainAverageWindInputTableValid) {
    ui->treeWidget->topLevelItem(1)->child(3)->child(0)->setIcon(0, tickIcon);
    ui->treeWidget->topLevelItem(1)->child(3)->child(0)->setToolTip(0, "");
    state.isDomainAverageInitializationValid = true;
  } else if (state.isDomainAverageInitializationToggled && !state.isDomainAverageWindInputTableValid){
    ui->treeWidget->topLevelItem(1)->child(3)->child(0)->setIcon(0, xIcon);
    ui->treeWidget->topLevelItem(1)->child(3)->child(0)->setToolTip(0, "Bad wind inputs; hover over red cells for explanation.");
    state.isDomainAverageInitializationValid = false;
  } else {
    ui->treeWidget->topLevelItem(1)->child(3)->child(0)->setIcon(0, bulletIcon);
    ui->treeWidget->topLevelItem(1)->child(3)->child(0)->setToolTip(0, "");
    state.isDomainAverageInitializationValid = false;
  }

  // Update point initialization
  if (state.isPointInitializationToggled) {
    ui->treeWidget->topLevelItem(1)->child(3)->child(1)->setIcon(0, tickIcon);
    state.isPointInitializationValid = true;
  } else {
    ui->treeWidget->topLevelItem(1)->child(3)->child(1)->setIcon(0, bulletIcon);
    state.isPointInitializationValid = false;
  }

  // Update weather model initialization
  if (state.isWeatherModelInitializationToggled) {
    ui->treeWidget->topLevelItem(1)->child(3)->child(2)->setIcon(0, tickIcon);
    state.isWeatherModelInitializationValid = true;
  } else {
    ui->treeWidget->topLevelItem(1)->child(3)->child(2)->setIcon(0, bulletIcon);
    state.isWeatherModelInitializationValid = false;
  }

  //  Update wind input
  if (state.isDomainAverageInitializationValid || state.isPointInitializationValid || state.isWeatherModelInitializationValid) {
    ui->treeWidget->topLevelItem(1)->child(3)->setIcon(0, tickIcon);
    state.isWindInputValid = true;
  } else {
    ui->treeWidget->topLevelItem(1)->child(3)->setIcon(0, xIcon);
    state.isWindInputValid = false;
  }

  // Update overall input UI state
  if (state.isSurfaceInputValid && state.isWindInputValid) {
    state.isInputsValid = true;
    ui->treeWidget->topLevelItem(1)->setIcon(0, tickIcon);
    ui->treeWidget->topLevelItem(1)->setToolTip(0, "");
  } else if (!state.isSurfaceInputValid && !state.isWindInputValid) {
    state.isInputsValid = false;
    ui->treeWidget->topLevelItem(1)->setIcon(0, xIcon);
    ui->treeWidget->topLevelItem(1)->setToolTip(0, "Bad surface and wind inputs.");
  } else if (!state.isSurfaceInputValid) {
    state.isInputsValid = false;
    ui->treeWidget->topLevelItem(1)->setIcon(0, xIcon);
    ui->treeWidget->topLevelItem(1)->setToolTip(0, "Bad surface input.");
  } else if (!state.isWindInputValid) {
    state.isInputsValid = false;
    ui->treeWidget->topLevelItem(1)->setIcon(0, xIcon);
    ui->treeWidget->topLevelItem(1)->setToolTip(0, "Bad wind input.");
  }

  // Update solve state
  if (state.isSolverMethodologyValid && state.isInputsValid) {
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

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow)
{
  ui->setupUi(this);
  checkMessages();
  resize(1200, 700);

  timeZoneAllZonesCheckBoxClicked();

  // Immediately call a UI refresh to set initial states
  refreshUI();

  ui->treeWidget->expandAll();

  /*
   * Create file handler window for point init screen
   */
  QString downloadsPath = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
  // Enable QFileSystemModel to process directories and files
  QFileSystemModel *model = new QFileSystemModel(this);
  model->setFilter(QDir::NoDotAndDotDot | QDir::AllDirs | QDir::AllEntries);  // Ensure files appear
  model->setRootPath(downloadsPath);

  // Enable file watching so contents refresh properly
  model->setReadOnly(false);
  model->setResolveSymlinks(true);

  // Set the correct root index inside Downloads
  QModelIndex rootIndex = model->index(downloadsPath);
  ui->treeFileExplorer->setModel(model);
  ui->treeFileExplorer->setRootIndex(rootIndex);

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
  header->setSectionResizeMode(0, QHeaderView::Interactive);
  header->setSectionResizeMode(3, QHeaderView::Stretch);
  model->setHeaderData(0, Qt::Horizontal, "Name");
  model->setHeaderData(3, Qt::Horizontal, "Date Modified");

  ui->treeFileExplorer->expandAll();

  /*
   * Functionality for the map widget
   */
  QWebEngineProfile::defaultProfile()->settings()->setAttribute(QWebEngineSettings::LocalContentCanAccessRemoteUrls, true);
  QWebEngineProfile::defaultProfile()->settings()->setAttribute(QWebEngineSettings::LocalContentCanAccessFileUrls, true);

  QString filePath = QString(MAP_PATH);

  webView = new QWebEngineView(ui->mapPanelWidget);
  QUrl url = QUrl::fromLocalFile(filePath);
  webView->setUrl(url);

  surfaceInput = new SurfaceInput();

  QVBoxLayout *layout = new QVBoxLayout();
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(webView);

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

  connect(ui->treeWidget, &QTreeWidget::itemClicked, this, &MainWindow::treeItemClicked);

  /*
   * Downloaded Forecast explorer
   */

  populateForecastDownloads();

  /*
   * Basic initial setup steps
   */
  // Surface Input window
  ui->elevationInputFileOpenButton->setIcon(QIcon(":/folder.png"));
  ui->elevationInputFileDownloadButton->setIcon(QIcon(":/swoop_final.png"));

  // Solver window
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
  connect(ui->momentumSolverCheckBox, &QCheckBox::clicked, this, &MainWindow::momentumSolverCheckBoxClicked);

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
  connect(ui->surfaceInputDownloadButton, &QPushButton::clicked, this, &MainWindow::surfaceInputDownloadButtonClicked);

  menuBar = new MenuBar(ui, this);
  connect(ui->exitWindNinjaAction, &QAction::triggered, this, &QMainWindow::close);  // just close the mainWindow (behavior of the old qt4 code)
  connect(ui->openElevationInputFileMenuAction, &QAction::triggered, this, &MainWindow::elevationInputFileOpenButtonClicked);
}

MainWindow::~MainWindow()
{
  delete menuBar;
  delete ui;
}


void MainWindow::treeItemClicked(QTreeWidgetItem *item, int column) {
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


void MainWindow::massSolverCheckBoxClicked()
{
  AppState& state = AppState::instance();

  if (state.isMomentumSolverToggled) {
    ui->momentumSolverCheckBox->setChecked(false);
    state.isMomentumSolverToggled = ui->momentumSolverCheckBox->isChecked();
  }
  state.isMassSolverToggled = ui->massSolverCheckBox->isChecked();

  ui->meshResolutionSpinBox->setValue(surfaceInput->computeMeshResolution(ui->meshResolutionComboBox->currentIndex(), ui->momentumSolverCheckBox->isChecked()));
  refreshUI();
}

void MainWindow::momentumSolverCheckBoxClicked()
{
  AppState& state = AppState::instance();

  if (state.isMassSolverToggled) {
    ui->massSolverCheckBox->setChecked(false);
    state.isMassSolverToggled = ui->massSolverCheckBox->isChecked();
  }
  state.isMomentumSolverToggled = ui->momentumSolverCheckBox->isChecked();

  ui->meshResolutionSpinBox->setValue(surfaceInput->computeMeshResolution(ui->meshResolutionComboBox->currentIndex(), ui->momentumSolverCheckBox->isChecked()));
  refreshUI();
}

void MainWindow::elevationInputFileLineEditTextChanged(const QString &arg1)
{
  surfaceInput->computeDEMFile(currentDemFilePath);
  surfaceInput->computeMeshResolution(ui->meshResolutionComboBox->currentIndex(), ui->momentumSolverCheckBox->isChecked());

  ui->meshResolutionSpinBox->setValue(surfaceInput->computeMeshResolution(ui->meshResolutionComboBox->currentIndex(), ui->momentumSolverCheckBox->isChecked()));
  refreshUI();
}

void MainWindow::elevationInputFileOpenButtonClicked()
{
  QString directoryPath;
  if(!currentDemFilePath.isEmpty())
  {
    directoryPath = currentDemFilePath;
  }
  else {
    directoryPath = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
  }
  QString demFilePath = QFileDialog::getOpenFileName(this, "Select a file", directoryPath, "(*.tif);;All Files (*)");

  if (demFilePath.isEmpty()) {
    if (!currentDemFilePath.isEmpty()) {
      ui->elevationInputFileLineEdit->setText(QFileInfo(currentDemFilePath).fileName());
      ui->elevationInputFileLineEdit->setToolTip(currentDemFilePath);
    }
    return;
  }

  currentDemFilePath = demFilePath;
  ui->elevationInputFileLineEdit->setText(QFileInfo(demFilePath).fileName());
  ui->elevationInputFileLineEdit->setToolTip(demFilePath);
}

void MainWindow::elevationInputFileDownloadButtonClicked()
{
  int currentIndex = ui->inputsStackedWidget->currentIndex();
  ui->inputsStackedWidget->setCurrentIndex(currentIndex+1);
}

void MainWindow::meshResolutionComboBoxCurrentIndexChanged(int index)
{
  if (index == 3) {
    ui->meshResolutionSpinBox->setEnabled(true);
  } else {
    ui->meshResolutionSpinBox->setEnabled(false);
  }
  ui->meshResolutionSpinBox->setValue(surfaceInput->computeMeshResolution(ui->meshResolutionComboBox->currentIndex(), ui->momentumSolverCheckBox->isChecked()));
}

void MainWindow::meshResolutionMetersRadioButtonToggled(bool checked)
{
  if (checked) {
    ui->meshResolutionSpinBox->setValue(ui->meshResolutionSpinBox->value() * 0.3048);
  }
}

void MainWindow::meshResolutionFeetRadioButtonToggled(bool checked)
{
  if (checked) {
    ui->meshResolutionSpinBox->setValue(ui->meshResolutionSpinBox->value() * 3.28084);
  }
}

void MainWindow::timeZoneComboBoxCurrentIndexChanged(int index)
{
  QString currentTimeZone = ui->timeZoneComboBox->currentText();
  QString timeZoneDetails = surfaceInput->fetchTimeZoneDetails(currentTimeZone);
  ui->timeZoneDetailsTextEdit->setText(timeZoneDetails);
}

// User toggles show all time zones
void MainWindow::timeZoneAllZonesCheckBoxClicked()
{
  AppState& state = AppState::instance();
  state.isShowAllTimeZonesSelected = ui->timeZoneAllZonesCheckBox->isChecked();

  bool isShowAllTimeZonesSelected = ui->timeZoneAllZonesCheckBox->isChecked();
  QVector<QVector<QString>> displayData = surfaceInput->fetchAllTimeZones(isShowAllTimeZonesSelected);

  ui->timeZoneComboBox->clear();
  for (const QVector<QString>& zone : displayData) {
    if (!zone.isEmpty()) {
      ui->timeZoneComboBox->addItem(zone[0]);
    }
  }

  // Default to America/Denver
  ui->timeZoneComboBox->setCurrentText("America/Denver");
}

void MainWindow::timeZoneDetailsCheckBoxClicked()
{
  AppState& state = AppState::instance();
  state.isDisplayTimeZoneDetailsSelected = ui->timeZoneDetailsCheckBox->isChecked();
  ui->timeZoneDetailsTextEdit->setVisible(state.isDisplayTimeZoneDetailsSelected);
}

void MainWindow::diurnalCheckBoxClicked()
{
  AppState& state = AppState::instance();
  state.isDiurnalInputToggled = ui->diurnalCheckBox->isChecked();

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

  refreshUI();
}

void MainWindow::stabilityCheckBoxClicked()
{
  AppState& state = AppState::instance();
  state.isStabilityInputToggled = ui->stabilityCheckBox->isChecked();

  refreshUI();
}

void MainWindow::domainAverageCheckBoxClicked()
{
  AppState& state = AppState::instance();
  state.isDomainAverageInitializationToggled = ui->domainAverageCheckBox->isChecked();

  if (state.isDomainAverageInitializationToggled) {
    ui->pointInitializationCheckBox->setChecked(false);
    ui->useWeatherModelInit->setChecked(false);
    state.isPointInitializationToggled = ui->pointInitializationCheckBox->isChecked();
    state.isWeatherModelInitializationToggled = ui->useWeatherModelInit->isChecked();
  }

  refreshUI();
}

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

void MainWindow::clearTableButtonClicked()
{
  AppState& state = AppState::instance();
  AppState::instance().isDomainAverageWindInputTableValid = true;

  ui->domainAverageTable->clearContents();
  invalidDAWCells.clear();

  refreshUI();
}

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

  AppState::instance().isDomainAverageWindInputTableValid = invalidDAWCells.isEmpty();
  refreshUI();
}

void MainWindow::pointInitializationCheckBoxClicked()
{
  AppState& state = AppState::instance();
  state.isPointInitializationToggled = ui->pointInitializationCheckBox->isChecked();

  if (state.isPointInitializationToggled) {
    ui->domainAverageCheckBox->setChecked(false);
    ui->useWeatherModelInit->setChecked(false);
    state.isDomainAverageInitializationToggled = ui->domainAverageCheckBox->isChecked();
    state.isWeatherModelInitializationToggled = ui->useWeatherModelInit->isChecked();
  }

  refreshUI();
}

void MainWindow::useWeatherModelInitClicked()
{
  AppState& state = AppState::instance();

  state.isWeatherModelInitializationToggled = ui->useWeatherModelInit->isChecked();

  if (state.isWeatherModelInitializationToggled) {
    ui->domainAverageCheckBox->setChecked(false);
    ui->pointInitializationCheckBox->setChecked(false);
    state.isDomainAverageInitializationToggled = ui->domainAverageCheckBox->isChecked();
    state.isPointInitializationToggled = ui->pointInitializationCheckBox->isChecked();
  }

  refreshUI();
}

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

void MainWindow::numberOfProcessorsSolveButtonClicked()
{
  ui->solveButton->click();
}

void MainWindow::solveButtonClicked()
{
  // // Alias app state, used to determine which type of solution to run
  // AppState& state = AppState::instance();

  //        // Determine which run to perform
  // if (state.isDomainAverageInitializationValid) {
  //   DomainAverageWind domainAvgWind = setDomainAverageWind();
  //   provider.domain_average_exec(domainAvgWind);
  // }else if(state.isPointInitializationValid){
  //   PointInitialization pointInit = setPointInitialization();
  //   provider.point_exec(pointInit);
  // }else if(state.isWeatherModelInitializationValid){
  //   WeatherModel weatherModel = setWeatherModel();
  //   provider.wxmodel_exec(weatherModel);
  // }


  // vector<string> outputFileList = provider.getOutputFileNames(
  //     view->getUi()->elevationInputFileLineEdit->text(),
  //     view->getUi()->domainAverageTable,
  //     view->getUi()->meshResolutionSpinBox->text(),
  //     provider.parseDomainAvgTable(view->getUi()->domainAverageTable).size(),
  //     view->getUi()->outputDirectoryTextEdit->toPlainText());

  // view->loadMapKMZ(outputFileList);
}

void MainWindow::treeWidgetItemDoubleClicked(QTreeWidgetItem *item, int column)
{
  if (item->text(0) == "Conservation of Mass") {
    ui->massSolverCheckBox->click();
  } else if (item->text(0) == "Conservation of Mass and Momentum") {
    ui->momentumSolverCheckBox->click();
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

void MainWindow::surfaceInputDownloadCancelButtonClicked()
{
  int currentIndex = ui->inputsStackedWidget->currentIndex();
  ui->inputsStackedWidget->setCurrentIndex(currentIndex-1);
}

void MainWindow::surfaceInputDownloadButtonClicked()
{
  double boundingBox[] = {  ui->boundingBoxNorthLineEdit->text().toDouble(),
                            ui->boundingBoxEastLineEdit->text().toDouble(),
                            ui->boundingBoxSouthLineEdit->text().toDouble(),
                            ui->boundingBoxWestLineEdit->text().toDouble(),
                          };

  double resolution = 30;

  QString defaultName = "";
  QString downloadsPath = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
  QDir dir(downloadsPath);
  QString fullPath = dir.filePath(defaultName);
  QString demFilePath = QFileDialog::getSaveFileName(this, "Save DEM File", fullPath, "TIF Files (*.tif)");
  if (!demFilePath.endsWith(".tif", Qt::CaseInsensitive)) {
    demFilePath += ".tif";
  }
  currentDemFilePath = demFilePath;
  std::string demFile = demFilePath.toStdString();

  std::string fetchType;
  switch(ui->elevationFileTypeComboBox->currentIndex())
  {
  case 0:
    fetchType = "srtm";
    break;
  case 1:
    fetchType = "gmted";
    break;
  case 2:
    fetchType = "lcp";
    break;
  }

  int result = surfaceInput->fetchDEMFile(boundingBox, demFile, resolution, fetchType);

  ui->elevationInputFileLineEdit->setText(QFileInfo(demFilePath).fileName());
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
