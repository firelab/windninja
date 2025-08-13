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
    ui->pointInitializationRefreshButton->setIcon(QIcon(":/arrow_rotate_clockwise.png"));
    ui->pointInitializationWriteStationKMLCheckBox->setIcon(QIcon(":/weather_cloudy.png"));
    ui->weatherStationDataDownloadButton->setIcon(QIcon(":/server_go.png"));
    ui->weatherStationDataDownloadCancelButton->setIcon(QIcon(":/cancel.png"));
    ui->downloadBetweenDatesStartTimeDateTimeEdit->setDateTime(QDateTime::currentDateTime().addDays(-1));
    ui->downloadBetweenDatesEndTimeDateTimeEdit->setDateTime(QDateTime::currentDateTime().addDays(-1));

    connect(ui->pointInitializationGroupBox, &QGroupBox::toggled, this, &PointInitializationInput::pointInitializationGroupBoxToggled);
    connect(ui->pointInitializationDownloadDataButton, &QPushButton::clicked, this, &PointInitializationInput::pointInitializationDownloadDataButtonClicked);
    connect(ui->weatherStationDataDownloadCancelButton, &QPushButton::clicked, this, &PointInitializationInput::weatherStationDataDownloadCancelButtonClicked);
    connect(ui->weatherStationDataSourceComboBox, &QComboBox::currentIndexChanged, this, &PointInitializationInput::weatherStationDataSourceComboBoxCurrentIndexChanged);
    connect(ui->weatherStationDataTimeComboBox, &QComboBox::currentIndexChanged, this, &PointInitializationInput::weatherStationDataTimeComboBoxCurrentIndexChanged);
    connect(ui->weatherStationDataDownloadButton, &QPushButton::clicked, this, &PointInitializationInput::weatherStationDataDownloadButtonClicked);
    connect(ui->pointInitializationRefreshButton, &QPushButton::clicked, this, &PointInitializationInput::pointInitialziationRefreshButtonClicked);
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
    QVector<int> year   = {start.date().year(),   end.date().year()};
    QVector<int> month  = {start.date().month(),  end.date().month()};
    QVector<int> day    = {start.date().day(),    end.date().day()};
    QVector<int> hour   = {start.time().hour(),   end.time().hour()};
    QVector<int> minute = {start.time().minute(), end.time().minute()};

    bool fetchLatestFlag = false;
    if(ui->weatherStationDataTimeComboBox->currentIndex() == 0)
    {
        fetchLatestFlag = true;
    }

    QString outputPath = ui->outputDirectoryLineEdit->text();
    QString elevationFile = ui->elevationInputFileLineEdit->property("fullpath").toString();
    QString osTimeZone = "UTC";


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
                                   osTimeZone, fetchLatestFlag, outputPath);
    }
    else
    {
        QString stationList = ui->downloadFromStationIDLineEdit->text();
        future = QtConcurrent::run(&PointInitializationInput::fetchStationByName,
                                   year, month, day, hour, minute,
                                   elevationFile, stationList,
                                   osTimeZone, fetchLatestFlag, outputPath);
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
    ui->pointInitializationRefreshButton->click();
}

void PointInitializationInput::weatherStationDataSourceComboBoxCurrentIndexChanged(int index)
{
    ui->weatherStationDataSourceStackedWidget->setCurrentIndex(index);
}

void PointInitializationInput::weatherStationDataTimeComboBoxCurrentIndexChanged(int index)
{
    ui->weatherStationDataTimeStackedWidget->setCurrentIndex(index);
}

void PointInitializationInput::pointInitialziationRefreshButtonClicked()
{
    stationFileSystemModel = new QFileSystemModel;
    QString path = ui->elevationInputFileLineEdit->property("fullpath").toString();
    if(path.isEmpty())
    {
        delete stationFileSystemModel;
        return;
    }
    QFileInfo fileInfo(path);
    stationFileSystemModel->setRootPath(fileInfo.absolutePath());

    QStringList filters;
    filters<<"*.csv";
    filters<<"WXSTATIONS-*";
    stationFileSystemModel->setNameFilters(filters);
    stationFileSystemModel->setFilter(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot);
    stationFileSystemModel->setNameFilterDisables(false);

    ui->pointInitializationTreeView->setModel(stationFileSystemModel);
    ui->pointInitializationTreeView->setRootIndex(stationFileSystemModel->index(fileInfo.absolutePath()));
    ui->pointInitializationTreeView->header()->setSectionResizeMode(QHeaderView::Stretch);
    ui->pointInitializationTreeView->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->pointInitializationTreeView->header()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    ui->pointInitializationTreeView->hideColumn(1);
    ui->pointInitializationTreeView->hideColumn(2);

    connect(ui->pointInitializationTreeView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &PointInitializationInput::pointInitializationTreeViewItemSelectionChanged);

}

