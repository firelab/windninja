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
    webEngineView = new QWebEngineView(ui->mapPanelWidget);
    webChannel = new QWebChannel(webEngineView->page());
    mapBridge = new MapBridge(this);
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
    connect(ui->massSolverCheckBox, &QCheckBox::clicked, this, &MainWindow::massSolverCheckBoxClicked);
    connect(ui->momentumSolverCheckBox, &QCheckBox::clicked, this, &MainWindow::momentumSolverCheckBoxClicked);
    connect(ui->diurnalCheckBox, &QCheckBox::clicked, this, &MainWindow::diurnalCheckBoxClicked);
    connect(ui->stabilityCheckBox, &QCheckBox::clicked, this, &MainWindow::stabilityCheckBoxClicked);
    connect(ui->treeWidget, &QTreeWidget::itemDoubleClicked, this, &MainWindow::treeWidgetItemDoubleClicked);
    connect(ui->numberOfProcessorsSolveButton, &QPushButton::clicked, this, &MainWindow::solveButtonClicked);
    connect(ui->outputDirectoryButton, &QPushButton::clicked, this, &MainWindow::outputDirectoryButtonClicked);
    connect(ui->treeWidget, &QTreeWidget::itemClicked, this, &MainWindow::treeItemClicked);

    connect(menuBar, &MenuBar::writeToConsole, this, &MainWindow::writeToConsole);
    connect(mapBridge, &MapBridge::boundingBoxReceived, surfaceInput, &SurfaceInput::boundingBoxReceived);
    connect(surfaceInput, &SurfaceInput::updateTreeView, pointInitializationInput, &PointInitializationInput::updateTreeView);
    connect(surfaceInput, &SurfaceInput::updateTreeView, weatherModelInput, &WeatherModelInput::updateTreeView);
    connect(weatherModelInput, &WeatherModelInput::updateState, &AppState::instance(), &AppState::updateWeatherModelInputState);
    connect(webEngineView, &QWebEngineView::loadFinished, this, &MainWindow::readSettings);

    connect(this, &MainWindow::updateDirunalState, &AppState::instance(), &AppState::updateDiurnalInputState);
    connect(this, &MainWindow::updateStabilityState, &AppState::instance(), &AppState::updateStabilityInputState);
    connect(this, &MainWindow::updateMetholodyState, &AppState::instance(), &AppState::updateSolverMethodologyState);
    connect(this, &MainWindow::updateProgressValueSignal, this, &MainWindow::updateProgressValue, Qt::QueuedConnection);
    connect(this, &MainWindow::updateProgressMessageSignal, this, &MainWindow::updateProgressMessage, Qt::QueuedConnection);
    connect(this, &MainWindow::writeToConsoleSignal, this, &MainWindow::writeToConsole, Qt::QueuedConnection);

    connect(surfaceInput, &SurfaceInput::writeToConsoleSignal, this, &MainWindow::writeToConsole, Qt::QueuedConnection);
    connect(weatherModelInput, &WeatherModelInput::writeToConsoleSignal, this, &MainWindow::writeToConsole, Qt::QueuedConnection);
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

static void comMessageHandler(const char *pszMessage, void *pUser)
{
    MainWindow *self = static_cast<MainWindow*>(pUser);

    std::string msg = pszMessage;
    if( msg.substr(msg.size()-1, 1) == "\n")
    {
        msg = msg.substr(0, msg.size()-1);
    }

    int runNumber = -1;
    sscanf(msg.c_str(), "Run %d", &runNumber);

    size_t pos;
    size_t startPos;
    size_t endPos;
    std::string clipStr;

    int runProgress;
    endPos = msg.find("% complete");
    if( endPos != msg.npos )
    {
        clipStr = msg.substr(0, endPos);
        //std::cout << "clipStr = \"" << clipStr << "\"" << std::endl;
        pos = clipStr.rfind(": ");
        startPos = pos+2;
        clipStr = clipStr.substr(startPos);
        //std::cout << "clipStr = \"" << clipStr << "\"" << std::endl;
        runProgress = atoi(clipStr.c_str());

        emit self->updateProgressValueSignal(runNumber, runProgress);
    }

//"Run 1 ERROR: Multiple runs were requested with the same input parameters."
//"Run 0 ERROR: Exception caught: I WANT CHOCOLATE!!! Yum."
//"Run 0: Exception caught: Simulation was cancelled by the user."
//"Run 1: Exception canceled by user caught: Simulation was cancelled by the user."

    if( msg.find("Exception caught: ") != msg.npos || msg.find("ERROR: ") != msg.npos || msg.find("Exception canceled by user caught: ") != msg.npos )
    {
        if( msg.find("Exception caught: ") != msg.npos )
        {
            pos = msg.find("Exception caught: ");
            startPos = pos+18;
        }
        else if( msg.find("Exception canceled by user caught: ") != msg.npos )
        {
            pos = msg.find("Exception canceled by user caught: ");
            startPos = pos+35;
        }
        else // if( msg.find("ERROR: ") != msg.npos )
        {
            pos = msg.find("ERROR: ");
            startPos = pos+7;
        }
        clipStr = msg.substr(startPos);
        //std::cout << "clipStr = \"" << clipStr << "\"" << std::endl;
        //emit self->updateProgressMessageSignal(QString::fromStdString(clipStr));
        //emit self->writeToConsoleSignal(QString::fromStdString(clipStr));
        if( clipStr == "Simulation was cancelled by the user." )
        {
            emit self->updateProgressMessageSignal(QString::fromStdString("Simulation cancelled"));
            emit self->writeToConsoleSignal(QString::fromStdString("Simulation cancelled by user"), Qt::yellow);
        }
        else if( clipStr == "Cannot determine exception type." )
        {
            emit self->updateProgressMessageSignal(QString::fromStdString("Simulation ended with unknown error"));
            emit self->writeToConsoleSignal(QString::fromStdString("unknown solver error"), Qt::red);
        }
        else
        {
            emit self->updateProgressMessageSignal(QString::fromStdString("Simulation ended in error:\n"+clipStr));
            emit self->writeToConsoleSignal(QString::fromStdString("Solver error: "+clipStr), Qt::red);
        }
    }
    else if( msg.find("Warning: ") != msg.npos )
    {
        if( msg.find("Warning: ") != msg.npos )
        {
            pos = msg.find("Warning: ");
            startPos = pos+9;
        }
        clipStr = msg.substr(startPos);
        //std::cout << "clipStr = \"" << clipStr << "\"" << std::endl;
        //emit self->updateProgressMessageSignal(QString::fromStdString(clipStr));
        //emit self->writeToConsoleSignal(QString::fromStdString(clipStr));
        emit self->updateProgressMessageSignal(QString::fromStdString("Solver ended in warning:\n"+clipStr));
        emit self->writeToConsoleSignal(QString::fromStdString("Solver warning: "+clipStr), Qt::yellow);
    }
    else
    {
        emit self->updateProgressMessageSignal(QString::fromStdString(msg));
        emit self->writeToConsoleSignal(QString::fromStdString(msg));
    }
}

