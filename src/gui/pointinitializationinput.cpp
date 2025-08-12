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
    QFileSystemModel *model = new QFileSystemModel;
    QString path = ui->elevationInputFileLineEdit->property("fullpath").toString();
    if(path.isEmpty())
    {
        delete model;
        return;
    }
    QFileInfo fileInfo(path);
    model->setRootPath(fileInfo.absolutePath());

    QStringList filters;
    filters<<"*.csv";
    filters<<"WXSTATIONS-*";
    model->setNameFilters(filters);
    model->setFilter(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot);
    model->setNameFilterDisables(false);

    ui->pointInitializationTreeView->setModel(model);
    ui->pointInitializationTreeView->setRootIndex(model->index(fileInfo.absolutePath()));
    ui->pointInitializationTreeView->header()->setSectionResizeMode(QHeaderView::Stretch);
    ui->pointInitializationTreeView->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->pointInitializationTreeView->header()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    ui->pointInitializationTreeView->hideColumn(1);
    ui->pointInitializationTreeView->hideColumn(2);

    connect(ui->pointInitializationTreeView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &PointInitializationInput::pointInitializationTreeViewItemSelectionChanged);

}

void PointInitializationInput::pointInitializationTreeViewItemSelectionChanged()
{
    const char* xFileName = "/home/mason/Downloads/testfolder/WXSTATIONS-2025-08-10-1537-2025-08-11-1537-test/CBFI1-2025-08-10_1537-2025-08-11_1537-1.csv";
    char** papszOptions = nullptr;
    int stationHeader = NinjaGetHeaderVersion(xFileName, papszOptions);
    int instant = 0;
    qDebug() << "Station Header is: " << stationHeader;
    std::string idx3;
    stringstream ssidx;

    if(stationHeader != 1)
    {
        OGRDataSourceH hDS;
        OGRLayer *poLayer;
        OGRFeature *poFeature;
        OGRFeatureDefn *poFeatureDefn;
        OGRFieldDefn *poFieldDefn;
        OGRLayerH hLayer;

        hDS = OGROpen( xFileName, FALSE, NULL );

        poLayer = (OGRLayer*)OGR_DS_GetLayer( hDS, 0 );
        hLayer=OGR_DS_GetLayer(hDS,0);
        OGR_L_ResetReading(hLayer);
        poLayer->ResetReading();

        GIntBig iBig = 1;
        GIntBig idx0 = poLayer->GetFeatureCount();
        GIntBig idx1 = idx0-iBig;
        GIntBig idx2;

        idx2 = poLayer->GetFeatureCount();

        CPLDebug("STATION_FETCH","Number of Time Entries: %llu",idx2); //How many lines are on disk
        QString qFileName = QFileInfo(xFileName).fileName();
        const char* emptyChair; //Muy Importante!

        poFeature = poLayer->GetFeature(iBig);

        //    startTime = poFeature->GetFieldAsString(15);
        std::string start_datetime(poFeature->GetFieldAsString(15));
        poFeature = poLayer->GetFeature(idx2);
        std::string stop_datetime(poFeature->GetFieldAsString(15));

        CPLDebug("STATION_FETCH","STATION START TIME: %s",start_datetime.c_str());
        CPLDebug("STATION_FETCH","STATION END TIME: %s",stop_datetime.c_str());

        if (start_datetime.empty()==true && stop_datetime.empty()==true)
        {
            //Means that there is not a time series
            CPLDebug("STATION_FETCH", "File cannot be used for Time Series");
            instant = 1;
        }
        if (start_datetime.empty()==false && stop_datetime.empty()==false) //Definately some sort of time series
        {
            CPLDebug("STATION_FETCH","File can be used for Times Series");
            CPLDebug("STATION_FETCH","Suggesting Potentially Reasonable Time Series Parameters...");

            QString q_time_format = "yyyy-MM-ddTHH:mm:ssZ";
            QString start_utcX = QString::fromStdString(start_datetime);
            QString end_utcX = QString::fromStdString(stop_datetime);

            QDateTime start_qat = QDateTime::fromString(start_utcX,q_time_format);
            QDateTime end_qat = QDateTime::fromString(end_utcX,q_time_format);

            start_qat.setTimeSpec(Qt::UTC);
            end_qat.setTimeSpec(Qt::UTC);

            //readStationTime(start_datetime,stop_datetime,idx2); //Turns the Start and Stop times into local timess......
            ssidx<<idx2;
            idx3=ssidx.str();
        }
    }

    if(stationHeader == 2 && instant == 0)
    {
        ui->pointInitializationDataTimeStackedWidget->setCurrentIndex(0);
        ui->weatherStationDataStartDateTimeEdit->setDateTime(ui->downloadBetweenDatesStartTimeDateTimeEdit->dateTime());
        ui->weatherStationDataEndDateTimeEdit->setDateTime(ui->downloadBetweenDatesEndTimeDateTimeEdit->dateTime());
    }


    if(stationHeader == 2 && instant == 1)
    {
        ui->pointInitializationDataTimeStackedWidget->setCurrentIndex(1);
        QDateTime time = QFileInfo(xFileName).birthTime();
        //updateSingleTime()
        QString text = "Simulation time set to: " + time.toString();
        ui->weatherStationDataTextEdit->setText(text);

    }

}
