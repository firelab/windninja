 /******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja Qt GUI
 * Purpose:  Hands GUI related logic for the Weather Model Page
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

#include "weatherModelInput.h"

WeatherModelInput::WeatherModelInput(Ui::MainWindow* ui, QObject* parent)
    : QObject(parent),
    ui(ui)
{
    ninjaTools = NinjaMakeTools();

    ui->pastcastGroupBox->hide();
    int identifiersSize = 0;
    const char** identifiers = NinjaGetAllWeatherModelIdentifiers(ninjaTools, &identifiersSize);
    for (int i = 0; i < identifiersSize; i++)
    {
        ui->weatherModelComboBox->addItem(identifiers[i]);
    }
    NinjaFreeAllWeatherModelIdentifiers(identifiers, identifiersSize);

    weatherModelComboBoxCurrentIndexChanged(0);
    updatePastcastDateTimeEdits();

    connect(ui->weatherModelGroupBox, &QGroupBox::toggled, this, &WeatherModelInput::weatherModelGroupBoxToggled);
    connect(ui->weatherModelDownloadButton, &QPushButton::clicked, this, &WeatherModelInput::weatherModelDownloadButtonClicked);
    connect(ui->weatherModelComboBox, &QComboBox::currentIndexChanged, this, &WeatherModelInput::weatherModelComboBoxCurrentIndexChanged);
    connect(ui->weatherModelTimeSelectAllButton, &QPushButton::clicked, this, &WeatherModelInput::weatherModelTimeSelectAllButtonClicked);
    connect(ui->weatherModelTimeSelectNoneButton, &QPushButton::clicked, this, &WeatherModelInput::weatherModelTimeSelectNoneButtonClicked);
    connect(ui->timeZoneComboBox, &QComboBox::currentTextChanged, this, &WeatherModelInput::updatePastcastDateTimeEdits);
}

void WeatherModelInput::weatherModelDownloadButtonClicked()
{   
    int hours = ui->weatherModelSpinBox->value();

    progress = new QProgressDialog("Fetching Forecast Data...", QString(), 0, 0, ui->centralwidget);
    progress->setWindowModality(Qt::WindowModal);
    progress->setCancelButton(nullptr);
    progress->setMinimumDuration(0);
    progress->setAutoClose(true);
    progress->show();

    futureWatcher = new QFutureWatcher<int>(this);
    QFuture<int> future;

    if (ui->weatherModelComboBox->currentText().contains("PASTCAST"))
    {
        progress->setLabelText("Fetching Pastcast Data...");

        QDateTime start = ui->pastcastStartDateTimeEdit->dateTime();
        QDateTime end   = ui->pastcastEndDateTimeEdit->dateTime();

        future = QtConcurrent::run(
            WeatherModelInput::fetchPastcastWeather,
            ninjaTools,
            ui->weatherModelComboBox->currentText(),
            ui->elevationInputFileLineEdit->property("fullpath").toString(),
            ui->timeZoneComboBox->currentText(),
            start.date().year(), start.date().month(), start.date().day(), start.time().hour(),
            end.date().year(),   end.date().month(),   end.date().day(),   end.time().hour());
    }
    else
    {
        future = QtConcurrent::run(
            WeatherModelInput::fetchForecastWeather,
            ninjaTools,
            ui->weatherModelComboBox->currentText(),
            ui->elevationInputFileLineEdit->property("fullpath").toString(),
            hours);
    }

    futureWatcher->setFuture(future);

    connect(futureWatcher, &QFutureWatcher<int>::finished,
            this, &WeatherModelInput::weatherModelDownloadFinished);
}

int WeatherModelInput::fetchForecastWeather(
    NinjaToolsH* ninjaTools,
    const QString& modelIdentifierStr,
    const QString& demFileStr,
    int hours)
{
    QByteArray modelIdentifierTemp = modelIdentifierStr.toUtf8();
    QByteArray demFileTemp = demFileStr.toUtf8();

    const char* modelIdentifier = modelIdentifierTemp.constData();
    const char* demFile = demFileTemp.constData();

    NinjaErr ninjaErr = NinjaFetchWeatherData(ninjaTools, modelIdentifier, demFile, hours);
    if (ninjaErr != NINJA_SUCCESS)
    {
        qDebug() << "NinjaFetchWeatherData: ninjaErr =" << ninjaErr;
    }

    return ninjaErr;
}

int WeatherModelInput::fetchPastcastWeather(
    NinjaToolsH* ninjaTools,
    const QString& modelIdentifierStr,
    const QString& demFileStr,
    const QString& timeZoneStr,
    int startYear, int startMonth, int startDay, int startHour,
    int endYear, int endMonth, int endDay, int endHour)
{
    QByteArray modelIdentifierTemp = modelIdentifierStr.toUtf8();
    QByteArray demFileTemp = demFileStr.toUtf8();
    QByteArray timeZoneTemp = timeZoneStr.toUtf8();

    const char* modelIdentifier = modelIdentifierTemp.constData();
    const char* demFile = demFileTemp.constData();
    const char* timeZone = timeZoneTemp.constData();

    NinjaErr ninjaErr = NinjaFetchArchiveWeatherData(
        ninjaTools, modelIdentifier, demFile, timeZone,
        startYear, startMonth, startDay, startHour,
        endYear, endMonth, endDay, endHour
        );

    if (ninjaErr != NINJA_SUCCESS)
    {
        qDebug() << "NinjaFetchArchiveWeatherData: ninjaErr =" << ninjaErr;
    }

    return ninjaErr;
}

void WeatherModelInput::weatherModelDownloadFinished()
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
}

void WeatherModelInput::weatherModelComboBoxCurrentIndexChanged(int index)
{
    if(ui->weatherModelComboBox->currentText().contains("PASTCAST"))
    {
        ui->weatherModelSpinBox->setDisabled(true);
        ui->pastcastGroupBox->setVisible(true);

        return;
    }

    QStringList tooltipList;
    QString weatherModel = ui->weatherModelComboBox->currentText();
    for(int i = 0; i < modelGlossary.size(); i++)
    {
        int pos = modelGlossary[i].indexOf('=');
        if (pos <= 0)
        {
            continue;
        }

        QString key = modelGlossary[i].left(pos);
        if(weatherModel.contains(key, Qt::CaseInsensitive))
        {
            tooltipList << modelGlossary[i].mid(pos + 1);
        }
    }
    ui->weatherModelComboBox->setToolTip(tooltipList.join(", "));

    QByteArray modelIdentifierByte = ui->weatherModelComboBox->currentText().toUtf8();
    const char* modelIdentifier = modelIdentifierByte.constData();
    int starHour, endHour;

    NinjaErr ninjaErr = NinjaGetWeatherModelHours(ninjaTools, modelIdentifier, &starHour, &endHour);
    if (ninjaErr != NINJA_SUCCESS)
    {
        qDebug() << "NinjaGetWeatherModelHours: ninjaErr=" << ninjaErr;
    }

    ui->weatherModelSpinBox->setMinimum(starHour);
    ui->weatherModelSpinBox->setMaximum(endHour);
}

void WeatherModelInput::updateTreeView()
{
    AppState& state = AppState::instance();
    state.isWeatherModelForecastValid = false;
    emit updateState();

    // File Tree View
    fileModel = new QFileSystemModel(this);
    QString demFilePath = ui->elevationInputFileLineEdit->property("fullpath").toString();
    QFileInfo demFileInfo(demFilePath);

    fileModel->setRootPath(demFileInfo.absolutePath());
    fileModel->setFilter(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot);
    fileModel->setNameFilterDisables(false);

    QStringList filters;
    for(int i = 0; i < ui->weatherModelComboBox->count(); i++)
    {
        filters << ui->weatherModelComboBox->itemText(i) + "-" + demFileInfo.fileName();
    }
    filters << "20*.zip";
    filters << "20*T*";
    filters << "*.nc";
    fileModel->setNameFilters(filters);

    ui->weatherModelFileTreeView->setModel(fileModel);
    ui->weatherModelFileTreeView->setRootIndex(fileModel->index(demFileInfo.absolutePath()));
    ui->weatherModelFileTreeView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->weatherModelFileTreeView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->weatherModelFileTreeView->setAnimated(true);
    ui->weatherModelFileTreeView->setUniformRowHeights(true);
    ui->weatherModelFileTreeView->hideColumn(1);
    ui->weatherModelFileTreeView->hideColumn(2);
    ui->weatherModelFileTreeView->collapseAll();

    QHeaderView *fileHeader = ui->weatherModelFileTreeView->header();
    fileHeader->setStretchLastSection(false);
    fileHeader->setSectionResizeMode(0, QHeaderView::Stretch);
    fileHeader->resizeSection(0, 400);
    fileHeader->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    fileHeader->setSectionResizeMode(1, QHeaderView::ResizeToContents);

    // Time Tree View
    timeModel = new QStandardItemModel(this);

    ui->weatherModelTimeTreeView->setModel(timeModel);
    ui->weatherModelTimeTreeView->setSortingEnabled(true);
    ui->weatherModelTimeTreeView->sortByColumn(0, Qt::AscendingOrder);
    ui->weatherModelTimeTreeView->setSelectionMode(QAbstractItemView::MultiSelection);
    ui->weatherModelTimeTreeView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->weatherModelTimeTreeView->selectAll();

    QHeaderView *timeHeader = ui->weatherModelTimeTreeView->header();
    timeHeader->setVisible(false);

    connect(ui->weatherModelFileTreeView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &WeatherModelInput::weatherModelFileTreeViewItemSelectionChanged);
}

void WeatherModelInput::weatherModelFileTreeViewItemSelectionChanged(const QItemSelection &selected)
{
    AppState& state = AppState::instance();

    if (selected.indexes().empty())
    {
        state.isWeatherModelForecastValid = false;

        return;
    }

    QModelIndex index = selected.indexes().first();
    QFileInfo fileInfo = fileModel->fileInfo(index);
    if(fileInfo.isDir())
    {
        timeModel->clear();

        state.isWeatherModelForecastValid = false;
        emit updateState();

        return;
    }

    state.isWeatherModelForecastValid = true;

    std::string modelFilePath = fileModel->filePath(index).toStdString();
    std::string timeZone = ui->timeZoneComboBox->currentText().toStdString();
    int timeListSize = 0;

    const char **timeList = NinjaGetWeatherModelTimeList(ninjaTools, &timeListSize, modelFilePath.c_str(), timeZone.c_str());
    if(timeList == NULL)
    {
        qDebug() << "NinjaGetWeatherModelTimeList: Empty Time List";
    }

    timeModel->clear();
    for (int i = 0; i < timeListSize; i++)
    {
        QString timestep = QString::fromUtf8(timeList[i]);
        timeModel->appendRow(new QStandardItem(timestep));
    }

    ui->weatherModelTimeTreeView->selectAll();

    NinjaErr ninjaErr = NinjaFreeWeatherModelTimeList(timeList, timeListSize);
    if(ninjaErr == NINJA_SUCCESS)
    {
        qDebug() << "NinjaFreeWeatherModelTimeList: ninjaErr=" << ninjaErr;
    }

    emit updateState();
}

void WeatherModelInput::weatherModelGroupBoxToggled(bool toggled)
{
    ui->rawWeatherModelOutputCheckBox->setEnabled(toggled);

    AppState& state = AppState::instance();
    state.isWeatherModelInitializationToggled = toggled;

    if (state.isWeatherModelInitializationToggled)
    {
        ui->domainAverageGroupBox->setChecked(false);
        ui->pointInitializationGroupBox->setChecked(false);
        state.isDomainAverageInitializationToggled = ui->domainAverageGroupBox->isChecked();
        state.isPointInitializationToggled = ui->pointInitializationGroupBox->isChecked();
    }

    emit updateState();
}

void WeatherModelInput::weatherModelTimeSelectAllButtonClicked()
{
    ui->weatherModelTimeTreeView->selectAll();
}

void WeatherModelInput::weatherModelTimeSelectNoneButtonClicked()
{
    ui->weatherModelTimeTreeView->clearSelection();
}

void WeatherModelInput::updatePastcastDateTimeEdits()
{
    QTimeZone timeZone(ui->timeZoneComboBox->currentText().toUtf8());

    // Update Minimum Time
    QDate earliestDate(2014, 7, 30);
    QDateTime utcDateTime(earliestDate, QTime(18, 0), Qt::UTC);
    QDateTime localDateTime = utcDateTime.toTimeZone(timeZone);
    ui->pastcastGroupBox->setTitle("Earliest Pastcast Datetime: " + localDateTime.toString("MM/dd/yyyy hh:mm"));
    ui->pastcastGroupBox->updateGeometry();

    // Update Date Time Edits
    QDateTime demTime = QDateTime::currentDateTime().toTimeZone(timeZone);
    // Has to be set to avoid unnecessary conversions, use timeZoneComboBox for time zone info
    demTime.setTimeSpec(Qt::LocalTime);

    ui->pastcastStartDateTimeEdit->setDateTime(demTime);
    ui->pastcastEndDateTimeEdit->setDateTime(demTime);
}

