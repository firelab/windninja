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
    connect(ui->weatherModelDataDownloadButton, &QPushButton::clicked, this, &WeatherModelInput::weatherModelDataDownloadButtonClicked);
}

void WeatherModelInput::weatherModelDataDownloadButtonClicked()
{
    QString wxModelType = ui->weatherModelDataComboBox->currentText();
    QString DEMFile = ui->elevationInputFileLineEdit->property("fullpath").toString();
    int nHours = ui->weatherModelDataSpinBox->value();
    NinjaArmyH *ninjaArmy = nullptr;
    char ** options = nullptr;
    NinjaErr err = NinjaFetchForecast(ninjaArmy, wxModelType.toUtf8().constData(), nHours, DEMFile.toUtf8().constData(), options);
    if(err != NINJA_SUCCESS)
    {
        qDebug() << "NinjaFetchForecast: err =" << err;
    }

    setUpTreeView();
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

    ui->weatherModelDataTreeView->setModel(weatherModelFileSystemModel);
    ui->weatherModelDataTreeView->setRootIndex(weatherModelFileSystemModel->index(fileInfo.absolutePath()));
    ui->weatherModelDataTreeView->setSelectionMode(QAbstractItemView::MultiSelection);
    ui->weatherModelDataTreeView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->weatherModelDataTreeView->setAnimated(true);
    ui->weatherModelDataTreeView->setUniformRowHeights(true);

    QHeaderView *header = ui->weatherModelDataTreeView->header();
    header->setStretchLastSection(false);
    header->setSectionResizeMode(0, QHeaderView::Stretch);
    header->resizeSection(0, 400);
    header->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    header->setSectionResizeMode(1, QHeaderView::ResizeToContents);

    ui->weatherModelDataTreeView->hideColumn(1);
    ui->weatherModelDataTreeView->hideColumn(2);

    connect(ui->weatherModelDataTreeView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &WeatherModelInput::weatherModelDataTreeViewItemSelectionChanged);
}

void WeatherModelInput::weatherModelDataTreeViewItemSelectionChanged()
{

}

void WeatherModelInput::weatherModelGroupBoxToggled(bool checked)
{
    AppState& state = AppState::instance();
    state.isWeatherModelInitializationToggled = checked;

    if (state.isWeatherModelInitializationToggled)
    {
        ui->domainAverageGroupBox->setChecked(false);
        ui->pointInitializationGroupBox->setChecked(false);
        state.isDomainAverageInitializationToggled = ui->domainAverageGroupBox->isChecked();
        state.isPointInitializationToggled = ui->pointInitializationGroupBox->isChecked();
    }

    emit requestRefresh();
}

