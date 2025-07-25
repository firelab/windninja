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
    ui->treeWidget->topLevelItem(1)->setData(0, Qt::UserRole, 4);
    ui->treeWidget->topLevelItem(2)->setData(0, Qt::UserRole, 12);
    ui->treeWidget->topLevelItem(3)->setData(0, Qt::UserRole, 18);

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

    // Sub-items for Outputs
    ui->treeWidget->topLevelItem(2)->child(0)->setData(0, Qt::UserRole, 13);  // Surface Input (Page 6)
    ui->treeWidget->topLevelItem(2)->child(1)->setData(0, Qt::UserRole, 14);  // Dirunal Input (Page 7)
    ui->treeWidget->topLevelItem(2)->child(2)->setData(0, Qt::UserRole, 15);  // Stability Input (Page 8)
    ui->treeWidget->topLevelItem(2)->child(3)->setData(0, Qt::UserRole, 16);  // Wind Input (Page 9)
    ui->treeWidget->topLevelItem(2)->child(4)->setData(0, Qt::UserRole, 17);  // Wind Input (Page 9)


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
    ui->elevationInputFileDownloadButton->setIcon(QIcon(":/server_go.png"));
    ui->elevationInputTypePushButton->setIcon(QIcon(":/swoop_final.png"));

    // Solver window
    int nCPUs = QThread::idealThreadCount();
    ui->availableProcessorsTextEdit->setPlainText("Available Processors:  " + QString::number(nCPUs));
    ui->numberOfProcessorsSpinBox->setMaximum(nCPUs);
    ui->numberOfProcessorsSpinBox->setValue(nCPUs);

    // Wind Input -> Point Init window
    ui->downloadPointInitData->setIcon(QIcon(":/application_get"));

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

    connectSignals();
}

MainWindow::~MainWindow()
{
  delete webView;
  delete channel;
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
    connect(ui->exitWindNinjaAction, &QAction::triggered, this, &QMainWindow::close);
    connect(mapBridge, &MapBridge::boundingBoxReceived, surfaceInput, &SurfaceInput::boundingBoxReceived);
    connect(surfaceInput, &SurfaceInput::requestRefresh, this, &MainWindow::refreshUI);
    connect(domainAverageInput, &DomainAverageInput::requestRefresh, this, &MainWindow::refreshUI);
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

// Function to populate weatherModelDataTreeView with .tif parent directories and all nested contents
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

    ui->weatherModelDataTreeView->setModel(model);
    ui->weatherModelDataTreeView->header()->setSectionResizeMode(QHeaderView::Stretch);

    // Disable editing and enable double-click expansion
    ui->weatherModelDataTreeView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->weatherModelDataTreeView->setExpandsOnDoubleClick(true);
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


void MainWindow::pointInitializationCheckBoxClicked()
{
    AppState& state = AppState::instance();
    state.isPointInitializationToggled = ui->pointInitializationCheckBox->isChecked();

    if (state.isPointInitializationToggled) {
        ui->domainAverageCheckBox->setChecked(false);
        ui->weatherModelCheckBox->setChecked(false);
        state.isDomainAverageInitializationToggled = ui->domainAverageCheckBox->isChecked();
        state.isWeatherModelInitializationToggled = ui->weatherModelCheckBox->isChecked();
    }

    refreshUI();
}

void MainWindow::useWeatherModelInitClicked()
{
    AppState& state = AppState::instance();

    state.isWeatherModelInitializationToggled = ui->weatherModelCheckBox->isChecked();

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
    // QString currentPath = ui->outputDirectoryLineEdit->toPlainText();
    // QString downloadsPath = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
    // QString dirPath = QFileDialog::getExistingDirectory(this, "Select a directory", currentPath, QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    // if (!dirPath.isEmpty()) {
    //     ui->outputDirectoryLineEdit->setText(dirPath);
    //     ui->outputDirectoryLineEdit->setToolTip(dirPath);
    // }
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

    vector<string> outputFiles;
    QDir outDir(ui->outputDirectoryLineEdit->text());
    QString demName = QFileInfo(ui->elevationInputFileLineEdit->text()).baseName();
    int meshInt = static_cast<int>(std::round(ui->meshResolutionSpinBox->value()));
    QString meshSize = QString::number(meshInt) + "m";

    for (int i = 0; i < numNinjas; i++) {
        QString filePath = outDir.filePath(QString("%1_%2_%3_%4.kmz")
                                               .arg(demName)
                                               .arg(directions[i])
                                               .arg(speeds[i])
                                               .arg(meshSize));
        outputFiles.push_back(filePath.toStdString());
    }

    for (const auto& dir : outputFiles) {
        QString qDir = QString::fromStdString(dir);

        QFile f(qDir);
        f.open(QIODevice::ReadOnly);
        QByteArray data = f.readAll();
        QString base64 = data.toBase64();

        webView->page()->runJavaScript("loadKmzFromBase64('"+base64+"')");
    }
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
        ui->pointInitializationCheckBox->click();
    } else if (item->text(0) == "Weather Model")
    {
        ui->weatherModelCheckBox->click();
    } else if (item->text(0) == "Surface Input")
    {
        surfaceInput->elevationInputFileOpenButtonClicked();
    }
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

        err = NinjaSetDem(ninjaArmy, i, surfaceInput->getDEMFilePath().toUtf8().constData(), papszOptions);
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

        err = NinjaSetOutputWindHeight(ninjaArmy, i, ui->inputWindHeightSpinBox->value(), "ft", papszOptions);
        if(err != NINJA_SUCCESS)
        {
            printf("NinjaSetOutputWindHeight: err = %d\n", err);
        }

        err = NinjaSetOutputSpeedUnits(ninjaArmy, i, ui->tableSpeedUnits->currentText().toUtf8().constData(), papszOptions);
        if(err != NINJA_SUCCESS)
        {
            printf("NinjaSetOutputSpeedUnits: err = %d\n", err);
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

        err = NinjaSetOutputPath(ninjaArmy, i, ui->outputDirectoryLineEdit->text().toUtf8().constData(), papszOptions);
        if(err != NINJA_SUCCESS)
        {
            printf("NinjaSetOutputPath: err = %d\n", err);
        }

        err = NinjaSetGoogOutFlag(ninjaArmy, i, true, papszOptions);
        if(err != NINJA_SUCCESS)
        {
            printf("NinjaSetGoogOutFlag: err = %d\n", err);
        }
    }
}


