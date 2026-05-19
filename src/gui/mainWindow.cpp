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
    state.updateSolverMethodologyState();

    lineNumber = 1;

    serverBridge = new ServerBridge();
    serverBridge->checkMessages();

    QString dataPath = QString::fromUtf8(CPLGetConfigOption("WINDNINJA_DATA", ""));
    QString mapPath = QDir(dataPath).filePath("map.html");
    webEngineView = new QWebEngineView(ui->mapPanelWidget);
    webEngineView->settings()->setAttribute(QWebEngineSettings::LocalContentCanAccessFileUrls, true);
    webEngineView->settings()->setAttribute(QWebEngineSettings::LocalContentCanAccessRemoteUrls, true);
    webEngineView->settings()->setAttribute(QWebEngineSettings::LocalStorageEnabled, true);
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
    menuBar = new MenuBar(ui, webEngineView, this);
    surfaceInput = new SurfaceInput(ui, webEngineView, this);
    domainAverageInput = new DomainAverageInput(ui, this);
    pointInitializationInput = new PointInitializationInput(ui, this);
    weatherModelInput = new WeatherModelInput(ui, this);
    outputs = new Outputs(ui, this);

    ui->treeWidget->topLevelItem(0)->setData(0, Qt::UserRole, 1);
    ui->treeWidget->topLevelItem(0)->child(0)->setData(0, Qt::UserRole, 1);
    ui->treeWidget->topLevelItem(0)->child(1)->setData(0, Qt::UserRole, 2);
    ui->treeWidget->topLevelItem(1)->setData(0, Qt::UserRole, 3);
    ui->treeWidget->topLevelItem(1)->child(0)->setData(0, Qt::UserRole, 3);
    ui->treeWidget->topLevelItem(1)->child(1)->setData(0, Qt::UserRole, 4);
    ui->treeWidget->topLevelItem(1)->child(2)->setData(0, Qt::UserRole, 5);
    ui->treeWidget->topLevelItem(1)->child(3)->setData(0, Qt::UserRole, 6);
    QTreeWidgetItem *windInputItem = ui->treeWidget->topLevelItem(1)->child(3);
    windInputItem->child(0)->setData(0, Qt::UserRole, 6);
    windInputItem->child(1)->setData(0, Qt::UserRole, 7);
    windInputItem->child(2)->setData(0, Qt::UserRole, 8);
    ui->treeWidget->topLevelItem(2)->setData(0, Qt::UserRole, 9);
    ui->treeWidget->topLevelItem(2)->child(0)->setData(0, Qt::UserRole, 10);
    ui->treeWidget->topLevelItem(2)->child(1)->setData(0, Qt::UserRole, 11);
    ui->treeWidget->topLevelItem(2)->child(2)->setData(0, Qt::UserRole, 12);
    ui->treeWidget->topLevelItem(2)->child(3)->setData(0, Qt::UserRole, 13);
    ui->treeWidget->topLevelItem(2)->child(4)->setData(0, Qt::UserRole, 14);
    ui->treeWidget->topLevelItem(2)->child(5)->setData(0, Qt::UserRole, 15);
    ui->treeWidget->topLevelItem(3)->setData(0, Qt::UserRole, 16);

    connectSignals();

    ui->treeWidget->blockSignals(true);
    ui->treeWidget->topLevelItem(0)->setSelected(true);
    ui->inputsStackedWidget->setCurrentIndex(1);
    ui->treeWidget->blockSignals(false);

    int nCPUs = QThread::idealThreadCount();
    ui->availableProcessorsLabel->setText("Available Processors:  " + QString::number(nCPUs));
    ui->numberOfProcessorsSpinBox->setMaximum(nCPUs);
    ui->numberOfProcessorsSpinBox->setValue(nCPUs);

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
    connect(ui->massSolverCheckBox, &QCheckBox::toggled, this, &MainWindow::massSolverCheckBoxToggled);
    connect(ui->momentumSolverCheckBox, &QCheckBox::toggled, this, &MainWindow::momentumSolverCheckBoxToggled);
    connect(ui->diurnalCheckBox, &QCheckBox::toggled, this, &MainWindow::diurnalCheckBoxToggled);
    connect(ui->stabilityCheckBox, &QCheckBox::toggled, this, &MainWindow::stabilityCheckBoxToggled);
    connect(ui->treeWidget, &QTreeWidget::itemDoubleClicked, this, &MainWindow::treeWidgetItemDoubleClicked);
    connect(ui->numberOfProcessorsSolveButton, &QPushButton::clicked, this, &MainWindow::solveButtonClicked);
    connect(ui->outputDirectoryButton, &QPushButton::clicked, this, &MainWindow::outputDirectoryButtonClicked);
    connect(ui->outputDirectoryOpenButton, &QPushButton::clicked, this, &MainWindow::outputDirectoryOpenButtonClicked);
    connect(ui->treeWidget, &QTreeWidget::itemSelectionChanged, this, &MainWindow::treeWidgetItemSelectionChanged);

    connect(menuBar, &MenuBar::writeToConsoleSignal, this, &MainWindow::writeToConsole);
    connect(mapBridge, &MapBridge::boundingBoxReceived, surfaceInput, &SurfaceInput::boundingBoxReceived);
    connect(surfaceInput, &SurfaceInput::updateTreeView, pointInitializationInput, &PointInitializationInput::updateTreeView);
    connect(surfaceInput, &SurfaceInput::updateTreeView, weatherModelInput, &WeatherModelInput::updateTreeView);
    connect(webEngineView, &QWebEngineView::loadFinished, this, &MainWindow::readSettings);

    connect(this, &MainWindow::updateMetholodyState, &AppState::instance(), &AppState::updateSolverMethodologyState);
    connect(this, &MainWindow::updateDirunalState, &AppState::instance(), &AppState::updateDiurnalInputState);
    connect(this, &MainWindow::updateStabilityState, &AppState::instance(), &AppState::updateStabilityInputState);
    connect(this, &MainWindow::updateProgressValueSignal, this, &MainWindow::updateProgressValue, Qt::QueuedConnection);
    connect(this, &MainWindow::updateProgressMessageSignal, this, &MainWindow::updateProgressMessage, Qt::QueuedConnection);
    connect(this, &MainWindow::writeToConsoleSignal, this, &MainWindow::writeToConsole, Qt::QueuedConnection);

    connect(surfaceInput, &SurfaceInput::writeToConsoleSignal, this, &MainWindow::writeToConsole, Qt::QueuedConnection);
    connect(pointInitializationInput, &PointInitializationInput::writeToConsoleSignal, this, &MainWindow::writeToConsole, Qt::QueuedConnection);
    connect(weatherModelInput, &WeatherModelInput::writeToConsoleSignal, this, &MainWindow::writeToConsole, Qt::QueuedConnection);
    connect(mapBridge, &MapBridge::writeToConsoleSignal, this, &MainWindow::writeToConsole, Qt::QueuedConnection);

    connect(mapBridge, &MapBridge::mapLayersLoadingFinishedSignal, menuBar, &MenuBar::kmzLoadFinished);
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
    if(progressDialog)
    {
        progressDialog->setLabelText(message);
    }
    else
    {
        QMessageBox::critical(
            nullptr,
            QApplication::tr("Error"),
            message+"\n"
        );
    }
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

    // both the writeToConsole() function and the QProgressDialog do NOT like having a "\n" at the end of a given message,
    // writeToConsole() adds extra empty lines all over the place and the QProgressDialog adds a weird looking space between the message and the progress bar.
    // but ninjaCom likes to add "\n" to stuff and currently sends one "\n". So need to strip the "\n" character off of the message.
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
            emit self->writeToConsoleSignal(QString::fromStdString("Simulation cancelled by user"), QColor(255, 140, 0));
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
        emit self->writeToConsoleSignal(QString::fromStdString("Solver warning: "+clipStr), QColor(255, 140, 0));
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
    //writeToConsole( "Canceling...", QColor(255, 140, 0));

    char **papszOptions = nullptr;
    ninjaErr  = NinjaCancel(ninjaArmy, papszOptions);
    if( ninjaErr != NINJA_SUCCESS )
    {
        qDebug() << "NinjaCancel: ninjaErr =" << ninjaErr;
    }
}

