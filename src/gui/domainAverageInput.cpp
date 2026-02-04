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

void DomainAverageInput::domainAverageTableCheckRows()
{
    //bool valid = false;
    // count numRuns
    int numRuns = 0;
    for(int rowIdx = 0; rowIdx < ui->domainAverageTable->rowCount(); rowIdx++)
    {
        QDoubleSpinBox* speedSpin = qobject_cast<QDoubleSpinBox*>(ui->domainAverageTable->cellWidget(rowIdx, 0));
        QDoubleSpinBox* directionSpin = qobject_cast<QDoubleSpinBox*>(ui->domainAverageTable->cellWidget(rowIdx, 1));
        QTimeEdit* timeEdit = qobject_cast<QTimeEdit*>(ui->domainAverageTable->cellWidget(rowIdx, 2));
        QDateEdit* dateEdit = qobject_cast<QDateEdit*>(ui->domainAverageTable->cellWidget(rowIdx, 3));
        QDoubleSpinBox* cloudCoverSpin = qobject_cast<QDoubleSpinBox*>(ui->domainAverageTable->cellWidget(rowIdx, 4));
        QDoubleSpinBox* airTempSpin = qobject_cast<QDoubleSpinBox*>(ui->domainAverageTable->cellWidget(rowIdx, 5));

        if(!speedSpin || !directionSpin || !timeEdit || !dateEdit || !cloudCoverSpin || !airTempSpin)
        {
            continue;
        }

        if(speedSpin->value() != 0.0 || directionSpin->value() != 0.0)
        {
            numRuns = numRuns + 1;
        }
    }

    bool valid = true;
    if(numRuns == 0 && ui->diurnalCheckBox->isChecked() == false)
    {
        //ui->treeWidget->topLevelItem(1)->child(3)->child(0)->setIcon(0, crossIcon);
        //ui->treeWidget->topLevelItem(1)->child(3)->child(0)->setToolTip(0, "No runs have been added, diurnal is not active");
        qDebug() << "No runs have been added, diurnal is not active";
        valid = false;
    }
    else if(numRuns == 0 && ui->diurnalCheckBox->isChecked() == true)
    {
        //ui->treeWidget->topLevelItem(1)->child(3)->child(0)->setIcon(0, warnIcon);
        //ui->treeWidget->topLevelItem(1)->child(3)->child(0)->setToolTip(0, "No runs have been added, one run will be done at speed = 0, dir = 0 while using diurnal");
        qDebug() << "No runs have been added, one run will be done at speed = 0, dir = 0 while using diurnal";
        valid = true;
    }
    else
    {
        //ui->treeWidget->topLevelItem(1)->child(3)->child(0)->setIcon(0, tickIcon);
        //ui->treeWidget->topLevelItem(1)->child(3)->child(0)->setToolTip(0, QString::number(numRuns)+" runs");
        qDebug() << QString::number(numRuns)+" runs";
        valid = true;
        // override if any 0.0 wind speed runs are detected, warn and run if diurnal, stop if not diurnal
        for(int runIdx=0; runIdx < numRuns; runIdx++)
        {
            QDoubleSpinBox* speedSpin = qobject_cast<QDoubleSpinBox*>(ui->domainAverageTable->cellWidget(runIdx, 0));
            if(!speedSpin)
            {
                continue;
            }

            if(speedSpin->value() == 0.0)
            {
                if(ui->diurnalCheckBox->isChecked() == false)
                {
                    //ui->treeWidget->topLevelItem(1)->child(3)->child(0)->setIcon(0, crossIcon);
                    //ui->treeWidget->topLevelItem(1)->child(3)->child(0)->setToolTip(0, QString::number(numRuns)+" runs have been added, but detecting at least one 0.0 wind speed run without diurnal being active");
                    qDebug() << QString::number(numRuns)+" runs have been added, but detecting at least one 0.0 wind speed run without diurnal being active";
                    valid = false;
                }
                else  // if(ui->diurnalCheckBox->isChecked() == true)
                {
                    //ui->treeWidget->topLevelItem(1)->child(3)->child(0)->setIcon(0, warnIcon);
                    //ui->treeWidget->topLevelItem(1)->child(3)->child(0)->setToolTip(0, QString::number(numRuns)+" runs have been added, detecting at least one 0.0 wind speed run, diurnal is active so will continue the runs");
                    qDebug() << QString::number(numRuns)+" runs have been added, detecting at least one 0.0 wind speed run, diurnal is active so will continue the runs";
                    valid = true;
                }
                break;
            }
        }
    }

    AppState::instance().isDomainAverageWindInputTableValid = valid;
qDebug() << "appState =" << valid;
    emit updateState();
}