void MainWindow::cancelSolve()
{
    progressDialog->setLabelText("Canceling...");
    //qDebug() << "Canceling...";
    //writeToConsole( "Canceling...", Qt::yellow);

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
        surfaceInput->updateMeshResolutionByUnits();
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
        surfaceInput->updateMeshResolutionByUnits();
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

void MainWindow::solveButtonClicked()
{
    AppState& state = AppState::instance();

    maxProgress = 100;
    //progressDialog = new QProgressDialog("Initializing Runs...", "Cancel", 0, maxProgress, ui->centralwidget);
    progressDialog = new QProgressDialog(ui->centralwidget);
    progressDialog->setRange(0, maxProgress);
    progressDialog->setValue(0);
    progressDialog->setLabelText("Initializing Runs...");
    progressDialog->setCancelButtonText("Cancel");

    progressDialog->setWindowModality(Qt::WindowModal);
    progressDialog->setMinimumDuration(0);
    progressDialog->setAutoClose(false);
    progressDialog->setAutoReset(false);

    int numNinjas = 0;
    ninjaArmy = nullptr;
    char **papszOptions = nullptr;
    const char *initializationMethod = nullptr;

    ninjaArmy = NinjaInitializeArmy();

    ninjaErr = NinjaSetArmyComMessageHandler(ninjaArmy, &comMessageHandler, this, papszOptions);
    if(ninjaErr != NINJA_SUCCESS)
    {
        qDebug() << "NinjaSetArmyComMessageHandler(): ninjaErr =" << ninjaErr;
    }

    if (state.isDomainAverageInitializationValid)
    {
        initializationMethod = "domain_average";
        QList<double> speeds;
        QList<double> directions;
//        QList<int> years;
//        QList<int> months;
//        QList<int> days;
//        QList<int> hours;
//        QList<int> minutes;
//        QList<double> airTemps;
//        QList<double> cloudCovers;

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
//        QString DEMTimeZone = ui->timeZoneComboBox->currentText();
//        QString airTempUnits =  ui->tableAirTempUnits->currentText();
//        QString cloudCoverUnits =  ui->tableCloudCoverUnits->currentText();

        QDateTime currentDateTimeUtc = QDateTime::currentDateTimeUtc();
        QDate currentDateUtc = currentDateTimeUtc.date();
        QTime currentTimeUtc = currentDateTimeUtc.time();
        int currentYearUtc = currentDateUtc.year();
        int currentMonthUtc = currentDateUtc.month();
        int currentDayUtc = currentDateUtc.day();
        int currentHoursUtc = currentTimeUtc.hour();
        int currentMinutesUtc = currentTimeUtc.minute();

        QList<int> years(numNinjas, currentYearUtc);
        QList<int> months(numNinjas, currentMonthUtc);
        QList<int> days(numNinjas, currentDayUtc);
        QList<int> hours(numNinjas, currentHoursUtc);
        QList<int> minutes(numNinjas, 0);
        const char * timeZoneUtc = "UTC";
        QList<double> airTemps(numNinjas, 72.0);
        const char * airTempUnits = "F";
        QList<double> cloudCovers(numNinjas, 15.0);
        const char * cloudCoverUnits = "percent";
        for(size_t ninjaIdx = 1; ninjaIdx < numNinjas; ninjaIdx++)
        {
            minutes[ninjaIdx] = minutes[ninjaIdx] + 1;
            // better not get more than 60 ninjas during our simple tests, cause this would break down really fast ...
            //if( minutes[ninjaIdx] > 59 )
            //{
            //    minutes[ninjaIdx] = minutes[ninjaIdx] - 60;
            //    hours[ninjaIdx] = hours[ninjaIdx] + 1;
            //    if( hours[ninjaIdx] > 23 )
            //    {
            //        hours[ninjaIdx] = hours[ninjaIdx] - 24;
            //    }
            //}
        }

//        ninjaErr = NinjaMakeDomainAverageArmy(ninjaArmy, numNinjas, momentumFlag, speeds.data(), speedUnits.toUtf8().constData(), directions.data(), years.data(), months.data(), days.data(), hours.data(), minutes.data(), DEMTimeZone.toUtf8().data(), airTemps.data(), airTempUnits.toUtf8().constData(), cloudCovers.data(), cloudCoverUnits.toUtf8().constData(), papszOptions);
        ninjaErr = NinjaMakeDomainAverageArmy(ninjaArmy, numNinjas, momentumFlag, speeds.data(), speedUnits.toUtf8().constData(), directions.data(), years.data(), months.data(), days.data(), hours.data(), minutes.data(), timeZoneUtc, airTemps.data(), airTempUnits, cloudCovers.data(), cloudCoverUnits, papszOptions);
        if(ninjaErr != NINJA_SUCCESS)
        {
            qDebug() << "NinjaMakeDomainAverageArmy: ninjaErr =" << ninjaErr;
        }
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
                    qDebug() << "NinjaGenerateSingleTimeObject: ninjaErr =" << ninjaErr;
                }

                outYear[0] = endYear;
                outMonth[0] = endMonth;
                outDay[0] = endDay;
                outHour[0] = endHour;
                outMinute[0] = endMinute;
            }
            else
            {
                ninjaErr = NinjaGetTimeList(
                    year.data(), month.data(), day.data(),
                    hour.data(), minute.data(),
                    outYear.data(), outMonth.data(), outDay.data(),
                    outHour.data(), outMinute.data(),
                    nTimeSteps, timeZoneBytes.data()
                );
                if(ninjaErr != NINJA_SUCCESS)
                {
                    qDebug() << "NinjaGetTimeList: ninjaErr =" << ninjaErr;
                }
            }

            if(ninjaErr == NINJA_SUCCESS)
            {
                numNinjas = ui->weatherStationDataTimestepsSpinBox->value();

                ninjaErr = NinjaMakePointArmy( ninjaArmy,
                    outYear.data(), outMonth.data(), outDay.data(),
                    outHour.data(), outMinute.data(), nTimeSteps,
                    DEMTimeZone.toUtf8().data(), stationFileNames.data(),
                    stationFileNames.size(), DEMPath.toUtf8().data(),
                    true, momentumFlag, papszOptions
                    );
                if(ninjaErr != NINJA_SUCCESS)
                {
                    qDebug() << "NinjaMakePointArmy: ninjaErr =" << ninjaErr;
                }
            }
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
                qDebug() << "NinjaGenerateSingleTimeObject: ninjaErr =" << ninjaErr;
            }

            QVector<int> yearVec   = { outYear };
            QVector<int> monthVec  = { outMonth };
            QVector<int> dayVec    = { outDay };
            QVector<int> hourVec   = { outHour };
            QVector<int> minuteVec = { outMinute };

            numNinjas = 1;
            int nTimeSteps = 1;

            if(ninjaErr == NINJA_SUCCESS)
            {
                ninjaErr = NinjaMakePointArmy( ninjaArmy,
                    yearVec.data(), monthVec.data(), dayVec.data(),
                    hourVec.data(), minuteVec.data(), nTimeSteps,
                    DEMTimeZone.toUtf8().data(),
                    stationFileNames.data(),
                    static_cast<int>(stationFileNames.size()),
                    DEMPath.toUtf8().data(),
                    true, momentumFlag, papszOptions
                );
                if(ninjaErr != NINJA_SUCCESS)
                {
                    qDebug() << "NinjaMakePointArmy =" << ninjaErr;
                }
            }
        }
    }
    else //if (state.isWeatherModelInitializationValid)
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

        ninjaErr = NinjaMakeWeatherModelArmy(ninjaArmy, filePath.c_str(), timeZone.c_str(), inputTimeList, timeListSize, ui->momentumSolverCheckBox->isChecked(), papszOptions);
        if(ninjaErr != NINJA_SUCCESS)
        {
            qDebug() << "NinjaMakeWeatherModelArmy =" << ninjaErr;
        }
    }

    if(ninjaErr != NINJA_SUCCESS)
    {
        progressDialog->setValue(maxProgress);
        progressDialog->setCancelButtonText("Close");

        // do cleanup before the return, similar to finishedSolve()

        //disconnect(progressDialog, SIGNAL(canceled()), this, SLOT(cancelSolve()));

        char **papszOptions = nullptr;
        int ninjaErr = NinjaDestroyArmy(ninjaArmy, papszOptions);
        if(ninjaErr != NINJA_SUCCESS)
        {
            printf("NinjaDestroyRuns: ninjaErr = %d\n", ninjaErr);
        }

        // clear the progress values for the next set of runs
        //runProgress.clear();

        //futureWatcher->deleteLater();

        return;
    }

    writeToConsole(QString::number( numNinjas ) + " runs initialized. Starting solver...");

    maxProgress = numNinjas*100;

    progressDialog->setRange(0, maxProgress);
    progressDialog->setValue(0);
    progressDialog->setLabelText("Solving...");

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

    bool retVal = prepareArmy(ninjaArmy, numNinjas, initializationMethod);
    if( retVal == false )
    {
        progressDialog->setValue(maxProgress);
        progressDialog->setCancelButtonText("Close");

        // do cleanup before the return, similar to finishedSolve()

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

        return;
    }

    // set progress dialog initial value and initial text for the set of runs
    progressDialog->setValue(0);
    progressDialog->setLabelText("Running...");

    qDebug() << "Initializing runs...";
    writeToConsole( "Initializing runs..." );

    connect(futureWatcher, &QFutureWatcher<int>::finished, this, &MainWindow::finishedSolve);

    QFuture<int> future = QtConcurrent::run(&MainWindow::startSolve, this, ui->numberOfProcessorsSpinBox->value());
    futureWatcher->setFuture(future);

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

