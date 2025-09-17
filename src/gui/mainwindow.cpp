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

void MainWindow::writeToConsole(QString message, QColor color)
{
    // if( ui->consoleDockWidget->isFloating() && color == Qt::white )
    // {
    //     color = Qt::black;
    // }

    ui->consoleTextEdit->setTextColor(color);
    ui->consoleTextEdit->append(QString::number(lineNumber) + ": " + message);
    ui->consoleTextEdit->repaint();
    lineNumber++;
}

void MainWindow::updateProgressMessage(const QString message)
{
    progressDialog->setLabelText(message);
}

void MainWindow::updateProgressValue(int run, int progress)
{
    // update the stored progress value for the current run
    if( runProgress[run] > progress )
    {
        // if the stored progress is bigger than what we are seeing in the currently emitted progress
        // ignore it. This happens for pointInitialization, when the match points is iterating,
        // sometimes its next solution is worse and then it would make the progress bar go backwards
        // by ignoring it, the progress bar just stays where it is
        runProgress[run] = runProgress[run];
    }
    else
    {
        // otherwise, store the progress for the current run
        runProgress[run] = progress;
    }

    // update the total progress value
    // calculate the total progress from scratch each time, summing up the progress from each run
    totalProgress = 0;  // Initialize the progress bar each time
    for(unsigned int i = 0; i < runProgress.size(); i++)
    {
        totalProgress = totalProgress + runProgress[i];
    }

    // update the progress bar
    progressDialog->setValue(totalProgress);
}

void MainWindow::cancelSolve()
{
    progressDialog->setLabelText("Canceling...");

    char **papszOptions = nullptr;
    int err = NinjaCancel(ninjaArmy, papszOptions);
    if( err != NINJA_SUCCESS )
    {
        qDebug() << "NinjaCancel: err =" << err;
    }
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow)
{
    lineNumber = 1;

    serverBridge = new ServerBridge();
    serverBridge->checkMessages();

    ui->setupUi(this);    
    resize(1200, 700);
    refreshUI();
    ui->treeWidget->expandAll();

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
    pointInitializationInput = new PointInitializationInput(ui, this);
    weatherModelInput = new WeatherModelInput(ui, this);

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


    QString version(NINJA_VERSION_STRING);
    version = "Welcome to WindNinja " + version;

    writeToConsole(version, Qt::blue);

    writeToConsole("WINDNINJA_DATA=" + dataPath);
}