void MainWindow::treeWidgetItemSelectionChanged()
{
    webEngineView->page()->runJavaScript("stopRectangleDrawing();");

    int column = ui->treeWidget->currentColumn();
    int pageIndex = ui->treeWidget->selectedItems().first()->data(column, Qt::UserRole).toInt(); // assume 0 since no multi selection
    ui->inputsStackedWidget->setCurrentIndex(pageIndex);
}

void MainWindow::massSolverCheckBoxToggled()
{
    if(!ui->massSolverCheckBox->isChecked())
        return;

    ui->stabilityCheckBox->setDisabled(false);
    ui->ninjafoamCaseGroupBox->setVisible(false);

    if(ui->momentumSolverCheckBox->isChecked())
    {
        ui->momentumSolverCheckBox->setChecked(false);
    }

    if(!ui->elevationInputFileLineEdit->text().isEmpty())
    {
        ui->meshResolutionSpinBox->setValue(surfaceInput->computeMeshResolution(ui->meshResolutionComboBox->currentIndex(), ui->momentumSolverCheckBox->isChecked()));
        surfaceInput->updateMeshResolutionByUnits();
    }

    emit updateMetholodyState();
    emit updateStabilityState();
}

void MainWindow::momentumSolverCheckBoxToggled()
{
    if(!ui->momentumSolverCheckBox->isChecked())
        return;

    ui->stabilityCheckBox->setChecked(false);
    ui->stabilityCheckBox->setDisabled(true);
    ui->ninjafoamCaseGroupBox->setVisible(true);

    if(ui->massSolverCheckBox->isChecked())
    {
        ui->massSolverCheckBox->setChecked(false);
    }

    if(!ui->elevationInputFileLineEdit->text().isEmpty())
    {
        ui->meshResolutionSpinBox->setValue(surfaceInput->computeMeshResolution(ui->meshResolutionComboBox->currentIndex(), ui->momentumSolverCheckBox->isChecked()));
        surfaceInput->updateMeshResolutionByUnits();
    }

    emit updateMetholodyState();
    emit updateStabilityState();
}