bool MainWindow::prepareArmy(NinjaArmyH *ninjaArmy, int numNinjas, const char* initializationMethod)
{
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

    // can this one even be tested?? The way it is organized also makes it tough to setup a ninjaCom message
    ninjaErr = NinjaSetAsciiAtmFile(ninjaArmy, ui->fireBehaviorResolutionCheckBox->isChecked(), papszOptions);
    if(ninjaErr != NINJA_SUCCESS)
    {
        qDebug() << "NinjaSetAsciiAtmFile: ninjaErr =" << ninjaErr;
        return false;
    }

    for(unsigned int i=0; i<numNinjas; i++)
    {
        /*
       * Sets Simulation Variables
       */
        if(ui->pointInitializationGroupBox->isChecked())
        {
            if(ui->pointInitializationWriteStationKMLCheckBox->isChecked())
            {
                // function needs MAJOR rework to get the testing to work, direct call to non-ninjaArmy function makes this process tougher
                ninjaErr = NinjaSetStationKML(ninjaArmy, i, ui->elevationInputFileLineEdit->property("fullpath").toString().toUtf8().constData(), ui->outputDirectoryLineEdit->text().toUtf8().constData(), ui->outputSpeedUnitsComboBox->currentText().toUtf8().constData(), papszOptions);
                //ninjaErr = NinjaSetStationKML(ninjaArmy, i+10, ui->elevationInputFileLineEdit->property("fullpath").toString().toUtf8().constData(), ui->outputDirectoryLineEdit->text().toUtf8().constData(), ui->outputSpeedUnitsComboBox->currentText().toUtf8().constData(), papszOptions);  // test error handling  // function needs reorganized to handle this test
                //ninjaErr = NinjaSetStationKML(ninjaArmy, i, ui->elevationInputFileLineEdit->property("fullpath").toString().toUtf8().constData(), ui->outputDirectoryLineEdit->text().toUtf8().constData(), "fudge", papszOptions);  // test error handling  // ran, but the functions need reorganized for proper messaging
                //ninjaErr = NinjaSetStationKML(ninjaArmy, i, ui->elevationInputFileLineEdit->property("fullpath").toString().toUtf8().constData(), "fudge", ui->outputSpeedUnitsComboBox->currentText().toUtf8().constData(), papszOptions);  // test error handling  // function needs reorganized to handle this test
                //ninjaErr = NinjaSetStationKML(ninjaArmy, i, "fudge", ui->outputDirectoryLineEdit->text().toUtf8().constData(), ui->outputSpeedUnitsComboBox->currentText().toUtf8().constData(), papszOptions);  // test error handling  // function needs reorganized to handle this test
                if(ninjaErr != NINJA_SUCCESS)
                {
                    printf("NinjaSetStationKML: ninjaErr = %d\n", ninjaErr);
                    return false;
                }
            }
        }

        ninjaErr = NinjaSetNumberCPUs(ninjaArmy, i, ui->numberOfProcessorsSpinBox->value(), papszOptions);
        //ninjaErr = NinjaSetNumberCPUs(ninjaArmy, i+10, ui->numberOfProcessorsSpinBox->value(), papszOptions);  // test error handling
        //ninjaErr = NinjaSetNumberCPUs(ninjaArmy, i, -1, papszOptions);  // test error handling  // requires the try/catch form of IF_VALID_INDEX_TRY in ninjaArmy.h
        if(ninjaErr != NINJA_SUCCESS)
        {
            qDebug() << "NinjaSetNumberCPUs: ninjaErr =" << ninjaErr;
            return false;
        }

        ninjaErr = NinjaSetInitializationMethod(ninjaArmy, i, initializationMethod, ui->pointInitializationGroupBox->isChecked(), papszOptions);
        //ninjaErr = NinjaSetInitializationMethod(ninjaArmy, i+10, initializationMethod, ui->pointInitializationGroupBox->isChecked(), papszOptions);  // test error handling  // hrm, ninjaCom isn't triggering for this one, though the error returns, leading to it hanging without a proper message.
        //ninjaErr = NinjaSetInitializationMethod(ninjaArmy, i, "fudge", ui->pointInitializationGroupBox->isChecked(), papszOptions);  // test error handling
        if(ninjaErr != NINJA_SUCCESS)
        {
            qDebug() << "NinjaSetInitializationMethod: ninjaErr =" << ninjaErr;
            return false;
        }

        ninjaErr = NinjaSetDem(ninjaArmy, i, ui->elevationInputFileLineEdit->property("fullpath").toString().toUtf8().constData(), papszOptions);
        //ninjaErr = NinjaSetDem(ninjaArmy, i+10, ui->elevationInputFileLineEdit->property("fullpath").toString().toUtf8().constData(), papszOptions);
        //ninjaErr = NinjaSetDem(ninjaArmy, i, "fudge", papszOptions);  // test error handling  // requires the try/catch form of IF_VALID_INDEX_TRY in ninjaArmy.h
        if(ninjaErr != NINJA_SUCCESS)
        {
            qDebug() << "NinjaSetDem: ninjaErr =" << ninjaErr;
            return false;
        }

        ninjaErr = NinjaSetPosition(ninjaArmy, i, papszOptions);  // if setting up ninja.cpp function call to simply throw, this breaks, this requires the try/catch form of IF_VALID_INDEX_TRY in ninjaArmy.h
        //ninjaErr = NinjaSetPosition(ninjaArmy, i+10, papszOptions);  // test error handling
        if(ninjaErr != NINJA_SUCCESS)
        {
            qDebug() << "NinjaSetPosition: ninjaErr =" << ninjaErr;
            return false;
        }

        ninjaErr = NinjaSetInputWindHeight(ninjaArmy, i, ui->inputWindHeightSpinBox->value(), "m", papszOptions);
        //ninjaErr = NinjaSetInputWindHeight(ninjaArmy, i+10, ui->inputWindHeightSpinBox->value(), "m", papszOptions);  // test error handling  // hrm, ninjaCom isn't triggering for this one, though the error returns, leading to it hanging without a proper message.
        //ninjaErr = NinjaSetInputWindHeight(ninjaArmy, i, ui->inputWindHeightSpinBox->value(), "fudge", papszOptions);  // test error handling
        //ninjaErr = NinjaSetInputWindHeight(ninjaArmy, i, -1, "m", papszOptions);  // test error handling
        if(ninjaErr != NINJA_SUCCESS)
        {
            qDebug() << "NinjaSetInputWindHeight: ninjaErr =" << ninjaErr;
            return false;
        }

        ninjaErr = NinjaSetDiurnalWinds(ninjaArmy, i, ui->diurnalCheckBox->isChecked(), papszOptions);
        //ninjaErr = NinjaSetDiurnalWinds(ninjaArmy, i+10, ui->diurnalCheckBox->isChecked(), papszOptions);  // test error handling
        if(ninjaErr != NINJA_SUCCESS)
        {
            qDebug() << "NinjaSetDiurnalWinds: ninjaErr =" << ninjaErr;
            return false;
        }

        if(ui->vegetationStackedWidget->currentIndex() == 0)
        {
            ninjaErr = NinjaSetUniVegetation(ninjaArmy, i, ui->vegetationComboBox->currentText().toLower().toUtf8().constData(), papszOptions);
            //ninjaErr = NinjaSetUniVegetation(ninjaArmy, i+10, ui->vegetationComboBox->currentText().toLower().toUtf8().constData(), papszOptions);  // test error handling  // hrm, ninjaCom isn't triggering for this one, though the error returns, leading to it hanging without a proper message.
            //ninjaErr = NinjaSetUniVegetation(ninjaArmy, i, "fudge", papszOptions);  // test error handling
            if(ninjaErr != NINJA_SUCCESS)
            {
                qDebug() << "NinjaSetUniVegetation: ninjaErr =" << ninjaErr;
                return false;
            }
        }

        if(ui->meshResolutionComboBox->currentIndex() == 3) // custom res
        {
            ninjaErr = NinjaSetMeshResolution(ninjaArmy, i, ui->meshResolutionSpinBox->value(), ui->meshResolutionUnitsComboBox->itemData(ui->meshResolutionUnitsComboBox->currentIndex()).toString().toUtf8().constData(), papszOptions);
            if(ninjaErr != NINJA_SUCCESS)
            {
                qDebug() << "NinjaSetMeshResolution: ninjaErr =" << ninjaErr;
                return false;
            }
        } else
        {
            ninjaErr = NinjaSetMeshResolutionChoice(ninjaArmy, i, ui->meshResolutionComboBox->currentText().toLower().toUtf8().constData(), papszOptions);
            //ninjaErr = NinjaSetMeshResolutionChoice(ninjaArmy, i+10, ui->meshResolutionComboBox->currentText().toLower().toUtf8().constData(), papszOptions);  // test error handling  // hrm, ninjaCom isn't triggering for this one, though the error returns, leading to it hanging without a proper message.
            //ninjaErr = NinjaSetMeshResolutionChoice(ninjaArmy, i, "fudge", papszOptions);  // test error handling
            if(ninjaErr != NINJA_SUCCESS)
            {
                qDebug() << "NinjaSetMeshResolutionChoice: ninjaErr =" << ninjaErr;
                return false;
            }
        }

        ninjaErr = NinjaSetNumVertLayers(ninjaArmy, i, 20, papszOptions);
        //ninjaErr = NinjaSetNumVertLayers(ninjaArmy, i+10, 20, papszOptions);  // test error handling
        //ninjaErr = NinjaSetNumVertLayers(ninjaArmy, i, -1, papszOptions);  // test error handling  // requires the try/catch form of IF_VALID_INDEX_TRY in ninjaArmy.h
        if(ninjaErr != NINJA_SUCCESS)
        {
            qDebug() << "NinjaSetNumVertLayers: ninjaErr =" << ninjaErr;
            return false;
        }

        bool retVal = setOutputFlags(ninjaArmy, i, numNinjas, PDFSize);
        if( retVal == false )
        {
            return false;
        }
    }

    return true;
}

