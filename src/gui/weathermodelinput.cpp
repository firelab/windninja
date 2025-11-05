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
    tools = NinjaMakeTools();
    int count = 0;
    const char** identifiers = NinjaGetAllWeatherModelIdentifiers(tools, &count);
    for (int i = 0; i < count; i++)
    {
        ui->weatherModelComboBox->addItem(identifiers[i]);
    }
    NinjaFreeAllWeatherModelIdentifiers(identifiers, count);

    connect(ui->weatherModelGroupBox, &QGroupBox::toggled, this, &WeatherModelInput::weatherModelGroupBoxToggled);
    connect(ui->weatherModelDownloadButton, &QPushButton::clicked, this, &WeatherModelInput::weatherModelDownloadButtonClicked);
    connect(ui->weatherModelComboBox, &QComboBox::currentIndexChanged, this, &WeatherModelInput::weatherModelComboBoxCurrentIndexChanged);
    connect(ui->weatherModelTimeSelectAllButton, &QPushButton::clicked, this, &WeatherModelInput::weatherModelTimeSelectAllButtonClicked);
    connect(ui->weatherModelTimeSelectNoneButton, &QPushButton::clicked, this, &WeatherModelInput::weatherModelTimeSelectNoneButtonClicked);

    weatherModelComboBoxCurrentIndexChanged(0);
}

void WeatherModelInput::weatherModelDownloadButtonClicked()
{
    QByteArray modelNameByte = ui->weatherModelComboBox->currentText().toUtf8();
    QByteArray demFileByte   = ui->elevationInputFileLineEdit->property("fullpath").toString().toUtf8();

    const char* modelName = modelNameByte.constData();
    const char* demFile   = demFileByte.constData();
    int hours = ui->weatherModelSpinBox->value();

    int err = NinjaFetchWeatherData(tools, modelName, demFile, hours);
    if (err != NINJA_SUCCESS)
    {
        qDebug() << "NinjaFetchWeatherData: " << err;
    }

    setUpTreeView();
}

void WeatherModelInput::weatherModelComboBoxCurrentIndexChanged(int index)
{
    int starHour, endHour;

    QByteArray modelNameByte = ui->weatherModelComboBox->currentText().toUtf8();
    const char* modelName = modelNameByte.constData();

    int NinjaErr = NinjaGetWeatherModelHours(tools, modelName, &starHour, &endHour);
    if (NinjaErr != NINJA_SUCCESS)
    {
        qDebug() << "NinjaGetWeatherModelHours: " << NinjaErr;
    }
    ui->weatherModelSpinBox->setMinimum(starHour);
    ui->weatherModelSpinBox->setMaximum(endHour);
}

void WeatherModelInput::setUpTreeView()
{
    weatherModelFileSystemModel = new QFileSystemModel(this);
    QString path = ui->elevationInputFileLineEdit->property("fullpath").toString();
    QFileInfo fileInfo(path);

    weatherModelFileSystemModel->setRootPath(fileInfo.absolutePath());
    weatherModelFileSystemModel->setNameFilters({"*.zip", "NOMADS-*", "20*"});
    weatherModelFileSystemModel->setFilter(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot);
    weatherModelFileSystemModel->setNameFilterDisables(false);

    ui->weatherModelFileTreeView->setModel(weatherModelFileSystemModel);
    ui->weatherModelFileTreeView->setRootIndex(weatherModelFileSystemModel->index(fileInfo.absolutePath()));
    ui->weatherModelFileTreeView->setSelectionMode(QAbstractItemView::MultiSelection);
    ui->weatherModelFileTreeView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->weatherModelFileTreeView->setAnimated(true);
    ui->weatherModelFileTreeView->setUniformRowHeights(true);

    QHeaderView *header = ui->weatherModelFileTreeView->header();
    header->setStretchLastSection(false);
    header->setSectionResizeMode(0, QHeaderView::Stretch);
    header->resizeSection(0, 400);
    header->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    header->setSectionResizeMode(1, QHeaderView::ResizeToContents);

    ui->weatherModelFileTreeView->hideColumn(1);
    ui->weatherModelFileTreeView->hideColumn(2);

    connect(ui->weatherModelFileTreeView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &WeatherModelInput::weatherModelFileTreeViewItemSelectionChanged);
}

void WeatherModelInput::weatherModelFileTreeViewItemSelectionChanged()
{
    QItemSelectionModel *selectionModel = ui->weatherModelFileTreeView->selectionModel();
    QModelIndexList indexes = selectionModel->selectedIndexes();

    if (indexes.isEmpty())
        return;

    QModelIndex index = indexes.first();
    QString filePath;

    QFileSystemModel *model = qobject_cast<QFileSystemModel*>(ui->weatherModelFileTreeView->model());
    if (model)
        filePath = model->filePath(index);

    std::string filepath = filePath.toStdString();
    std::string timeZone = ui->timeZoneComboBox->currentText().toStdString();
    int timeListCount = 0;
    const char ** timeList = NinjaGetWeatherModelTimeList(tools, &timeListCount, filepath.c_str(), timeZone.c_str());

    QStandardItemModel *timestepModel = new QStandardItemModel(this);
    for (int i = 0; i < timeListCount; ++i) {
        QString timestep = QString::fromUtf8(timeList[i]);
        timestepModel->appendRow(new QStandardItem(timestep));
    }

    ui->weatherModelTimeTreeView->setModel(timestepModel);
    ui->weatherModelTimeTreeView->setSortingEnabled(true);
    ui->weatherModelTimeTreeView->sortByColumn(0, Qt::AscendingOrder);
    ui->weatherModelTimeTreeView->setSelectionMode(QAbstractItemView::MultiSelection);
    ui->weatherModelTimeTreeView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->weatherModelTimeTreeView->selectAll();
    QHeaderView *header = ui->weatherModelTimeTreeView->header();
    header->setVisible(false);
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

