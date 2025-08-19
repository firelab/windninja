/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja Qt GUI
 * Purpose:  Main Window that handles GUI scraping and state changes
 * Author:   Mason Willman <mason.willman@usda.gov>
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

#include "mainwindow.h"

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
    if (state.isPointInitializationToggled && state.isStationFileSelectionValid && state.isStationFileSelected) {
        ui->treeWidget->topLevelItem(1)->child(3)->child(1)->setIcon(0, tickIcon);
        ui->treeWidget->topLevelItem(1)->child(3)->child(1)->setToolTip(0, "");
        state.isPointInitializationValid = true;
    } else if(state.isPointInitializationToggled && !state.isStationFileSelected) {
        ui->treeWidget->topLevelItem(1)->child(3)->child(1)->setIcon(0, xIcon);
        ui->treeWidget->topLevelItem(1)->child(3)->child(1)->setToolTip(0, "No station file selected.");
        state.isPointInitializationValid = false;
    } else if(state.isPointInitializationToggled && !state.isStationFileSelectionValid){
        ui->treeWidget->topLevelItem(1)->child(3)->child(1)->setIcon(0, xIcon);
        ui->treeWidget->topLevelItem(1)->child(3)->child(1)->setToolTip(0, "Conflicting files selected.");
        state.isPointInitializationValid = false;
    } else {
        ui->treeWidget->topLevelItem(1)->child(3)->child(1)->setIcon(0, bulletIcon);
        ui->treeWidget->topLevelItem(1)->child(3)->child(1)->setToolTip(0, "");
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

    if(state.isGoogleEarthToggled)
    {
        if(state.isSurfaceInputValid)
        {
            state.isGoogleEarthValid = true;
            ui->treeWidget->topLevelItem(2)->child(0)->setIcon(0, tickIcon);
            ui->treeWidget->topLevelItem(2)->setToolTip(0, "");
        }
        else
        {
            state.isGoogleEarthValid = false;
            ui->treeWidget->topLevelItem(2)->setIcon(0, xIcon);
            ui->treeWidget->topLevelItem(2)->setToolTip(0, "Cannot read DEM File");
            ui->treeWidget->topLevelItem(2)->child(0)->setIcon(0, xIcon);
            ui->treeWidget->topLevelItem(2)->child(0)->setToolTip(0, "Cannot read DEM File");
        }
    }
    else
    {
        state.isGoogleEarthValid = false;
        ui->treeWidget->topLevelItem(2)->child(0)->setIcon(0, bulletIcon);
    }
    if(state.isFireBehaviorToggled)
    {
        if(state.isSurfaceInputValid)
        {
            state.isFireBehaviorValid = true;
            ui->treeWidget->topLevelItem(2)->child(1)->setIcon(0, tickIcon);
            ui->treeWidget->topLevelItem(2)->setToolTip(0, "");
        }
        else
        {
            state.isFireBehaviorValid = false;
            ui->treeWidget->topLevelItem(2)->setIcon(0, xIcon);
            ui->treeWidget->topLevelItem(2)->setToolTip(0, "Cannot read DEM File");
            ui->treeWidget->topLevelItem(2)->child(1)->setIcon(0, xIcon);
            ui->treeWidget->topLevelItem(2)->child(1)->setToolTip(0, "Cannot read DEM File");
        }
    }
    else
    {
        state.isFireBehaviorValid = false;
        ui->treeWidget->topLevelItem(2)->child(1)->setIcon(0, bulletIcon);
    }
    if(state.isShapeFilesToggled)
    {
        if(state.isSurfaceInputValid)
        {
            state.isShapeFilesValid = true;
            ui->treeWidget->topLevelItem(2)->child(2)->setIcon(0, tickIcon);
            ui->treeWidget->topLevelItem(2)->setToolTip(0, "");
        }
        else
        {
            state.isShapeFilesValid = false;
            ui->treeWidget->topLevelItem(2)->setIcon(0, xIcon);
            ui->treeWidget->topLevelItem(2)->setToolTip(0, "Cannot read DEM File");
            ui->treeWidget->topLevelItem(2)->child(2)->setIcon(0, xIcon);
            ui->treeWidget->topLevelItem(2)->child(2)->setToolTip(0, "Cannot read DEM File");
        }
    }
    else
    {
        state.isShapeFilesValid = false;
        ui->treeWidget->topLevelItem(2)->child(2)->setIcon(0, bulletIcon);
    }
    if(state.isGeoSpatialPDFFilesToggled)
    {
        if(state.isSurfaceInputValid)
        {
            state.isGeoSpatialPDFFilesValid = true;
            ui->treeWidget->topLevelItem(2)->child(3)->setIcon(0, tickIcon);
            ui->treeWidget->topLevelItem(2)->setToolTip(0, "");
        }
        else
        {
            state.isGeoSpatialPDFFilesValid = false;
            ui->treeWidget->topLevelItem(2)->setIcon(0, xIcon);
            ui->treeWidget->topLevelItem(2)->setToolTip(0, "Cannot read DEM File");
            ui->treeWidget->topLevelItem(2)->child(3)->setIcon(0, xIcon);
            ui->treeWidget->topLevelItem(2)->child(3)->setToolTip(0, "Cannot read DEM File");
        }
    }
    else
    {
        state.isGeoSpatialPDFFilesValid = false;
        ui->treeWidget->topLevelItem(2)->child(3)->setIcon(0, bulletIcon);
    }
    if(state.isVTKFilesToggled)
    {
        if(state.isSurfaceInputValid)
        {
            state.isVTKFilesValid = true;
            ui->treeWidget->topLevelItem(2)->child(4)->setIcon(0, tickIcon);
            ui->treeWidget->topLevelItem(2)->setToolTip(0, "");
        }
        else
        {
            state.isVTKFilesValid = false;
            ui->treeWidget->topLevelItem(2)->setIcon(0, xIcon);
            ui->treeWidget->topLevelItem(2)->setToolTip(0, "Cannot read DEM File");
            ui->treeWidget->topLevelItem(2)->child(4)->setIcon(0, xIcon);
            ui->treeWidget->topLevelItem(2)->child(4)->setToolTip(0, "Cannot read DEM File");
        }
    }
    else
    {
        state.isVTKFilesValid = false;
        ui->treeWidget->topLevelItem(2)->child(4)->setIcon(0, bulletIcon);
    }


    if(state.isGoogleEarthValid || state.isFireBehaviorValid || state.isShapeFilesValid || state.isGeoSpatialPDFFilesValid || state.isVTKFilesValid)
    {
        state.isOutputsValid = true;
        ui->treeWidget->topLevelItem(2)->setIcon(0, tickIcon);
        ui->treeWidget->topLevelItem(2)->setToolTip(0, "");
    }
    else
    {
        ui->treeWidget->topLevelItem(2)->setIcon(0, xIcon);
        ui->treeWidget->topLevelItem(2)->setToolTip(0, "No Output Selected");
    }

    if (state.isSolverMethodologyValid && state.isInputsValid && state.isOutputsValid) {
        ui->solveButton->setEnabled(true);
        ui->numberOfProcessorsSolveButton->setEnabled(true);
        ui->solveButton->setToolTip("");
        ui->numberOfProcessorsSolveButton->setToolTip("");
        ui->treeWidget->topLevelItem(3)->setIcon(0, tickIcon);
        ui->treeWidget->topLevelItem(3)->setToolTip(0, "");
    } else {
        ui->solveButton->setEnabled(false);
        ui->numberOfProcessorsSolveButton->setEnabled(false);
        ui->solveButton->setToolTip("Solver Methodology and Inputs must be passing to solve.");
        ui->numberOfProcessorsSolveButton->setToolTip("Solver Methodology and Inputs must be passing to solve.");
        ui->treeWidget->topLevelItem(3)->setIcon(0, xIcon);
        ui->treeWidget->topLevelItem(3)->setToolTip(0, "There are errors in the inputs or outputs");
    }
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    serverBridge = new ServerBridge();
    serverBridge->checkMessages();
    resize(1200, 700);
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

    QWebEngineProfile::defaultProfile()->settings()->setAttribute(QWebEngineSettings::LocalContentCanAccessRemoteUrls, true);
    QWebEngineProfile::defaultProfile()->settings()->setAttribute(QWebEngineSettings::LocalContentCanAccessFileUrls, true);

    QString filePath = QString(MAP_PATH);
    channel = new QWebChannel(this);
    mapBridge = new MapBridge(this);
    webView = new QWebEngineView(ui->mapPanelWidget);
    channel->registerObject(QStringLiteral("bridge"), mapBridge);
    webView->page()->setWebChannel(channel);
    QUrl url = QUrl::fromLocalFile(filePath);
    webView->setUrl(url);

    menuBar = new MenuBar(ui, this);

    surfaceInput = new SurfaceInput(ui, webView, this);
    surfaceInput->timeZoneAllZonesCheckBoxClicked();
    domainAverageInput = new DomainAverageInput(ui, this);
    pointInitializationInput = new PointInitializationInput(ui, this);

    QVBoxLayout *layout = new QVBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(webView);

    ui->mapPanelWidget->setLayout(layout);

    /*
    * Connect tree items to stacked tab window
    */
    // Top-level items
    ui->inputsStackedWidget->setCurrentIndex(0);
    ui->treeWidget->topLevelItem(0)->setData(0, Qt::UserRole, 1);
    // Sub-items for Solver Methodology
    ui->treeWidget->topLevelItem(0)->child(0)->setData(0, Qt::UserRole, 2);  // Conservation of Mass (Page 1)
    ui->treeWidget->topLevelItem(0)->child(1)->setData(0, Qt::UserRole, 3);  // Conservation of Mass and Momentum (Page 2)

    ui->treeWidget->topLevelItem(1)->setData(0, Qt::UserRole, 4);
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

    ui->treeWidget->topLevelItem(2)->setData(0, Qt::UserRole, 12);
    // Sub-items for Outputs
    ui->treeWidget->topLevelItem(2)->child(0)->setData(0, Qt::UserRole, 13);  // Surface Input (Page 6)
    ui->treeWidget->topLevelItem(2)->child(1)->setData(0, Qt::UserRole, 14);  // Dirunal Input (Page 7)
    ui->treeWidget->topLevelItem(2)->child(2)->setData(0, Qt::UserRole, 15);  // Stability Input (Page 8)
    ui->treeWidget->topLevelItem(2)->child(3)->setData(0, Qt::UserRole, 16);  // Wind Input (Page 9)
    ui->treeWidget->topLevelItem(2)->child(4)->setData(0, Qt::UserRole, 17);  // Wind Input (Page 9)

    ui->treeWidget->topLevelItem(3)->setData(0, Qt::UserRole, 18);

    /*
    * Basic initial setup steps
    */
    // Surface Input window
    ui->elevationInputFileOpenButton->setIcon(QIcon(":/folder.png"));
    ui->elevationInputFileDownloadButton->setIcon(QIcon(":/server_go.png"));
    ui->elevationInputTypePushButton->setIcon(QIcon(":/swoop_final.png"));

    // Solver window
    int nCPUs = QThread::idealThreadCount();
    ui->availableProcessorsTextEdit->setPlainText("Available Processors:  " + QString::number(nCPUs));
    ui->numberOfProcessorsSpinBox->setMaximum(nCPUs);
    ui->numberOfProcessorsSpinBox->setValue(nCPUs);

    // Populate default location for output location
    ui->outputDirectoryLineEdit->setText(downloadsPath);
    ui->outputDirectoryButton->setIcon(QIcon(":/folder.png"));

    // Set initial visibility of time zone details
    ui->timeZoneDetailsTextEdit->setVisible(false);

    // Set initial formatting of domain average input table
    ui->domainAverageTable->hideColumn(2);
    ui->domainAverageTable->hideColumn(3);
    ui->domainAverageTable->hideColumn(4);
    ui->domainAverageTable->hideColumn(5);
    ui->domainAverageTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    ui->outputWindHeightUnitsComboBox->setItemData(0, "ft");
    ui->outputWindHeightUnitsComboBox->setItemData(1, "m");
    ui->meshResolutionUnitsComboBox->setItemData(0, "m");
    ui->meshResolutionUnitsComboBox->setItemData(1, "ft");
    ui->googleEarthMeshResolutionComboBox->setItemData(0, "m");
    ui->googleEarthMeshResolutionComboBox->setItemData(1, "ft");
    ui->fireBehaviorMeshResolutionComboBox->setItemData(0, "m");
    ui->fireBehaviorMeshResolutionComboBox->setItemData(1, "ft");
    ui->shapeFilesMeshResolutionComboBox->setItemData(0, "m");
    ui->shapeFilesMeshResolutionComboBox->setItemData(1, "ft");
    ui->geospatialPDFFilesMeshResolutionComboBox->setItemData(0, "m");
    ui->geospatialPDFFilesMeshResolutionComboBox->setItemData(1, "ft");

    ui->legendComboBox->setItemData(0, "equal_interval");
    ui->legendComboBox->setItemData(0, "equal_color");

    connectSignals();
}