void MainWindow::diurnalCheckBoxToggled()
{
    bool enabled = ui->diurnalCheckBox->isChecked() || ui->stabilityCheckBox->isChecked();
    for(int row = 0; row < ui->domainAverageTable->rowCount(); row++)
    {
        domainAverageInput->timeEdits[row]->setEnabled(enabled);
        domainAverageInput->dateEdits[row]->setEnabled(enabled);
        domainAverageInput->cloudSpins[row]->setEnabled(enabled);
        domainAverageInput->airTempSpins[row]->setEnabled(enabled);
    }

    emit updateDirunalState();
}

void MainWindow::stabilityCheckBoxToggled()
{
    bool enabled = ui->diurnalCheckBox->isChecked() || ui->stabilityCheckBox->isChecked();
    for(int row = 0; row < ui->domainAverageTable->rowCount(); row++)
    {
        domainAverageInput->timeEdits[row]->setEnabled(enabled);
        domainAverageInput->dateEdits[row]->setEnabled(enabled);
        domainAverageInput->cloudSpins[row]->setEnabled(enabled);
        domainAverageInput->airTempSpins[row]->setEnabled(enabled);
    }

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

void MainWindow::outputDirectoryOpenButtonClicked()
{
    QDesktopServices::openUrl(QUrl::fromLocalFile(outputDir.absolutePath()));
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

    progressDialog->setMinimumSize(380, 100);
    progressDialog->show();

    ninjaErr = NINJA_SUCCESS;

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

    if(state.isDomainAverageInitializationValid)
    {
        initializationMethod = "domain_average";
        QList<double> speeds;
        QList<double> directions;
        QList<int> years;
        QList<int> months;
        QList<int> days;
        QList<int> hours;
        QList<int> minutes;
        QList<double> cloudCovers;
        QList<double> airTemps;

        QString DEMTimeZone = ui->timeZoneComboBox->currentText();

        numNinjas = domainAverageInput->countNumRuns();

        // countNumRuns() returns 0 when ALL rows are 0.0, 0.0 spd, dir rows,
        // but if diurnal is checked, we actually DO want to run that first 0.0, 0.0 spd, dir row as a single run
        if(numNinjas == 0 && ui->diurnalCheckBox->isChecked() == true)
        {
            numNinjas = 1;
        }

        for(int runIdx = 0; runIdx < numNinjas; runIdx++)
        {
            speeds << domainAverageInput->speedSpins[runIdx]->value();
            directions << domainAverageInput->dirSpins[runIdx]->value();

            // always grab the values from the diurnal/stability inputs,
            // whether they are the default values, or whatever the user has changed them to be

            // constructs using machine local time, may need to convert from machine local time to UTC time
            QDateTime currentDateTime = QDateTime(domainAverageInput->dateEdits[runIdx]->date(), domainAverageInput->timeEdits[runIdx]->time());

            years << currentDateTime.date().year();
            months << currentDateTime.date().month();
            days << currentDateTime.date().day();
            hours << currentDateTime.time().hour();
            minutes << currentDateTime.time().minute();

            cloudCovers << domainAverageInput->cloudSpins[runIdx]->value();
            airTemps << domainAverageInput->airTempSpins[runIdx]->value();
        }

        bool momentumFlag = ui->momentumSolverCheckBox->isChecked();
        QString speedUnits =  ui->tableSpeedUnits->currentText();
        QString airTempUnits =  ui->tableTempUnits->currentText().remove("°");
        QString cloudCoverUnits = "percent";
        if(ninjaErr == NINJA_SUCCESS)
        {
            if(ui->diurnalCheckBox->isChecked() || ui->stabilityCheckBox->isChecked())
            {
                ninjaErr = NinjaMakeDomainAverageArmyThermalParameterization(ninjaArmy, numNinjas, momentumFlag, speeds.data(), speedUnits.toUtf8().constData(), directions.data(), years.data(), months.data(), days.data(), hours.data(), minutes.data(), DEMTimeZone.toUtf8().data(), airTemps.data(), airTempUnits.toUtf8().constData(), cloudCovers.data(), cloudCoverUnits.toUtf8().constData(), papszOptions);
            }
            else
            {
                ninjaErr = NinjaMakeDomainAverageArmy(ninjaArmy, numNinjas, momentumFlag, speeds.data(), speedUnits.toUtf8().constData(), directions.data(), papszOptions);
            }
            if(ninjaErr != NINJA_SUCCESS)
            {
                qDebug() << "NinjaMakeDomainAverageArmy: ninjaErr =" << ninjaErr;
            }
        }
    }
    else if(state.isPointInitializationValid)
    {
        initializationMethod = "point";

        NinjaToolsH* ninjaTools = NinjaMakeTools();

        ninjaErr = NinjaSetToolsComMessageHandler(ninjaTools, &comMessageHandler, this, papszOptions);
        if(ninjaErr != NINJA_SUCCESS)
        {
            qDebug() << "NinjaSetToolsComMessageHandler(): ninjaErr =" << ninjaErr;
        }

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
            CPLDebug("STATION_FETCH", "Time Series option selected...");

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
                CPLDebug("STATION_FETCH", "USER WANTS 1 STEP, USING START TIME...");

                int startYear = year[0];
                int startMonth = month[0];
                int startDay = day[0];
                int startHour = hour[0];
                int startMinute = minute[0];

                int endYear, endMonth, endDay, endHour, endMinute;

                ninjaErr = NinjaGenerateSingleTimeObject(
                    ninjaTools,
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
                CPLDebug("STATION_FETCH", "USING TIME LIST...");

                ninjaErr = NinjaGetTimeList(
                    ninjaTools,
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
                CPLDebug("STATION_FETCH", "TIME LIST GENERATED...");

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

            if(ninjaErr != NINJA_SUCCESS)
            {
                // do cleanup before the return, similar to finishedSolve()

//                ninjaErr = NinjaDestroyTools(ninjaTools, papszOptions);
//                if(ninjaErr != NINJA_SUCCESS)
//                {
//                    printf("NinjaDestroyTools: ninjaErr = %d\n", ninjaErr);
//                }
            }
        }
        else
        {
            CPLDebug("STATION_FETCH", "USING CURRENT/LATEST TIME DATA...");

            int year, month, day, hour, minute;
            QDateTime date = ui->weatherStationDataLabel->property("simulationTime").toDateTime();
            year = date.date().year();
            month = date.date().month();
            day = date.date().day();
            hour = date.time().hour();
            minute = date.time().minute();

            int outYear, outMonth, outDay, outHour, outMinute;

            ninjaErr = NinjaGenerateSingleTimeObject(
                ninjaTools,
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
                CPLDebug("STATION_FETCH", "TIME LIST GENERATED...");

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
                    qDebug() << "NinjaMakePointArmy ninjaErr =" << ninjaErr;
                }
            }
        }
    }
    else if(state.isWeatherModelInitializationValid)
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
            qDebug() << "NinjaMakeWeatherModelArmy ninjaErr =" << ninjaErr;
        }
    }
    else
    {
        ninjaErr = NINJA_E_INVALID;
        qCritical() << "ERROR: Invalid initialization inputs to Solve().";
        comMessageHandler("ERROR: Invalid initialization inputs to Solve().", this);
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
        ui->massSolverCheckBox->setChecked(!ui->massSolverCheckBox->isChecked());
    }
    else if (item->text(0) == "Conservation of Mass and Momentum")
    {
        ui->momentumSolverCheckBox->setChecked(!ui->momentumSolverCheckBox->isChecked());
    }
    else if (item->text(0) == "Diurnal Input")
    {
        ui->diurnalCheckBox->setChecked(!ui->diurnalCheckBox->isChecked());
    }
    else if (item->text(0) == "Stability Input")
    {
        ui->stabilityCheckBox->setChecked(!ui->stabilityCheckBox->isChecked());
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
        ui->VTKFilesCheckBox->setChecked(!ui->VTKFilesCheckBox->isChecked());
    }
    else if (item->text(0) == "Map Visualization")
    {
        if(ui->mapVisualizationCheckBox->isEnabled())
        {
            ui->mapVisualizationCheckBox->setChecked(!ui->mapVisualizationCheckBox->isChecked());
        }
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
                if(i == 0)
                {
                    writeToConsole("Writing Weather Station .kml");
                }
                ninjaErr = NinjaSetStationKML(ninjaArmy, i, ui->elevationInputFileLineEdit->property("fullpath").toString().toUtf8().constData(), ui->outputDirectoryLineEdit->text().toUtf8().constData(), ui->outputSpeedUnitsComboBox->currentText().toUtf8().constData(), papszOptions);
                if(ninjaErr != NINJA_SUCCESS)
                {
                    printf("NinjaSetStationKML: ninjaErr = %d\n", ninjaErr);
                    return false;
                }
            }
        }

        ninjaErr = NinjaSetNumberCPUs(ninjaArmy, i, ui->numberOfProcessorsSpinBox->value(), papszOptions);
        if(ninjaErr != NINJA_SUCCESS)
        {
            qDebug() << "NinjaSetNumberCPUs: ninjaErr =" << ninjaErr;
            return false;
        }

        ninjaErr = NinjaSetInitializationMethod(ninjaArmy, i, initializationMethod, ui->pointInitializationGroupBox->isChecked(), papszOptions);
        if(ninjaErr != NINJA_SUCCESS)
        {
            qDebug() << "NinjaSetInitializationMethod: ninjaErr =" << ninjaErr;
            return false;
        }

        ninjaErr = NinjaSetDem(ninjaArmy, i, ui->elevationInputFileLineEdit->property("fullpath").toString().toUtf8().constData(), papszOptions);
        if(ninjaErr != NINJA_SUCCESS)
        {
            qDebug() << "NinjaSetDem: ninjaErr =" << ninjaErr;
            return false;
        }

        if(ui->momentumSolverCheckBox->isChecked() && !ui->ninjafoamCaseLineEdit->text().isEmpty())
        {
            ninjaErr = NinjaSetExistingCaseDirectory(ninjaArmy, i, ui->ninjafoamCaseLineEdit->property("fullpath").toString().toUtf8().constData(), papszOptions);
            if(ninjaErr != NINJA_SUCCESS)
            {
                qDebug() << "NinjaSetExistingCaseDirectory: ninjaErr =" << ninjaErr;
                return false;
            }
        }

        ninjaErr = NinjaSetPosition(ninjaArmy, i, papszOptions);
        if(ninjaErr != NINJA_SUCCESS)
        {
            qDebug() << "NinjaSetPosition: ninjaErr =" << ninjaErr;
            return false;
        }

        ninjaErr = NinjaSetInputWindHeight(ninjaArmy, i, ui->inputWindHeightSpinBox->value(), ui->inputWindHeightUnitsComboBox->itemData(ui->inputWindHeightUnitsComboBox->currentIndex()).toString().toUtf8().constData(), papszOptions);
        if(ninjaErr != NINJA_SUCCESS)
        {
            qDebug() << "NinjaSetInputWindHeight: ninjaErr =" << ninjaErr;
            return false;
        }

        ninjaErr = NinjaSetDiurnalWinds(ninjaArmy, i, ui->diurnalCheckBox->isChecked(), papszOptions);
        if(ninjaErr != NINJA_SUCCESS)
        {
            qDebug() << "NinjaSetDiurnalWinds: ninjaErr =" << ninjaErr;
            return false;
        }

        ninjaErr = NinjaSetStabilityFlag(ninjaArmy, i, ui->stabilityCheckBox->isChecked(), papszOptions);
        if(ninjaErr != NINJA_SUCCESS)
        {
            qDebug() << "NinjaSetStabilityFlag: ninjaErr =" << ninjaErr;
            return false;
        }

        if(ui->vegetationStackedWidget->currentIndex() == 0)
        {
            ninjaErr = NinjaSetUniVegetation(ninjaArmy, i, ui->vegetationComboBox->currentText().toLower().toUtf8().constData(), papszOptions);
            if(ninjaErr != NINJA_SUCCESS)
            {
                qDebug() << "NinjaSetUniVegetation: ninjaErr =" << ninjaErr;
                return false;
            }
        }

        #ifdef NINJAFOAM
        // the cli and the GUI use a value of 300 instead of the default value of 1000.
        ninjaErr = NinjaSetNumberOfIterations(ninjaArmy, i, 300, papszOptions);
        if(ninjaErr != NINJA_SUCCESS)
        {
            qDebug() << "NinjaSetNumberOfIterations: ninjaErr =" << ninjaErr;
            return false;
        }
        #endif //NINJAFOAM

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
            if(ninjaErr != NINJA_SUCCESS)
            {
                qDebug() << "NinjaSetMeshResolutionChoice: ninjaErr =" << ninjaErr;
                return false;
            }
        }

        ninjaErr = NinjaSetNumVertLayers(ninjaArmy, i, 20, papszOptions);
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
    if (ninjaErr != NINJA_SUCCESS)
    {
        qDebug() << "NinjaSetOutputPath: ninjaErr =" << ninjaErr;
        return false;
    }

    ninjaErr = NinjaSetOutputWindHeight(ninjaArmy, i, ui->outputWindHeightSpinBox->value(), ui->outputWindHeightUnitsComboBox->itemData(ui->outputWindHeightUnitsComboBox->currentIndex()).toString().toUtf8().constData(), papszOptions);
    if (ninjaErr != NINJA_SUCCESS)
    {
        qDebug() << "NinjaSetOutputWindHeight: ninjaErr =" << ninjaErr;
        return false;
    }

    ninjaErr = NinjaSetOutputSpeedUnits(ninjaArmy, i, ui->outputSpeedUnitsComboBox->currentText().toUtf8().constData(), papszOptions);
    if (ninjaErr != NINJA_SUCCESS)
    {
        qDebug() << "NinjaSetOutputSpeedUnits: ninjaErr =" << ninjaErr;
        return false;
    }

    ninjaErr = NinjaSetOutputBufferClipping(ninjaArmy, i, ui->clipOutputSpinBox->value(), papszOptions);
    if (ninjaErr != NINJA_SUCCESS)
    {
        qDebug() << "NinjaSetOutputBufferClipping: ninjaErr =" << ninjaErr;
        return false;
    }

    ninjaErr = NinjaSetGoogOutFlag(ninjaArmy, i, ui->googleEarthGroupBox->isChecked(), papszOptions);
    if (ninjaErr != NINJA_SUCCESS)
    {
        qDebug() << "NinjaSetGoogOutFlag: ninjaErr =" << ninjaErr;
        return false;
    }

    ninjaErr = NinjaSetGoogResolution(ninjaArmy, i, ui->googleEarthMeshResolutionSpinBox->value(), ui->googleEarthMeshResolutionComboBox->itemData(ui->googleEarthMeshResolutionComboBox->currentIndex()).toString().toUtf8().constData(), papszOptions);
    if (ninjaErr != NINJA_SUCCESS)
    {
        qDebug() << "NinjaSetGoogResolution: ninjaErr =" << ninjaErr;
        return false;
    }

    ninjaErr = NinjaSetGoogSpeedScaling(ninjaArmy, i, ui->legendComboBox->itemData(ui->legendComboBox->currentIndex()).toString().toUtf8().constData(), papszOptions);
    if (ninjaErr != NINJA_SUCCESS)
    {
        qDebug() << "NinjaSetGoogSpeedScaling: ninjaErr =" << ninjaErr;
        return false;
    }

    ninjaErr = NinjaSetGoogLineWidth(ninjaArmy, i, ui->googleEarthVectorsSpinBox->value(), papszOptions);
    if (ninjaErr != NINJA_SUCCESS)
    {
        qDebug() << "NinjaSetGoogLineWidth: ninjaErr =" << ninjaErr;
        return false;
    }

    ninjaErr = NinjaSetGoogColor(ninjaArmy, i, ui->alternativeColorSchemeComboBox->itemData(ui->alternativeColorSchemeComboBox->currentIndex()).toString().toUtf8().constData(), ui->googleEarthVectorScalingCheckBox->isChecked(), papszOptions);
    if (ninjaErr != NINJA_SUCCESS)
    {
        qDebug() << "NinjaSetGoogColor: ninjaErr =" << ninjaErr;
        return false;
    }

    ninjaErr = NinjaSetGoogConsistentColorScale(ninjaArmy, i, ui->legendCheckBox->isChecked(), numNinjas, papszOptions);
    if (ninjaErr != NINJA_SUCCESS)
    {
        qDebug() << "NinjaSetGoogConsistentColorScale: ninjaErr =" << ninjaErr;
        return false;
    }

    ninjaErr = NinjaSetAsciiOutFlag(ninjaArmy, i, ui->fireBehaviorGroupBox->isChecked(), papszOptions);
    if (ninjaErr != NINJA_SUCCESS)
    {
        qDebug() << "NinjaSetAsciiOutFlag: ninjaErr =" << ninjaErr;
        return false;
    }

    ninjaErr = NinjaSetAsciiAaigridOutFlag(ninjaArmy, i, ui->fireBehaviorGroupBox->isChecked(), papszOptions);
    if (ninjaErr != NINJA_SUCCESS)
    {
        qDebug() << "NinjaSetAsciiAaigridOutFlag: ninjaErr =" << ninjaErr;
        return false;
    }

    ninjaErr = NinjaSetAsciiProjOutFlag(ninjaArmy, i, ui->fireBehaviorGroupBox->isChecked(), papszOptions);
    if (ninjaErr != NINJA_SUCCESS)
    {
        qDebug() << "NinjaSetAsciiProjOutFlag: ninjaErr =" << ninjaErr;
        return false;
    }

    ninjaErr = NinjaSetAsciiResolution(ninjaArmy, i, ui->fireBehaviorMeshResolutionSpinBox->value(), ui->fireBehaviorMeshResolutionComboBox->itemData(ui->fireBehaviorMeshResolutionComboBox->currentIndex()).toString().toUtf8().constData(), papszOptions);
    if (ninjaErr != NINJA_SUCCESS)
    {
        qDebug() << "NinjaSetAsciiResolution: ninjaErr =" << ninjaErr;
        return false;
    }

    ninjaErr = NinjaSetShpOutFlag(ninjaArmy, i, ui->shapeFilesGroupBox->isChecked(), papszOptions);
    if (ninjaErr != NINJA_SUCCESS)
    {
        qDebug() << "NinjaSetShpOutFlag: ninjaErr =" << ninjaErr;
        return false;
    }

    ninjaErr = NinjaSetShpResolution(ninjaArmy, i, ui->shapeFilesMeshResolutionSpinBox->value(), ui->shapeFilesMeshResolutionComboBox->itemData(ui->shapeFilesMeshResolutionComboBox->currentIndex()).toString().toUtf8().constData(), papszOptions);
    if (ninjaErr != NINJA_SUCCESS)
    {
        qDebug() << "NinjaSetShpResolution: ninjaErr =" << ninjaErr;
        return false;
    }

    ninjaErr = NinjaSetPDFOutFlag(ninjaArmy, i, ui->geospatialPDFFilesGroupBox->isChecked(), papszOptions);
    if (ninjaErr != NINJA_SUCCESS)
    {
        qDebug() << "NinjaSetPDFOutFlag: ninjaErr =" << ninjaErr;
        return false;
    }

    ninjaErr = NinjaSetPDFLineWidth(ninjaArmy, i, ui->geospatialPDFFilesVectorsSpinBox->value(), papszOptions);
    if (ninjaErr != NINJA_SUCCESS)
    {
        qDebug() << "NinjaSetPDFLineWidth: ninjaErr =" << ninjaErr;
        return false;
    }

    ninjaErr = NinjaSetPDFBaseMap(ninjaArmy, i, ui->basemapComboBox->currentIndex(), papszOptions);
    if (ninjaErr != NINJA_SUCCESS)
    {
        qDebug() << "NinjaSetPDFBaseMap: ninjaErr =" << ninjaErr;
        return false;
    }

    ninjaErr = NinjaSetPDFDEM(ninjaArmy, i, ui->elevationInputFileLineEdit->property("fullpath").toString().toUtf8().constData(), papszOptions);
    if (ninjaErr != NINJA_SUCCESS)
    {
        qDebug() << "NinjaSetPDFDEM: ninjaErr =" << ninjaErr;
        return false;
    }

    ninjaErr = NinjaSetPDFSize(ninjaArmy, i, PDFSize.PDFHeight, PDFSize.PDFWidth, PDFSize.PDFDpi, papszOptions);
    if (ninjaErr != NINJA_SUCCESS)
    {
        qDebug() << "NinjaSetPDFSize: ninjaErr =" << ninjaErr;
        return false;
    }

    ninjaErr = NinjaSetPDFResolution(ninjaArmy, i, ui->geospatialPDFFilesMeshResolutionSpinBox->value(), ui->geospatialPDFFilesMeshResolutionComboBox->itemData(ui->geospatialPDFFilesMeshResolutionComboBox->currentIndex()).toString().toUtf8().constData(), papszOptions);
    if (ninjaErr != NINJA_SUCCESS)
    {
        qDebug() << "NinjaSetPDFResolution: ninjaErr =" << ninjaErr;
        return false;
    }

    ninjaErr = NinjaSetVtkOutFlag(ninjaArmy, i, ui->VTKFilesCheckBox->isChecked(), papszOptions);
    if (ninjaErr != NINJA_SUCCESS)
    {
        qDebug() << "NinjaSetVtkOutFlag: ninjaErr =" << ninjaErr;
        return false;
    }

    ninjaErr = NinjaSetFlatGeoBufFlag(ninjaArmy, i, ui->mapVisualizationCheckBox->isChecked(), papszOptions);
    if (ninjaErr != NINJA_SUCCESS)
    {
        qDebug() << "NinjaSetFlatGeoBufFlag: ninjaErr =" << ninjaErr;
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
        progressDialog->setLabelText("Rendering map layers...");
        progressDialog->setCancelButtonText("Cancel");

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

    ui->outputDirectoryOpenButton->setDisabled(false);
    outputDir.setPath(ui->outputDirectoryLineEdit->text());

    // one more process to do after finishedSolve() stuff
    disconnect(mapBridge, &MapBridge::mapLayersLoadingFinishedSignal,
               menuBar, &MenuBar::kmzLoadFinished);
    connect(mapBridge, &MapBridge::mapLayersLoadingFinishedSignal,
            this, &MainWindow::finishedLoadingMap,
            Qt::UniqueConnection);
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
        char **stationKmlFilenames = NULL;
        char **weatherModelKmzFilenames = NULL;

        char **papszOptions = nullptr;
        ninjaErr = NinjaGetRunKmzFilenames(ninjaArmy, &numRuns, &kmzFilenames, &stationKmlFilenames, &weatherModelKmzFilenames, papszOptions);
        if(ninjaErr != NINJA_SUCCESS)
        {
            printf("NinjaGetRunKmzFilenames: ninjaErr = %d\n", ninjaErr);
        }

        std::vector<std::string> kmzFilenamesStr;
        std::vector<std::string> stationKmlFilenamesStr;
        std::vector<std::string> wxModelKmzFilenamesStr;

        kmzFilenamesStr.reserve(numRuns);
        stationKmlFilenamesStr.reserve(numRuns);
        wxModelKmzFilenamesStr.reserve(numRuns);
        for(int i = 0; i < numRuns; i++)
        {
            kmzFilenamesStr.emplace_back(kmzFilenames[i]);
            stationKmlFilenamesStr.emplace_back(stationKmlFilenames[i]);
            wxModelKmzFilenamesStr.emplace_back(weatherModelKmzFilenames[i]);
        }

        outputKmzFilenames.push_back(std::move( kmzFilenamesStr ));
        outputStationKmlFilenames.push_back(std::move( stationKmlFilenamesStr ));
        outputWxModelKmzFilenames.push_back(std::move( wxModelKmzFilenamesStr ));

        for(int i = 0; i < numRuns; i++)
        {
            // plot the output kmz of the run
            QString outFileStr = QString::fromStdString(kmzFilenames[i]);
            qDebug() << "kmz outFile =" << outFileStr;

            webEngineView->page()->runJavaScript("clearWindNinjaOutputTree();");
            webEngineView->page()->runJavaScript("clearInitializationOutputTree();");
            webEngineView->page()->runJavaScript("clearStationOutputTree();");
            webEngineView->page()->runJavaScript("clearUnknownOutputTree();");
            QString filePath = QUrl::fromLocalFile(outFileStr).toString();
            QFileInfo info(outFileStr);
            QString fileName = info.fileName();
            qDebug() << "file url =" << filePath;
            QString jsCall = QString("loadSimulation('%1', '%2');").arg(filePath, fileName);
            webEngineView->page()->runJavaScript(jsCall);

            // if it is a point initialization run, and station kmls were created for the run,
            // then plot the station kmls of the run
            if(ui->pointInitializationGroupBox->isChecked() && ui->pointInitializationWriteStationKMLCheckBox->isChecked())
            {
                QString outFileStr = QString::fromStdString(stationKmlFilenames[i]);
                qDebug() << "station kml outFile =" << outFileStr;

                QString filePath = QUrl::fromLocalFile(outFileStr).toString();
                QFileInfo info(outFileStr);
                QString fileName = info.fileName();
                QString jsCall = QString("loadSimulation('%1', '%2');").arg(filePath, fileName);
                webEngineView->page()->runJavaScript(jsCall);
            }

            // if it is a weather model run, and weather model kmzs were created for the run,
            // then plot the weather model kmz of the run
            if(ui->weatherModelGroupBox->isChecked() && ui->rawWeatherModelOutputCheckBox->isChecked())
            {
                QString outFileStr = QString::fromStdString(weatherModelKmzFilenames[i]);
                qDebug() << "wx model kmz outFile =" << outFileStr;

                QString filePath = QUrl::fromLocalFile(outFileStr).toString();
                QFileInfo info(outFileStr);
                QString fileName = info.fileName();
                QString jsCall = QString("loadSimulation('%1', '%2');").arg(filePath, fileName);
                webEngineView->page()->runJavaScript(jsCall);
            }
        }

        ninjaErr = NinjaDestroyRunKmzFilenames(numRuns, kmzFilenames, stationKmlFilenames, weatherModelKmzFilenames, papszOptions);
        if(ninjaErr != NINJA_SUCCESS)
        {
            printf("NinjaDestroyRunKmzFilenames: ninjaErr = %d\n", ninjaErr);
        }

    } // if(result == 1 && !progressDialog->wasCanceled() && ui->googleEarthGroupBox->isChecked() == true)
}

