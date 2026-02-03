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
    connect(ui->domainAverageTable, &QTableWidget::cellChanged, this, &DomainAverageInput::domainAverageTableCellChanged);
    connect(ui->diurnalCheckBox, &QCheckBox::clicked, this, &DomainAverageInput::domainAverageTableCheckRows);
    connect(ui->stabilityCheckBox, &QCheckBox::clicked, this, &DomainAverageInput::domainAverageTableCheckRows);
    connect(ui->domainAverageGroupBox, &QGroupBox::toggled, this, &DomainAverageInput::domainAverageGroupBoxToggled);
    connect(this, &DomainAverageInput::updateState, &AppState::instance(), &AppState::updateDomainAverageInputState);
}

void DomainAverageInput::domainAverageTableCellChanged(int row, int column)
{
    QTableWidget* table = ui->domainAverageTable;
    QTableWidgetItem* item = table->item(row, column);
    if(item)
    {
        QString value = item->text().trimmed();
        bool valid = false;
        QString errorMessage;

        // Allow empty input
        if (value.isEmpty())
        {
            valid = true;
        }
        else
        {
            switch (column)
            {
            case 0:
            {
                double dbl = value.toDouble(&valid);
                if (!valid || dbl < 0.0)
                {
                    valid = false;
                    errorMessage = "Must be a positive number";
                }
                break;
            }
            case 1:
            {
                double dbl = value.toDouble(&valid);
                if (!valid || dbl < 0.0 || dbl > 359.9)
                {
                    valid = false;
                    errorMessage = "Must be a number between 0.0 and 359.9";
                }
                break;
            }
            case 2:
            {
                QTime t = QTime::fromString(value, "HH:mm");
                valid = t.isValid();
                if (!valid)
                {
                    errorMessage = "Must be a valid 24h time (HH:mm)";
                }
                break;
            }
            case 3:
            {
                QDate d = QDate::fromString(value, "MM/dd/yyyy");
                valid = d.isValid();
                if (!valid)
                {
                    errorMessage = "Must be a valid date (MM/dd/yyyy)";
                }
                break;
            }
            case 4:
            {
                double dbl = value.toDouble(&valid);
                if (!valid || dbl < 0.0 || dbl > 100.0)
                {
                    valid = false;
                    errorMessage = "Must be a number between 0.0 and 100.0";
                }
                break;
            }
            case 5:
            {
                double dbl = value.toDouble(&valid);
                if (!valid || dbl < -40.0 || dbl > 200.0)
                {
                    valid = false;
                    errorMessage = "Must be a number between -40.0 and 200.0";
                }
                break;
            }
            default:
                valid = true;
            }
        }

        QPair<int, int> cell(row, column);
        if (!valid)
        {
            invalidDAWCells.insert(cell);
            item->setBackground(Qt::red);
            item->setToolTip(errorMessage);
        }
        else
        {
            invalidDAWCells.remove(cell);
            item->setBackground(QBrush());  // Reset to default
            item->setToolTip("");
        }
    }  // if(item)

    AppState::instance().isDomainAverageWindInputTableValid = invalidDAWCells.isEmpty();

    emit updateState();

    domainAverageTableCheckRows();
}

void DomainAverageInput::domainAverageTableCheckRows()
{
    QTableWidget* table = ui->domainAverageTable;

    int numTableCols = 6;  // table->columnCount()
    if(!ui->diurnalCheckBox->isChecked() && !ui->stabilityCheckBox->isChecked())
    {
        numTableCols = 2;
    }

    int existingRowsCount = 0;
    int filledRowsCount = 0;
    int invalidCellsCount = 0;
    for(int rowIdx = 0; rowIdx < table->rowCount(); rowIdx++)
    {
        int numFilledTableCols = 0;
        for(int colIdx = 0; colIdx < numTableCols; colIdx++)
        {
            QTableWidgetItem* tableItem = table->item(rowIdx, colIdx);
            if(tableItem && !tableItem->text().trimmed().isEmpty())
            {
                numFilledTableCols = numFilledTableCols + 1;
            }
        }

        if(numFilledTableCols != 0)
        {
            existingRowsCount = existingRowsCount + 1;
        }
        if(numFilledTableCols == numTableCols)
        {
            filledRowsCount = filledRowsCount + 1;
        }

        for(const QPair<int,int>& cell : invalidDAWCells)
        {
            if(rowIdx == cell.first && cell.second < numTableCols)
            {
                invalidCellsCount = invalidCellsCount + 1;
            }
        }
    }

    bool valid = true;
    if(invalidCellsCount != 0)
    {
        valid = false;
    }
    if(filledRowsCount == 0)
    {
        valid = false;
    }
    if(filledRowsCount != existingRowsCount)
    {
        valid = false;
    }

    AppState::instance().isDomainAverageWindInputTableValid = valid;

    emit updateState();
}

void DomainAverageInput::clearTableButtonClicked()
{
    AppState& state = AppState::instance();
    AppState::instance().isDomainAverageWindInputTableValid = false;

    ui->domainAverageTable->clearContents();
    invalidDAWCells.clear();

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