MainWindow::~MainWindow()
{
  delete webView;
  delete channel;
  delete mapBridge;
  delete surfaceInput;
  delete menuBar;
  delete domainAverageInput;
  delete pointInitializationInput;
  delete ui;
}

void MainWindow::connectSignals()
{
    connect(ui->elevationInputTypeComboBox, &QComboBox::currentIndexChanged, ui->elevationInputTypeStackedWidget, &QStackedWidget::setCurrentIndex);
    connect(ui->massSolverCheckBox, &QCheckBox::clicked, this, &MainWindow::massSolverCheckBoxClicked);
    connect(ui->momentumSolverCheckBox, &QCheckBox::clicked, this, &MainWindow::momentumSolverCheckBoxClicked);
    connect(ui->diurnalCheckBox, &QCheckBox::clicked, this, &MainWindow::diurnalCheckBoxClicked);
    connect(ui->stabilityCheckBox, &QCheckBox::clicked, this, &MainWindow::stabilityCheckBoxClicked);
    connect(ui->treeWidget, &QTreeWidget::itemDoubleClicked, this, &MainWindow::treeWidgetItemDoubleClicked);
    connect(ui->solveButton, &QPushButton::clicked, this, &MainWindow::solveButtonClicked);
    connect(ui->numberOfProcessorsSolveButton, &QPushButton::clicked, this, &MainWindow::numberOfProcessorsSolveButtonClicked);
    connect(ui->exitWindNinjaAction, &QAction::triggered, this, &QMainWindow::close);
    connect(mapBridge, &MapBridge::boundingBoxReceived, surfaceInput, &SurfaceInput::boundingBoxReceived);
    connect(surfaceInput, &SurfaceInput::requestRefresh, this, &MainWindow::refreshUI);
    connect(domainAverageInput, &DomainAverageInput::requestRefresh, this, &MainWindow::refreshUI);
    connect(pointInitializationInput, &PointInitializationInput::requestRefresh, this, &MainWindow::refreshUI);
    connect(ui->googleEarthGroupBox, &QGroupBox::toggled, this, &MainWindow::googleEarthGroupBoxToggled);
    connect(ui->fireBehaviorGroupBox, &QGroupBox::toggled, this, &MainWindow::fireBehaviorGroupBoxToggled);
    connect(ui->shapeFilesGroupBox, &QGroupBox::toggled, this, &MainWindow::shapeFilesGroupBoxToggled);
    connect(ui->geospatialPDFFilesGroupBox, &QGroupBox::toggled, this, &MainWindow::geospatialPDFFilesGroupBoxToggled);
    connect(ui->VTKFilesCheckBox, &QCheckBox::clicked, this, &MainWindow::VTKFilesCheckBoxClicked);
    connect(ui->googleEarthMeshResolutionGroupBox, &QGroupBox::toggled, this, &MainWindow::googleEarthMeshResolutionGroupBoxToggled);
    connect(ui->fireBehaviorMeshResolutionGroupBox, &QGroupBox::toggled, this, &MainWindow::fireBehaviorMeshResolutionGroupBoxToggled);
    connect(ui->shapeFilesMeshResolutionGroupBox, &QGroupBox::toggled, this, &MainWindow::shapeFilesMeshResolutionGroupBoxToggled);
    connect(ui->geospatialPDFFilesMeshResolutionGroupBox, &QGroupBox::toggled, this, &MainWindow::geospatialPDFFilesMeshResolutionGroupBoxToggled);
    connect(ui->outputDirectoryButton, &QPushButton::clicked, this, &MainWindow::outputDirectoryButtonClicked);
    connect(ui->treeWidget, &QTreeWidget::itemClicked, this, &MainWindow::treeItemClicked);
    connect(surfaceInput, SIGNAL(setupTreeView()), pointInitializationInput, SLOT(setupTreeView()));
}