void DomainAverageInput::clearTableButtonClicked()
{
    AppState& state = AppState::instance();
    AppState::instance().isDomainAverageWindInputTableValid = false;

    for(int rowIdx = 0; rowIdx < ui->domainAverageTable->rowCount(); rowIdx++)
    {
        QDoubleSpinBox* speedSpin = qobject_cast<QDoubleSpinBox*>(ui->domainAverageTable->cellWidget(rowIdx, 0));
        QDoubleSpinBox* directionSpin = qobject_cast<QDoubleSpinBox*>(ui->domainAverageTable->cellWidget(rowIdx, 1));
        QTimeEdit* timeEdit = qobject_cast<QTimeEdit*>(ui->domainAverageTable->cellWidget(rowIdx, 2));
        QDateEdit* dateEdit = qobject_cast<QDateEdit*>(ui->domainAverageTable->cellWidget(rowIdx, 3));
        QDoubleSpinBox* cloudCoverSpin = qobject_cast<QDoubleSpinBox*>(ui->domainAverageTable->cellWidget(rowIdx, 4));
        QDoubleSpinBox* airTempSpin = qobject_cast<QDoubleSpinBox*>(ui->domainAverageTable->cellWidget(rowIdx, 5));

        disconnect(speedSpin, &QDoubleSpinBox::valueChanged, this, &DomainAverageInput::domainAverageTableCheckRows);
        disconnect(directionSpin, &QDoubleSpinBox::valueChanged, this, &DomainAverageInput::domainAverageTableCheckRows);
        disconnect(timeEdit, &QTimeEdit::timeChanged, this, &DomainAverageInput::domainAverageTableCheckRows);
        disconnect(dateEdit, &QDateEdit::dateChanged, this, &DomainAverageInput::domainAverageTableCheckRows);
        disconnect(cloudCoverSpin, &QDoubleSpinBox::valueChanged, this, &DomainAverageInput::domainAverageTableCheckRows);
        disconnect(airTempSpin, &QDoubleSpinBox::valueChanged, this, &DomainAverageInput::domainAverageTableCheckRows);

        //tableWidget->setCellWidget(rowIdx, colIdx, nullptr); // Destroys the old widget connections
    }

    ui->domainAverageTable->clearContents();

    setupDomainAverageTableWidgets();

    emit updateState();
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

    for (int row = 0; row < rows; ++row)
    {
        QDoubleSpinBox* speedSpin = new QDoubleSpinBox(table);
        speedSpin->setRange(0.0, 500.0);
        speedSpin->setDecimals(2);
        table->setCellWidget(row, 0, speedSpin);

        QDoubleSpinBox* dirSpin = new QDoubleSpinBox(table);
        dirSpin->setRange(0.0, 359.9);
        dirSpin->setDecimals(0);
        table->setCellWidget(row, 1, dirSpin);

        QTimeEdit* timeEdit = new QTimeEdit(QTime::currentTime(), table);
        timeEdit->setDisplayFormat("HH:mm");
        table->setCellWidget(row, 2, timeEdit);

        QDateEdit* dateEdit = new QDateEdit(QDate::currentDate(), table);
        dateEdit->setCalendarPopup(true);
        dateEdit->setDisplayFormat("MM/dd/yyyy");
        table->setCellWidget(row, 3, dateEdit);

        QDoubleSpinBox* cloudSpin = new QDoubleSpinBox(table);
        cloudSpin->setRange(0.0, 100.0);
        cloudSpin->setDecimals(0);
        table->setCellWidget(row, 4, cloudSpin);

        QDoubleSpinBox* airTempSpin = new QDoubleSpinBox(table);
        airTempSpin->setRange(-40.0, 200.0);
        airTempSpin->setDecimals(0);
        airTempSpin->setValue(72.0);
        table->setCellWidget(row, 5, airTempSpin);

        connect(speedSpin, &QDoubleSpinBox::valueChanged, this, &DomainAverageInput::domainAverageTableCheckRows);
        connect(dirSpin, &QDoubleSpinBox::valueChanged, this, &DomainAverageInput::domainAverageTableCheckRows);
        connect(timeEdit, &QTimeEdit::timeChanged, this, &DomainAverageInput::domainAverageTableCheckRows);
        connect(dateEdit, &QDateEdit::dateChanged, this, &DomainAverageInput::domainAverageTableCheckRows);
        connect(cloudSpin, &QDoubleSpinBox::valueChanged, this, &DomainAverageInput::domainAverageTableCheckRows);
        connect(airTempSpin, &QDoubleSpinBox::valueChanged, this, &DomainAverageInput::domainAverageTableCheckRows);
    }

    //connect(speedSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &DomainAverageInput::domainAverageTableCheckRows);
    //connect(dirSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &DomainAverageInput::domainAverageTableCheckRows);
    //connect(timeEdit, &QTimeEdit::timeChanged, this, &DomainAverageInput::domainAverageTableCheckRows);
    //connect(dateEdit, &QDateEdit::dateChanged, this, &DomainAverageInput::domainAverageTableCheckRows);

    //for(int rowIdx = 0; rowIdx < rows; rowIdx++)
    //{
    //    connect(tree->wind->windTable->speed[i], SIGNAL(valueChanged(double)), this, &DomainAverageInput::domainAverageTableCheckRows);
    //    connect(tree->wind->windTable->dir[i], SIGNAL(valueChanged(int)), this, &DomainAverageInput::domainAverageTableCheckRows);
    //}

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

