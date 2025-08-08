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
    ui->downloadBetweenDatesStartTimeDateTimeEdit->setDateTime(QDateTime::currentDateTime());
    ui->downloadBetweenDatesEndTimeDateTimeEdit->setDateTime(QDateTime::currentDateTime());

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
    bool fetchLatestFlag = true;

    QString outputPath = ui->outputDirectoryLineEdit->text();
    QString units = ui->downloadFromDEMComboBox->currentText();
    QString elevationFile = ui->elevationInputFileLineEdit->property("fullpath").toString();
    QString osTimeZone = "UTC";

    double buffer = ui->downloadFromDEMSpinBox->value();

    progress = new QProgressDialog("Fetching Station Data...", QString(), 0, 0, ui->centralwidget);
    progress->setWindowModality(Qt::WindowModal);
    progress->setCancelButton(nullptr);
    progress->setMinimumDuration(0);
    progress->setAutoClose(true);
    progress->show();

    futureWatcher = new QFutureWatcher<int>(this);
    QFuture<int> future = QtConcurrent::run([=]() {
        return fetchStationData(year, month, day, hour, minute,
                                elevationFile, buffer, units,
                                osTimeZone, fetchLatestFlag, outputPath);
    });
    futureWatcher->setFuture(future);

    connect(futureWatcher, &QFutureWatcher<int>::finished,
            this, &PointInitializationInput::fetchStationDataFinished);
}

int PointInitializationInput::fetchStationData(QVector<int> year,
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
    NinjaArmyH* ninjaArmy = NULL;
    char ** papszOptions = NULL;
    NinjaErr err = 0;

    err = NinjaFetchStation(year.data(), month.data(), day.data(), hour.data(), minute.data(), year.size(), elevationFile.toUtf8().constData(), buffer, units.toUtf8().constData(), osTimeZone.toUtf8().constData(), fetchLatestFlag, outputPath.toUtf8().constData(), papszOptions);
    if (err != NINJA_SUCCESS){
        qDebug() << "NinjaFetchDEMBBox: err =" << err;
        return err;
    }
    else
    {
        return NINJA_SUCCESS;
    }
}

void PointInitializationInput::fetchStationDataFinished()
{
    if (progress) {
        progress->close();
        progress->deleteLater();
        progress = nullptr;
    }

    if (futureWatcher) {
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
}
