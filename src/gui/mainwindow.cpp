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
    serverBridge = new ServerBridge();
    serverBridge->checkMessages();

    ui->setupUi(this);    
    resize(1200, 700);
    refreshUI();
    ui->treeWidget->expandAll();
    ui->treeFileExplorer->expandAll();

    QWebEngineProfile::defaultProfile()->settings()->setAttribute(QWebEngineSettings::LocalContentCanAccessRemoteUrls, true);
    QWebEngineProfile::defaultProfile()->settings()->setAttribute(QWebEngineSettings::LocalContentCanAccessFileUrls, true);
    QString dataPath = QString::fromUtf8(CPLGetConfigOption("WINDNINJA_DATA", ""));
    QString mapPath = QDir(dataPath).filePath("map.html");
    webChannel = new QWebChannel(this);
    mapBridge = new MapBridge(this);
    webEngineView = new QWebEngineView(ui->mapPanelWidget);
    webChannel->registerObject(QStringLiteral("bridge"), mapBridge);
    webEngineView->page()->setWebChannel(webChannel);
    QUrl url = QUrl::fromLocalFile(mapPath);
    webEngineView->setUrl(url);
    QVBoxLayout *layout = new QVBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(webEngineView);
    ui->mapPanelWidget->setLayout(layout);
    menuBar = new MenuBar(ui, this);
    surfaceInput = new SurfaceInput(ui, webEngineView, this);
    domainAverageInput = new DomainAverageInput(ui, this);

    ui->inputsStackedWidget->setCurrentIndex(0);
    ui->treeWidget->topLevelItem(0)->setData(0, Qt::UserRole, 1);
    ui->treeWidget->topLevelItem(0)->child(0)->setData(0, Qt::UserRole, 2);
    ui->treeWidget->topLevelItem(0)->child(1)->setData(0, Qt::UserRole, 3);
    ui->treeWidget->topLevelItem(1)->setData(0, Qt::UserRole, 4);
    ui->treeWidget->topLevelItem(1)->child(0)->setData(0, Qt::UserRole, 5);
    ui->treeWidget->topLevelItem(1)->child(1)->setData(0, Qt::UserRole, 6);
    ui->treeWidget->topLevelItem(1)->child(2)->setData(0, Qt::UserRole, 7);
    ui->treeWidget->topLevelItem(1)->child(3)->setData(0, Qt::UserRole, 8);
    QTreeWidgetItem *windInputItem = ui->treeWidget->topLevelItem(1)->child(3);
    windInputItem->child(0)->setData(0, Qt::UserRole, 9);
    windInputItem->child(1)->setData(0, Qt::UserRole, 10);
    windInputItem->child(2)->setData(0, Qt::UserRole, 11);
    ui->treeWidget->topLevelItem(2)->setData(0, Qt::UserRole, 12);
    ui->treeWidget->topLevelItem(2)->child(0)->setData(0, Qt::UserRole, 13);
    ui->treeWidget->topLevelItem(2)->child(1)->setData(0, Qt::UserRole, 14);
    ui->treeWidget->topLevelItem(2)->child(2)->setData(0, Qt::UserRole, 15);
    ui->treeWidget->topLevelItem(2)->child(3)->setData(0, Qt::UserRole, 16);
    ui->treeWidget->topLevelItem(2)->child(4)->setData(0, Qt::UserRole, 17);
    ui->treeWidget->topLevelItem(3)->setData(0, Qt::UserRole, 18);

    int nCPUs = QThread::idealThreadCount();
    ui->availableProcessorsTextEdit->setPlainText("Available Processors:  " + QString::number(nCPUs));
    ui->numberOfProcessorsSpinBox->setMaximum(nCPUs);
    ui->numberOfProcessorsSpinBox->setValue(nCPUs);
    ui->downloadPointInitData->setIcon(QIcon(":/application_get"));

    QString downloadsPath = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
    ui->outputDirectoryLineEdit->setText(downloadsPath);
    ui->outputDirectoryButton->setIcon(QIcon(":/folder.png"));

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
    ui->alternativeColorSchemeComboBox->setItemData(0, "default");
    ui->alternativeColorSchemeComboBox->setItemData(1, "ROPGW");
    ui->alternativeColorSchemeComboBox->setItemData(2, "oranges");
    ui->alternativeColorSchemeComboBox->setItemData(3, "blues");
    ui->alternativeColorSchemeComboBox->setItemData(4, "pinks");
    ui->alternativeColorSchemeComboBox->setItemData(5, "greens");
    ui->alternativeColorSchemeComboBox->setItemData(6, "magic_beans");
    ui->alternativeColorSchemeComboBox->setItemData(7, "pink_to_green");
    ui->legendComboBox->setItemData(0, "equal_interval");
    ui->legendComboBox->setItemData(1, "equal_color");

    connectSignals();
}