void MainWindow::finishedLoadingMap()
{
    progressDialog->setValue(maxProgress);
    progressDialog->setLabelText("Simulation Finished.");
    progressDialog->setCancelButtonText("Close");
    disconnect(mapBridge, &MapBridge::mapLayersLoadingFinishedSignal, this, &MainWindow::finishedLoadingMap);
    connect(mapBridge, &MapBridge::mapLayersLoadingFinishedSignal, menuBar, &MenuBar::kmzLoadFinished);
}

void MainWindow::writeSettings()
{
    writeToConsole("Saving settings...");

    QString app(NINJA_VERSION_STRING);
    app = "WindNinja-" + app;
    QSettings settings(QSettings::UserScope, "Firelab", app);
    settings.setDefaultFormat(QSettings::IniFormat);

    QString demFilePath = ui->elevationInputFileLineEdit->property("fullpath").toString();
    QFileInfo demFileInfo(demFilePath);
    settings.setValue("inputFileDir", demFileInfo.absolutePath());
    settings.setValue("momentumFlag", ui->momentumSolverCheckBox->isChecked());
    settings.setValue("vegChoice", ui->vegetationComboBox->currentIndex());
    settings.setValue("meshChoice", ui->meshResolutionComboBox->currentIndex());
    settings.setValue("meshUnits", ui->meshResolutionUnitsComboBox->currentIndex());
    settings.setValue("customRes", ui->meshResolutionSpinBox->value());
    settings.setValue("nProcessors", ui->numberOfProcessorsSpinBox->value());

    writeToConsole("Settings saved successfully.", Qt::darkGreen);
}

