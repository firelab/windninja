 /******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja Qt GUI
 * Purpose:  Handles GUI related logic for Point Initialization Page
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

#include "pointInitializationInput.h"

PointInitializationInput::PointInitializationInput(Ui::MainWindow* ui, QObject* parent)
    : QObject(parent),
    ui(ui)
{    
    ui->pointInitializationDataTimeStackedWidget->setCurrentIndex(0);
    ui->weatherStationDataSourceStackedWidget->setCurrentIndex(0);
    ui->weatherStationDataTimeStackedWidget->setCurrentIndex(0);
    ui->weatherStationDataStartDateTimeEdit->setDateTime(QDateTime::currentDateTime());
    ui->weatherStationDataEndDateTimeEdit->setDateTime(QDateTime::currentDateTime());

    updateDateTime();

    connect(ui->pointInitializationGroupBox, &QGroupBox::toggled, this, &PointInitializationInput::pointInitializationGroupBoxToggled);
    connect(ui->pointInitializationDownloadDataButton, &QPushButton::clicked, this, &PointInitializationInput::pointInitializationDownloadDataButtonClicked);
    connect(ui->downloadBetweenDatesStartTimeDateTimeEdit, &QDateTimeEdit::dateTimeChanged, this, &PointInitializationInput::weatherStationDownloadBetweenDatesStartTimeDateTimeEditChanged);
    connect(ui->downloadBetweenDatesEndTimeDateTimeEdit, &QDateTimeEdit::dateTimeChanged, this, &PointInitializationInput::weatherStationDownloadBetweenDatesEndTimeDateTimeEditChanged);
    connect(ui->weatherStationDataDownloadCancelButton, &QPushButton::clicked, this, &PointInitializationInput::weatherStationDataDownloadCancelButtonClicked);
    connect(ui->weatherStationDataSourceComboBox, &QComboBox::currentIndexChanged, this, &PointInitializationInput::weatherStationDataSourceComboBoxCurrentIndexChanged);
    connect(ui->weatherStationDataTimeComboBox, &QComboBox::currentIndexChanged, this, &PointInitializationInput::weatherStationDataTimeComboBoxCurrentIndexChanged);
    connect(ui->weatherStationDataDownloadButton, &QPushButton::clicked, this, &PointInitializationInput::weatherStationDataDownloadButtonClicked);
    connect(ui->pointInitializationSelectAllButton, &QPushButton::clicked, this, &PointInitializationInput::pointInitializationSelectAllButtonClicked);
    connect(ui->pointInitializationSelectNoneButton, &QPushButton::clicked, this, &PointInitializationInput::pointInitializationSelectNoneButtonClicked);
    connect(ui->pointInitializationTreeView, &QTreeView::expanded, this, &PointInitializationInput::folderExpanded);
    connect(ui->pointInitializationTreeView, &QTreeView::collapsed, this, &PointInitializationInput::folderCollapsed);
    connect(ui->weatherStationDataTimestepsSpinBox, &QSpinBox::valueChanged, this, &PointInitializationInput::weatherStationDataTimestepsSpinBoxValueChanged);
    connect(ui->weatherStationDataStartDateTimeEdit, &QDateTimeEdit::dateTimeChanged, this, &PointInitializationInput::weatherStationDataStartDateTimeEditChanged);
    connect(ui->weatherStationDataEndDateTimeEdit, &QDateTimeEdit::dateTimeChanged, this, &PointInitializationInput::weatherStationDataEndDateTimeEditChanged);
    connect(ui->timeZoneComboBox, &QComboBox::currentIndexChanged, this, &PointInitializationInput::updateDateTime);
    connect(this, &PointInitializationInput::updateState, &AppState::instance(), &AppState::updatePointInitializationInputState);

    connect(this, &PointInitializationInput::updateProgressMessageSignal, this, &PointInitializationInput::updateProgressMessage, Qt::QueuedConnection);
}

void PointInitializationInput::pointInitializationGroupBoxToggled()
{
    if(ui->pointInitializationGroupBox->isChecked())
    {
        ui->domainAverageGroupBox->setChecked(false);
        ui->weatherModelGroupBox->setChecked(false);
    }

    emit updateState();
}

void PointInitializationInput::pointInitializationDownloadDataButtonClicked()
{
    ui->weatherStationDataSourceComboBox->setCurrentIndex(0);
    ui->weatherStationDataTimeComboBox->setCurrentIndex(0);

    ui->downloadFromStationIDLineEdit->clear();
    ui->downloadFromDEMSpinBox->setValue(0);

    ui->inputsStackedWidget->setCurrentIndex(17);
}

void PointInitializationInput::weatherStationDownloadBetweenDatesStartTimeDateTimeEditChanged()
{
    if(ui->downloadBetweenDatesEndTimeDateTimeEdit->dateTime() < ui->downloadBetweenDatesStartTimeDateTimeEdit->dateTime())
    {
        emit writeToConsoleSignal("Start Time is greater than End Time!, fixing End Time...");
        CPLDebug("STATION_FETCH", "START TIME > END TIME, FIXING END TIME");
        ui->downloadBetweenDatesEndTimeDateTimeEdit->setDateTime(ui->downloadBetweenDatesStartTimeDateTimeEdit->dateTime().addSecs(3600));
    }
    updateTimeSteps();
}

void PointInitializationInput::weatherStationDownloadBetweenDatesEndTimeDateTimeEditChanged()
{
    if(ui->downloadBetweenDatesEndTimeDateTimeEdit->dateTime() < ui->downloadBetweenDatesStartTimeDateTimeEdit->dateTime())
    {
        emit writeToConsoleSignal("Start Time is greater than End Time!, fixing Start Time...");
        CPLDebug("STATION_FETCH", "START TIME > END TIME, FIXING START TIME");
        ui->downloadBetweenDatesStartTimeDateTimeEdit->setDateTime(ui->downloadBetweenDatesEndTimeDateTimeEdit->dateTime().addSecs(-3600));
    }
    updateTimeSteps();
}

void PointInitializationInput::weatherStationDataDownloadCancelButtonClicked()
{
    ui->pointInitializationTreeView->collapseAll();
    ui->inputsStackedWidget->setCurrentIndex(7);
}

void PointInitializationInput::updateProgressMessage(const QString message)
{
    if(progress)
    {
        progress->setLabelText(message);
        progress->setWindowTitle(tr("Error"));
        progress->setCancelButtonText(tr("Close"));
        progress->setAutoClose(false);
        progress->setAutoReset(false);
        progress->setRange(0, 1);
        progress->setValue(progress->maximum());
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

static void comMessageHandler(const char *pszMessage, void *pUser)
{
    PointInitializationInput *self = static_cast<PointInitializationInput*>(pUser);

    std::string msg = pszMessage;

    // both the writeToConsole() function and the QProgressDialog do NOT like having a "\n" at the end of a given message,
    // writeToConsole() adds extra empty lines all over the place and the QProgressDialog adds a weird looking space between the message and the progress bar.
    // but ninjaCom likes to add "\n" to stuff and currently sends one "\n". So need to strip the "\n" character off of the message.
    if( msg.substr(msg.size()-1, 1) == "\n")
    {
        msg = msg.substr(0, msg.size()-1);
    }

    size_t pos;
    size_t startPos;
    size_t endPos;
    std::string clipStr;

    if( msg.find("Exception caught: ") != msg.npos || msg.find("ERROR: ") != msg.npos )
    {
        if( msg.find("Exception caught: ") != msg.npos )
        {
            pos = msg.find("Exception caught: ");
            startPos = pos+18;
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
        if( clipStr == "Cannot determine exception type." )
        {
            emit self->updateProgressMessageSignal(QString::fromStdString("StationFetch ended with unknown error"));
            emit self->writeToConsoleSignal(QString::fromStdString("unknown StationFetch error"), Qt::red);
        }
        else
        {
            emit self->updateProgressMessageSignal(QString::fromStdString("StationFetch ended in error:\n"+clipStr));
            emit self->writeToConsoleSignal(QString::fromStdString("StationFetch error: "+clipStr), Qt::red);
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
        emit self->updateProgressMessageSignal(QString::fromStdString("StationFetch ended in warning:\n"+clipStr));
        emit self->writeToConsoleSignal(QString::fromStdString("StationFetch warning: "+clipStr), QColor(255, 140, 0));
    }
    else
    {
        emit self->updateProgressMessageSignal(QString::fromStdString(msg));
        emit self->writeToConsoleSignal(QString::fromStdString(msg));
    }
}

void PointInitializationInput::weatherStationDataDownloadButtonClicked()
{
    emit writeToConsoleSignal("Downloading station data...");

    progress = new QProgressDialog("Downloading Station Data...", QString(), 0, 0, ui->centralwidget);
    progress->setWindowModality(Qt::WindowModal);
    progress->setCancelButton(nullptr);
    progress->setMinimumDuration(0);
    progress->setAutoClose(true);
    progress->show();

    NinjaToolsH* ninjaTools = NinjaMakeTools();

    char **papszOptions = NULL;
    NinjaErr ninjaErr = NinjaSetToolsComMessageHandler(ninjaTools, &comMessageHandler, this, papszOptions);
    if(ninjaErr != NINJA_SUCCESS)
    {
        qDebug() << "NinjaSetToolsComMessageHandler(): ninjaErr =" << ninjaErr;
    }

    CPLDebug("STATION_FETCH", "Fetch Station GUI Function");
    CPLDebug("STATION_FETCH", "---------------------------------------");

    QString DEMTimeZone = ui->timeZoneComboBox->currentText();
    QByteArray DEMTimeZoneBytes = ui->timeZoneComboBox->currentText().toUtf8();
    QDateTime start = ui->downloadBetweenDatesStartTimeDateTimeEdit->dateTime();
    QDateTime end = ui->downloadBetweenDatesEndTimeDateTimeEdit->dateTime();

    CPLDebug("STATION_FETCH", "DEM FILE NAME: %s", QFileInfo(ui->elevationInputFileLineEdit->property("fullpath").toString()).absoluteFilePath().toStdString().c_str());
    CPLDebug("STATION_FETCH", "TIME ZONE: %s", DEMTimeZone.toStdString().c_str());
    CPLDebug("STATION_FETCH", "geoLoc: %i, (0=\"Download From DEM\",1=\"Download From Station ID\")", ui->weatherStationDataSourceComboBox->currentIndex());
    CPLDebug("STATION_FETCH", "timeLoc: %i, (0=\"Download Most Recent Data\",1=\"Download Between Two Dates\")", ui->weatherStationDataTimeComboBox->currentIndex());
    CPLDebug("STATION_FETCH", "---------------------------------------");
    CPLDebug("STATION_FETCH", "USING DEM: %s", ui->elevationInputFileLineEdit->text().toStdString().c_str());

    QVector<int> year   = {start.date().year(),   end.date().year()};
    QVector<int> month  = {start.date().month(),  end.date().month()};
    QVector<int> day    = {start.date().day(),    end.date().day()};
    QVector<int> hour   = {start.time().hour(),   end.time().hour()};
    QVector<int> minute = {start.time().minute(), end.time().minute()};
    QVector<int> outYear(2), outMonth(2), outDay(2), outHour(2), outMinute(2);

    ninjaErr = NinjaGetTimeList(
        ninjaTools,
        year.data(), month.data(), day.data(),
        hour.data(), minute.data(),
        outYear.data(), outMonth.data(), outDay.data(),
        outHour.data(), outMinute.data(),
        2, DEMTimeZoneBytes.data()
        );
    if(ninjaErr != NINJA_SUCCESS)
    {
        qDebug() << "NinjaGetTimeList: ninjaErr =" << ninjaErr;

        progress->setWindowTitle(tr("Error"));
        progress->setCancelButtonText("Close");
        progress->setAutoClose(false);
        progress->setAutoReset(false);
        progress->setRange(0, 1);
        progress->setValue(progress->maximum());

        // do cleanup before the return, similar to finishedSolve()

//        ninjaErr = NinjaDestroyTools(ninjaTools, papszOptions);
//        if(ninjaErr != NINJA_SUCCESS)
//        {
//            printf("NinjaDestroyTools: ninjaErr = %d\n", ninjaErr);
//        }

        //futureWatcher->deleteLater();

        return;
    }

    if(ui->weatherStationDataTimeComboBox->currentIndex() == 1)
    {
        ninjaErr = NinjaCheckTimeDuration(ninjaTools, outYear.data(), outMonth.data(), outDay.data(), outHour.data(), outMinute.data(), 2, papszOptions);
        if(ninjaErr != NINJA_SUCCESS)
        {
            qDebug() << "NinjaCheckTimeDuration ninjaErr=" << ninjaErr;

            progress->setWindowTitle(tr("Error"));
            progress->setCancelButtonText("Close");
            progress->setAutoClose(false);
            progress->setAutoReset(false);
            progress->setRange(0, 1);
            progress->setValue(progress->maximum());

            // do cleanup before the return, similar to finishedSolve()

//            ninjaErr = NinjaDestroyTools(ninjaTools, papszOptions);
//            if(ninjaErr != NINJA_SUCCESS)
//            {
//                printf("NinjaDestroyTools: ninjaErr = %d\n", ninjaErr);
//            }

            //futureWatcher->deleteLater();

            return;
        }
    }

    bool fetchLatestFlag = ui->weatherStationDataTimeComboBox->currentIndex() ? 0 : 1;
    QString outputPath = ui->outputDirectoryLineEdit->text();
    QString elevationFile = ui->elevationInputFileLineEdit->property("fullpath").toString();

    futureWatcher = new QFutureWatcher<int>(this);
    QFuture<int> future;
    if(ui->weatherStationDataSourceComboBox->currentIndex() == 0)
    {
        if(fetchLatestFlag == true)
        {
            CPLDebug("STATION_FETCH", "Fetch Params: DEM and Current Data");
        }
        else
        {
            CPLDebug("STATION_FETCH", "Fetch Params: DEM and Time series");
        }
        QString units = ui->downloadFromDEMComboBox->currentText();
        double buffer = ui->downloadFromDEMSpinBox->value();
        future = QtConcurrent::run(&PointInitializationInput::fetchStationFromBbox, this,
                                   ninjaTools,
                                   outYear, outMonth, outDay, outHour, outMinute,
                                   elevationFile, buffer, units,
                                   DEMTimeZone, fetchLatestFlag, outputPath);
    }
    else
    {
        if(fetchLatestFlag == true)
        {
            CPLDebug("STATION_FETCH", "STID and Current Data");
        }
        else
        {
            CPLDebug("STATION_FETCH", "STID and Timeseries");
        }
        QString stationList = ui->downloadFromStationIDLineEdit->text();
        future = QtConcurrent::run(&PointInitializationInput::fetchStationByName, this,
                                   ninjaTools,
                                   outYear, outMonth, outDay, outHour, outMinute,
                                   elevationFile, stationList,
                                   DEMTimeZone, fetchLatestFlag, outputPath);
    }
    futureWatcher->setFuture(future);

    connect(futureWatcher, &QFutureWatcher<int>::finished, this, &PointInitializationInput::fetchStationDataFinished);
    connect(progress, &QProgressDialog::canceled, this, &PointInitializationInput::fetchStationDataFinished);
}

int PointInitializationInput::fetchStationFromBbox(NinjaToolsH* ninjaTools,
                                                   QVector<int> year,
                                                   QVector<int> month,
                                                   QVector<int> day,
                                                   QVector<int> hour,
                                                   QVector<int> minute,
                                                   QString elevationFile,
                                                   double buffer,
                                                   QString units,
                                                   QString osTimeZone,
                                                   bool fetchLatestFlag,
                                                   QString outputPath)
{
    char ** options = NULL;
    NinjaErr ninjaErr = NinjaFetchStationFromBBox(
        ninjaTools,
        year.data(), month.data(), day.data(),
        hour.data(), minute.data(), year.size(),
        elevationFile.toUtf8().constData(), buffer,
        units.toUtf8().constData(), osTimeZone.toUtf8().constData(),
        fetchLatestFlag, outputPath.toUtf8().constData(),
        false, options
        );

    if (ninjaErr != NINJA_SUCCESS)
    {
        qDebug() << "NinjaFetchStationFromBbox: ninjaErr =" << ninjaErr;
    }

    return ninjaErr;
}

int PointInitializationInput::fetchStationByName(NinjaToolsH* ninjaTools,
                                                 QVector<int> year,
                                                 QVector<int> month,
                                                 QVector<int> day,
                                                 QVector<int> hour,
                                                 QVector<int> minute,
                                                 QString elevationFile,
                                                 QString stationList,
                                                 QString osTimeZone,
                                                 bool fetchLatestFlag,
                                                 QString outputPath)
{
    char ** options = NULL;
    NinjaErr ninjaErr = NinjaFetchStationByName(
        ninjaTools,
        year.data(), month.data(), day.data(),
        hour.data(), minute.data(), year.size(),
        elevationFile.toUtf8().constData(), stationList.toUtf8().constData(),
        osTimeZone.toUtf8().constData(), fetchLatestFlag,
        outputPath.toUtf8().constData(), false, options
        );

    if (ninjaErr != NINJA_SUCCESS)
    {
        qDebug() << "NinjaFetchStationByName: ninjaErr =" << ninjaErr;
    }

    return ninjaErr;
}

void PointInitializationInput::fetchStationDataFinished()
{
    if(!futureWatcher)
    {
        return;
    }

    if(progress && progress->wasCanceled())
    {
        futureWatcher->waitForFinished();
    }
    else
    {
        // get the return value of the QtConcurrent::run() function
        int result = futureWatcher->future().result();
        CPLDebug("STATION_FETCH", "station fetch return value: %i", result);

        if(result == NINJA_SUCCESS)
        {
            emit writeToConsoleSignal("Finished downloading station data.", Qt::darkGreen);

            if (progress)
            {
                progress->close();
                progress->deleteLater();
                progress = nullptr;
            }

            ui->inputsStackedWidget->setCurrentIndex(7);

        } else
        {
            emit writeToConsoleSignal("Failed to download station data.");
        }
    }
    // delete the futureWatcher every time, whether success or failure
    if (futureWatcher)
    {
        futureWatcher->deleteLater();
        futureWatcher = nullptr;
    }
}

void PointInitializationInput::weatherStationDataSourceComboBoxCurrentIndexChanged(int index)
{
    ui->weatherStationDataSourceStackedWidget->setCurrentIndex(index);
}

void PointInitializationInput::weatherStationDataTimeComboBoxCurrentIndexChanged(int index)
{
    ui->weatherStationDataTimeStackedWidget->setCurrentIndex(index);
}

void PointInitializationInput::updateTreeView()
{
    AppState& state = AppState::instance();
    state.isStationFileSelectionValid = false;
    emit updateState();

    stationFileSystemModel = new QFileSystemModel(this);
    QString path = ui->elevationInputFileLineEdit->property("fullpath").toString();
    QFileInfo fileInfo(path);

    stationFileSystemModel->setRootPath(fileInfo.absolutePath());
    stationFileSystemModel->setNameFilters({"*.csv", "WXSTATIONS-*"});
    stationFileSystemModel->setFilter(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot);
    stationFileSystemModel->setNameFilterDisables(false);


    ui->pointInitializationTreeView->setModel(stationFileSystemModel);
    ui->pointInitializationTreeView->setRootIndex(stationFileSystemModel->index(fileInfo.absolutePath()));
    ui->pointInitializationTreeView->setSelectionMode(QAbstractItemView::MultiSelection);
    ui->pointInitializationTreeView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->pointInitializationTreeView->setAnimated(true);
    ui->pointInitializationTreeView->header()->setSectionResizeMode(QHeaderView::Stretch);
    ui->pointInitializationTreeView->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->pointInitializationTreeView->header()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    ui->pointInitializationTreeView->setUniformRowHeights(true);
    ui->pointInitializationTreeView->hideColumn(1);
    ui->pointInitializationTreeView->hideColumn(2);

    connect(ui->pointInitializationTreeView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &PointInitializationInput::pointInitializationTreeViewItemSelectionChanged);
}

void PointInitializationInput::pointInitializationTreeViewItemSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    AppState& state = AppState::instance();
    state.isStationFileSelected = false;
    state.isStationDataValid = false;
    state.isStationFileSelectionValid = false;

    QModelIndexList selectedRows = ui->pointInitializationTreeView->selectionModel()->selectedRows();
    CPLDebug("STATION_FETCH", "========================================");
    CPLDebug("STATION_FETCH", "NUMBER OF SELECTED STATIONS: %lli", selectedRows.count());

    stationFiles.clear();
    stationFileTypes.clear();

    maxStationLocalDateTime = QDateTime();
    minStationLocalDateTime = QDateTime();

    if(selectedRows.count() > 0)
    {
        state.isStationFileSelected = true;
    }

    for(int i = 0; i < selectedRows.count(); i++)
    {
        // If its a directory, make it so that it can't be selected
        if(stationFileSystemModel->isDir(selectedRows[i]))
        {
            CPLDebug("STATION_FETCH", "IGNORING SELECTED DIRECTORY!");
            ui->pointInitializationTreeView->selectionModel()->select(selectedRows[i], QItemSelectionModel::Deselect | QItemSelectionModel::Rows);
            // no change to appState for this operation
            return;
        }

        CPLDebug("STATION_FETCH", "----------------------------------------");
        CPLDebug("STATION_FETCH", "STATION NAME: %s", stationFileSystemModel->filePath(selectedRows[i]).toStdString().c_str());

        QString recentFileSelected = stationFileSystemModel->filePath(selectedRows[i]);
        stationFiles.push_back(recentFileSelected);  // note, selected vs valid are two separate things
        //qDebug() << "[GUI-Point] Selected file path:" << recentFileSelected;
        CPLDebug("STATION_FETCH", "Selected file path: %s", recentFileSelected.toStdString().c_str());

        QByteArray filePathBytes = recentFileSelected.toUtf8();
        const char* filePath = filePathBytes.constData();
        char** options = nullptr;
        int stationHeader = NinjaGetWxStationHeaderVersion(filePath, options);
        //qDebug() << "[GUI-Point] Station Header: " << stationHeader;
        CPLDebug("STATION_FETCH", "STATION HEADER TYPE: %i", stationHeader);

        if(stationHeader == 1)
        {
            emit writeToConsoleSignal("Station has old station format, which is no longer allowed!");
            state.isStationDataValid = false;
            emit updateState();
            return;
        }
        else if(stationHeader == -1)
        {
            emit writeToConsoleSignal("Station has invalid header data");
            state.isStationDataValid = false;
            emit updateState();
            return;
        }

        bool timeSeriesFlag = true;
        if(stationHeader != 1)
        {
            GDALDataset* hDS = (GDALDataset*) GDALOpenEx(
                filePath,
                GDAL_OF_VECTOR | GDAL_OF_READONLY,
                NULL,
                NULL,
                NULL
            );

            if(hDS == NULL)
            {
                emit writeToConsoleSignal("Cannot open station file!");
                state.isStationDataValid = false;
                emit updateState();
                return;
            }

            OGRLayer* poLayer = hDS->GetLayer(0);
            poLayer->ResetReading();
            qint64 lastIndex = poLayer->GetFeatureCount();  // How many lines are on disk
            //qDebug() << "[GUI-Point] Number of Time Entries:" << lastIndex;
            CPLDebug("STATION_FETCH", "Number of Time Entries: %llu", lastIndex);

            OGRFeature* poFeature = poLayer->GetFeature(1);         // Skip header, row 1 is first time in series
            if(poFeature == NULL)
            {
                emit writeToConsoleSignal("No station data found in file!");
                state.isStationDataValid = false;
                emit updateState();
                return;
            }

            QString startDateTimeStr(poFeature->GetFieldAsString(15)); // Time should be in 15th (last) column (0-14)

            poFeature = poLayer->GetFeature(lastIndex);             // last time in series
            QString stopDateTimeStr(poFeature->GetFieldAsString(15));

            //qDebug() << "[GUI-Point] Station start time:" << startDateTimeStr;
            //qDebug() << "[GUI-Point] Station end   time:" <<  stopDateTimeStr;
            CPLDebug("STATION_FETCH", "STATION START TIME: %s", startDateTimeStr.toStdString().c_str());
            CPLDebug("STATION_FETCH", "STATION END   TIME: %s",  stopDateTimeStr.toStdString().c_str());

            if(startDateTimeStr.isEmpty() && stopDateTimeStr.isEmpty())  // No time series
            {
                //qDebug() << "[GUI-Point] File cannot be used for Time Series";
                CPLDebug("STATION_FETCH", "File cannot be used for Time Series");
                timeSeriesFlag = false;
                stationFileTypes.push_back(0);
            }
            else if(!startDateTimeStr.isEmpty() && !stopDateTimeStr.isEmpty())  // Some type of time series
            {
                //qDebug() << "[GUI-Point] File can be used for Time Series, suggesting time series parameters...";
                CPLDebug("STATION_FETCH", "File can be used for Times Series, suggesting time series parameters...");
                CPLDebug("STATION_FETCH", "Suggesting Potentially Reasonable Time Series Parameters...");
                readStationTime(startDateTimeStr, stopDateTimeStr);
                stationFileTypes.push_back(1);
            }
        }

        CPLDebug("STATION_FETCH", "Type of Station File: %i", stationFileTypes[i]);

        ui->pointInitializationDataTimeStackedWidget->setCurrentIndex(timeSeriesFlag ? 0 : 1);

        if (!timeSeriesFlag)
        {
            QDateTime dateModified = QFileInfo(recentFileSelected).birthTime();
            ui->weatherStationDataLabel->setText("Simulation time set to: " + dateModified.toString());
            ui->weatherStationDataLabel->setProperty("simulationTime", dateModified);
        }
        ui->pointInitializationTreeView->setProperty("timeSeriesFlag", timeSeriesFlag);
    }
    state.isStationDataValid = true;

    state.isStationFileSelectionValid = true;
    for(int i = 0; i < stationFileTypes.size(); i++)
    {
        CPLDebug("STATION_FETCH", "stationFileTypes[%i] = %i", i, stationFileTypes[i]);
        if(stationFileTypes[i] != stationFileTypes[0])
        {
            CPLDebug("STATION_FETCH", "found unique stationFileType at: %i", i);
            CPLDebug("STATION_FETCH", "WARNING NOT ALL CSVS ARE OF THE SAME TYPE, CANNOT CONTINUE");
            state.isStationFileSelectionValid = false;
            break;
        }
    }

    emit updateState();
}

void PointInitializationInput::pointInitializationSelectAllButtonClicked()
{
    QItemSelectionModel *selectionModel = ui->pointInitializationTreeView->selectionModel();

    QModelIndex folderIndex;
    if (openStationFolders.isEmpty())
    {
        QFileInfo fileInfo(ui->elevationInputFileLineEdit->property("fullpath").toString());
        folderIndex = stationFileSystemModel->index(fileInfo.absolutePath());
    }
    else
    {
        folderIndex = stationFileSystemModel->index(openStationFolders.back());
    }

    int rowCount = stationFileSystemModel->rowCount(folderIndex);
    QItemSelection selection;
    for (int i = 0; i < rowCount; i++) {
        QModelIndex stationFile = stationFileSystemModel->index(i, 0, folderIndex);

        if (stationFileSystemModel->isDir(stationFile))
            continue;

        selection.select(stationFile, stationFile);
    }

    selectionModel->select(selection, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
}

void PointInitializationInput::readStationTime(QString startDateTimeStr, QString stopDateTimeStr)
{
    QTimeZone timeZone(ui->timeZoneComboBox->currentText().toUtf8());

    QDateTime startUtcDateTime = QDateTime::fromString(startDateTimeStr, Qt::ISODate);
    QDateTime stopUtcDateTime  = QDateTime::fromString( stopDateTimeStr, Qt::ISODate);

    QDateTime startLocalDateTime = startUtcDateTime.toTimeZone(timeZone);
    QDateTime stopLocalDateTime  =  stopUtcDateTime.toTimeZone(timeZone);

    if(minStationLocalDateTime.isNull() || startLocalDateTime < minStationLocalDateTime)
    {
        minStationLocalDateTime = startLocalDateTime;
    }
    if(maxStationLocalDateTime.isNull() || stopLocalDateTime > maxStationLocalDateTime)
    {
        maxStationLocalDateTime = stopLocalDateTime;
    }

    ui->weatherStationMinTimeLabel->setText(
        "Current Min Time: " +
        minStationLocalDateTime.toString("MM/dd/yy hh:mm") +
        " " + timeZone.abbreviation(minStationLocalDateTime)
    );
    ui->weatherStationMaxTimeLabel->setText(
        "Current Max Time: " +
        maxStationLocalDateTime.toString("MM/dd/yy hh:mm") +
        " " + timeZone.abbreviation(maxStationLocalDateTime)
    );

    ui->weatherStationDataStartDateTimeEdit->setTimeZone(timeZone);
    ui->weatherStationDataEndDateTimeEdit->setTimeZone(timeZone);

    ui->weatherStationDataStartDateTimeEdit->setDateTimeRange(minStationLocalDateTime, maxStationLocalDateTime);
    ui->weatherStationDataEndDateTimeEdit->setDateTimeRange(minStationLocalDateTime, maxStationLocalDateTime);

    ui->weatherStationDataStartDateTimeEdit->setDisplayFormat("MM/dd/yyyy HH:mm");
    ui->weatherStationDataEndDateTimeEdit->setDisplayFormat("MM/dd/yyyy HH:mm");

    ui->weatherStationDataStartDateTimeEdit->setDateTime(minStationLocalDateTime);
    ui->weatherStationDataEndDateTimeEdit->setDateTime(maxStationLocalDateTime);

    ui->weatherStationDataStartDateTimeEdit->setEnabled(true);
    ui->weatherStationDataEndDateTimeEdit->setEnabled(true);

    CPLDebug("STATION_FETCH", "minStationLocalDateTime = %s", minStationLocalDateTime.toString("MM/dd/yyyy HH:mm").toStdString().c_str());
    CPLDebug("STATION_FETCH", "maxStationLocalDateTime = %s", maxStationLocalDateTime.toString("MM/dd/yyyy HH:mm").toStdString().c_str());
    emit writeToConsoleSignal("Start Time (local): "+minStationLocalDateTime.toString());
    emit writeToConsoleSignal("End   Time (local): "+maxStationLocalDateTime.toString());

    updateTimeSteps();
}

void PointInitializationInput::pointInitializationSelectNoneButtonClicked()
{
    ui->pointInitializationTreeView->selectionModel()->clearSelection();
}

void PointInitializationInput::folderExpanded(const QModelIndex &index)
{
    openStationFolders.push_back(stationFileSystemModel->filePath(index));
}

void PointInitializationInput::folderCollapsed(const QModelIndex &index)
{
    QString path = stationFileSystemModel->filePath(index);
    openStationFolders.removeOne(path);
}

void PointInitializationInput::weatherStationDataTimestepsSpinBoxValueChanged(int value)
{
    ui->weatherStationDataEndDateTimeEdit->setDisabled(value == 1);
}

QVector<QString> PointInitializationInput::getStationFiles()
{
    return stationFiles;
}

void PointInitializationInput::weatherStationDataStartDateTimeEditChanged()
{
    if(ui->weatherStationDataEndDateTimeEdit->dateTime() < ui->weatherStationDataStartDateTimeEdit->dateTime())
    {
        emit writeToConsoleSignal("Start Time is greater than End Time!, fixing End Time...");
        CPLDebug("STATION_FETCH", "START TIME > END TIME, FIXING END TIME!");
        ui->weatherStationDataEndDateTimeEdit->setDateTime(ui->weatherStationDataStartDateTimeEdit->dateTime().addSecs(3600));
    }
    updateTimeSteps();
}

void PointInitializationInput::weatherStationDataEndDateTimeEditChanged()
{
    if(ui->weatherStationDataEndDateTimeEdit->dateTime() < ui->weatherStationDataStartDateTimeEdit->dateTime())
    {
        emit writeToConsoleSignal("Start Time is greater than End Time!, fixing Start Time...");
        CPLDebug("STATION_FETCH", "START TIME > END TIME, FIXING START TIME!");
        ui->weatherStationDataStartDateTimeEdit->setDateTime(ui->weatherStationDataEndDateTimeEdit->dateTime().addSecs(-3600));
    }
    updateTimeSteps();
}

void PointInitializationInput::updateTimeSteps()
{
    CPLDebug("STATION_FETCH", "Updating suggested time steps...");

    int timesteps;

    if(ui->weatherStationDataStartDateTimeEdit->dateTime() == ui->weatherStationDataEndDateTimeEdit->dateTime())
    {
        timesteps = 1;
    }
    else
    {
        timesteps = qMax(2, static_cast<int>(ui->weatherStationDataStartDateTimeEdit->dateTime().secsTo(ui->weatherStationDataEndDateTimeEdit->dateTime()) / 3600));
    }

    ui->weatherStationDataTimestepsSpinBox->setValue(timesteps);
    ui->weatherStationDataTimestepsSpinBox->setEnabled(true);

    if(timesteps == 1)
    {
        CPLDebug("STATION_FETCH", "One Step Set for Timeseries, greying out stop time!");
        ui->weatherStationDataEndDateTimeEdit->setEnabled(false);
        ui->weatherStationDataEndDateTimeEdit->setToolTip("Stop time is disabled for 1 time step simulations");
    }
    else
    {
        ui->weatherStationDataEndDateTimeEdit->setEnabled(true);
        ui->weatherStationDataEndDateTimeEdit->setToolTip("Enter the simulation stop time");
    }
}

void PointInitializationInput::updateDateTime()
{
    // Update download date time info
    QTimeZone timeZone(ui->timeZoneComboBox->currentText().toUtf8());

    QDateTime currentLocalDateTime = QDateTime::currentDateTime().toTimeZone(timeZone);

    ui->downloadBetweenDatesStartTimeDateTimeEdit->setDateTime(currentLocalDateTime.addDays(-1));
    ui->downloadBetweenDatesEndTimeDateTimeEdit->setDateTime(currentLocalDateTime);

    ui->downloadBetweenDatesStartTimeDateTimeEdit->setMaximumDateTime(currentLocalDateTime);
    ui->downloadBetweenDatesEndTimeDateTimeEdit->setMaximumDateTime(currentLocalDateTime);

    // Update selected station time series
    QItemSelectionModel *selectionModel = ui->pointInitializationTreeView->selectionModel();
    if(!selectionModel || !selectionModel->hasSelection())
    {
        return;
    }

    pointInitializationTreeViewItemSelectionChanged(
        selectionModel->selection(),
        QItemSelection()
    );
}


