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

#include "mainWindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    resize(1200, 700);
    ui->treeWidget->expandAll();

    AppState& state = AppState::instance();
    state.setUi(ui);
    ui->massSolverCheckBox->setChecked(true);
    ui->treeWidget->setMouseTracking(true);
    state.isMassSolverToggled = true;

    lineNumber = 1;

    serverBridge = new ServerBridge();
    serverBridge->checkMessages();

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
    outputs = new Outputs(ui, this);

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

    connectSignals();

    QString version(NINJA_VERSION_STRING);
    version = "Welcome to WindNinja " + version;

    writeToConsole(version, Qt::blue);

    writeToConsole("WINDNINJA_DATA=" + dataPath);

    state.setState();
}

MainWindow::~MainWindow()
{
    delete serverBridge;
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
    connect(ui->outputDirectoryButton, &QPushButton::clicked, this, &MainWindow::outputDirectoryButtonClicked);
    connect(ui->treeWidget, &QTreeWidget::itemClicked, this, &MainWindow::treeItemClicked);

    connect(menuBar, &MenuBar::writeToConsole, this, &MainWindow::writeToConsole);
    connect(mapBridge, &MapBridge::boundingBoxReceived, surfaceInput, &SurfaceInput::boundingBoxReceived);
    connect(surfaceInput, &SurfaceInput::updateTreeView, pointInitializationInput, &PointInitializationInput::updateTreeView);
    connect(surfaceInput, &SurfaceInput::updateTreeView, weatherModelInput, &WeatherModelInput::updateTreeView);
    connect(weatherModelInput, &WeatherModelInput::updateState, &AppState::instance(), &AppState::updateWeatherModelInputState);
    connect(this, &MainWindow::updateDirunalState, &AppState::instance(), &AppState::updateDiurnalInputState);
    connect(this, &MainWindow::updateStabilityState, &AppState::instance(), &AppState::updateStabilityInputState);
    connect(this, &MainWindow::updateMetholodyState, &AppState::instance(), &AppState::updateSolverMethodologyState);
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
    ninjaErr  = NinjaCancel(ninjaArmy, papszOptions);
    if( ninjaErr != NINJA_SUCCESS )
    {
        qDebug() << "NinjaCancel: ninjaErr =" << ninjaErr;
    }
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
    emit updateMetholodyState();
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
    emit updateMetholodyState();
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

    emit updateDirunalState();
}