void MainWindow::readSettings()
{
    writeToConsole("Reading settings...");

    QString app(NINJA_VERSION_STRING);
    app = "WindNinja-" + app;
    QSettings settings(QSettings::UserScope, "Firelab", app);
    settings.setDefaultFormat(QSettings::IniFormat);

    if(settings.contains("inputFileDir"))
    {
        QFileInfo dir(settings.value("inputFileDir").toString());
        if (dir.exists())
        {
            surfaceInput->setInputFileDir(dir.absoluteFilePath());
        }
    }
    if(settings.contains("momentumFlag") && settings.value("momentumFlag").toBool())
    {
        ui->momentumSolverCheckBox->click();
    }
    if(settings.contains("vegChoice"))
    {
        ui->vegetationComboBox->setCurrentIndex(settings.value("vegChoice").toInt());
    }
    if(settings.contains("meshChoice"))
    {
        ui->meshResolutionUnitsComboBox->setCurrentIndex(settings.value("meshUnits").toInt());
        int choice = settings.value("meshChoice", 0).toInt();
        ui->meshResolutionComboBox->setCurrentIndex(choice);
        if(choice == 3 && settings.contains("customRes"))
        {
            ui->meshResolutionSpinBox->setValue(settings.value("customRes").toDouble());
        }
    }
    if(settings.contains("nProcessors"))
    {
        ui->numberOfProcessorsSpinBox->setValue(settings.value("nProcessors").toInt());
    }

    writeToConsole("Settings read successfully.", Qt::darkGreen);
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
