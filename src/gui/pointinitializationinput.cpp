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

#include "pointinitializationinput.h"

PointInitializationInput::PointInitializationInput(Ui::MainWindow* ui, QObject* parent)
    : QObject(parent),
    ui(ui)
{
    ui->weatherStationDataSourceStackedWidget->setCurrentIndex(0);
    ui->pointInitializationDataTimeStackedWidget->setCurrentIndex(0);
    ui->weatherStationDataTimeStackedWidget->setCurrentIndex(0);

    ui->pointInitializationDownloadDataButton->setIcon(QIcon(":/server_go.png"));
    ui->pointInitializationWriteStationKMLCheckBox->setIcon(QIcon(":/weather_cloudy.png"));
    ui->weatherStationDataDownloadButton->setIcon(QIcon(":/server_go.png"));
    ui->weatherStationDataDownloadCancelButton->setIcon(QIcon(":/cancel.png"));

    ui->downloadBetweenDatesStartTimeDateTimeEdit->setDateTime(QDateTime::currentDateTime().addDays(-1));
    ui->downloadBetweenDatesEndTimeDateTimeEdit->setDateTime(QDateTime::currentDateTime().addDays(-1));
    ui->weatherStationDataStartDateTimeEdit->setDateTime(QDateTime::currentDateTime().addDays(-1));
    ui->weatherStationDataEndDateTimeEdit->setDateTime(QDateTime::currentDateTime());

    connect(ui->pointInitializationGroupBox, &QGroupBox::toggled, this, &PointInitializationInput::pointInitializationGroupBoxToggled);
    connect(ui->pointInitializationDownloadDataButton, &QPushButton::clicked, this, &PointInitializationInput::pointInitializationDownloadDataButtonClicked);
    connect(ui->weatherStationDataDownloadCancelButton, &QPushButton::clicked, this, &PointInitializationInput::weatherStationDataDownloadCancelButtonClicked);
    connect(ui->weatherStationDataSourceComboBox, &QComboBox::currentIndexChanged, this, &PointInitializationInput::weatherStationDataSourceComboBoxCurrentIndexChanged);
    connect(ui->weatherStationDataTimeComboBox, &QComboBox::currentIndexChanged, this, &PointInitializationInput::weatherStationDataTimeComboBoxCurrentIndexChanged);
    connect(ui->weatherStationDataDownloadButton, &QPushButton::clicked, this, &PointInitializationInput::weatherStationDataDownloadButtonClicked);
    connect(ui->pointInitializationSelectAllButton, &QPushButton::clicked, this, &PointInitializationInput::pointInitializationSelectAllButtonClicked);
    connect(ui->pointInitializationSelectNoneButton, &QPushButton::clicked, this, &PointInitializationInput::pointInitializationSelectNoneButtonClicked);
    connect(ui->pointInitializationTreeView, &QTreeView::expanded, this, &PointInitializationInput::folderExpanded);
    connect(ui->pointInitializationTreeView, &QTreeView::collapsed, this, &PointInitializationInput::folderCollapsed);
}

void PointInitializationInput::pointInitializationGroupBoxToggled(bool checked)
{
    AppState& state = AppState::instance();
    state.isPointInitializationToggled = ui->pointInitializationGroupBox->isChecked();

    if (state.isPointInitializationToggled) {
        ui->domainAverageCheckBox->setChecked(false);
        ui->weatherModelCheckBox->setChecked(false);
        state.isDomainAverageInitializationToggled = ui->domainAverageCheckBox->isChecked();
        state.isWeatherModelInitializationToggled = ui->weatherModelCheckBox->isChecked();
    }

    emit requestRefresh();
}

void PointInitializationInput::pointInitializationDownloadDataButtonClicked()
{
    ui->inputsStackedWidget->setCurrentIndex(20);
}

void PointInitializationInput::weatherStationDataDownloadCancelButtonClicked()
{
    ui->inputsStackedWidget->setCurrentIndex(10);
}

