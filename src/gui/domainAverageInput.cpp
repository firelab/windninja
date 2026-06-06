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

    ui->domainAverageTable->setTabKeyNavigation(false);

    spinBoxLineEditWasBlank = false;

    connect(ui->inputWindHeightComboBox, &QComboBox::currentIndexChanged, this, &DomainAverageInput::windHeightComboBoxCurrentIndexChanged);
    connect(ui->clearTableButton, &QPushButton::clicked, this, &DomainAverageInput::clearTableButtonClicked);
    connect(ui->domainAverageTable, &QTableWidget::cellChanged, this, &DomainAverageInput::domainAverageTableCheckRows);
    connect(ui->diurnalCheckBox, &QCheckBox::clicked, this, &DomainAverageInput::domainAverageTableCheckRows);
    connect(ui->stabilityCheckBox, &QCheckBox::clicked, this, &DomainAverageInput::domainAverageTableCheckRows);
    connect(ui->domainAverageGroupBox, &QGroupBox::toggled, this, &DomainAverageInput::domainAverageGroupBoxToggled);
    connect(ui->domainAverageGroupBox, &QGroupBox::toggled, this, &DomainAverageInput::domainAverageTableCheckRows);
    connect(this, &DomainAverageInput::updateState, &AppState::instance(), &AppState::updateDomainAverageInputState);
}

void DomainAverageInput::domainAverageTableOnSpinBoxLineEditTextEdited(const QString &text)
{
    spinBoxLineEditWasBlank = text.trimmed().isEmpty();
}

void DomainAverageInput::domainAverageTableOnSpinBoxLineEditEditingFinished()
{
    if(!spinBoxLineEditWasBlank)
    {
        return;
    }

    QDoubleSpinBox* spin = qobject_cast<QDoubleSpinBox*>(sender());
    if(!spin)
    {
        return;
    }

    spin->setValue(spin->property("defaultValue").toDouble());
    spinBoxLineEditWasBlank = false;
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
    AppState& state = AppState::instance();

    int numRuns = countNumRuns();

    int numZeroRuns = 0;
    for(int runIdx = 0; runIdx < numRuns; runIdx++)
    {
        if(speedSpins[runIdx]->value() == 0.0)
        {
            numZeroRuns = numZeroRuns + 1;
        }
    }

    state.DomainAvgTableNumRuns = numRuns;
    state.DomainAvgTableNumZeroRuns = numZeroRuns;

    emit updateState();
}

void DomainAverageInput::clearTableButtonClicked()
{
    // AppState::instance().DomainAvgTableNumRuns and AppState::instance().DomainAvgTableNumZeroRuns are set
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
        ui->inputWindHeightUnitsComboBox->setEnabled(true);
        break;
    }
}

void DomainAverageInput::domainAverageGroupBoxToggled()
{
    if(ui->domainAverageGroupBox->isChecked())
    {
        ui->pointInitializationGroupBox->setChecked(false);
        ui->weatherModelGroupBox->setChecked(false);
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
        speedSpins[row]->setProperty("defaultValue", 0.0);
        speedSpins[row]->setValue(speedSpins[row]->property("defaultValue").toDouble());
        table->setCellWidget(row, 0, speedSpins[row]);

        dirSpins[row] = new QDoubleSpinBox(table);
        dirSpins[row]->setRange(0.0, 359.9);
        dirSpins[row]->setDecimals(0);
        dirSpins[row]->setProperty("defaultValue", 0.0);
        dirSpins[row]->setValue(dirSpins[row]->property("defaultValue").toDouble());
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
        cloudSpins[row]->setProperty("defaultValue", 0.0);
        cloudSpins[row]->setValue(cloudSpins[row]->property("defaultValue").toDouble());
        table->setCellWidget(row, 4, cloudSpins[row]);

        airTempSpins[row] = new QDoubleSpinBox(table);
        airTempSpins[row]->setRange(-40.0, 200.0);
        airTempSpins[row]->setDecimals(0);
        airTempSpins[row]->setProperty("defaultValue", 72.0);
        airTempSpins[row]->setValue(airTempSpins[row]->property("defaultValue").toDouble());
        table->setCellWidget(row, 5, airTempSpins[row]);

        connect(speedSpins[row], &QDoubleSpinBox::valueChanged, this, &DomainAverageInput::domainAverageTableCheckRows);
        connect(dirSpins[row], &QDoubleSpinBox::valueChanged, this, &DomainAverageInput::domainAverageTableCheckRows);
        connect(timeEdits[row], &QTimeEdit::timeChanged, this, &DomainAverageInput::domainAverageTableCheckRows);
        connect(dateEdits[row], &QDateEdit::dateChanged, this, &DomainAverageInput::domainAverageTableCheckRows);
        connect(cloudSpins[row], &QDoubleSpinBox::valueChanged, this, &DomainAverageInput::domainAverageTableCheckRows);
        connect(airTempSpins[row], &QDoubleSpinBox::valueChanged, this, &DomainAverageInput::domainAverageTableCheckRows);

        QLineEdit* speedSpinLineEdit = speedSpins[row]->findChild<QLineEdit*>();
        QLineEdit* dirSpinLineEdit = dirSpins[row]->findChild<QLineEdit*>();
        QLineEdit* cloudSpinLineEdit = cloudSpins[row]->findChild<QLineEdit*>();
        QLineEdit* airTempSpinLineEdit = airTempSpins[row]->findChild<QLineEdit*>();
        connect(speedSpinLineEdit, &QLineEdit::textEdited, this, &DomainAverageInput::domainAverageTableOnSpinBoxLineEditTextEdited);
        connect(dirSpinLineEdit, &QLineEdit::textEdited, this, &DomainAverageInput::domainAverageTableOnSpinBoxLineEditTextEdited);
        connect(cloudSpinLineEdit, &QLineEdit::textEdited, this, &DomainAverageInput::domainAverageTableOnSpinBoxLineEditTextEdited);
        connect(airTempSpinLineEdit, &QLineEdit::textEdited, this, &DomainAverageInput::domainAverageTableOnSpinBoxLineEditTextEdited);

        connect(speedSpins[row], &QDoubleSpinBox::editingFinished, this, &DomainAverageInput::domainAverageTableOnSpinBoxLineEditEditingFinished);
        connect(dirSpins[row], &QDoubleSpinBox::editingFinished, this, &DomainAverageInput::domainAverageTableOnSpinBoxLineEditEditingFinished);
        connect(cloudSpins[row], &QDoubleSpinBox::editingFinished, this, &DomainAverageInput::domainAverageTableOnSpinBoxLineEditEditingFinished);
        connect(airTempSpins[row], &QDoubleSpinBox::editingFinished, this, &DomainAverageInput::domainAverageTableOnSpinBoxLineEditEditingFinished);
    }

    bool enabled = ui->diurnalCheckBox->isChecked() || ui->stabilityCheckBox->isChecked();
    for(int row = 0; row < rows; row++)
    {
        timeEdits[row]->setEnabled(enabled);
        dateEdits[row]->setEnabled(enabled);
        cloudSpins[row]->setEnabled(enabled);
        airTempSpins[row]->setEnabled(enabled);
    }
}