MainWindow::~MainWindow()
{
    delete webEngineView;
    delete webChannel;
    delete mapBridge;
    delete surfaceInput;
    delete domainAverageInput;
    delete pointInitializationInput;
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
    connect(ui->solveButton, &QPushButton::clicked, this, &MainWindow::solveButtonClicked);
    connect(ui->numberOfProcessorsSolveButton, &QPushButton::clicked, this, &MainWindow::numberOfProcessorsSolveButtonClicked);
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

    connect(menuBar, &MenuBar::writeToConsole, this, &MainWindow::writeToConsole);
//    connect(menuBar, SIGNAL( writeToConsole(QString, QColor) ), this, SLOT( writeToConsole(QString, QColor) ));  // other way to do it
    connect(mapBridge, &MapBridge::boundingBoxReceived, surfaceInput, &SurfaceInput::boundingBoxReceived);
    connect(surfaceInput, &SurfaceInput::requestRefresh, this, &MainWindow::refreshUI);
    connect(surfaceInput, &SurfaceInput::setupTreeView, pointInitializationInput, &PointInitializationInput::setupTreeView);
    connect(surfaceInput, &SurfaceInput::setupTreeView, weatherModelInput, &WeatherModelInput::setUpTreeView);
    connect(domainAverageInput, &DomainAverageInput::requestRefresh, this, &MainWindow::refreshUI);
    connect(pointInitializationInput, &PointInitializationInput::requestRefresh, this, &MainWindow::refreshUI);
    connect(weatherModelInput, &WeatherModelInput::requestRefresh, this, &MainWindow::refreshUI);
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
    ninjaArmy = nullptr;
    char **papszOptions = nullptr;
    const char *initializationMethod = nullptr;

    if (state.isDomainAverageInitializationValid)
    {
        initializationMethod = "domain_average";
        QList<double> speeds;
        QList<double> directions;

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
    else if (state.isPointInitializationValid)
    {
        initializationMethod = "point";

        QVector<QString> stationFiles = pointInitializationInput->getStationFiles();
        QString DEMTimeZone = ui->timeZoneComboBox->currentText();
        QByteArray timeZoneBytes = ui->timeZoneComboBox->currentText().toUtf8();

        std::vector<QByteArray> stationFilesBytes;
        stationFilesBytes.reserve(stationFiles.size());
        std::vector<const char*> stationFileNames;
        stationFileNames.reserve(stationFiles.size());
        for (int i = 0; i < stationFiles.size(); i++)
        {
            stationFilesBytes.push_back(stationFiles[i].toUtf8());
            stationFileNames.push_back(stationFilesBytes.back().constData());
        }

        QString DEMPath = ui->elevationInputFileLineEdit->property("fullpath").toString();
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

            if(nTimeSteps == 1)
            {
                int startYear = year[0];
                int startMonth = month[0];
                int startDay = day[0];
                int startHour = hour[0];
                int startMinute = minute[0];

                int endYear, endMonth, endDay, endHour, endMinute;

                NinjaErr err = NinjaGenerateSingleTimeObject(
                    startYear, startMonth, startDay, startHour, startMinute,
                    timeZoneBytes.constData(),
                    &endYear, &endMonth, &endDay, &endHour, &endMinute
                    );
                if(err != NINJA_SUCCESS)
                {
                    qDebug() << "NinjaGenerateSingleTimeObject: err = " << err;
                }

                outYear[0] = endYear;
                outMonth[0] = endMonth;
                outDay[0] = endDay;
                outHour[0] = endHour;
                outMinute[0] = endMinute;
            }
            else {
                NinjaErr err = NinjaGetTimeList(
                    year.data(), month.data(), day.data(),
                    hour.data(), minute.data(),
                    outYear.data(), outMonth.data(), outDay.data(),
                    outHour.data(), outMinute.data(),
                    nTimeSteps, timeZoneBytes.data()
                );
                if(err != NINJA_SUCCESS)
                {
                    qDebug() << "NinjaGetTimeList: err = " << err;
                }
            }

            numNinjas = ui->weatherStationDataTimestepsSpinBox->value();

            ninjaArmy = NinjaMakePointArmy(
                outYear.data(), outMonth.data(), outDay.data(),
                outHour.data(), outMinute.data(), nTimeSteps,
                DEMTimeZone.toUtf8().data(), stationFileNames.data(),
                stationFileNames.size(), DEMPath.toUtf8().data(),
                true, momentumFlag, papszOptions
                );
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
                qDebug() << "NinjaGenerateSingleTimeObject: err = " << err;
            }

            QVector<int> yearVec   = { outYear };
            QVector<int> monthVec  = { outMonth };
            QVector<int> dayVec    = { outDay };
            QVector<int> hourVec   = { outHour };
            QVector<int> minuteVec = { outMinute };

            numNinjas = 1;
            int nTimeSteps = 1;

            ninjaArmy = NinjaMakePointArmy(
                yearVec.data(), monthVec.data(), dayVec.data(),
                hourVec.data(), minuteVec.data(), nTimeSteps,
                DEMTimeZone.toUtf8().data(),
                stationFileNames.data(),
                static_cast<int>(stationFileNames.size()),
                DEMPath.toUtf8().data(),
                true, momentumFlag, papszOptions
            );
        }
    }
    writeToConsole(QString::number( numNinjas ) + " runs initialized. Starting solver...");

    maxProgress = numNinjas*100;
    progressDialog = new QProgressDialog("Solving...", "Cancel", 0, maxProgress, ui->centralwidget);
    progressDialog->setWindowModality(Qt::WindowModal);
    progressDialog->setMinimumDuration(0);
    progressDialog->setAutoClose(false);
    progressDialog->setAutoReset(false);

    progressDialog->setCancelButtonText("Cancel");
    connect( progressDialog, SIGNAL( canceled() ), this, SLOT( cancelSolve() ) );

    // initialize the progress values for the current set of runs
    totalProgress = 0;
    for(unsigned int i = 0; i < numNinjas; i++)
    {
        runProgress.push_back(0);
    }

    futureWatcher = new QFutureWatcher<int>(this);

    progressDialog->show();

    prepareArmy(ninjaArmy, numNinjas, initializationMethod);

    // set progress dialog initial value and initial text for the set of runs
    progressDialog->setValue(0);
    progressDialog->setLabelText("Running...");

    writeToConsole( "Initializing runs..." );

    connect(futureWatcher, &QFutureWatcher<int>::finished, this, &MainWindow::finishedSolve);

    QFuture<int> future = QtConcurrent::run(&MainWindow::startSolve, this, ui->numberOfProcessorsSpinBox->value());
    futureWatcher->setFuture(future);

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
        ui->domainAverageGroupBox->setChecked(!ui->domainAverageGroupBox->isChecked());
    }
    else if (item->text(0) == "Point Initialization")
    {
        ui->pointInitializationGroupBox->setChecked(!ui->pointInitializationGroupBox->isChecked());
    }
    else if (item->text(0) == "Weather Model")
    {
        ui->weatherModelGroupBox->setChecked(!ui->weatherModelGroupBox->isChecked());
    }
    else if (item->text(0) == "Surface Input")
    {
        ui->elevationInputFileOpenButton->click();
    }
    else if (item->text(0) == "Google Earth")
    {
        ui->googleEarthGroupBox->setChecked(!ui->googleEarthGroupBox->isChecked());
    }
    else if (item->text(0) == "Fire Behavior")
    {
        ui->fireBehaviorGroupBox->setChecked(!ui->fireBehaviorGroupBox->isChecked());
    }
    else if (item->text(0) == "Shape Files")
    {
        ui->shapeFilesGroupBox->setChecked(!ui->shapeFilesGroupBox->isChecked());
    }
    else if (item->text(0) == "Geospatial PDF Files")
    {
        ui->geospatialPDFFilesGroupBox->setChecked(!ui->geospatialPDFFilesGroupBox->isChecked());
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
        err = NinjaSetCommunication(ninjaArmy, i, "gui", papszOptions);
        if(err != NINJA_SUCCESS)
        {
            qDebug() << "NinjaSetCommunication: err =" << err;
        }
        /*
       * Sets Simulation Variables
       */
        if(ui->pointInitializationGroupBox->isChecked())
        {
            if(ui->pointInitializationWriteStationKMLCheckBox->isChecked())
            {
                err = NinjaSetStationKML(ninjaArmy, i, ui->elevationInputFileLineEdit->property("fullpath").toString().toUtf8().constData(), ui->outputDirectoryLineEdit->text().toUtf8().constData(), ui->outputSpeedUnitsComboBox->currentText().toUtf8().constData(), papszOptions);
                if(err != NINJA_SUCCESS)
                {
                    printf("NinjaSetStationKML: err = %d\n", err);
                }
            }
        }

        //connect( static_cast<ninjaGUIComHandler*>(NinjaGetCommunication( ninjaArmy, i, papszOptions )), &ninjaGUIComHandler::sendMessage, this, &MainWindow::writeToConsole );  // more exact way of doing it
        connect( NinjaGetCommunication( ninjaArmy, i, papszOptions ), SIGNAL( sendMessage(QString, QColor) ), this, SLOT( writeToConsole(QString, QColor) ) );  // other way of doing it

        //connect( static_cast<ninjaGUIComHandler*>(NinjaGetCommunication( ninjaArmy, i, papszOptions )), &ninjaGUIComHandler::sendMessage, this, &MainWindow::updateProgressMessage );
        connect( NinjaGetCommunication( ninjaArmy, i, papszOptions ), SIGNAL( sendMessage(QString, QColor) ), this, SLOT( updateProgressMessage( QString ) ) );

        //connect( static_cast<ninjaGUIComHandler*>(NinjaGetCommunication( ninjaArmy, i, papszOptions )), &ninjaGUIComHandler::sendProgress, this, &MainWindow::updateProgressValue );
        connect( NinjaGetCommunication( ninjaArmy, i, papszOptions ), SIGNAL( sendProgress( int, int ) ), this, SLOT( updateProgressValue( int, int ) ) );

//        // old code style method (see this in the old qt4 gui code)
//        connect( NinjaGetCommunication( ninjaArmy, i, papszOptions ), SIGNAL( sendMessage(QString, QColor) ), this, SLOT( writeToConsole(QString, QColor) ), Qt::AutoConnection );
//        connect( NinjaGetCommunication( ninjaArmy, i, papszOptions ), SIGNAL( sendMessage(QString, QColor) ), this, SLOT( updateProgressMessage( QString ) ), Qt::AutoConnection );
//        connect( NinjaGetCommunication( ninjaArmy, i, papszOptions ), SIGNAL( sendProgress( int, int ) ), this, SLOT( updateProgressValue( int, int ) ), Qt::AutoConnection );

//        // new code style method, chatgpt seems to prefer this one, though the AutoConnection seems to have slightly better results, well maybe
//        connect( NinjaGetCommunication( ninjaArmy, i, papszOptions ), SIGNAL( sendMessage(QString, QColor) ), this, SLOT( writeToConsole(QString, QColor) ), Qt::QueuedConnection );
//        connect( NinjaGetCommunication( ninjaArmy, i, papszOptions ), SIGNAL( sendMessage(QString, QColor) ), this, SLOT( updateProgressMessage( QString ) ), Qt::QueuedConnection );
//        connect( NinjaGetCommunication( ninjaArmy, i, papszOptions ), SIGNAL( sendProgress( int, int ) ), this, SLOT( updateProgressValue( int, int ) ), Qt::QueuedConnection );

        err = NinjaSetNumberCPUs(ninjaArmy, i, ui->numberOfProcessorsSpinBox->value(), papszOptions);
        if(err != NINJA_SUCCESS)
        {
            qDebug() << "NinjaSetNumberCPUs: err =" << err;
        }

        err = NinjaSetInitializationMethod(ninjaArmy, i, initializationMethod, ui->pointInitializationGroupBox->isChecked(), papszOptions);
        if(err != NINJA_SUCCESS)
        {
            qDebug() << "NinjaSetInitializationMethod: err =" << err;
        }

        err = NinjaSetDem(ninjaArmy, i, ui->elevationInputFileLineEdit->property("fullpath").toString().toUtf8().constData(), papszOptions);
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

    err = NinjaSetPDFDEM(ninjaArmy, i, ui->elevationInputFileLineEdit->property("fullpath").toString().toUtf8().constData(), papszOptions);
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

int MainWindow::startSolve(int numProcessors)
{
    try {

        char **papszOptions = nullptr; // found that I could pass this in as an argument after all, but makes more sense to just define it here
        
        //// calling prepareArmy here is causing all kinds of troubles. Local variables aren't properly being passed on,
        //// or aren't properly copied ([=] type thing), or aren't properly in scope. The other values are .h variables,
        //// so they would at least be in the proper scope. But the out of scope variables leads to all kinds
        //// of "QObject::connect: Cannot connect" and "err = 2" type messages. It is still somehow continuing to run though.
        ////
        //// seems the only way to put prepareArmy into a QFutureWatcher function, if it would even work,
        //// would be to have two separate QFutureWatcher functions, needs to be separated out from NinjaStartRuns()
        ////prepareArmy(ninjaArmy, ui->numberOfProcessorsSpinBox->value(), initializationMethod);

        return NinjaStartRuns(ninjaArmy, ui->numberOfProcessorsSpinBox->value(), papszOptions); // huh? I guess because "this" was used, it still has access to numNinjas this way
        //return NinjaStartRuns(ninjaArmy, numProcessors, papszOptions);

    } catch (cancelledByUser& e) {  // I forgot that the cancelSolve() works by doing a throw, I'm surprised that this throw is propagating out of the solver though

        qWarning() << "Solver error:" << e.what();

        // no message with this error, and it is a known error,
        // so probably better to update the message in the finished() function, than in QtConcurrent::run()
        //QMetaObject::invokeMethod(this, [this]() {
        //    progressDialog->setLabelText("Simulation cancelled by user");
        //    progressDialog->setCancelButtonText("Close");
        //    progressDialog->setValue(this->maxProgress);
        //    writeToConsole( "Simulation cancelled by user", Qt::yellow);
        //}, Qt::QueuedConnection);

        ////throw; // will propagate to the future. We purposefully want to skip passing it on for this case, use the QFutureWatcher->future()->result() value instead. However, the return/result value was 0, not the NINJA_E_CANCELLED value of 7. Hrm.
        return NINJA_E_CANCELLED;  // turns out NinjaStartRuns() simply didn't return a value because cancelSolve() runs by triggering a throw before a return value can be given. So just have to return the appropriate value here.

    } catch (const std::exception &e) { // Store error message somewhere (thread-safe)

        qWarning() << "Solver error:" << e.what();

        QString errorMsg = QString::fromStdString(e.what()); // copy out of 'e' before creating the thread safe invokeMethod lambda function
        QMetaObject::invokeMethod(this, [this, errorMsg]() {
            progressDialog->setLabelText("Simulation ended in error\n"+errorMsg);
            progressDialog->setCancelButtonText("Close");
            progressDialog->setValue(this->maxProgress);
            writeToConsole("Solver error: "+errorMsg, Qt::red);
        }, Qt::QueuedConnection);

        throw; // will propagate to the future

    } catch (...) {

        qWarning() << "unknown solver error";

        QMetaObject::invokeMethod(this, [this]() {
            progressDialog->setLabelText("Simulation ended with unknown error");
            progressDialog->setCancelButtonText("Close");
            progressDialog->setValue(this->maxProgress);
            writeToConsole("unknown solver error", Qt::red);
        }, Qt::QueuedConnection);

        throw; // will propagate to the future

    }
}

void MainWindow::finishedSolve()
{
    try {

        // get the return value of the QtConcurrent::run() function
        // Note that if an error was thrown during QtConcurrent::run(), this throws instead
        // but the thrown error comes out truncated, it loses the details of the original error message
        int result = futureWatcher->future().result();

        if( result == 1 ) // simulation properly finished
        {
            progressDialog->setValue(maxProgress);
            progressDialog->setLabelText("Simulations finished");
            progressDialog->setCancelButtonText("Close");

            qDebug() << "Finished with simulations";
            writeToConsole("Finished with simulations", Qt::darkGreen);
        }
        ////else if( futureWatcher->isCanceled() ) // this doesn't get triggered as reliably as the QProgressDialog cancel button
        //else if( result == NINJA_E_CANCELLED ) // this is probably the proper way to do this, but checking progressDialog->wasCanceled() seems way safer
        else if( progressDialog->wasCanceled() ) // simulation was cancelled
        {
            progressDialog->setValue(maxProgress);
            progressDialog->setLabelText("Simulation cancelled");
            progressDialog->setCancelButtonText("Close");
            //progressDialog->close();

            qDebug() << "Simulation cancelled by user";
            //writeToConsole( "Simulation cancelled by user", Qt::orange);  // orange isn't a predefined QColor
            //writeToConsole( "Simulation cancelled by user", Qt::QColor::fromRgb(255, 165, 0) );  // orange
            writeToConsole( "Simulation cancelled by user", Qt::yellow);
        }
        else // simulation ended in some known error
        {
            progressDialog->setValue(maxProgress);
            progressDialog->setLabelText("Simulation ended in error\nerror: "+QString::number(result));
            progressDialog->setCancelButtonText("Close");

            qWarning() << "Solver error:" << result;
            writeToConsole("Solver error: "+QString::number(result), Qt::red);
        }

    } catch (const std::exception &e) {

        // message got truncated, use the QtConcurrent::run() messaging
        // ooh, with the thread safe method, things are now updating appropriately
        //progressDialog->setValue(maxProgress);
        //progressDialog->setLabelText("Simulation ended in error\n"+QString(e.what()));
        //progressDialog->setCancelButtonText("Close");

        //qWarning() << "Solver error:" << e.what();
        //writeToConsole("Solver error: "+QString(e.what()), Qt::red);

    } catch (...) {

        // message got truncated, use the QtConcurrent::run() messaging
        //progressDialog->setValue(maxProgress);
        //progressDialog->setLabelText("Simulation ended with unknown error");
        //progressDialog->setCancelButtonText("Close");

        //qWarning() << "unknown solver error";
        //writeToConsole("unknown solver error", Qt::red);
    }

    disconnect(progressDialog, SIGNAL(canceled()), this, SLOT(cancelSolve()));

    char **papszOptions = nullptr;
    int err = NinjaDestroyArmy(ninjaArmy, papszOptions);
    if(err != NINJA_SUCCESS)
    {
        printf("NinjaDestroyRuns: err = %d\n", err);
    }

    // clear the progress values for the next set of runs
    runProgress.clear();

    futureWatcher->deleteLater();
}