bool MainWindow::setOutputFlags(NinjaArmyH* ninjaArmy,
                                int i,
                                int numNinjas,
                                OutputPDFSize PDFSize)
{
    char **papszOptions = nullptr;
    int ninjaErr;

    ninjaErr = NinjaSetOutputPath(ninjaArmy, i, ui->outputDirectoryLineEdit->text().toUtf8().constData(), papszOptions);
    //ninjaErr = NinjaSetOutputPath(ninjaArmy, i+10, ui->outputDirectoryLineEdit->text().toUtf8().constData(), papszOptions);  // test error handling
    if (ninjaErr != NINJA_SUCCESS)
    {
        qDebug() << "NinjaSetOutputPath: ninjaErr =" << ninjaErr;
        return false;
    }

    ninjaErr = NinjaSetOutputWindHeight(ninjaArmy, i, ui->outputWindHeightSpinBox->value(), ui->outputWindHeightUnitsComboBox->itemData(ui->outputWindHeightUnitsComboBox->currentIndex()).toString().toUtf8().constData(), papszOptions);
    //ninjaErr = NinjaSetOutputWindHeight(ninjaArmy, i+10, ui->outputWindHeightSpinBox->value(), ui->outputWindHeightUnitsComboBox->itemData(ui->outputWindHeightUnitsComboBox->currentIndex()).toString().toUtf8().constData(), papszOptions);  // test error handling  // hrm, ninjaCom isn't triggering for this one, though the error returns, leading to it hanging without a proper message.
    //ninjaErr = NinjaSetOutputWindHeight(ninjaArmy, i, ui->outputWindHeightSpinBox->value(), "fudge", papszOptions);  // test error handling
    //ninjaErr = NinjaSetOutputWindHeight(ninjaArmy, i, -1, ui->outputWindHeightUnitsComboBox->itemData(ui->outputWindHeightUnitsComboBox->currentIndex()).toString().toUtf8().constData(), papszOptions);  // test error handling
    if (ninjaErr != NINJA_SUCCESS)
    {
        qDebug() << "NinjaSetOutputWindHeight: ninjaErr =" << ninjaErr;
        return false;
    }

    ninjaErr = NinjaSetOutputSpeedUnits(ninjaArmy, i, ui->outputSpeedUnitsComboBox->currentText().toUtf8().constData(), papszOptions);
    //ninjaErr = NinjaSetOutputSpeedUnits(ninjaArmy, i+10, ui->outputSpeedUnitsComboBox->currentText().toUtf8().constData(), papszOptions);  // test error handling  // hrm, ninjaCom isn't triggering for this one, though the error returns, leading to it hanging without a proper message.
    //ninjaErr = NinjaSetOutputSpeedUnits(ninjaArmy, i, "fudge", papszOptions);  // test error handling
    if (ninjaErr != NINJA_SUCCESS)
    {
        qDebug() << "NinjaSetOutputSpeedUnits: ninjaErr =" << ninjaErr;
        return false;
    }

    ninjaErr = NinjaSetGoogOutFlag(ninjaArmy, i, ui->googleEarthGroupBox->isChecked(), papszOptions);
    //ninjaErr = NinjaSetGoogOutFlag(ninjaArmy, i+10, ui->googleEarthGroupBox->isChecked(), papszOptions);  // test error handling
    if (ninjaErr != NINJA_SUCCESS)
    {
        qDebug() << "NinjaSetGoogOutFlag: ninjaErr =" << ninjaErr;
        return false;
    }

    ninjaErr = NinjaSetGoogResolution(ninjaArmy, i, ui->googleEarthMeshResolutionSpinBox->value(), ui->googleEarthMeshResolutionComboBox->itemData(ui->googleEarthMeshResolutionComboBox->currentIndex()).toString().toUtf8().constData(), papszOptions);
    //ninjaErr = NinjaSetGoogResolution(ninjaArmy, i+10, ui->googleEarthMeshResolutionSpinBox->value(), ui->googleEarthMeshResolutionComboBox->itemData(ui->googleEarthMeshResolutionComboBox->currentIndex()).toString().toUtf8().constData(), papszOptions);  // test error handling  // hrm, ninjaCom isn't triggering for this one, though the error returns, leading to it hanging without a proper message.
    //ninjaErr = NinjaSetGoogResolution(ninjaArmy, i, ui->googleEarthMeshResolutionSpinBox->value(), "fudge", papszOptions);  // test error handling
    if (ninjaErr != NINJA_SUCCESS)
    {
        qDebug() << "NinjaSetGoogResolution: ninjaErr =" << ninjaErr;
        return false;
    }

    ninjaErr = NinjaSetGoogSpeedScaling(ninjaArmy, i, ui->legendComboBox->itemData(ui->legendComboBox->currentIndex()).toString().toUtf8().constData(), papszOptions);
    //ninjaErr = NinjaSetGoogSpeedScaling(ninjaArmy, i+10, ui->legendComboBox->itemData(ui->legendComboBox->currentIndex()).toString().toUtf8().constData(), papszOptions);  // test error handling  // hrm, ninjaCom isn't triggering for this one, though the error returns, leading to it hanging without a proper message.
    //ninjaErr = NinjaSetGoogSpeedScaling(ninjaArmy, i, "fudge", papszOptions);  // test error handling
    if (ninjaErr != NINJA_SUCCESS)
    {
        qDebug() << "NinjaSetGoogSpeedScaling: ninjaErr =" << ninjaErr;
        return false;
    }

    ninjaErr = NinjaSetGoogLineWidth(ninjaArmy, i, ui->googleEarthVectorsSpinBox->value(), papszOptions);
    //ninjaErr = NinjaSetGoogLineWidth(ninjaArmy, i+10, ui->googleEarthVectorsSpinBox->value(), papszOptions);  // test error handling
    //ninjaErr = NinjaSetGoogLineWidth(ninjaArmy, i, -1, papszOptions);  // test error handling  // requires the try/catch form of IF_VALID_INDEX_TRY in ninjaArmy.h
    //ninjaErr = NinjaSetGoogLineWidth(ninjaArmy, i, 101, papszOptions);  // test error handling
    if (ninjaErr != NINJA_SUCCESS)
    {
        qDebug() << "NinjaSetGoogLineWidth: ninjaErr =" << ninjaErr;
        return false;
    }

    ninjaErr = NinjaSetGoogColor(ninjaArmy, i, ui->alternativeColorSchemeComboBox->itemData(ui->alternativeColorSchemeComboBox->currentIndex()).toString().toUtf8().constData(), ui->googleEarthVectorScalingCheckBox->isChecked(), papszOptions);
    //ninjaErr = NinjaSetGoogColor(ninjaArmy, i+10, ui->alternativeColorSchemeComboBox->itemData(ui->alternativeColorSchemeComboBox->currentIndex()).toString().toUtf8().constData(), ui->googleEarthVectorScalingCheckBox->isChecked(), papszOptions);  // test error handling
    ////ninjaErr = NinjaSetGoogColor(ninjaArmy, i, "fudge", ui->googleEarthVectorScalingCheckBox->isChecked(), papszOptions);  // test error handling  // requires the try/catch form of IF_VALID_INDEX_TRY in ninjaArmy.h  // actually, the colorScheme string appears to not even be checked
    if (ninjaErr != NINJA_SUCCESS)
    {
        qDebug() << "NinjaSetGoogColor: ninjaErr =" << ninjaErr;
        return false;
    }

    ninjaErr = NinjaSetGoogConsistentColorScale(ninjaArmy, i, ui->legendCheckBox->isChecked(), numNinjas, papszOptions);
    //ninjaErr = NinjaSetGoogConsistentColorScale(ninjaArmy, i+10, ui->legendCheckBox->isChecked(), numNinjas, papszOptions);  // test error handling
    if (ninjaErr != NINJA_SUCCESS)
    {
        qDebug() << "NinjaSetGoogConsistentColorScale: ninjaErr =" << ninjaErr;
        return false;
    }

    ninjaErr = NinjaSetAsciiOutFlag(ninjaArmy, i, ui->fireBehaviorGroupBox->isChecked(), papszOptions);
    //ninjaErr = NinjaSetAsciiOutFlag(ninjaArmy, i+10, ui->fireBehaviorGroupBox->isChecked(), papszOptions);  // test error handling
    if (ninjaErr != NINJA_SUCCESS)
    {
        qDebug() << "NinjaSetAsciiOutFlag: ninjaErr =" << ninjaErr;
        return false;
    }

    ninjaErr = NinjaSetAsciiResolution(ninjaArmy, i, ui->fireBehaviorMeshResolutionSpinBox->value(), ui->fireBehaviorMeshResolutionComboBox->itemData(ui->fireBehaviorMeshResolutionComboBox->currentIndex()).toString().toUtf8().constData(), papszOptions);
    //ninjaErr = NinjaSetAsciiResolution(ninjaArmy, i+10, ui->fireBehaviorMeshResolutionSpinBox->value(), ui->fireBehaviorMeshResolutionComboBox->itemData(ui->fireBehaviorMeshResolutionComboBox->currentIndex()).toString().toUtf8().constData(), papszOptions);  // test error handling  // hrm, ninjaCom isn't triggering for this one, though the error returns, leading to it hanging without a proper message.
    //ninjaErr = NinjaSetAsciiResolution(ninjaArmy, i, ui->fireBehaviorMeshResolutionSpinBox->value(), "fudge", papszOptions);  // test error handling
    if (ninjaErr != NINJA_SUCCESS)
    {
        qDebug() << "NinjaSetAsciiResolution: ninjaErr =" << ninjaErr;
        return false;
    }

    ninjaErr = NinjaSetShpOutFlag(ninjaArmy, i, ui->shapeFilesGroupBox->isChecked(), papszOptions);
    //ninjaErr = NinjaSetShpOutFlag(ninjaArmy, i+10, ui->shapeFilesGroupBox->isChecked(), papszOptions);  // test error handling
    if (ninjaErr != NINJA_SUCCESS)
    {
        qDebug() << "NinjaSetShpOutFlag: ninjaErr =" << ninjaErr;
        return false;
    }

    ninjaErr = NinjaSetShpResolution(ninjaArmy, i, ui->shapeFilesMeshResolutionSpinBox->value(), ui->shapeFilesMeshResolutionComboBox->itemData(ui->shapeFilesMeshResolutionComboBox->currentIndex()).toString().toUtf8().constData(), papszOptions);
    //ninjaErr = NinjaSetShpResolution(ninjaArmy, i+10, ui->shapeFilesMeshResolutionSpinBox->value(), ui->shapeFilesMeshResolutionComboBox->itemData(ui->shapeFilesMeshResolutionComboBox->currentIndex()).toString().toUtf8().constData(), papszOptions);  // test error handling  // hrm, ninjaCom isn't triggering for this one, though the error returns, leading to it hanging without a proper message.
    //ninjaErr = NinjaSetShpResolution(ninjaArmy, i, ui->shapeFilesMeshResolutionSpinBox->value(), "fudge", papszOptions);  // test error handling
    if (ninjaErr != NINJA_SUCCESS)
    {
        qDebug() << "NinjaSetShpResolution: ninjaErr =" << ninjaErr;
        return false;
    }

    ninjaErr = NinjaSetPDFOutFlag(ninjaArmy, i, ui->geospatialPDFFilesGroupBox->isChecked(), papszOptions);
    //ninjaErr = NinjaSetPDFOutFlag(ninjaArmy, i+10, ui->geospatialPDFFilesGroupBox->isChecked(), papszOptions);  // test error handling
    if (ninjaErr != NINJA_SUCCESS)
    {
        qDebug() << "NinjaSetPDFOutFlag: ninjaErr =" << ninjaErr;
        return false;
    }

    ninjaErr = NinjaSetPDFLineWidth(ninjaArmy, i, ui->geospatialPDFFilesVectorsSpinBox->value(), papszOptions);
    //ninjaErr = NinjaSetPDFLineWidth(ninjaArmy, i+10, ui->geospatialPDFFilesVectorsSpinBox->value(), papszOptions);  // test error handling
    if (ninjaErr != NINJA_SUCCESS)
    {
        qDebug() << "NinjaSetPDFLineWidth: ninjaErr =" << ninjaErr;
        return false;
    }

    ninjaErr = NinjaSetPDFBaseMap(ninjaArmy, i, ui->basemapComboBox->currentIndex(), papszOptions);
    //ninjaErr = NinjaSetPDFBaseMap(ninjaArmy, i+10, ui->basemapComboBox->currentIndex(), papszOptions);  // test error handling
    if (ninjaErr != NINJA_SUCCESS)
    {
        qDebug() << "NinjaSetPDFBaseMap: ninjaErr =" << ninjaErr;
        return false;
    }

    ninjaErr = NinjaSetPDFDEM(ninjaArmy, i, ui->elevationInputFileLineEdit->property("fullpath").toString().toUtf8().constData(), papszOptions);
    //ninjaErr = NinjaSetPDFDEM(ninjaArmy, i+10, ui->elevationInputFileLineEdit->property("fullpath").toString().toUtf8().constData(), papszOptions);  // test error handling
    ////ninjaErr = NinjaSetPDFDEM(ninjaArmy, i, "fudge", papszOptions);  // test error handling  // the dem string is not even checked
    if (ninjaErr != NINJA_SUCCESS)
    {
        qDebug() << "NinjaSetPDFDEM: ninjaErr =" << ninjaErr;
        return false;
    }

    ninjaErr = NinjaSetPDFSize(ninjaArmy, i, PDFSize.PDFHeight, PDFSize.PDFWidth, PDFSize.PDFDpi, papszOptions);
    //ninjaErr = NinjaSetPDFSize(ninjaArmy, i+10, PDFSize.PDFHeight, PDFSize.PDFWidth, PDFSize.PDFDpi, papszOptions);  // test error handling
    if (ninjaErr != NINJA_SUCCESS)
    {
        qDebug() << "NinjaSetPDFSize: ninjaErr =" << ninjaErr;
        return false;
    }

    ninjaErr = NinjaSetPDFResolution(ninjaArmy, i, ui->geospatialPDFFilesMeshResolutionSpinBox->value(), ui->geospatialPDFFilesMeshResolutionComboBox->itemData(ui->geospatialPDFFilesMeshResolutionComboBox->currentIndex()).toString().toUtf8().constData(), papszOptions);
    //ninjaErr = NinjaSetPDFResolution(ninjaArmy, i+10, ui->geospatialPDFFilesMeshResolutionSpinBox->value(), ui->geospatialPDFFilesMeshResolutionComboBox->itemData(ui->geospatialPDFFilesMeshResolutionComboBox->currentIndex()).toString().toUtf8().constData(), papszOptions);  // test error handling  // hrm, ninjaCom isn't triggering for this one, though the error returns, leading to it hanging without a proper message.
    //ninjaErr = NinjaSetPDFResolution(ninjaArmy, i, ui->geospatialPDFFilesMeshResolutionSpinBox->value(), "fudge", papszOptions);  // test error handling
    if (ninjaErr != NINJA_SUCCESS)
    {
        qDebug() << "NinjaSetPDFResolution: ninjaErr =" << ninjaErr;
        return false;
    }

    ninjaErr = NinjaSetVtkOutFlag(ninjaArmy, i, ui->VTKFilesCheckBox->isChecked(), papszOptions);
    //ninjaErr = NinjaSetVtkOutFlag(ninjaArmy, i+10, ui->VTKFilesCheckBox->isChecked(), papszOptions);  // test error handling
    if (ninjaErr != NINJA_SUCCESS)
    {
        qDebug() << "NinjaSetVtkOutFlag: ninjaErr =" << ninjaErr;
        return false;
    }

    if(ui->rawWeatherModelOutputCheckBox->isCheckable() && ui->rawWeatherModelOutputCheckBox->isChecked())
    {
        ninjaErr = NinjaSetWxModelGoogOutFlag(ninjaArmy, i, ui->googleEarthGroupBox->isChecked(), papszOptions);
        if (ninjaErr != NINJA_SUCCESS)
        {
            qDebug() << "NinjaSetWxModelGoogOutFlag: ninjaErr =" << ninjaErr;
            return false;
        }

        ninjaErr = NinjaSetWxModelShpOutFlag(ninjaArmy, i, ui->shapeFilesGroupBox->isChecked(), papszOptions);
        if (ninjaErr != NINJA_SUCCESS)
        {
            qDebug() << "NinjaSetWxModelShpOutFlag: ninjaErr =" << ninjaErr;
            return false;
        }

        ninjaErr = NinjaSetWxModelAsciiOutFlag(ninjaArmy, i, ui->fireBehaviorGroupBox->isChecked(), papszOptions);
        if (ninjaErr != NINJA_SUCCESS)
        {
            qDebug() << "NinjaSetWxModelAsciiOutFlag: ninjaErr =" << ninjaErr;
            return false;
        }
    }

    return true;
}