void PointInitializationInput::pointInitializationTreeViewItemSelectionChanged()
{
    QModelIndexList selectedIndexes = ui->pointInitializationTreeView->selectionModel()->selectedIndexes();
    QModelIndex index = selectedIndexes.first();
    if (index.column() != 0)
        index = index.sibling(index.row(), 0);

    QString recentFileSelected = stationFileSystemModel->filePath(index);
    qDebug() << "[STATION FETCH] Selected file path:" << recentFileSelected;

    QByteArray filePathBytes = recentFileSelected.toUtf8();
    const char* filePath = filePathBytes.constData();
    char** papszOptions = nullptr;
    int stationHeader = NinjaGetHeaderVersion(filePath, papszOptions);
    qDebug() << "[STATION FETCH] Station Header: " << stationHeader;

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
        qDebug() << "[STATION FETCH] Number of Time Entries:" << lastIndex;

        OGRFeature* poFeature = poLayer->GetFeature(1);         // Skip header, first time in series
        QString startDateTime(poFeature->GetFieldAsString(15)); // Time should be in 15th column (0-14)
        qDebug() << "[STATION FETCH] Station start time:" << startDateTime;

        poFeature = poLayer->GetFeature(lastIndex);             // last time in series
        QString stopDateTime(poFeature->GetFieldAsString(15));
        qDebug() << "[STATION FETCH] Station end Time:" << stopDateTime;

        if (startDateTime.isEmpty() && stopDateTime.isEmpty()) // No time series
        {
            qDebug() << "[STATION FETCH] File cannot be used for Time Series";
            timeSeriesFlag = false;
        }
        else if (!startDateTime.isEmpty() && !stopDateTime.isEmpty()) // Some type of time series
        {
            qDebug() << "[STATION FETCH] File can be used for Time Series, suggesting time series parameters...";
            readStationTime(startDateTime, stopDateTime);
        }
    }


    if(stationHeader == 2 && timeSeriesFlag)
    {
        ui->pointInitializationDataTimeStackedWidget->setCurrentIndex(0);
    }


    if(stationHeader == 2 && !timeSeriesFlag)
    {
        ui->pointInitializationDataTimeStackedWidget->setCurrentIndex(1);
        QDateTime dateModified = QFileInfo(recentFileSelected).birthTime();
        //updateSingleTime()
        QString simulationTimeText = "Simulation time set to: " + dateModified.toString();
        ui->weatherStationDataTextEdit->setText(simulationTimeText);
    }
}

void PointInitializationInput::readStationTime(QString startDateTime, QString stopDateTime)
{
    QString stationTimeFormat = "yyyy-MM-ddTHH:mm:ssZ";

    QDateTime startTimeUTC = QDateTime::fromString(startDateTime, stationTimeFormat);
    QDateTime endTimeUTC  = QDateTime::fromString(stopDateTime, stationTimeFormat);
    startTimeUTC.setTimeSpec(Qt::UTC);
    endTimeUTC.setTimeSpec(Qt::UTC);

    QDateTime startTimeLocal = startTimeUTC.toLocalTime();
    QDateTime endTimeLocal  = endTimeUTC.toLocalTime();
    qDebug() << "[STATION FETCH] Start Time (local):" << startTimeLocal.toString();
    qDebug() << "[STATION FETCH] Stop Time (local):"  << endTimeLocal.toString();

    ui->weatherStationDataStartDateTimeEdit->setDateTime(startTimeLocal);
    ui->weatherStationDataEndDateTimeEdit->setDateTime(endTimeLocal);

    updateTimeSteps();
}

void PointInitializationInput::updateTimeSteps()
{
    qDebug() << "[STATION FETCH] Updating Suggested Time steps...";

    QDateTime start = ui->weatherStationDataStartDateTimeEdit->dateTime();
    QDateTime stop  = ui->weatherStationDataEndDateTimeEdit->dateTime();

    qint64 diffSecs = start.secsTo(stop); // difference in seconds

    int timesteps;
    if (diffSecs <= 0)
    {
        timesteps = 1;
    }
    else
    {
        timesteps = static_cast<int>(diffSecs / 3600); // convert seconds to hours
        if (timesteps < 2)
            timesteps = 2;
    }

    qDebug() << "[STATION FETCH] Suggested Timesteps:" << timesteps;
    ui->weatherStationDataTimestepsSpinBox->setValue(timesteps);
}