void MainWindow::stabilityCheckBoxClicked()
{
    AppState& state = AppState::instance();
    state.isStabilityInputToggled = ui->stabilityCheckBox->isChecked();

    emit updateStabilityState();
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

                ninjaErr = NinjaGenerateSingleTimeObject(
                    startYear, startMonth, startDay, startHour, startMinute,
                    timeZoneBytes.constData(),
                    &endYear, &endMonth, &endDay, &endHour, &endMinute
                    );
                if(ninjaErr != NINJA_SUCCESS)
                {
                    qDebug() << "NinjaGenerateSingleTimeObject: ninjaErr = " << ninjaErr;
                }

                outYear[0] = endYear;
                outMonth[0] = endMonth;
                outDay[0] = endDay;
                outHour[0] = endHour;
                outMinute[0] = endMinute;
            }
            else {
                ninjaErr = NinjaGetTimeList(
                    year.data(), month.data(), day.data(),
                    hour.data(), minute.data(),
                    outYear.data(), outMonth.data(), outDay.data(),
                    outHour.data(), outMinute.data(),
                    nTimeSteps, timeZoneBytes.data()
                );
                if(ninjaErr != NINJA_SUCCESS)
                {
                    qDebug() << "NinjaGetTimeList: ninjaErr = " << ninjaErr;
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

            ninjaErr = NinjaGenerateSingleTimeObject(
                year, month, day, hour, minute,
                timeZoneBytes.constData(),
                &outYear, &outMonth, &outDay, &outHour, &outMinute
                );
            if (ninjaErr != NINJA_SUCCESS)
            {
                qDebug() << "NinjaGenerateSingleTimeObject: ninjaErr = " << ninjaErr;
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
    else
    {
        QModelIndexList selectedIndexes = ui->weatherModelTimeTreeView->selectionModel()->selectedIndexes();
        int timeListSize = selectedIndexes.count();
        numNinjas = timeListSize;
        initializationMethod = "wxmodel";
        std::string timeZone = ui->timeZoneComboBox->currentText().toStdString();

        QModelIndex index = ui->weatherModelFileTreeView->currentIndex();
        QFileSystemModel *model = qobject_cast<QFileSystemModel *>(ui->weatherModelFileTreeView->model());
        std::string filePath = model->filePath(index).toStdString();

        // Allocate the char** array
        const char **inputTimeList = new const char*[timeListSize];

        for (int i = 0; i < timeListSize; ++i)
        {
            QString qstr = selectedIndexes[i].data().toString();
            std::string str = qstr.toStdString();
            inputTimeList[i] = strdup(str.c_str()); // allocate and copy each string
        }

        ninjaArmy = NinjaMakeWeatherModelArmy(filePath.c_str(), timeZone.c_str(), inputTimeList, timeListSize, ui->momentumSolverCheckBox->isChecked(), papszOptions);
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
    ninjaErr = NinjaSetAsciiAtmFile(ninjaArmy, ui->fireBehaviorResolutionCheckBox->isChecked(), papszOptions);
    if(ninjaErr != NINJA_SUCCESS)
    {
        qDebug() << "NinjaSetAsciiAtmFile: ninjaErr =" << ninjaErr;
    }

    for(unsigned int i=0; i<numNinjas; i++)
    {
        ninjaErr = NinjaSetCommunication(ninjaArmy, i, "gui", papszOptions);
        if(ninjaErr != NINJA_SUCCESS)
        {
            qDebug() << "NinjaSetCommunication: ninjaErr =" << ninjaErr;
        }
        /*
       * Sets Simulation Variables
       */
        if(ui->pointInitializationGroupBox->isChecked())
        {
            if(ui->pointInitializationWriteStationKMLCheckBox->isChecked())
            {
                ninjaErr = NinjaSetStationKML(ninjaArmy, i, ui->elevationInputFileLineEdit->property("fullpath").toString().toUtf8().constData(), ui->outputDirectoryLineEdit->text().toUtf8().constData(), ui->outputSpeedUnitsComboBox->currentText().toUtf8().constData(), papszOptions);
                if(ninjaErr != NINJA_SUCCESS)
                {
                    printf("NinjaSetStationKML: ninjaErr = %d\n", ninjaErr);
                }
            }
        }

        //connect( static_cast<ninjaGUIComHandler*>(NinjaGetCommunication( ninjaArmy, i, papszOptions )), &ninjaGUIComHandler::sendMessage, this, &MainWindow::writeToConsole );  // more exact way of doing it
        connect(NinjaGetCommunication(ninjaArmy, i, papszOptions), SIGNAL(sendMessage(QString,QColor)), this, SLOT(writeToConsole(QString,QColor)));  // other way of doing it

        //connect( static_cast<ninjaGUIComHandler*>(NinjaGetCommunication( ninjaArmy, i, papszOptions )), &ninjaGUIComHandler::sendMessage, this, &MainWindow::updateProgressMessage );
        connect(NinjaGetCommunication(ninjaArmy, i, papszOptions), SIGNAL(sendMessage(QString,QColor)), this, SLOT(updateProgressMessage(QString)));

        //connect( static_cast<ninjaGUIComHandler*>(NinjaGetCommunication( ninjaArmy, i, papszOptions )), &ninjaGUIComHandler::sendProgress, this, &MainWindow::updateProgressValue );
        connect(NinjaGetCommunication(ninjaArmy, i, papszOptions), SIGNAL(sendProgress(int,int)), this, SLOT(updateProgressValue(int,int)));

//        // old code style method (see this in the old qt4 gui code)
//        connect( NinjaGetCommunication( ninjaArmy, i, papszOptions ), SIGNAL( sendMessage(QString, QColor) ), this, SLOT( writeToConsole(QString, QColor) ), Qt::AutoConnection );
//        connect( NinjaGetCommunication( ninjaArmy, i, papszOptions ), SIGNAL( sendMessage(QString, QColor) ), this, SLOT( updateProgressMessage( QString ) ), Qt::AutoConnection );
//        connect( NinjaGetCommunication( ninjaArmy, i, papszOptions ), SIGNAL( sendProgress( int, int ) ), this, SLOT( updateProgressValue( int, int ) ), Qt::AutoConnection );

//        // new code style method, chatgpt seems to prefer this one, though the AutoConnection seems to have slightly better results, well maybe
//        connect( NinjaGetCommunication( ninjaArmy, i, papszOptions ), SIGNAL( sendMessage(QString, QColor) ), this, SLOT( writeToConsole(QString, QColor) ), Qt::QueuedConnection );
//        connect( NinjaGetCommunication( ninjaArmy, i, papszOptions ), SIGNAL( sendMessage(QString, QColor) ), this, SLOT( updateProgressMessage( QString ) ), Qt::QueuedConnection );
//        connect( NinjaGetCommunication( ninjaArmy, i, papszOptions ), SIGNAL( sendProgress( int, int ) ), this, SLOT( updateProgressValue( int, int ) ), Qt::QueuedConnection );

        ninjaErr = NinjaSetNumberCPUs(ninjaArmy, i, ui->numberOfProcessorsSpinBox->value(), papszOptions);
        if(ninjaErr != NINJA_SUCCESS)
        {
            qDebug() << "NinjaSetNumberCPUs: ninjaErr =" << ninjaErr;
        }

        ninjaErr = NinjaSetInitializationMethod(ninjaArmy, i, initializationMethod, ui->pointInitializationGroupBox->isChecked(), papszOptions);
        if(ninjaErr != NINJA_SUCCESS)
        {
            qDebug() << "NinjaSetInitializationMethod: ninjaErr =" << ninjaErr;
        }

        ninjaErr = NinjaSetDem(ninjaArmy, i, ui->elevationInputFileLineEdit->property("fullpath").toString().toUtf8().constData(), papszOptions);
        if(ninjaErr != NINJA_SUCCESS)
        {
            qDebug() << "NinjaSetDem: ninjaErr =" << ninjaErr;
        }

        ninjaErr = NinjaSetPosition(ninjaArmy, i, papszOptions);
        if(ninjaErr != NINJA_SUCCESS)
        {
            qDebug() << "NinjaSetPosition: ninjaErr =" << ninjaErr;
        }

        ninjaErr = NinjaSetInputWindHeight(ninjaArmy, i, ui->inputWindHeightSpinBox->value(), "m", papszOptions);
        if(ninjaErr != NINJA_SUCCESS)
        {
            qDebug() << "NinjaSetInputWindHeight: ninjaErr =" << ninjaErr;
        }

        ninjaErr = NinjaSetDiurnalWinds(ninjaArmy, i, ui->diurnalCheckBox->isChecked(), papszOptions);
        if(ninjaErr != NINJA_SUCCESS)
        {
            qDebug() << "NinjaSetDiurnalWinds: ninjaErr =" << ninjaErr;
        }

        ninjaErr = NinjaSetUniVegetation(ninjaArmy, i, ui->vegetationComboBox->currentText().toLower().toUtf8().constData(), papszOptions);
        if(ninjaErr != NINJA_SUCCESS)
        {
            qDebug() << "NinjaSetUniVegetation: ninjaErr =" << ninjaErr;
        }

        ninjaErr = NinjaSetMeshResolutionChoice(ninjaArmy, i, ui->meshResolutionComboBox->currentText().toLower().toUtf8().constData(), papszOptions);
        if(ninjaErr != NINJA_SUCCESS)
        {
            qDebug() << "NinjaSetMeshResolutionChoice: ninjaErr =" << ninjaErr;
        }

        ninjaErr = NinjaSetNumVertLayers(ninjaArmy, i, 20, papszOptions);
        if(ninjaErr != NINJA_SUCCESS)
        {
            qDebug() << "NinjaSetNumVertLayers: ninjaErr =" << ninjaErr;
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
    int ninjaErr;

    ninjaErr = NinjaSetOutputPath(ninjaArmy, i, ui->outputDirectoryLineEdit->text().toUtf8().constData(), papszOptions);
    if (ninjaErr != NINJA_SUCCESS)
    {
        qDebug() << "NinjaSetOutputPath: ninjaErr =" << ninjaErr;
    }

    ninjaErr = NinjaSetOutputWindHeight(ninjaArmy, i, ui->outputWindHeightSpinBox->value(), ui->outputWindHeightUnitsComboBox->itemData(ui->outputWindHeightUnitsComboBox->currentIndex()).toString().toUtf8().constData(), papszOptions);
    if (ninjaErr != NINJA_SUCCESS)
    {
        qDebug() << "NinjaSetOutputWindHeight: ninjaErr =" << ninjaErr;
    }

    ninjaErr = NinjaSetOutputSpeedUnits(ninjaArmy, i, ui->outputSpeedUnitsComboBox->currentText().toUtf8().constData(), papszOptions);
    if (ninjaErr != NINJA_SUCCESS)
    {
        qDebug() << "NinjaSetOutputSpeedUnits: ninjaErr =" << ninjaErr;
    }

    ninjaErr = NinjaSetGoogOutFlag(ninjaArmy, i, ui->googleEarthGroupBox->isChecked(), papszOptions);
    if (ninjaErr != NINJA_SUCCESS)
    {
        qDebug() << "NinjaSetGoogOutFlag: ninjaErr =" << ninjaErr;
    }

    ninjaErr = NinjaSetGoogResolution(ninjaArmy, i, googleEarth.resolution, googleEarth.units.constData(), papszOptions);
    if (ninjaErr != NINJA_SUCCESS)
    {
        qDebug() << "NinjaSetGoogResolution: ninjaErr =" << ninjaErr;
    }

    ninjaErr = NinjaSetGoogSpeedScaling(ninjaArmy, i, ui->legendComboBox->itemData(ui->legendComboBox->currentIndex()).toString().toUtf8().constData(), papszOptions);
    if (ninjaErr != NINJA_SUCCESS)
    {
        qDebug() << "NinjaSetGoogSpeedScaling: ninjaErr =" << ninjaErr;
    }

    ninjaErr = NinjaSetGoogLineWidth(ninjaArmy, i, ui->googleEarthVectorsSpinBox->value(), papszOptions);
    if (ninjaErr != NINJA_SUCCESS)
    {
        qDebug() << "NinjaSetGoogLineWidth: ninjaErr =" << ninjaErr;
    }

    ninjaErr = NinjaSetGoogColor(ninjaArmy, i, ui->alternativeColorSchemeComboBox->itemData(ui->alternativeColorSchemeComboBox->currentIndex()).toString().toUtf8().constData(), ui->googleEarthVectorScalingCheckBox->isChecked(), papszOptions);
    if (ninjaErr != NINJA_SUCCESS)
    {
        qDebug() << "NinjaSetGoogColor: ninjaErr =" << ninjaErr;
    }

    ninjaErr = NinjaSetGoogConsistentColorScale(ninjaArmy, i, ui->legendCheckBox->isChecked(), numNinjas, papszOptions);
    if (ninjaErr != NINJA_SUCCESS)
    {
        qDebug() << "NinjaSetGoogConsistentColorScale: ninjaErr =" << ninjaErr;
    }

    ninjaErr = NinjaSetAsciiOutFlag(ninjaArmy, i, ui->fireBehaviorGroupBox->isChecked(), papszOptions);
    if (ninjaErr != NINJA_SUCCESS)
    {
        qDebug() << "NinjaSetAsciiOutFlag: ninjaErr =" << ninjaErr;
    }

    ninjaErr = NinjaSetAsciiResolution(ninjaArmy, i, fireBehavior.resolution, fireBehavior.units.constData(), papszOptions);
    if (ninjaErr != NINJA_SUCCESS)
    {
        qDebug() << "NinjaSetAsciiResolution: ninjaErr =" << ninjaErr;
    }

    ninjaErr = NinjaSetShpOutFlag(ninjaArmy, i, ui->shapeFilesGroupBox->isChecked(), papszOptions);
    if (ninjaErr != NINJA_SUCCESS)
    {
        qDebug() << "NinjaSetShpOutFlag: ninjaErr =" << ninjaErr;
    }

    ninjaErr = NinjaSetShpResolution(ninjaArmy, i, shapeFiles.resolution, shapeFiles.units.constData(), papszOptions);
    if (ninjaErr != NINJA_SUCCESS)
    {
        qDebug() << "NinjaSetShpResolution: ninjaErr =" << ninjaErr;
    }

    ninjaErr = NinjaSetPDFOutFlag(ninjaArmy, i, ui->geospatialPDFFilesGroupBox->isChecked(), papszOptions);
    if (ninjaErr != NINJA_SUCCESS)
    {
        qDebug() << "NinjaSetPDFOutFlag: ninjaErr =" << ninjaErr;
    }

    ninjaErr = NinjaSetPDFLineWidth(ninjaArmy, i, ui->geospatialPDFFilesVectorsSpinBox->value(), papszOptions);
    if (ninjaErr != NINJA_SUCCESS)
    {
        qDebug() << "NinjaSetPDFLineWidth: ninjaErr =" << ninjaErr;
    }

    ninjaErr = NinjaSetPDFBaseMap(ninjaArmy, i, ui->basemapComboBox->currentIndex(), papszOptions);
    if (ninjaErr != NINJA_SUCCESS)
    {
        qDebug() << "NinjaSetPDFBaseMap: ninjaErr =" << ninjaErr;
    }

    ninjaErr = NinjaSetPDFDEM(ninjaArmy, i, ui->elevationInputFileLineEdit->property("fullpath").toString().toUtf8().constData(), papszOptions);
    if (ninjaErr != NINJA_SUCCESS)
    {
        qDebug() << "NinjaSetPDFDEM: ninjaErr =" << ninjaErr;
    }

    ninjaErr = NinjaSetPDFSize(ninjaArmy, i, PDFSize.PDFHeight, PDFSize.PDFWidth, PDFSize.PDFDpi, papszOptions);
    if (ninjaErr != NINJA_SUCCESS)
    {
        qDebug() << "NinjaSetPDFSize: ninjaErr =" << ninjaErr;
    }

    ninjaErr = NinjaSetPDFResolution(ninjaArmy, i, geospatialPDFs.resolution, geospatialPDFs.units.constData(), papszOptions);
    if (ninjaErr != NINJA_SUCCESS)
    {
        qDebug() << "NinjaSetPDFResolution: ninjaErr =" << ninjaErr;
    }

    ninjaErr = NinjaSetVtkOutFlag(ninjaArmy, i, ui->VTKFilesCheckBox->isChecked(), papszOptions);
    if (ninjaErr != NINJA_SUCCESS)
    {
        qDebug() << "NinjaSetVtkOutFlag: ninjaErr =" << ninjaErr;
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
        //// of "QObject::connect: Cannot connect" and "ninjaErr = 2" type messages. It is still somehow continuing to run though.
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
    int ninjaErr = NinjaDestroyArmy(ninjaArmy, papszOptions);
    if(ninjaErr != NINJA_SUCCESS)
    {
        printf("NinjaDestroyRuns: ninjaErr = %d\n", ninjaErr);
    }

    // clear the progress values for the next set of runs
    runProgress.clear();

    futureWatcher->deleteLater();
}