int MainWindow::startSolve(int numProcessors)
{
    char **papszOptions = nullptr;
    return NinjaStartRuns(ninjaArmy, ui->numberOfProcessorsSpinBox->value(), papszOptions);
}

void MainWindow::finishedSolve()
{
    // get the return value of the QtConcurrent::run() function
    int result = futureWatcher->future().result();

    // ninjaCom handles most of the progress dialog, cli, and console window messaging now
    if( result == 1 ) // simulation properly finished
    {
        progressDialog->setValue(maxProgress);
        progressDialog->setLabelText("Simulations finished");
        progressDialog->setCancelButtonText("Close");

        qDebug() << "Finished with simulations";
        writeToConsole("Finished with simulations", Qt::darkGreen);
    }
    //else if( result == NINJA_E_CANCELLED ) // the proper way to do this, but checking progressDialog->wasCanceled() seems way safer
    else if( progressDialog->wasCanceled() ) // simulation was cancelled
    {
        progressDialog->setValue(maxProgress);
        progressDialog->setCancelButtonText("Close");
    }
    else // simulation ended in some known error
    {
        progressDialog->setValue(maxProgress);
        progressDialog->setCancelButtonText("Close");
    }

    disconnect(progressDialog, SIGNAL(canceled()), this, SLOT(cancelSolve()));

    // one more process to do after finishedSolve() stuff
    plotKmzOutputs();

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

void MainWindow::plotKmzOutputs()
{
    // get the return value of the QtConcurrent::run() function
    int result = futureWatcher->future().result();

    if(result == 1 && !progressDialog->wasCanceled() && ui->googleEarthGroupBox->isChecked() == true)
    {
        // enable QWebInspector for degugging the google maps widget
        if(CSLTestBoolean(CPLGetConfigOption("ENABLE_QWEBINSPECTOR", "NO")))
        {
            QWidget* inspectorWindow = new QWidget(this);
            inspectorWindow->setWindowTitle("Web Inspector - Developer Tools");
            inspectorWindow->setMinimumSize(800, 600);

            QWebEngineView* inspectorView = new QWebEngineView(inspectorWindow);
            inspectorView->page()->setInspectedPage(webEngineView->page());

            QVBoxLayout* layout = new QVBoxLayout(inspectorWindow);
            layout->addWidget(inspectorView);
            layout->setContentsMargins(0, 0, 0, 0);

            inspectorWindow->show();
        }

        // vars to be filled
        int numRuns = 0;
        char **kmzFilenames = NULL;
        int numStationKmls = 0;
        char **stationKmlFilenames = NULL;
        char **weatherModelKmzFilenames = NULL;

        char **papszOptions = nullptr;
        ninjaErr = NinjaGetRunKmzFilenames(ninjaArmy, &numRuns, &kmzFilenames, &numStationKmls, &stationKmlFilenames, &weatherModelKmzFilenames, papszOptions);
        if(ninjaErr != NINJA_SUCCESS)
        {
            printf("NinjaGetRunKmzFilenames: ninjaErr = %d\n", ninjaErr);
        }

        std::vector<std::string> kmzFilenamesStr;
        std::vector<std::string> stationKmlFilenamesStr;
        std::vector<std::string> wxModelKmzFilenamesStr;

        kmzFilenamesStr.reserve(numRuns);
        wxModelKmzFilenamesStr.reserve(numRuns);
        for(int i = 0; i < numRuns; i++)
        {
            kmzFilenamesStr.emplace_back(kmzFilenames[i]);
            wxModelKmzFilenamesStr.emplace_back(weatherModelKmzFilenames[i]);
        }

        stationKmlFilenamesStr.reserve(numStationKmls);
        for(int j = 0; j < numStationKmls; j++)
        {
            stationKmlFilenamesStr.emplace_back(stationKmlFilenames[j]);
        }

        outputKmzFilenames.push_back(std::move( kmzFilenamesStr ));
        outputStationKmlFilenames.push_back(std::move( stationKmlFilenamesStr ));
        outputWxModelKmzFilenames.push_back(std::move( wxModelKmzFilenamesStr ));

        for(int i = 0; i < numRuns; i++)
        {
            // plot the output kmz of the run
            QString outFileStr = QString::fromStdString(kmzFilenames[i]);
            qDebug() << "kmz outFile =" << outFileStr;
            QFile outFile(outFileStr);

            outFile.open(QIODevice::ReadOnly);
            QByteArray data = outFile.readAll();
            QString base64 = data.toBase64();

            webEngineView->page()->runJavaScript("loadKmzFromBase64('"+base64+"')");

            // if it is a point initialization run, and station kmls were created for the run,
            // plot the station kmls of the first run
            // (first run, because station kmls are SHARED across runs)
            if(ui->pointInitializationGroupBox->isChecked() && ui->pointInitializationWriteStationKMLCheckBox->isChecked() && i == 0)
            {
                for(int j = 0; j < numStationKmls; j++)
                {
                    QString outFileStr = QString::fromStdString(stationKmlFilenames[j]);
                    qDebug() << "station kml outFile =" << outFileStr;
                    QFile outFile(outFileStr);

                    outFile.open(QIODevice::ReadOnly);
                    QByteArray data = outFile.readAll();
                    QString base64 = data.toBase64();

                    webEngineView->page()->runJavaScript("loadKmzFromBase64('"+base64+"')");
                }
            }

            // if it is a weather model run, and weather model kmzs were created for the run,
            // plot the weather model kmz of the run
            if(ui->weatherModelGroupBox->isChecked() && ui->googleEarthGroupBox->isChecked())
            {
                QString outFileStr = QString::fromStdString(weatherModelKmzFilenames[i]);
                qDebug() << "wx model kmz outFile =" << outFileStr;
                QFile outFile(outFileStr);

                outFile.open(QIODevice::ReadOnly);
                QByteArray data = outFile.readAll();
                QString base64 = data.toBase64();

                webEngineView->page()->runJavaScript("loadKmzFromBase64('"+base64+"')");
            }
        }

        ninjaErr = NinjaDestroyRunKmzFilenames(numRuns, kmzFilenames, numStationKmls, stationKmlFilenames, weatherModelKmzFilenames, papszOptions);
        if(ninjaErr != NINJA_SUCCESS)
        {
            printf("NinjaDestroyRunKmzFilenames: ninjaErr = %d\n", ninjaErr);
        }

    } // if(result == 1 && !progressDialog->wasCanceled() && ui->googleEarthGroupBox->isChecked() == true)
}

void MainWindow::writeSettings()
{
    writeToConsole("Saving settings...");

    QSettings settings(QSettings::UserScope, "Firelab", "WindNinja");
    settings.setDefaultFormat(QSettings::IniFormat);
    //qDebug() << "settings filename =" << settings.fileName();

    //input file path
    settings.setValue("inputFileDir", ui->elevationInputFileLineEdit->property("fullpath"));

    //momentum flag
    settings.setValue("momentumFlag", ui->momentumSolverCheckBox->isChecked());
    //veg choice
    settings.setValue("vegChoice", ui->vegetationComboBox->currentIndex());
    //mesh choice
    settings.setValue("meshChoice", ui->meshResolutionComboBox->currentIndex());
    //mesh units
    settings.setValue("meshUnits", ui->meshResolutionUnitsComboBox->currentIndex());
    //number of processors
    settings.setValue("nProcessors", ui->numberOfProcessorsSpinBox->value());

    //time zone
    //settings.setValue("timeZone", ui->timeZoneComboBox->currentIndex());

    //settings.setValue("pointFile", tree->point->stationFileName );

    //if(ui->meshResolutionComboBox->currentIndex() == 3) // custom res
    //{
    //    settings.setValue("customRes", ui->meshResolutionSpinBox->value());
    //}
    // need to write it every time, the past value will get left there without getting updated otherwise, doesn't delete past settings values
    settings.setValue("customRes", ui->meshResolutionSpinBox->value());

    writeToConsole("Settings saved.");
}

void MainWindow::readSettings()
{
    QSettings settings(QSettings::UserScope, "Firelab", "WindNinja");
    settings.setDefaultFormat(QSettings::IniFormat);

    if(settings.contains("inputFileDir"))
    {
        if(QFile::exists(settings.value("inputFileDir").toString()))
        {
            ui->elevationInputFileLineEdit->setText(settings.value("inputFileDir").toString());
        }
    }
    else
    {
        // std::string oTmpPath = FindNinjaRootDir();
        // inputFileDir = CPLFormFilename(oTmpPath.c_str(), "etc/windninja/example-files", NULL);
    }

    // TODO: some of the following might be overriding the values computed by inputFileDir, when the other way around might be better
    if(settings.contains("momentumFlag"))
    {
        bool momentumFlag = settings.value("momentumFlag").toBool();
        if(momentumFlag == true)
        {
            ui->momentumSolverCheckBox->setChecked(true);
            emit momentumSolverCheckBoxClicked();
        }
    }
    if(settings.contains("vegChoice"))
    {
        ui->vegetationComboBox->setCurrentIndex(settings.value("vegChoice").toInt());
    }
    if(settings.contains("meshUnits"))  // putting this after loading meshChoice results in overwriting the value by an extra set of units
    {
        ui->meshResolutionUnitsComboBox->setCurrentIndex(settings.value("meshUnits").toInt());
    }
    if(settings.contains("meshChoice"))
    {
        int choice = settings.value("meshChoice").toInt();
        ui->meshResolutionComboBox->setCurrentIndex(choice);
        if(choice == 3)
        {
            if(!settings.contains("customRes"))
            {
                qDebug() << "Error. WindNinja settings does not contain \"customRes\"";
            }
            ui->meshResolutionSpinBox->setValue(settings.value("customRes").toDouble());
        }
    }
    if(settings.contains("nProcessors"))
    {
        ui->numberOfProcessorsSpinBox->setValue(settings.value("nProcessors").toInt());
    }
    // won't we want the timezone of the dem every time, to avoid accidentally doing a weird combination of time zones?
    if(settings.contains("timeZone"))
    {
        // QString v = settings.value("timeZone").toString();
        // int index = tree->surface->timeZone->tzComboBox->findText(v);
        // if(index == -1)
        //     tree->surface->timeZone->tzCheckBox->setChecked( true );
        // index = tree->surface->timeZone->tzComboBox->findText(v);
        // if( index == 0 )
        //     tree->surface->timeZone->tzComboBox->setCurrentIndex(index +  1);
        // true->surface->timeZone->tzComboBox->setCurrentIndex(index);
    }
    else
    {
        // tree->surface->timeZone->tzComboBox->setCurrentIndex(2);
        // tree->surface->timeZone->tzComboBox->setCurrentIndex(1);
    }
    if(settings.contains("pointFile"))
    {
        // QString f = settings.value("pointFile").toString();
        // tree->point->stationFileName = f;
    }
}

void MainWindow::showEvent(QShowEvent *event)
{
    QMainWindow::showEvent(event);

}

void MainWindow::closeEvent(QCloseEvent *event)
{
    writeSettings();
    QMainWindow::closeEvent(event);
}