void PointInitializationInput::weatherStationDataDownloadButtonClicked()
{
    QDateTime start = ui->downloadBetweenDatesStartTimeDateTimeEdit->dateTime();
    QDateTime end = ui->downloadBetweenDatesEndTimeDateTimeEdit->dateTime();
    QDateTime startUtc = start.toUTC();
    QDateTime endUtc   = end.toUTC();

    QVector<int> year   = {startUtc.date().year(),   endUtc.date().year()};
    QVector<int> month  = {startUtc.date().month(),  endUtc.date().month()};
    QVector<int> day    = {startUtc.date().day(),    endUtc.date().day()};
    QVector<int> hour   = {startUtc.time().hour(),   endUtc.time().hour()};
    QVector<int> minute = {startUtc.time().minute(), endUtc.time().minute()};

    if(ui->weatherStationDataTimeComboBox->currentIndex() == 1)
    {
        char ** options = nullptr;
        int err = NinjaCheckTimeDuration(year.data(), month.data(), day.data(), hour.data(), minute.data(), 2, options);
        if(err != NINJA_SUCCESS)
        {
            qDebug() << "NinjaCheckTimeDuration err=" << err;
        }
    }

    bool fetchLatestFlag = ui->weatherStationDataTimeComboBox->currentIndex() ? 0 : 1;

    QString outputPath = ui->outputDirectoryLineEdit->text();
    QString elevationFile = ui->elevationInputFileLineEdit->property("fullpath").toString();
    QString DEMTimeZone = ui->timeZoneComboBox->currentText();

    progress = new QProgressDialog("Fetching Station Data...", QString(), 0, 0, ui->centralwidget);
    progress->setWindowModality(Qt::WindowModal);
    progress->setCancelButton(nullptr);
    progress->setMinimumDuration(0);
    progress->setAutoClose(true);
    progress->show();

    futureWatcher = new QFutureWatcher<int>(this);
    QFuture<int> future;
    if(ui->weatherStationDataSourceComboBox->currentIndex() == 0)
    {
        QString units = ui->downloadFromDEMComboBox->currentText();
        double buffer = ui->downloadFromDEMSpinBox->value();
        future = QtConcurrent::run(&PointInitializationInput::fetchStationFromBbox,
                                   year, month, day, hour, minute,
                                   elevationFile, buffer, units,
                                   DEMTimeZone, fetchLatestFlag, outputPath);
    }
    else
    {
        QString stationList = ui->downloadFromStationIDLineEdit->text();
        future = QtConcurrent::run(&PointInitializationInput::fetchStationByName,year, month, day, hour, minute,
                                   elevationFile, stationList,
                                   DEMTimeZone, fetchLatestFlag, outputPath);
    }
    futureWatcher->setFuture(future);

    connect(futureWatcher, &QFutureWatcher<int>::finished,
            this, &PointInitializationInput::fetchStationDataFinished);
}

int PointInitializationInput::fetchStationFromBbox(QVector<int> year,
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
    char ** papszOptions = NULL;
    NinjaErr err = 0;

    err = NinjaFetchStationFromBBox(year.data(), month.data(), day.data(), hour.data(), minute.data(), year.size(), elevationFile.toUtf8().constData(), buffer, units.toUtf8().constData(), osTimeZone.toUtf8().constData(), fetchLatestFlag, outputPath.toUtf8().constData(), false, papszOptions);
    if (err != NINJA_SUCCESS){
        qDebug() << "NinjaFetchStationFromBbox: err =" << err;
        return err;
    }
    else
    {
        return NINJA_SUCCESS;
    }
}

int PointInitializationInput::fetchStationByName(QVector<int> year,
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
    char ** papszOptions = NULL;
    NinjaErr err = 0;

    err = NinjaFetchStationByName(year.data(), month.data(), day.data(), hour.data(), minute.data(), year.size(), elevationFile.toUtf8().constData(), stationList.toUtf8().constData(), osTimeZone.toUtf8().constData(), fetchLatestFlag, outputPath.toUtf8().constData(), false, papszOptions);
    if (err != NINJA_SUCCESS){
        qDebug() << "NinjaFetchFetchStationByName: err =" << err;
        return err;
    }
    else
    {
        return NINJA_SUCCESS;
    }
}

void PointInitializationInput::fetchStationDataFinished()
{
    if (progress)
    {
        progress->close();
        progress->deleteLater();
        progress = nullptr;
    }

    if (futureWatcher)
    {
        futureWatcher->deleteLater();
        futureWatcher = nullptr;
    }

    ui->inputsStackedWidget->setCurrentIndex(10);
}

void PointInitializationInput::weatherStationDataSourceComboBoxCurrentIndexChanged(int index)
{
    ui->weatherStationDataSourceStackedWidget->setCurrentIndex(index);
}

void PointInitializationInput::weatherStationDataTimeComboBoxCurrentIndexChanged(int index)
{
    ui->weatherStationDataTimeStackedWidget->setCurrentIndex(index);
}

