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

#include "weathermodelinput.h"

WeatherModelInput::WeatherModelInput(Ui::MainWindow* ui, QObject* parent)
    : QObject(parent),
    ui(ui)
{
    ninjaTools = NinjaMakeTools();

    ui->pastcastGroupBox->hide();
    ui->pastcastStartDateTimeEdit->setDateTime(QDateTime::currentDateTimeUtc());
    ui->pastcastEndDateTimeEdit->setDateTime(QDateTime::currentDateTimeUtc());

    int identifiersSize = 0;
    const char** identifiers = NinjaGetAllWeatherModelIdentifiers(ninjaTools, &identifiersSize);
    for (int i = 0; i < identifiersSize; i++)
    {
        ui->weatherModelComboBox->addItem(identifiers[i]);
    }
    NinjaFreeAllWeatherModelIdentifiers(identifiers, identifiersSize);

    weatherModelComboBoxCurrentIndexChanged(0);

    connect(ui->weatherModelGroupBox, &QGroupBox::toggled, this, &WeatherModelInput::weatherModelGroupBoxToggled);
    connect(ui->weatherModelDownloadButton, &QPushButton::clicked, this, &WeatherModelInput::weatherModelDownloadButtonClicked);
    connect(ui->weatherModelComboBox, &QComboBox::currentIndexChanged, this, &WeatherModelInput::weatherModelComboBoxCurrentIndexChanged);
    connect(ui->weatherModelTimeSelectAllButton, &QPushButton::clicked, this, &WeatherModelInput::weatherModelTimeSelectAllButtonClicked);
    connect(ui->weatherModelTimeSelectNoneButton, &QPushButton::clicked, this, &WeatherModelInput::weatherModelTimeSelectNoneButtonClicked);
}

void WeatherModelInput::weatherModelDownloadButtonClicked()
{
    QByteArray modelIdentifierByte = ui->weatherModelComboBox->currentText().toUtf8();
    QByteArray demFileByte   = ui->elevationInputFileLineEdit->property("fullpath").toString().toUtf8();
    const char* modelIdentifier = modelIdentifierByte.constData();
    const char* demFile = demFileByte.constData();
    int hours = ui->weatherModelSpinBox->value();

    if(ui->weatherModelComboBox->currentText().contains("PASTCAST"))
    {
        QDateTime startDT = ui->pastcastStartDateTimeEdit->dateTime();
        QDateTime endDT = ui->pastcastEndDateTimeEdit->dateTime();

        // Extract start date and time
        QDate startQDate = startDT.date();
        QTime startQTime = startDT.time();
        int startYear = startQDate.year();
        int startMonth = startQDate.month();
        int startDay = startQDate.day();
        int startHour = startQTime.hour();

        // Extract end date and time
        QDate endQDate = endDT.date();
        QTime endQTime = endDT.time();
        int endYear = endQDate.year();
        int endMonth = endQDate.month();
        int endDay = endQDate.day();
        int endHour = endQTime.hour();

        ninjaErr = NinjaFetchArchiveWeatherData(ninjaTools, modelIdentifier, demFile, startYear, startMonth, startDay, startHour, endYear, endMonth, endDay, endHour);
        if (ninjaErr != NINJA_SUCCESS)
        {
            qDebug() << "NinjaFetchArchiveWeatherData: " << ninjaErr;
        }


        return;
    }

    ninjaErr = NinjaFetchWeatherData(ninjaTools, modelIdentifier, demFile, hours);
    if (ninjaErr != NINJA_SUCCESS)
    {
        qDebug() << "NinjaFetchWeatherData: " << ninjaErr;
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
    QByteArray modelIdentifierByte = ui->weatherModelComboBox->currentText().toUtf8();
    const char* modelIdentifier = modelIdentifierByte.constData();
    int starHour, endHour;

    ninjaErr = NinjaGetWeatherModelHours(ninjaTools, modelIdentifier, &starHour, &endHour);
    if (ninjaErr != NINJA_SUCCESS)
    {
        qDebug() << "NinjaGetWeatherModelHours: " << ninjaErr;
    }

    ui->weatherModelSpinBox->setMinimum(starHour);
    ui->weatherModelSpinBox->setMaximum(endHour);
}

void WeatherModelInput::setUpTreeView()
{
    AppState& state = AppState::instance();
    state.isWeatherModelForecastValid = false;
    emit requestRefresh();

    // File Tree View
    fileModel = new QFileSystemModel(this);
    QString demFilePath = ui->elevationInputFileLineEdit->property("fullpath").toString();
    QFileInfo demFileInfo(demFilePath);

    fileModel->setRootPath(demFileInfo.absolutePath());
    fileModel->setNameFilters({"*.zip", "NOMADS-*", "20*", "UCAR-*", "PASTCAST-*"});
    fileModel->setFilter(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot);
    fileModel->setNameFilterDisables(false);

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

void WeatherModelInput::weatherModelFileTreeViewItemSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    AppState& state = AppState::instance();
    state.isWeatherModelForecastValid = true;

    if (selected.indexes().empty())
    {
        return;
    }

    QModelIndex index = selected.indexes().first();
    QFileInfo fileInfo = fileModel->fileInfo(index);
    if(fileInfo.isDir())
    {
        state.isWeatherModelForecastValid = false;
        timeModel->clear();
        emit requestRefresh();
        return;
    }

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

    emit requestRefresh();
}

void WeatherModelInput::weatherModelGroupBoxToggled(bool toggled)
{
    AppState& state = AppState::instance();
    state.isWeatherModelInitializationToggled = toggled;

    if (state.isWeatherModelInitializationToggled)
    {
        ui->domainAverageGroupBox->setChecked(false);
        ui->pointInitializationGroupBox->setChecked(false);
        state.isDomainAverageInitializationToggled = ui->domainAverageGroupBox->isChecked();
        state.isPointInitializationToggled = ui->pointInitializationGroupBox->isChecked();
    }

    emit requestRefresh();
}

void WeatherModelInput::weatherModelTimeSelectAllButtonClicked()
{
    ui->weatherModelTimeTreeView->selectAll();
}

void WeatherModelInput::weatherModelTimeSelectNoneButtonClicked()
{
    ui->weatherModelTimeTreeView->clearSelection();
}

