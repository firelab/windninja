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

#include "domainaverageinput.h"
#include "ui_mainwindow.h"

DomainAverageInput::DomainAverageInput(Ui::MainWindow* ui, QObject* parent)
    : QObject(parent),
    ui(ui)
{
    ui->domainAverageTable->hideColumn(2);
    ui->domainAverageTable->hideColumn(3);
    ui->domainAverageTable->hideColumn(4);
    ui->domainAverageTable->hideColumn(5);
    ui->domainAverageTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    connect(ui->inputWindHeightComboBox, &QComboBox::currentIndexChanged, this, &DomainAverageInput::windHeightComboBoxCurrentIndexChanged);
    connect(ui->clearTableButton, &QPushButton::clicked, this, &DomainAverageInput::clearTableButtonClicked);
    connect(ui->domainAverageTable, &QTableWidget::cellChanged, this, &DomainAverageInput::domainAverageTableCellChanged);
    connect(ui->domainAverageGroupBox, &QGroupBox::toggled, this, &DomainAverageInput::domainAverageGroupBoxToggled);
}

void DomainAverageInput::domainAverageTableCellChanged(int row, int column)
{
    QTableWidget* table = ui->domainAverageTable;
    QTableWidgetItem* item = table->item(row, column);
    if (!item) return;

    QString value = item->text().trimmed();
    bool valid = false;
    QString errorMessage;

    // Allow empty input
    if (value.isEmpty())
    {
        valid = true;
    } else
    {
        switch (column)
        {
        case 0:
        {
            double d = value.toDouble(&valid);
            if (!valid || d <= 0)
            {
                valid = false;
                errorMessage = "Must be a positive number";
            }
            break;
        }
        case 1:
        {
            int i = value.toDouble(&valid);
            if (!valid || i < 0 || i > 359.9)
            {
                valid = false;
                errorMessage = "Must be a number between 0 and 359";
            }
            break;
        }
        case 2:
        {
            QTime t = QTime::fromString(value, "hh:mm");
            valid = t.isValid();
            if (!valid) errorMessage = "Must be a valid 24h time (hh:mm)";
            break;
        }
        case 3:
        {
            QDate d = QDate::fromString(value, "MM/dd/yyyy");
            valid = d.isValid();
            if (!valid) errorMessage = "Must be a valid date (MM/DD/YYYY)";
            break;
        }
        case 4:
        {
            int i = value.toDouble(&valid);
            if (!valid || i < 0 || i > 100)
            {
                valid = false;
                errorMessage = "Must be a number between 0 and 100";
            }
            break;
        }
        case 5:
        {
            value.toInt(&valid);
            if (!valid) errorMessage = "Must be an integer";
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

    AppState::instance().isDomainAverageWindInputTableValid = invalidDAWCells.isEmpty();
    emit requestRefresh();
}


void DomainAverageInput::clearTableButtonClicked()
{
    AppState& state = AppState::instance();
    AppState::instance().isDomainAverageWindInputTableValid = true;

    ui->domainAverageTable->clearContents();
    invalidDAWCells.clear();

    emit requestRefresh();
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

    emit requestRefresh();
}