MainWindow::~MainWindow()
{
    delete webEngineView;
    delete webChannel;
    delete mapBridge;
    delete surfaceInput;
    delete menuBar;
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
    connect(ui->pointInitializationCheckBox, &QCheckBox::clicked, this, &MainWindow::pointInitializationCheckBoxClicked);
    connect(ui->solveButton, &QPushButton::clicked, this, &MainWindow::solveButtonClicked);
    connect(ui->numberOfProcessorsSolveButton, &QPushButton::clicked, this, &MainWindow::numberOfProcessorsSolveButtonClicked);
    connect(mapBridge, &MapBridge::boundingBoxReceived, surfaceInput, &SurfaceInput::boundingBoxReceived);
    connect(surfaceInput, &SurfaceInput::requestRefresh, this, &MainWindow::refreshUI);
    connect(domainAverageInput, &DomainAverageInput::requestRefresh, this, &MainWindow::refreshUI);
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
}

void MainWindow::treeItemClicked(QTreeWidgetItem *item, int column)
{
    int pageIndex = item->data(column, Qt::UserRole).toInt();
    ui->inputsStackedWidget->setCurrentIndex(pageIndex);
}

void MainWindow::massSolverCheckBoxClicked()
{
    AppState& state = AppState::instance();

    if (state.isMomentumSolverToggled)
    {
        ui->momentumSolverCheckBox->setChecked(false);
        state.isMomentumSolverToggled = ui->momentumSolverCheckBox->isChecked();
    }
    state.isMassSolverToggled = ui->massSolverCheckBox->isChecked();

    if(!ui->elevationInputFileLineEdit->text().isEmpty())
    {
        ui->meshResolutionSpinBox->setValue(surfaceInput->computeMeshResolution(ui->meshResolutionComboBox->currentIndex(), ui->momentumSolverCheckBox->isChecked()));
    }
    refreshUI();
}

void MainWindow::momentumSolverCheckBoxClicked()
{
    AppState& state = AppState::instance();

    if (state.isMassSolverToggled)
    {
        ui->massSolverCheckBox->setChecked(false);
        state.isMassSolverToggled = ui->massSolverCheckBox->isChecked();
    }
    state.isMomentumSolverToggled = ui->momentumSolverCheckBox->isChecked();

    if(!ui->elevationInputFileLineEdit->text().isEmpty())
    {
        ui->meshResolutionSpinBox->setValue(surfaceInput->computeMeshResolution(ui->meshResolutionComboBox->currentIndex(), ui->momentumSolverCheckBox->isChecked()));
    }
    refreshUI();
}