void PointInitializationInput::setupTreeView()
{
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

void PointInitializationInput::pointInitializationTreeViewItemSelectionChanged()
{
    AppState& state = AppState::instance();
    QModelIndexList selectedRows = ui->pointInitializationTreeView->selectionModel()->selectedRows();

    stationFiles.clear();
    QVector<int> stationFileTypes;

    maxStationTime = QDateTime();
    minStationTime = QDateTime();

    state.isStationFileSelected = false;
    if (selectedRows.count() > 0)
    {
        state.isStationFileSelected = true;
    }

    for(int i = 0; i < selectedRows.count(); i++)
    {
        if(stationFileSystemModel->isDir(selectedRows[i]))
        {
            ui->pointInitializationTreeView->selectionModel()->select(selectedRows[i], QItemSelectionModel::Deselect | QItemSelectionModel::Rows);
            return;
        }

        QString recentFileSelected = stationFileSystemModel->filePath(selectedRows[i]);
        stationFiles.push_back(recentFileSelected);
        qDebug() << "[GUI-Point] Selected file path:" << recentFileSelected;

        QByteArray filePathBytes = recentFileSelected.toUtf8();
        const char* filePath = filePathBytes.constData();
        char** papszOptions = nullptr;
        int stationHeader = NinjaGetHeaderVersion(filePath, papszOptions);
        qDebug() << "[GUI-Point] Station Header: " << stationHeader;

        bool timeSeriesFlag = true;
        if (stationHeader != 1)
        {
            GDALDataset* hDS = (GDALDataset*) GDALOpenEx(
                filePath,
                GDAL_OF_VECTOR | GDAL_OF_READONLY,
                NULL,
                NULL,
                NULL
            );

            OGRLayer* poLayer = hDS->GetLayer(0);
            poLayer->ResetReading();
            qint64 lastIndex = poLayer->GetFeatureCount();
            qDebug() << "[GUI-Point] Number of Time Entries:" << lastIndex;

            OGRFeature* poFeature = poLayer->GetFeature(1);         // Skip header, first time in series
            QString startDateTime(poFeature->GetFieldAsString(15)); // Time should be in 15th column (0-14)
            qDebug() << "[GUI-Point] Station start time:" << startDateTime;

            poFeature = poLayer->GetFeature(lastIndex);             // last time in series
            QString stopDateTime(poFeature->GetFieldAsString(15));
            qDebug() << "[GUI-Point] Station end Time:" << stopDateTime;

            if (startDateTime.isEmpty() && stopDateTime.isEmpty()) // No time series
            {
                qDebug() << "[GUI-Point] File cannot be used for Time Series";
                timeSeriesFlag = false;
                stationFileTypes.push_back(0);
            }
            else if (!startDateTime.isEmpty() && !stopDateTime.isEmpty()) // Some type of time series
            {
                qDebug() << "[GUI-Point] File can be used for Time Series, suggesting time series parameters...";
                readStationTime(startDateTime, stopDateTime);
                stationFileTypes.push_back(1);
            }
        }

        if (stationHeader == 2)
        {
            ui->pointInitializationDataTimeStackedWidget->setCurrentIndex(timeSeriesFlag ? 0 : 1);

            if (!timeSeriesFlag)
            {
                QDateTime dateModified = QFileInfo(recentFileSelected).birthTime();
                ui->weatherStationDataTextEdit->setText("Simulation time set to: " + dateModified.toString());
                ui->weatherStationDataTextEdit->setProperty("simulationTime", dateModified);
            }
        }
        ui->pointInitializationTreeView->setProperty("timeSeriesFlag", timeSeriesFlag);
    }

    state.isStationFileSelectionValid = true;
    for (int type : stationFileTypes) {
        if (type != stationFileTypes[0]) {
            state.isStationFileSelectionValid = false;
            break;
        }
    }
    emit requestRefresh();
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

void PointInitializationInput::readStationTime(QString startDateTime, QString stopDateTime)
{
    QString stationTimeFormat = "yyyy-MM-ddTHH:mm:ssZ";
    QString DEMTimeZone = ui->timeZoneComboBox->currentText();

    QTimeZone timeZone(DEMTimeZone.toUtf8());
    if (!timeZone.isValid()) {
        qWarning() << "[GUI-Point] Invalid time zone:" << DEMTimeZone;
        timeZone = QTimeZone::systemTimeZone();
    }

    QDateTime startTimeUTC = QDateTime::fromString(startDateTime, stationTimeFormat);
    QDateTime endTimeUTC   = QDateTime::fromString(stopDateTime, stationTimeFormat);
    startTimeUTC.setTimeSpec(Qt::UTC);
    endTimeUTC.setTimeSpec(Qt::UTC);

    QDateTime DEMStartTime = startTimeUTC.toTimeZone(timeZone);
    QDateTime DEMEndTime  = endTimeUTC.toTimeZone(timeZone);

    if (minStationTime.isNull() || DEMStartTime < minStationTime)
    {
        minStationTime = DEMStartTime;
    }
    if(maxStationTime.isNull() || DEMEndTime > maxStationTime)
    {
        maxStationTime = DEMEndTime;
    }

    qDebug() << "[GUI-Point] Start Time (" << DEMTimeZone << "):" << minStationTime.toString();
    qDebug() << "[GUI-Point] Stop Time ("  << DEMTimeZone << "):"  << maxStationTime.toString();

    ui->weatherStationDataStartTimeLabel->setText("Start Time (" + DEMTimeZone + "):");
    ui->weatherStationDataEndTimeLabel->setText("End Time (" + DEMTimeZone + "):");
    ui->weatherStationDataStartDateTimeEdit->setDateTime(minStationTime);
    ui->weatherStationDataEndDateTimeEdit->setDateTime(maxStationTime);
    ui->weatherStationDataStartDateTimeEdit->setEnabled(true);
    ui->weatherStationDataEndDateTimeEdit->setEnabled(true);
    ui->weatherStationDataTimestepsSpinBox->setEnabled(true);

    int timesteps = qMax(2, static_cast<int>(minStationTime.secsTo(maxStationTime) / 3600));
    ui->weatherStationDataTimestepsSpinBox->setValue(timesteps);
    qDebug() << "[GUI-Point] Suggested Timesteps:" << timesteps;
}

QVector<QString> PointInitializationInput::getStationFiles()
{
    return stationFiles;
}