void MainWindow::treeItemClicked(QTreeWidgetItem *item, int column) {
    int pageIndex = item->data(column, Qt::UserRole).toInt();
    if (pageIndex >= 0) {
    if(pageIndex >= 6) {
        ui->inputsStackedWidget->setCurrentIndex(pageIndex);
    }
    else {
        ui->inputsStackedWidget->setCurrentIndex(pageIndex);
    }
    }
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

void MainWindow::diurnalCheckBoxClicked()
{
    AppState& state = AppState::instance();
    state.isDiurnalInputToggled = ui->diurnalCheckBox->isChecked();

    QTableWidget* table = ui->domainAverageTable;
    if(!ui->diurnalCheckBox->isChecked()) {
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

void MainWindow::useWeatherModelInitClicked()
{
    AppState& state = AppState::instance();

    state.isWeatherModelInitializationToggled = ui->weatherModelCheckBox->isChecked();

    if (state.isWeatherModelInitializationToggled) {
        ui->domainAverageCheckBox->setChecked(false);
        ui->pointInitializationGroupBox->setChecked(false);
        state.isDomainAverageInitializationToggled = ui->domainAverageCheckBox->isChecked();
        state.isPointInitializationToggled = ui->pointInitializationGroupBox->isChecked();
    }

    refreshUI();
}

void MainWindow::outputDirectoryButtonClicked()
{
    QString currentPath = ui->outputDirectoryLineEdit->text();
    QString dirPath = QFileDialog::getExistingDirectory(this, "Select a directory", currentPath, QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if (!dirPath.isEmpty())
    {
        ui->outputDirectoryLineEdit->setText(dirPath);
        ui->outputDirectoryLineEdit->setToolTip(dirPath);
    } else
    {
        ui->outputDirectoryLineEdit->setText(currentPath);
        ui->outputDirectoryLineEdit->setToolTip(currentPath);
    }
}

void MainWindow::numberOfProcessorsSolveButtonClicked()
{
    ui->solveButton->click();
}

void MainWindow::solveButtonClicked()
{
    AppState& state = AppState::instance();

    int numNinjas = 0;
    NinjaArmyH *ninjaArmy = nullptr;
    char **papszOptions = nullptr;
    const char *initializationMethod;
    QList<double> speeds;
    QList<double> directions;

    if (state.isDomainAverageInitializationValid)
    {
        initializationMethod = "domain_average";
        int rowCount = ui->domainAverageTable->rowCount();
        for (int row = 0; row < rowCount; ++row) {
            QTableWidgetItem* speedItem = ui->domainAverageTable->item(row, 0);
            QTableWidgetItem* directionItem = ui->domainAverageTable->item(row, 1);

            if (speedItem && directionItem) {
                speeds << speedItem->text().toDouble();
                directions << directionItem->text().toDouble();
            }
        }
        numNinjas = speeds.size();
        bool momentumFlag = ui->momentumSolverCheckBox->isChecked();
        QString speedUnits =  ui->tableSpeedUnits->currentText();
        ninjaArmy = NinjaMakeDomainAverageArmy(numNinjas, momentumFlag, speeds.data(), speedUnits.toUtf8().constData(), directions.data(), papszOptions);
    }
    else if (state.isPointInitializationValid)
    {
        initializationMethod = "point";

        std::vector<QString> stationFiles = pointInitializationInput->getStationFiles();
        QString DEMTimeZone = ui->timeZoneComboBox->currentText();
        QByteArray timeZoneBytes = ui->timeZoneComboBox->currentText().toUtf8();

        std::vector<QByteArray> stationFilesBytes;
        stationFilesBytes.reserve(stationFiles.size());
        std::vector<const char*> stationFileNames;
        stationFileNames.reserve(stationFiles.size());
        for (const auto& file : stationFiles) {
            stationFilesBytes.push_back(file.toUtf8());
            stationFileNames.push_back(stationFilesBytes.back().constData());
        }

        QString DEMPath = ui->elevationInputFileLineEdit->property("fullpath").toString();

        numNinjas = 25;
        bool momentumFlag = ui->momentumSolverCheckBox->isChecked();

        if(ui->pointInitializationTreeView->property("timeSeriesFlag").toBool())
        {
            QDateTime start = ui->weatherStationDataStartDateTimeEdit->dateTime();
            QDateTime end   = ui->weatherStationDataEndDateTimeEdit->dateTime();

            QVector<int> year   = { start.date().year(),   end.date().year() };
            QVector<int> month  = { start.date().month(),  end.date().month() };
            QVector<int> day    = { start.date().day(),    end.date().day() };
            QVector<int> hour   = { start.time().hour(),   end.time().hour() };
            QVector<int> minute = { start.time().minute(), end.time().minute() };

            int nTimeSteps = ui->weatherStationDataTimestepsSpinBox->value();

            QVector<int> outYear(nTimeSteps);
            QVector<int> outMonth(nTimeSteps);
            QVector<int> outDay(nTimeSteps);
            QVector<int> outHour(nTimeSteps);
            QVector<int> outMinute(nTimeSteps);

            NinjaErr err = NinjaGetTimeList(
                year.data(), month.data(), day.data(),
                hour.data(), minute.data(),
                outYear.data(), outMonth.data(), outDay.data(),
                outHour.data(), outMinute.data(),
                nTimeSteps, timeZoneBytes.data()
            );
            if(err != NINJA_SUCCESS)
            {
                printf("NinjaGetTimeList: err = %d\n", err);
            }
            ninjaArmy = NinjaMakePointArmy
                (outYear.data(), outMonth.data(), outDay.data(), outHour.data(), outMinute.data(), nTimeSteps, DEMTimeZone.toUtf8().data(), stationFileNames.data(), stationFileNames.size(), DEMPath.toUtf8().data(), true, momentumFlag, papszOptions);
        }
        else
        {
            int year, month, day, hour, minute;
            QDateTime date = ui->weatherStationDataTextEdit->property("simulationTime").toDateTime();
            year = date.date().year();
            month = date.date().month();
            day = date.date().day();
            hour = date.time().hour();
            minute = date.time().minute();

            int outYear, outMonth, outDay, outHour, outMinute;

            NinjaErr err = NinjaGenerateSingleTimeObject(
                year, month, day, hour, minute,
                timeZoneBytes.constData(),
                &outYear, &outMonth, &outDay, &outHour, &outMinute
                );
            if (err != NINJA_SUCCESS)
            {
                printf("NinjaGenerateSingleTimeObject: err = %d\n", err);
            }

            QVector<int> yearVec   = { outYear };
            QVector<int> monthVec  = { outMonth };
            QVector<int> dayVec    = { outDay };
            QVector<int> hourVec   = { outHour };
            QVector<int> minuteVec = { outMinute };

            ninjaArmy = NinjaMakePointArmy(
                yearVec.data(), monthVec.data(), dayVec.data(),
                hourVec.data(), minuteVec.data(),
                1,
                DEMTimeZone.toUtf8().data(),
                stationFileNames.data(),
                static_cast<int>(stationFileNames.size()),
                DEMPath.toUtf8().data(),
                true,
                momentumFlag,
                papszOptions
            );
        }
    }

    prepareArmy(ninjaArmy, numNinjas, initializationMethod);

    int err = NinjaStartRuns(ninjaArmy, ui->numberOfProcessorsSpinBox->value(), papszOptions);
    if(err != 1) //NinjaStartRuns returns 1 on success
    {
        printf("NinjaStartRuns: err = %d\n", err);
    }

    err = NinjaDestroyArmy(ninjaArmy, papszOptions);
    if(err != NINJA_SUCCESS)
    {
        printf("NinjaDestroyRuns: err = %d\n", err);
    }

    // vector<string> outputFiles;
    // QDir outDir(ui->outputDirectoryLineEdit->text());
    // QString demName = QFileInfo(ui->elevationInputFileLineEdit->text()).baseName();
    // int meshInt = static_cast<int>(std::round(ui->meshResolutionSpinBox->value()));
    // QString meshSize = QString::number(meshInt) + "m";

    // for (int i = 0; i < numNinjas; i++) {
    //     QString filePath = outDir.filePath(QString("%1_%2_%3_%4.kmz")
    //                                            .arg(demName)
    //                                            .arg(directions[i])
    //                                            .arg(speeds[i])
    //                                            .arg(meshSize));
    //     outputFiles.push_back(filePath.toStdString());
    // }

    // for (const auto& dir : outputFiles) {
    //     QString qDir = QString::fromStdString(dir);

    //     QFile f(qDir);
    //     f.open(QIODevice::ReadOnly);
    //     QByteArray data = f.readAll();
    //     QString base64 = data.toBase64();

    //     webView->page()->runJavaScript("loadKmzFromBase64('"+base64+"')");
    // }
}

void MainWindow::treeWidgetItemDoubleClicked(QTreeWidgetItem *item, int column)
{
    if (item->text(0) == "Conservation of Mass")
    {
        ui->massSolverCheckBox->click();
    } else if (item->text(0) == "Conservation of Mass and Momentum")
    {
        ui->momentumSolverCheckBox->click();
    } else if (item->text(0) == "Diurnal Input")
    {
        ui->diurnalCheckBox->click();
    } else if (item->text(0) == "Stability Input")
    {
        ui->stabilityCheckBox->click();
    } else if (item->text(0) == "Domain Average Wind")
    {
        ui->domainAverageCheckBox->click();
    } else if (item->text(0) == "Point Initialization")
    {
        ui->pointInitializationGroupBox->setChecked(!ui->pointInitializationGroupBox->isChecked());
    } else if (item->text(0) == "Weather Model")
    {
        ui->weatherModelCheckBox->click();
    } else if (item->text(0) == "Surface Input")
    {
        surfaceInput->elevationInputFileOpenButtonClicked();
    } else if (item->text(0) == "Google Earth")
    {
        if(!ui->googleEarthGroupBox->isChecked())
        {
            ui->googleEarthGroupBox->setChecked(true);
        }else
        {
            ui->googleEarthGroupBox->setChecked(false);
        }
    } else if (item->text(0) == "Surface Input")
    {
        surfaceInput->elevationInputFileOpenButtonClicked();
    } else if (item->text(0) == "Google Earth")
    {
        if(!ui->googleEarthGroupBox->isChecked())
        {
            ui->googleEarthGroupBox->setChecked(true);
        }else
        {
            ui->googleEarthGroupBox->setChecked(false);
        }
    } else if (item->text(0) == "Fire Behavior")
    {
        if(!ui->fireBehaviorGroupBox->isChecked())
        {
            ui->fireBehaviorGroupBox->setChecked(true);
        }else
        {
            ui->fireBehaviorGroupBox->setChecked(false);
        }
    } else if (item->text(0) == "Shape Files")
    {
        if(!ui->shapeFilesGroupBox->isChecked())
        {
            ui->shapeFilesGroupBox->setChecked(true);
        }else
        {
            ui->shapeFilesGroupBox->setChecked(false);
        }
    } else if (item->text(0) == "Geospatial PDF Files")
    {
        if(!ui->geospatialPDFFilesGroupBox->isChecked())
        {
            ui->geospatialPDFFilesGroupBox->setChecked(true);
        }else
        {
            ui->geospatialPDFFilesGroupBox->setChecked(false);
        }
    } else if (item->text(0) == "VTK Files")
    {
        ui->VTKFilesCheckBox->click();
    }
}

void MainWindow::googleEarthGroupBoxToggled(bool checked)
{
    AppState& state = AppState::instance();
    state.isGoogleEarthToggled = checked;
    refreshUI();
}

void MainWindow::fireBehaviorGroupBoxToggled(bool checked)
{
    AppState& state = AppState::instance();
    state.isFireBehaviorToggled = checked;
    refreshUI();
}

void MainWindow::shapeFilesGroupBoxToggled(bool checked)
{
    AppState& state = AppState::instance();
    state.isShapeFilesToggled = checked;
    refreshUI();
}

void MainWindow::geospatialPDFFilesGroupBoxToggled(bool checked)
{
    AppState& state = AppState::instance();
    state.isGeoSpatialPDFFilesToggled = checked;
    refreshUI();
}

void MainWindow::VTKFilesCheckBoxClicked(bool checked)
{
    AppState& state = AppState::instance();
    state.isVTKFilesToggled = checked;
    refreshUI();
}

void MainWindow::googleEarthMeshResolutionGroupBoxToggled(bool checked)
{
    ui->googleEarthMeshResolutionSpinBox->setEnabled(!checked);
    ui->googleEarthMeshResolutionComboBox->setEnabled(!checked);
}

void MainWindow::fireBehaviorMeshResolutionGroupBoxToggled(bool checked)
{
    ui->fireBehaviorMeshResolutionSpinBox->setEnabled(!checked);
    ui->fireBehaviorMeshResolutionComboBox->setEnabled(!checked);
}

void MainWindow::shapeFilesMeshResolutionGroupBoxToggled(bool checked)
{
    ui->shapeFilesMeshResolutionSpinBox->setEnabled(!checked);
    ui->shapeFilesMeshResolutionComboBox->setEnabled(!checked);
}

void MainWindow::geospatialPDFFilesMeshResolutionGroupBoxToggled(bool checked)
{
    ui->geospatialPDFFilesMeshResolutionSpinBox->setEnabled(!checked);
    ui->geospatialPDFFilesMeshResolutionComboBox->setEnabled(!checked);
}

void MainWindow::prepareArmy(NinjaArmyH *ninjaArmy, int numNinjas, const char* initializationMethod)
{
    char **papszOptions = nullptr;
    int err;
    for(unsigned int i=0; i<numNinjas; i++)
    {
        /*
       * Sets Simulation Variables
       */
        err = NinjaSetCommunication(ninjaArmy, i, "cli", papszOptions);
        if(err != NINJA_SUCCESS)
        {
            printf("NinjaSetCommunication: err = %d\n", err);
        }

        err = NinjaSetNumberCPUs(ninjaArmy, i, ui->numberOfProcessorsSpinBox->value(), papszOptions);
        if(err != NINJA_SUCCESS)
        {
            printf("NinjaSetNumberCPUs: err = %d\n", err);
        }

        err = NinjaSetInitializationMethod(ninjaArmy, i, initializationMethod, papszOptions);
        if(err != NINJA_SUCCESS)
        {
            printf("NinjaSetInitializationMethod: err = %d\n", err);
        }

        err = NinjaSetDem(ninjaArmy, i, ui->elevationInputFileLineEdit->property("fullpath").toString().toUtf8().constData(), papszOptions);
        if(err != NINJA_SUCCESS)
        {
            printf("NinjaSetDem: err = %d\n", err);
        }

        err = NinjaSetPosition(ninjaArmy, i, papszOptions);
        if(err != NINJA_SUCCESS)
        {
            printf("NinjaSetPosition: err = %d\n", err);
        }

        err = NinjaSetInputWindHeight(ninjaArmy, i, ui->inputWindHeightSpinBox->value(), "m", papszOptions);
        if(err != NINJA_SUCCESS)
        {
            printf("NinjaSetInputWindHeight: err = %d\n", err);
        }

        err = NinjaSetDiurnalWinds(ninjaArmy, i, ui->diurnalCheckBox->isChecked(), papszOptions);
        if(err != NINJA_SUCCESS)
        {
            printf("NinjaSetDiurnalWinds: err = %d\n", err);
        }

        err = NinjaSetUniVegetation(ninjaArmy, i, ui->vegetationComboBox->currentText().toLower().toUtf8().constData(), papszOptions);
        if(err != NINJA_SUCCESS)
        {
            printf("NinjaSetUniVegetation: err = %d\n", err);
        }

        err = NinjaSetMeshResolutionChoice(ninjaArmy, i, ui->meshResolutionComboBox->currentText().toLower().toUtf8().constData(), papszOptions);
        if(err != NINJA_SUCCESS)
        {
            printf("NinjaSetMeshResolutionChoice: err = %d\n", err);
        }

        err = NinjaSetNumVertLayers(ninjaArmy, i, 20, papszOptions);
        if(err != NINJA_SUCCESS)
        {
            printf("NinjaSetNumVertLayers: err = %d\n", err);
        }

        setOutputFlags(ninjaArmy, i);
    }
}

void MainWindow::setOutputFlags(NinjaArmyH *ninjaArmy, int i)
{
    char **papszOptions = nullptr;
    int err;

    double googleEarthMeshResolution;
    QByteArray googleEarthMeshResolutionUnits;
    if (!ui->googleEarthMeshResolutionGroupBox->isChecked())
    {
        googleEarthMeshResolution = ui->googleEarthMeshResolutionSpinBox->value();
        googleEarthMeshResolutionUnits = ui->googleEarthMeshResolutionComboBox
                                             ->itemData(ui->googleEarthMeshResolutionComboBox->currentIndex())
                                             .toString().toUtf8();
    }
    else
    {
        googleEarthMeshResolution = ui->meshResolutionSpinBox->value();
        googleEarthMeshResolutionUnits = ui->meshResolutionUnitsComboBox
                                             ->itemData(ui->meshResolutionUnitsComboBox->currentIndex())
                                             .toString().toUtf8();
    }

    double fireBehaviorMeshResolution;
    QByteArray fireBehaviorMeshResolutionUnits;
    if (!ui->fireBehaviorMeshResolutionGroupBox->isChecked())
    {
        fireBehaviorMeshResolution = ui->fireBehaviorMeshResolutionSpinBox->value();
        fireBehaviorMeshResolutionUnits = ui->fireBehaviorMeshResolutionComboBox
                                              ->itemData(ui->fireBehaviorMeshResolutionComboBox->currentIndex())
                                              .toString().toUtf8();
    }
    else
    {
        fireBehaviorMeshResolution = ui->meshResolutionSpinBox->value();
        fireBehaviorMeshResolutionUnits = ui->meshResolutionUnitsComboBox
                                              ->itemData(ui->meshResolutionUnitsComboBox->currentIndex())
                                              .toString().toUtf8();
    }

    double shapeFilesMeshResolution;
    QByteArray shapeFilesMeshResolutionUnits;
    if (!ui->shapeFilesMeshResolutionGroupBox->isChecked())
    {
        shapeFilesMeshResolution = ui->shapeFilesMeshResolutionSpinBox->value();
        shapeFilesMeshResolutionUnits = ui->shapeFilesMeshResolutionComboBox
                                            ->itemData(ui->shapeFilesMeshResolutionComboBox->currentIndex())
                                            .toString().toUtf8();
    }
    else
    {
        shapeFilesMeshResolution = ui->meshResolutionSpinBox->value();
        shapeFilesMeshResolutionUnits = ui->meshResolutionUnitsComboBox
                                            ->itemData(ui->meshResolutionUnitsComboBox->currentIndex())
                                            .toString().toUtf8();
    }

    double geospatialPDFFilesMeshResolution;
    QByteArray geospatialPDFFilesMeshResolutionUnits;
    if (!ui->geospatialPDFFilesGroupBox->isChecked())
    {
        geospatialPDFFilesMeshResolution = ui->geospatialPDFFilesMeshResolutionSpinBox->value();
        geospatialPDFFilesMeshResolutionUnits = ui->geospatialPDFFilesMeshResolutionComboBox
                                                    ->itemData(ui->geospatialPDFFilesMeshResolutionComboBox->currentIndex())
                                                    .toString().toUtf8();
    }
    else
    {
        geospatialPDFFilesMeshResolution = ui->meshResolutionSpinBox->value();
        geospatialPDFFilesMeshResolutionUnits = ui->meshResolutionUnitsComboBox
                                                    ->itemData(ui->meshResolutionUnitsComboBox->currentIndex())
                                                    .toString().toUtf8();
    }

    double PDFHeight, PDFWidth, PDFDpi;
    switch(ui->sizeDimensionsComboBox->currentIndex())
    {
    case 0:
        PDFHeight = 11.0;
        PDFWidth = 8.5;
        PDFDpi = 150;
        break;
    case 1:
        PDFHeight = 14.0;
        PDFWidth = 8.5;
        PDFDpi = 150;
        break;
    case 2:
        PDFHeight = 17.0;
        PDFWidth = 11.0;
        PDFDpi = 150;
        break;
    }
    if (ui->sizeOrientationComboBox->currentIndex() == 1)
    {
        std::swap(PDFHeight, PDFWidth);
    }


    err = NinjaSetOutputPath(ninjaArmy, i, ui->outputDirectoryLineEdit->text().toUtf8().constData(), papszOptions);
    if (err != NINJA_SUCCESS)
    {
        qDebug() << "NinjaSetOutputPath: err =" << err;
    }

    err = NinjaSetOutputWindHeight(ninjaArmy, i, ui->outputWindHeightSpinBox->value(), ui->outputWindHeightUnitsComboBox->itemData(ui->outputWindHeightUnitsComboBox->currentIndex()).toString().toUtf8().constData(), papszOptions);
    if (err != NINJA_SUCCESS)
    {
        qDebug() << "NinjaSetOutputWindHeight: err =" << err;
    }

    err = NinjaSetOutputSpeedUnits(ninjaArmy, i, ui->outputSpeedUnitsComboBox->currentText().toUtf8().constData(), papszOptions);
    if (err != NINJA_SUCCESS)
    {
        qDebug() << "NinjaSetOutputSpeedUnits: err =" << err;
    }

    err = NinjaSetGoogOutFlag(ninjaArmy, i, ui->googleEarthGroupBox->isChecked(), papszOptions);
    if (err != NINJA_SUCCESS)
    {
        qDebug() << "NinjaSetGoogOutFlag: err =" << err;
    }

    err = NinjaSetGoogResolution(ninjaArmy, i, googleEarthMeshResolution, googleEarthMeshResolutionUnits.constData(), papszOptions);
    if (err != NINJA_SUCCESS)
    {
        qDebug() << "NinjaSetGoogResolution: err =" << err;
    }

    err = NinjaSetGoogSpeedScaling(ninjaArmy, i, ui->legendComboBox->itemData(ui->legendComboBox->currentIndex()).toString().toUtf8().constData(), papszOptions);
    if (err != NINJA_SUCCESS)
    {
        qDebug() << "NinjaSetGoogSpeedScaling: err =" << err;
    }

    err = NinjaSetGoogLineWidth(ninjaArmy, i, ui->googleEarthVectorsSpinBox->value(), papszOptions);
    if (err != NINJA_SUCCESS)
    {
        qDebug() << "NinjaSetGoogLineWidth: err =" << err;
    }

    err = NinjaSetAsciiOutFlag(ninjaArmy, i, ui->fireBehaviorGroupBox->isChecked(), papszOptions);
    if (err != NINJA_SUCCESS)
    {
        qDebug() << "NinjaSetAsciiOutFlag: err =" << err;
    }

    err = NinjaSetAsciiResolution(ninjaArmy, i, fireBehaviorMeshResolution, fireBehaviorMeshResolutionUnits.constData(), papszOptions);
    if (err != NINJA_SUCCESS)
    {
        qDebug() << "NinjaSetAsciiResolution: err =" << err;
    }

    err = NinjaSetShpOutFlag(ninjaArmy, i, ui->shapeFilesGroupBox->isChecked(), papszOptions);
    if (err != NINJA_SUCCESS)
    {
        qDebug() << "NinjaSetShpOutFlag: err =" << err;
    }

    err = NinjaSetShpResolution(ninjaArmy, i, shapeFilesMeshResolution, shapeFilesMeshResolutionUnits.constData(), papszOptions);
    if (err != NINJA_SUCCESS)
    {
        qDebug() << "NinjaSetShpResolution: err =" << err;
    }

    err = NinjaSetPDFOutFlag(ninjaArmy, i, ui->geospatialPDFFilesGroupBox->isChecked(), papszOptions);
    if (err != NINJA_SUCCESS)
    {
        qDebug() << "NinjaSetPDFOutFlag: err =" << err;
    }

    err = NinjaSetPDFLineWidth(ninjaArmy, i, ui->geospatialPDFFilesVectorsSpinBox->value(), papszOptions);
    if (err != NINJA_SUCCESS)
    {
        qDebug() << "NinjaSetPDFLineWidth: err =" << err;
    }

    err = NinjaSetPDFBaseMap(ninjaArmy, i, ui->basemapComboBox->currentIndex(), papszOptions);
    if (err != NINJA_SUCCESS)
    {
        qDebug() << "NinjaSetPDFBaseMap: err =" << err;
    }

    err = NinjaSetPDFDEM(ninjaArmy, i, ui->elevationInputFileLineEdit->property("fullpath").toString().toUtf8().constData(), papszOptions);
    if (err != NINJA_SUCCESS)
    {
        qDebug() << "NinjaSetPDFDEM: err =" << err;
    }

    err = NinjaSetPDFSize(ninjaArmy, i, PDFHeight, PDFWidth, PDFDpi, papszOptions);
    if (err != NINJA_SUCCESS)
    {
        qDebug() << "NinjaSetPDFSize: err =" << err;
    }

    err = NinjaSetPDFResolution(ninjaArmy, i, geospatialPDFFilesMeshResolution, geospatialPDFFilesMeshResolutionUnits.constData(), papszOptions);
    if (err != NINJA_SUCCESS)
    {
        qDebug() << "NinjaSetPDFResolution: err =" << err;
    }

    err = NinjaSetVtkOutFlag(ninjaArmy, i, ui->VTKFilesCheckBox->isChecked(), papszOptions);
    if (err != NINJA_SUCCESS)
    {
        qDebug() << "NinjaSetVtkOutFlag: err =" << err;
    }
}