void MainWindow::diurnalCheckBoxClicked()
{
    AppState& state = AppState::instance();
    state.isDiurnalInputToggled = ui->diurnalCheckBox->isChecked();

    QTableWidget* table = ui->domainAverageTable;
    if(!ui->diurnalCheckBox->isChecked())
    {
        table->hideColumn(2);
        table->hideColumn(3);
        table->hideColumn(4);
        table->hideColumn(5);
        ui->domainAverageTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    }
    else
    {
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


void MainWindow::pointInitializationCheckBoxClicked()
{
    AppState& state = AppState::instance();
    state.isPointInitializationToggled = ui->pointInitializationCheckBox->isChecked();

    if (state.isPointInitializationToggled)
    {
        ui->domainAverageGroupBox->setChecked(false);
        ui->weatherModelCheckBox->setChecked(false);
        state.isDomainAverageInitializationToggled = ui->domainAverageGroupBox->isChecked();
        state.isWeatherModelInitializationToggled = ui->weatherModelCheckBox->isChecked();
    }

    refreshUI();
}

void MainWindow::useWeatherModelInitClicked()
{
    AppState& state = AppState::instance();

    state.isWeatherModelInitializationToggled = ui->weatherModelCheckBox->isChecked();

    if (state.isWeatherModelInitializationToggled)
    {
        ui->domainAverageGroupBox->setChecked(false);
        ui->pointInitializationCheckBox->setChecked(false);
        state.isDomainAverageInitializationToggled = ui->domainAverageGroupBox->isChecked();
        state.isPointInitializationToggled = ui->pointInitializationCheckBox->isChecked();
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

    int numNinjas;
    NinjaArmyH *ninjaArmy;
    char **papszOptions;
    const char *initializationMethod;
    QList<double> speeds;
    QList<double> directions;

    if (state.isDomainAverageInitializationValid)
    {
        initializationMethod = "domain_average";

        int rowCount = ui->domainAverageTable->rowCount();
        for (int row = 0; row < rowCount; ++row)
        {
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

    if(ui->googleEarthGroupBox->isChecked())
    {
        vector<string> outputFiles;
        QDir outDir(ui->outputDirectoryLineEdit->text());
        QString demName = QFileInfo(ui->elevationInputFileLineEdit->text()).baseName();
        int meshInt = static_cast<int>(std::round(ui->meshResolutionSpinBox->value()));
        QString meshSize = QString::number(meshInt) + "m";

        for (int i = 0; i < numNinjas; i++)
        {
            QString filePath = outDir.filePath(QString("%1_%2_%3_%4.kmz")
                                                   .arg(demName)
                                                   .arg(directions[i])
                                                   .arg(speeds[i])
                                                   .arg(meshSize));
            outputFiles.push_back(filePath.toStdString());
        }

        for (const auto& dir : outputFiles)
        {
            QString qDir = QString::fromStdString(dir);

            QFile f(qDir);
            f.open(QIODevice::ReadOnly);
            QByteArray data = f.readAll();
            QString base64 = data.toBase64();

            webEngineView->page()->runJavaScript("loadKmzFromBase64('"+base64+"')");
        }
    }
}

void MainWindow::treeWidgetItemDoubleClicked(QTreeWidgetItem *item, int column)
{
    if (item->text(0) == "Conservation of Mass")
    {
        ui->massSolverCheckBox->click();
    }
    else if (item->text(0) == "Conservation of Mass and Momentum")
    {
        ui->momentumSolverCheckBox->click();
    }
    else if (item->text(0) == "Diurnal Input")
    {
        ui->diurnalCheckBox->click();
    }
    else if (item->text(0) == "Stability Input")
    {
        ui->stabilityCheckBox->click();
    }
    else if (item->text(0) == "Domain Average Wind")
    {
        if(!ui->domainAverageGroupBox->isChecked())
        {
            ui->domainAverageGroupBox->setChecked(true);
        }
        else
        {
            ui->domainAverageGroupBox->setChecked(false);
        }
    }
    else if (item->text(0) == "Point Initialization")
    {
        ui->pointInitializationCheckBox->click();
    }
    else if (item->text(0) == "Weather Model")
    {
        ui->weatherModelCheckBox->click();
    }
    else if (item->text(0) == "Surface Input")
    {
        ui->elevationInputFileOpenButton->click();
    }
    else if (item->text(0) == "Google Earth")
    {
        if(!ui->googleEarthGroupBox->isChecked())
        {
            ui->googleEarthGroupBox->setChecked(true);
        }
        else
        {
            ui->googleEarthGroupBox->setChecked(false);
        }
    }
    else if (item->text(0) == "Fire Behavior")
    {
        if(!ui->fireBehaviorGroupBox->isChecked())
        {
            ui->fireBehaviorGroupBox->setChecked(true);
        }
        else
        {
            ui->fireBehaviorGroupBox->setChecked(false);
        }
    }
    else if (item->text(0) == "Shape Files")
    {
        if(!ui->shapeFilesGroupBox->isChecked())
        {
            ui->shapeFilesGroupBox->setChecked(true);
        }
        else
        {
            ui->shapeFilesGroupBox->setChecked(false);
        }
    }
    else if (item->text(0) == "Geospatial PDF Files")
    {
        if(!ui->geospatialPDFFilesGroupBox->isChecked())
        {
            ui->geospatialPDFFilesGroupBox->setChecked(true);
        }
        else
        {
            ui->geospatialPDFFilesGroupBox->setChecked(false);
        }
    }
    else if (item->text(0) == "VTK Files")
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
    OutputMeshResolution googleEarth = getMeshResolution(
        ui->googleEarthMeshResolutionGroupBox->isChecked(),
        ui->googleEarthMeshResolutionSpinBox,
        ui->googleEarthMeshResolutionComboBox);

    OutputMeshResolution fireBehavior = getMeshResolution(
        ui->fireBehaviorMeshResolutionGroupBox->isChecked(),
        ui->fireBehaviorMeshResolutionSpinBox,
        ui->fireBehaviorMeshResolutionComboBox);

    OutputMeshResolution shapeFiles = getMeshResolution(
        ui->shapeFilesMeshResolutionGroupBox->isChecked(),
        ui->shapeFilesMeshResolutionSpinBox,
        ui->shapeFilesMeshResolutionComboBox);

    OutputMeshResolution geospatialPDFs = getMeshResolution(
        ui->geospatialPDFFilesMeshResolutionGroupBox->isChecked(),
        ui->geospatialPDFFilesMeshResolutionSpinBox,
        ui->geospatialPDFFilesMeshResolutionComboBox);

    OutputPDFSize PDFSize;
    switch(ui->sizeDimensionsComboBox->currentIndex())
    {
    case 0:
        PDFSize.PDFHeight = 11.0;
        PDFSize.PDFWidth = 8.5;
        PDFSize.PDFDpi = 150;
        break;
    case 1:
        PDFSize.PDFHeight = 14.0;
        PDFSize.PDFWidth = 8.5;
        PDFSize.PDFDpi = 150;
        break;
    case 2:
        PDFSize.PDFHeight = 17.0;
        PDFSize.PDFWidth = 11.0;
        PDFSize.PDFDpi = 150;
        break;
    }
    if (ui->sizeOrientationComboBox->currentIndex() == 1)
    {
        std::swap(PDFSize.PDFHeight, PDFSize.PDFWidth);
    }

    char **papszOptions = nullptr;
    int err;
    err = NinjaSetAsciiAtmFile(ninjaArmy, ui->fireBehaviorResolutionCheckBox->isChecked(), papszOptions);
    if(err != NINJA_SUCCESS)
    {
        qDebug() << "NinjaSetAsciiAtmFile: err =" << err;
    }

    for(unsigned int i=0; i<numNinjas; i++)
    {
        err = NinjaSetCommunication(ninjaArmy, i, "cli", papszOptions);
        if(err != NINJA_SUCCESS)
        {
            qDebug() << "NinjaSetCommunication: err =" << err;
        }

        err = NinjaSetNumberCPUs(ninjaArmy, i, ui->numberOfProcessorsSpinBox->value(), papszOptions);
        if(err != NINJA_SUCCESS)
        {
            qDebug() << "NinjaSetNumberCPUs: err =" << err;
        }

        err = NinjaSetInitializationMethod(ninjaArmy, i, initializationMethod, papszOptions);
        if(err != NINJA_SUCCESS)
        {
            qDebug() << "NinjaSetInitializationMethod: err =" << err;
        }

        err = NinjaSetDem(ninjaArmy, i, surfaceInput->getDEMFilePath().toUtf8().constData(), papszOptions);
        if(err != NINJA_SUCCESS)
        {
            qDebug() << "NinjaSetDem: err =" << err;
        }

        err = NinjaSetPosition(ninjaArmy, i, papszOptions);
        if(err != NINJA_SUCCESS)
        {
            qDebug() << "NinjaSetPosition: err =" << err;
        }

        err = NinjaSetInputWindHeight(ninjaArmy, i, ui->inputWindHeightSpinBox->value(), "m", papszOptions);
        if(err != NINJA_SUCCESS)
        {
            qDebug() << "NinjaSetInputWindHeight: err =" << err;
        }

        err = NinjaSetDiurnalWinds(ninjaArmy, i, ui->diurnalCheckBox->isChecked(), papszOptions);
        if(err != NINJA_SUCCESS)
        {
            qDebug() << "NinjaSetDiurnalWinds: err =" << err;
        }

        err = NinjaSetUniVegetation(ninjaArmy, i, ui->vegetationComboBox->currentText().toLower().toUtf8().constData(), papszOptions);
        if(err != NINJA_SUCCESS)
        {
            qDebug() << "NinjaSetUniVegetation: err =" << err;
        }

        err = NinjaSetMeshResolutionChoice(ninjaArmy, i, ui->meshResolutionComboBox->currentText().toLower().toUtf8().constData(), papszOptions);
        if(err != NINJA_SUCCESS)
        {
            qDebug() << "NinjaSetMeshResolutionChoice: err =" << err;
        }

        err = NinjaSetNumVertLayers(ninjaArmy, i, 20, papszOptions);
        if(err != NINJA_SUCCESS)
        {
            qDebug() << "NinjaSetNumVertLayers: err =" << err;
        }

        setOutputFlags(ninjaArmy, i, numNinjas, googleEarth, fireBehavior, shapeFiles, geospatialPDFs, PDFSize);
    }
}

void MainWindow::setOutputFlags(NinjaArmyH* ninjaArmy,
                                int i,
                                int numNinjas,
                                OutputMeshResolution googleEarth,
                                OutputMeshResolution fireBehavior,
                                OutputMeshResolution shapeFiles,
                                OutputMeshResolution geospatialPDFs,
                                OutputPDFSize PDFSize)
{
    char **papszOptions = nullptr;
    int err;

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

    err = NinjaSetGoogResolution(ninjaArmy, i, googleEarth.resolution, googleEarth.units.constData(), papszOptions);
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

    err = NinjaSetGoogColor(ninjaArmy, i, ui->alternativeColorSchemeComboBox->itemData(ui->alternativeColorSchemeComboBox->currentIndex()).toString().toUtf8().constData(), ui->googleEarthVectorScalingCheckBox->isChecked(), papszOptions);
    if (err != NINJA_SUCCESS)
    {
        qDebug() << "NinjaSetGoogColor: err =" << err;
    }

    err = NinjaSetGoogConsistentColorScale(ninjaArmy, i, ui->legendCheckBox->isChecked(), numNinjas, papszOptions);
    if (err != NINJA_SUCCESS)
    {
        qDebug() << "NinjaSetGoogConsistentColorScale: err =" << err;
    }

    err = NinjaSetAsciiOutFlag(ninjaArmy, i, ui->fireBehaviorGroupBox->isChecked(), papszOptions);
    if (err != NINJA_SUCCESS)
    {
        qDebug() << "NinjaSetAsciiOutFlag: err =" << err;
    }

    err = NinjaSetAsciiResolution(ninjaArmy, i, fireBehavior.resolution, fireBehavior.units.constData(), papszOptions);
    if (err != NINJA_SUCCESS)
    {
        qDebug() << "NinjaSetAsciiResolution: err =" << err;
    }

    err = NinjaSetShpOutFlag(ninjaArmy, i, ui->shapeFilesGroupBox->isChecked(), papszOptions);
    if (err != NINJA_SUCCESS)
    {
        qDebug() << "NinjaSetShpOutFlag: err =" << err;
    }

    err = NinjaSetShpResolution(ninjaArmy, i, shapeFiles.resolution, shapeFiles.units.constData(), papszOptions);
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

    err = NinjaSetPDFDEM(ninjaArmy, i, surfaceInput->getDEMFilePath().toUtf8().constData(), papszOptions);
    if (err != NINJA_SUCCESS)
    {
        qDebug() << "NinjaSetPDFDEM: err =" << err;
    }

    err = NinjaSetPDFSize(ninjaArmy, i, PDFSize.PDFHeight, PDFSize.PDFWidth, PDFSize.PDFDpi, papszOptions);
    if (err != NINJA_SUCCESS)
    {
        qDebug() << "NinjaSetPDFSize: err =" << err;
    }

    err = NinjaSetPDFResolution(ninjaArmy, i, geospatialPDFs.resolution, geospatialPDFs.units.constData(), papszOptions);
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

OutputMeshResolution MainWindow::getMeshResolution(
    bool useOutputMeshResolution,
    QDoubleSpinBox* outputMeshResolutionSpinBox,
    QComboBox* outputMeshResolutionComboBox)
{
    OutputMeshResolution result;

    if (!useOutputMeshResolution)
    {
        result.resolution = outputMeshResolutionSpinBox->value();
        result.units = outputMeshResolutionComboBox->itemData(outputMeshResolutionComboBox->currentIndex()).toString().toUtf8();
    }
    else
    {
        result.resolution = ui->meshResolutionSpinBox->value();
        result.units = ui->meshResolutionUnitsComboBox->itemData(ui->meshResolutionComboBox->currentIndex()).toString().toUtf8();
    }

    return result;
}



