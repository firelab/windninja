 /******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja Qt GUI
 * Purpose:  Handles GUI related logic for the Domain Average Page
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

#include "domainAverageInput.h"
#include "ui_mainWindow.h"

DomainAverageInput::DomainAverageInput(Ui::MainWindow* ui, QObject* parent)
    : QObject(parent),
    ui(ui)
{
    setupDomainAverageTableWidgets();
    ui->domainAverageTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    connect(ui->inputWindHeightComboBox, &QComboBox::currentIndexChanged, this, &DomainAverageInput::windHeightComboBoxCurrentIndexChanged);
    connect(ui->clearTableButton, &QPushButton::clicked, this, &DomainAverageInput::clearTableButtonClicked);
    connect(ui->domainAverageTable, &QTableWidget::cellChanged, this, &DomainAverageInput::domainAverageTableCheckRows);
    connect(ui->diurnalCheckBox, &QCheckBox::clicked, this, &DomainAverageInput::domainAverageTableCheckRows);
    connect(ui->stabilityCheckBox, &QCheckBox::clicked, this, &DomainAverageInput::domainAverageTableCheckRows);
    connect(ui->domainAverageGroupBox, &QGroupBox::toggled, this, &DomainAverageInput::domainAverageGroupBoxToggled);
    connect(ui->domainAverageGroupBox, &QGroupBox::toggled, this, &DomainAverageInput::domainAverageTableCheckRows);
    connect(this, &DomainAverageInput::updateState, &AppState::instance(), &AppState::updateDomainAverageInputState);
}

int DomainAverageInput::countNumRuns()
{
    int numActiveRows = 0;
    for(int rowIdx = 0; rowIdx < ui->domainAverageTable->rowCount(); rowIdx++)
    {
        if(speedSpins[rowIdx]->value() != 0.0 || dirSpins[rowIdx]->value() != 0.0)
        {
            //numActiveRows = numActiveRows + 1;  // this method skips adding up the in between 0.0, 0.0 spd, dir rows
            numActiveRows = rowIdx + 1;  // last active row, as a 1 to N count, rather than as a 0 to N-1 rowIdx, this method properly grabs the in between 0.0, 0.0 spd, dir rows
        }
    }

    return numActiveRows;
}

void DomainAverageInput::domainAverageTableCheckRows()
{
    int numRuns = countNumRuns();

    int numZeroRuns = 0;
    for(int runIdx = 0; runIdx < numRuns; runIdx++)
    {
        if(speedSpins[runIdx]->value() == 0.0)
        {
            numZeroRuns = numZeroRuns + 1;
        }
    }

    bool valid = true;
    if(numRuns == 0)
    {
        if(ui->diurnalCheckBox->isChecked() == false)
        {
            //ui->treeWidget->topLevelItem(1)->child(3)->child(0)->setIcon(0, crossIcon);
            //ui->treeWidget->topLevelItem(1)->child(3)->child(0)->setToolTip(0, "No runs have been added, diurnal is not active");
            qDebug() << "No runs have been added, diurnal is not active";
            valid = false;
        }
        else // if(ui->diurnalCheckBox->isChecked() == true)
        {
            //ui->treeWidget->topLevelItem(1)->child(3)->child(0)->setIcon(0, warnIcon);
            //ui->treeWidget->topLevelItem(1)->child(3)->child(0)->setToolTip(0, "No runs have been added, one run will be done at speed = 0, dir = 0 while using diurnal");
            qDebug() << "No runs have been added, one run will be done at speed = 0, dir = 0 while using diurnal";
            valid = true;
        }
    }
    else // if(numRuns != 0)
    {
        if(numZeroRuns == 0)
        {
            //ui->treeWidget->topLevelItem(1)->child(3)->child(0)->setIcon(0, tickIcon);
            //ui->treeWidget->topLevelItem(1)->child(3)->child(0)->setToolTip(0, QString::number(numRuns)+" runs");
            qDebug() << QString::number(numRuns)+" runs";
            valid = true;
        }
        else // if(numZeroRuns != 0)
        {
            if(ui->diurnalCheckBox->isChecked() == true)
            {
                //ui->treeWidget->topLevelItem(1)->child(3)->child(0)->setIcon(0, warnIcon);
                //ui->treeWidget->topLevelItem(1)->child(3)->child(0)->setToolTip(0, QString::number(numRuns)+" runs have been added, detecting "+QString::number(numZeroRuns)+" zero wind speed runs, diurnal is active so will continue the runs");
                qDebug() << QString::number(numRuns)+" runs have been added, detecting "+QString::number(numZeroRuns)+" zero wind speed runs, diurnal is active so will continue the runs";
                valid = true;
            }
            else // if(ui->diurnalCheckBox->isChecked() == false)
            {
                //ui->treeWidget->topLevelItem(1)->child(3)->child(0)->setIcon(0, crossIcon);
                //ui->treeWidget->topLevelItem(1)->child(3)->child(0)->setToolTip(0, QString::number(numRuns)+" runs have been added, but detecting "+QString::number(numZeroRuns)+" zero wind speed runs without diurnal being active");
                qDebug() << QString::number(numRuns)+" runs have been added, but detecting "+QString::number(numZeroRuns)+" zero wind speed runs without diurnal being active";
                valid = false;
            }
        }
    }

    AppState::instance().isDomainAverageWindInputTableValid = valid;
qDebug() << "appState =" << valid;
    emit updateState();
}

void DomainAverageInput::clearTableButtonClicked()
{
    // AppState::instance().isDomainAverageWindInputTableValid is set
    // and updateState() is emitted here, by the call to the domainAverageTableCheckRows() function

    speedSpins.clear();
    dirSpins.clear();
    timeEdits.clear();
    dateEdits.clear();
    cloudSpins.clear();
    airTempSpins.clear();

    ui->domainAverageTable->clearContents();

    setupDomainAverageTableWidgets();

    domainAverageTableCheckRows();
}

void DomainAverageInput::windHeightComboBoxCurrentIndexChanged(int index)
{
    switch(index)
    {
    case 0:
        ui->inputWindHeightSpinBox->setValue(20.00);
        ui->inputWindHeightSpinBox->setEnabled(false);
        ui->inputWindHeightUnitsComboBox->setCurrentIndex(0);
        break;

    case 1:
        ui->inputWindHeightSpinBox->setValue(10.00);
        ui->inputWindHeightSpinBox->setEnabled(false);
        ui->inputWindHeightUnitsComboBox->setCurrentIndex(1);
        break;

    case 2:
        ui->inputWindHeightSpinBox->setValue(0.00);
        ui->inputWindHeightSpinBox->setEnabled(true);
        ui->inputWindHeightUnitsComboBox->setCurrentIndex(0);
        ui->inputWindHeightUnitsComboBox->setEnabled(true);
        break;
    }
}

void DomainAverageInput::domainAverageGroupBoxToggled()
{
    AppState& state = AppState::instance();
    state.isDomainAverageInitializationToggled = ui->domainAverageGroupBox->isChecked();

    if (state.isDomainAverageInitializationToggled)
    {
        ui->pointInitializationGroupBox->setChecked(false);
        ui->weatherModelGroupBox->setChecked(false);
        state.isPointInitializationToggled = ui->pointInitializationGroupBox->isChecked();
        state.isWeatherModelInitializationToggled = ui->weatherModelGroupBox->isChecked();
    }

    emit updateState();
}

void DomainAverageInput::setupDomainAverageTableWidgets()
{
    QTableWidget* table = ui->domainAverageTable;
    int rows = table->rowCount();

    speedSpins.resize(rows);
    dirSpins.resize(rows);
    timeEdits.resize(rows);
    dateEdits.resize(rows);
    cloudSpins.resize(rows);
    airTempSpins.resize(rows);

    for(int row = 0; row < rows; row++)
    {
        speedSpins[row] = new QDoubleSpinBox(table);
        speedSpins[row]->setRange(0.0, 500.0);
        speedSpins[row]->setDecimals(2);
        table->setCellWidget(row, 0, speedSpins[row]);

        dirSpins[row] = new QDoubleSpinBox(table);
        dirSpins[row]->setRange(0.0, 359.9);
        dirSpins[row]->setDecimals(0);
        table->setCellWidget(row, 1, dirSpins[row]);

        timeEdits[row] = new QTimeEdit(QTime::currentTime(), table);
        timeEdits[row]->setDisplayFormat("HH:mm");
        table->setCellWidget(row, 2, timeEdits[row]);

        dateEdits[row] = new QDateEdit(QDate::currentDate(), table);
        dateEdits[row]->setCalendarPopup(true);
        dateEdits[row]->setDisplayFormat("MM/dd/yyyy");
        table->setCellWidget(row, 3, dateEdits[row]);

        cloudSpins[row] = new QDoubleSpinBox(table);
        cloudSpins[row]->setRange(0.0, 100.0);
        cloudSpins[row]->setDecimals(0);
        table->setCellWidget(row, 4, cloudSpins[row]);

        airTempSpins[row] = new QDoubleSpinBox(table);
        airTempSpins[row]->setRange(-40.0, 200.0);
        airTempSpins[row]->setDecimals(0);
        airTempSpins[row]->setValue(72.0);
        table->setCellWidget(row, 5, airTempSpins[row]);

        connect(speedSpins[row], &QDoubleSpinBox::valueChanged, this, &DomainAverageInput::domainAverageTableCheckRows);
        connect(dirSpins[row], &QDoubleSpinBox::valueChanged, this, &DomainAverageInput::domainAverageTableCheckRows);
        connect(timeEdits[row], &QTimeEdit::timeChanged, this, &DomainAverageInput::domainAverageTableCheckRows);
        connect(dateEdits[row], &QDateEdit::dateChanged, this, &DomainAverageInput::domainAverageTableCheckRows);
        connect(cloudSpins[row], &QDoubleSpinBox::valueChanged, this, &DomainAverageInput::domainAverageTableCheckRows);
        connect(airTempSpins[row], &QDoubleSpinBox::valueChanged, this, &DomainAverageInput::domainAverageTableCheckRows);
    }

    if(ui->diurnalCheckBox->isChecked() || ui->stabilityCheckBox->isChecked())
    {
        table->showColumn(2);
        table->showColumn(3);
        table->showColumn(4);
        table->showColumn(5);
        table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    }
    else
    {
        table->hideColumn(2);
        table->hideColumn(3);
        table->hideColumn(4);
        table->hideColumn(5);
        table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    }
}

