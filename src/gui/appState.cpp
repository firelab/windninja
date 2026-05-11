 /******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja Qt GUI
 * Purpose:  Stores the states for inputs in the GUI
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

#include "appState.h"

AppState& AppState::instance()
{
    static AppState s;
    return s;
}

AppState::AppState()
    : tickIcon(":/tick.png"),
    warnIcon(":/jason_caution.png"),
    crossIcon(":/cross.png"),
    bulletIcon(":/bullet_blue.png")
{}

void AppState::setUi(Ui::MainWindow* mainWindowUi)
{
    ui = mainWindowUi;
}

void AppState::setState()
{
    updateSolverMethodologyState();
    updateSurfaceInputState();
    updateDiurnalInputState();
    updateStabilityInputState();
    updateDomainAverageInputState();
    updatePointInitializationInputState();
    updateWeatherModelInputState();
    updateGoogleEarthOutputState();
    updateFireBehaviorOutputState();
    updateShapeFilesOutputState();
    updateGeoSpatialPDFFilesOutputState();
    updateVTKFilesOutputState();
}

void AppState::updateSolverMethodologyState()
{
    if(ui->massSolverCheckBox->isChecked())
    {
        isSolverMethodologyValid = true;
        ui->treeWidget->topLevelItem(0)->setIcon(0, tickIcon);
        ui->treeWidget->topLevelItem(0)->setToolTip(0, "Using Conservation of Mass solver");
        ui->treeWidget->topLevelItem(0)->child(0)->setIcon(0, tickIcon);
        ui->treeWidget->topLevelItem(0)->child(0)->setToolTip(0, "Conservation of Mass selected");
        ui->treeWidget->topLevelItem(0)->child(1)->setIcon(0, bulletIcon);
        ui->treeWidget->topLevelItem(0)->child(1)->setToolTip(0, "Conservation of Mass and Momentum not selected");
    }
    else if(ui->momentumSolverCheckBox->isChecked())
    {
        isSolverMethodologyValid = true;
        ui->treeWidget->topLevelItem(0)->setToolTip(1, "Using Conservation of Mass and Momentum solver");
        ui->treeWidget->topLevelItem(0)->setIcon(0, tickIcon);
        ui->treeWidget->topLevelItem(0)->child(1)->setIcon(0, tickIcon);
        ui->treeWidget->topLevelItem(0)->child(1)->setToolTip(0, "Conservation of Mass and Momentum selected");
        ui->treeWidget->topLevelItem(0)->child(0)->setIcon(0, bulletIcon);
        ui->treeWidget->topLevelItem(0)->child(0)->setToolTip(0, "Conservation of Mass not selected");
    }
    else
    {
        isSolverMethodologyValid = false;
        ui->treeWidget->topLevelItem(0)->setIcon(0, crossIcon);
        ui->treeWidget->topLevelItem(0)->setToolTip(0,"Select a solver");
        ui->treeWidget->topLevelItem(0)->child(0)->setIcon(0, bulletIcon);
        ui->treeWidget->topLevelItem(0)->child(0)->setToolTip(0, "Conservation of Mass not selected");
        ui->treeWidget->topLevelItem(0)->child(1)->setIcon(0, bulletIcon);
        ui->treeWidget->topLevelItem(0)->child(1)->setToolTip(0, "Conservation of Mass and Momentum not selected");
    }

    updateOverallState();
}

void AppState::updateSurfaceInputState()
{
    if(ui->elevationInputFileLineEdit->text() == "")
    {
        isSurfaceInputValid = false;
        ui->treeWidget->topLevelItem(1)->child(0)->setIcon(0, crossIcon);
        ui->treeWidget->topLevelItem(1)->child(0)->setToolTip(0, "Surface Input File not selected");
    }
    else if(!QFile::exists(ui->elevationInputFileLineEdit->property("fullpath").toString()))
    {
        isSurfaceInputValid = false;
        ui->treeWidget->topLevelItem(1)->child(0)->setIcon(0, crossIcon);
        ui->treeWidget->topLevelItem(1)->child(0)->setToolTip(0, "Surface Input File does not exist on disk");
    }
    else if(surfaceInputFileLoadSuccess == false)
    {
        // old code did this check, doing hasPrj, but the new code has many stages during loadDemMetadata() where a file could potentially fail to load
        // variables aren't replaced anyways until a fully successful load, so just do a single check at the end of the process.
        isSurfaceInputValid = false;
        ui->treeWidget->topLevelItem(1)->child(0)->setIcon(0, warnIcon);
        ui->treeWidget->topLevelItem(1)->child(0)->setToolTip(0, "Surface Input File failed to load");
    }
    else
    {
        isSurfaceInputValid = true;
        ui->treeWidget->topLevelItem(1)->child(0)->setIcon(0, tickIcon);
        ui->treeWidget->topLevelItem(1)->child(0)->setToolTip(0, "Valid");
    }

    updateInputState();
    updateGoogleEarthOutputState();
    updateFireBehaviorOutputState();
    updateShapeFilesOutputState();
    updateGeoSpatialPDFFilesOutputState();
    updateVTKFilesOutputState();
}

void AppState::updateDiurnalInputState()
{
    if(ui->diurnalCheckBox->isChecked())
    {
        isDiurnalValid = true;
        ui->treeWidget->topLevelItem(1)->child(1)->setIcon(0, tickIcon);
        ui->treeWidget->topLevelItem(1)->child(1)->setToolTip(0, "Valid");
    }
    else
    {
        isDiurnalValid = false;
        ui->treeWidget->topLevelItem(1)->child(1)->setIcon(0, bulletIcon);
        ui->treeWidget->topLevelItem(1)->child(1)->setToolTip(0, "No Diurnal Input");
    }

    //updateInputState();  // called by updateDomainAverageInputState()
    updateDomainAverageInputState();
}

void AppState::updateStabilityInputState()
{
    if(ui->stabilityCheckBox->isChecked())
    {
        isStabilityValid = true;
        ui->treeWidget->topLevelItem(1)->child(2)->setIcon(0, tickIcon);
        ui->treeWidget->topLevelItem(1)->child(2)->setToolTip(0, "Valid");
    }
    else
    {
        isStabilityValid = false;
        ui->treeWidget->topLevelItem(1)->child(2)->setIcon(0, bulletIcon);
        ui->treeWidget->topLevelItem(1)->child(2)->setToolTip(0, "No Stability Input");
    }

    updateInputState();
    //updateDomainAverageInputState();
}

void AppState::updateDomainAverageInputState()
{
    if(ui->domainAverageGroupBox->isChecked())
    {
        if(!isSurfaceInputValid)
        {
            ui->treeWidget->topLevelItem(1)->child(3)->child(0)->setIcon(0, crossIcon);
            ui->treeWidget->topLevelItem(1)->child(3)->child(0)->setToolTip(0, "Check Surface Input");
            isDomainAverageInitializationValid = false;
        }
        else if(DomainAvgTableNumRuns == 0)
        {
            if(ui->diurnalCheckBox->isChecked() == false)
            {
                ui->treeWidget->topLevelItem(1)->child(3)->child(0)->setIcon(0, crossIcon);
                ui->treeWidget->topLevelItem(1)->child(3)->child(0)->setToolTip(0, "No runs have been added, diurnal is not active");
                isDomainAverageInitializationValid = false;
            }
            else // if(ui->diurnalCheckBox->isChecked() == true)
            {
                ui->treeWidget->topLevelItem(1)->child(3)->child(0)->setIcon(0, warnIcon);
                ui->treeWidget->topLevelItem(1)->child(3)->child(0)->setToolTip(0, "No runs have been added, one run will be done at speed = 0, dir = 0 while using diurnal");
                isDomainAverageInitializationValid = true;
            }
        }
        else // if(DomainAvgTableNumRuns != 0)
        {
            if(DomainAvgTableNumZeroRuns == 0)
            {
                ui->treeWidget->topLevelItem(1)->child(3)->child(0)->setIcon(0, tickIcon);
                ui->treeWidget->topLevelItem(1)->child(3)->child(0)->setToolTip(0, QString::number(DomainAvgTableNumRuns)+" runs");
                isDomainAverageInitializationValid = true;
            }
            else // if(DomainAvgTableNumZeroRuns != 0)
            {
                if(ui->diurnalCheckBox->isChecked() == true)
                {
                    ui->treeWidget->topLevelItem(1)->child(3)->child(0)->setIcon(0, warnIcon);
                    ui->treeWidget->topLevelItem(1)->child(3)->child(0)->setToolTip(0, QString::number(DomainAvgTableNumRuns)+" runs have been added, detecting "+QString::number(DomainAvgTableNumZeroRuns)+" zero wind speed runs, diurnal is active so will continue the runs");
                    isDomainAverageInitializationValid = true;
                }
                else // if(ui->diurnalCheckBox->isChecked() == false)
                {
                    ui->treeWidget->topLevelItem(1)->child(3)->child(0)->setIcon(0, crossIcon);
                    ui->treeWidget->topLevelItem(1)->child(3)->child(0)->setToolTip(0, QString::number(DomainAvgTableNumRuns)+" runs have been added, but detecting "+QString::number(DomainAvgTableNumZeroRuns)+" zero wind speed runs without diurnal being active");
                    isDomainAverageInitializationValid = false;
                }
            }
        }
    }
    else
    {
        ui->treeWidget->topLevelItem(1)->child(3)->child(0)->setIcon(0, bulletIcon);
        ui->treeWidget->topLevelItem(1)->child(3)->child(0)->setToolTip(0, "Not selected");
        isDomainAverageInitializationValid = false;
    }

    updateInputState();
}

void AppState::updatePointInitializationInputState()
{
    if(ui->pointInitializationGroupBox->isChecked())
    {
        if(!isSurfaceInputValid)
        {
            isGoogleEarthValid = false;
            ui->treeWidget->topLevelItem(2)->child(0)->setIcon(0, crossIcon);
            ui->treeWidget->topLevelItem(2)->child(0)->setToolTip(0, "Check Surface Input");
        }
        else
        {
            if(!isStationFileSelected)
            {
                ui->treeWidget->topLevelItem(1)->child(3)->child(1)->setIcon(0, crossIcon);
                ui->treeWidget->topLevelItem(1)->child(3)->child(1)->setToolTip(0, "No Station files selected");
                isPointInitializationValid = false;
            }
            else if(!isStationDataValid)
            {
                ui->treeWidget->topLevelItem(1)->child(3)->child(1)->setIcon(0, crossIcon);
                ui->treeWidget->topLevelItem(1)->child(3)->child(1)->setToolTip(0, "Invalid Station data detected");
                isPointInitializationValid = false;
            }
            else if(!isStationFileSelectionValid)
            {
                ui->treeWidget->topLevelItem(1)->child(3)->child(1)->setIcon(0, crossIcon);
                ui->treeWidget->topLevelItem(1)->child(3)->child(1)->setToolTip(0, "Mismatched file types selected, cannot proceed");
                isPointInitializationValid = false;
            }
            else
            {
                ui->treeWidget->topLevelItem(1)->child(3)->child(1)->setIcon(0, tickIcon);
                ui->treeWidget->topLevelItem(1)->child(3)->child(1)->setToolTip(0, "Valid");
                isPointInitializationValid = true;
            }
        }
    }
    else
    {
        ui->treeWidget->topLevelItem(1)->child(3)->child(1)->setIcon(0, bulletIcon);
        ui->treeWidget->topLevelItem(1)->child(3)->child(1)->setToolTip(0, "Not selected");
        isPointInitializationValid = false;
    }

    updateInputState();
}

void AppState::updateWeatherModelInputState()
{
    if(ui->weatherModelGroupBox->isChecked())
    {
        if(!isSurfaceInputValid)
        {
            isGoogleEarthValid = false;
            ui->treeWidget->topLevelItem(2)->child(0)->setIcon(0, crossIcon);
            ui->treeWidget->topLevelItem(2)->child(0)->setToolTip(0, "Check Surface Input");
        }
        else
        {
            if(!isWeatherModelFileSelected)
            {
                isWeatherModelInitializationValid = false;
                ui->treeWidget->topLevelItem(1)->child(3)->child(2)->setIcon(0, crossIcon);
                ui->treeWidget->topLevelItem(1)->child(3)->child(2)->setToolTip(0, "No Forecast file selected");
            }
            else if(!isWeatherModelForecastValid)
            {
                isWeatherModelInitializationValid = false;
                ui->treeWidget->topLevelItem(1)->child(3)->child(2)->setIcon(0, crossIcon);
                ui->treeWidget->topLevelItem(1)->child(3)->child(2)->setToolTip(0, "Invalid Forecast data detected");
            }
            else if(!isWeatherModelTimeSelected)
            {
                isWeatherModelInitializationValid = false;
                ui->treeWidget->topLevelItem(1)->child(3)->child(2)->setIcon(0, crossIcon);
                ui->treeWidget->topLevelItem(1)->child(3)->child(2)->setToolTip(0, "Forecast time not selected");
            }
            else
            {
                isWeatherModelInitializationValid = true;
                ui->treeWidget->topLevelItem(1)->child(3)->child(2)->setIcon(0, tickIcon);
                ui->treeWidget->topLevelItem(1)->child(3)->child(2)->setToolTip(0, "Valid");
            }
        }
    }
    else
    {
        isWeatherModelInitializationValid = false;
        ui->treeWidget->topLevelItem(1)->child(3)->child(2)->setIcon(0, bulletIcon);
        ui->treeWidget->topLevelItem(1)->child(3)->child(2)->setToolTip(0, "Not selected");
    }

    updateInputState();
}

void AppState::updateGoogleEarthOutputState()
{
    if(ui->googleEarthCheckBox->isChecked())
    {
        if(!isSurfaceInputValid)
        {
            isGoogleEarthValid = false;
            ui->treeWidget->topLevelItem(2)->child(0)->setIcon(0, crossIcon);
            ui->treeWidget->topLevelItem(2)->child(0)->setToolTip(0, "Check Surface Input");
        }
        else if(!isInputValid)
        {
            isGoogleEarthValid = false;
            ui->treeWidget->topLevelItem(2)->child(0)->setIcon(0, crossIcon);
            ui->treeWidget->topLevelItem(2)->child(0)->setToolTip(0, "Check Inputs");
        }
        else
        {
            double XLength = GDALXSize * GDALCellSize;
            double YLength = GDALYSize * GDALCellSize;
            double noGoogleCellSize = std::sqrt((XLength * YLength) / noGoogleNumCells);
            if(noGoogleCellSize > ui->googleEarthMeshResolutionSpinBox->value())
            {
                isGoogleEarthValid = true;
                ui->treeWidget->topLevelItem(2)->child(0)->setIcon(0, warnIcon);
                ui->treeWidget->topLevelItem(2)->child(0)->setToolTip(0, "The resolution of the google file may be too fine");
            }
            else if(GDALCellSize > ui->googleEarthMeshResolutionSpinBox->value())
            {
                isGoogleEarthValid = true;
                ui->treeWidget->topLevelItem(2)->child(0)->setIcon(0, warnIcon);
                ui->treeWidget->topLevelItem(2)->child(0)->setToolTip(0, "The output resolution is finer than the Surface Input file resolution");
            }
            else
            {
                isGoogleEarthValid = true;
                ui->treeWidget->topLevelItem(2)->child(0)->setIcon(0, tickIcon);
                ui->treeWidget->topLevelItem(2)->child(0)->setToolTip(0, "Valid");
            }
        }
    }
    else
    {
        isGoogleEarthValid = false;
        ui->treeWidget->topLevelItem(2)->child(0)->setIcon(0, bulletIcon);
        ui->treeWidget->topLevelItem(2)->child(0)->setToolTip(0, "Not selected");
    }

    updateOutputState();
}

void AppState::updateFireBehaviorOutputState()
{
    if(ui->fireBehaviorGroupBox->isChecked())
    {
        if(!isSurfaceInputValid)
        {
            isFireBehaviorValid = false;
            ui->treeWidget->topLevelItem(2)->child(1)->setIcon(0, crossIcon);
            ui->treeWidget->topLevelItem(2)->child(1)->setToolTip(0, "Check Surface Input");
        }
        else if(!isInputValid)
        {
            isFireBehaviorValid = false;
            ui->treeWidget->topLevelItem(2)->child(1)->setIcon(0, crossIcon);
            ui->treeWidget->topLevelItem(2)->child(1)->setToolTip(0, "Check Inputs");
        }
        else if(GDALCellSize > ui->fireBehaviorMeshResolutionSpinBox->value())
        {
            isFireBehaviorValid = true;
            ui->treeWidget->topLevelItem(2)->child(1)->setIcon(0, warnIcon);
            ui->treeWidget->topLevelItem(2)->child(1)->setToolTip(0, "The output resolutions is finer than the Surface Input file resolution");
        }
        else
        {
            isFireBehaviorValid = true;
            ui->treeWidget->topLevelItem(2)->child(1)->setIcon(0, tickIcon);
            ui->treeWidget->topLevelItem(2)->child(1)->setToolTip(0, "Valid");
        }
    }
    else
    {
        isFireBehaviorValid = false;
        ui->treeWidget->topLevelItem(2)->child(1)->setIcon(0, bulletIcon);
        ui->treeWidget->topLevelItem(2)->child(1)->setToolTip(0, "Not selected");
    }

    updateOutputState();
}

void AppState::updateShapeFilesOutputState()
{
    if(ui->shapeFilesGroupBox->isChecked())
    {
        if(!isSurfaceInputValid)
        {
            isShapeFilesValid = false;
            ui->treeWidget->topLevelItem(2)->child(2)->setIcon(0, crossIcon);
            ui->treeWidget->topLevelItem(2)->child(2)->setToolTip(0, "Check Surface Input");
        }
        else if(!isInputValid)
        {
            isShapeFilesValid = false;
            ui->treeWidget->topLevelItem(2)->child(2)->setIcon(0, crossIcon);
            ui->treeWidget->topLevelItem(2)->child(2)->setToolTip(0, "Check Inputs");
        }
        else if(GDALCellSize > ui->shapeFilesMeshResolutionSpinBox->value())
        {
            isShapeFilesValid = true;
            ui->treeWidget->topLevelItem(2)->child(2)->setIcon(0, warnIcon);
            ui->treeWidget->topLevelItem(2)->child(2)->setToolTip(0, "The output resolutions is finer than the Surface Input file resolution");
        }
        else
        {
            isShapeFilesValid = true;
            ui->treeWidget->topLevelItem(2)->child(2)->setIcon(0, tickIcon);
            ui->treeWidget->topLevelItem(2)->child(2)->setToolTip(0, "Valid");
        }
    }
    else
    {
        isShapeFilesValid = false;
        ui->treeWidget->topLevelItem(2)->child(2)->setIcon(0, bulletIcon);
        ui->treeWidget->topLevelItem(2)->child(2)->setToolTip(0, "Not selected");
    }

    updateOutputState();
}

void AppState::updateGeoSpatialPDFFilesOutputState()
{
    if(ui->geospatialPDFFilesGroupBox->isChecked())
    {
        if(!isSurfaceInputValid)
        {
            isGeoSpatialPDFFilesValid = false;
            ui->treeWidget->topLevelItem(2)->child(3)->setIcon(0, crossIcon);
            ui->treeWidget->topLevelItem(2)->child(3)->setToolTip(0, "Check Surface Input");
        }
        else if(!isInputValid)
        {
            isGeoSpatialPDFFilesValid = false;
            ui->treeWidget->topLevelItem(2)->child(3)->setIcon(0, crossIcon);
            ui->treeWidget->topLevelItem(2)->child(3)->setToolTip(0, "Check Inputs");
        }
        else if(GDALCellSize > ui->geospatialPDFFilesMeshResolutionSpinBox->value())
        {
            isGeoSpatialPDFFilesValid = true;
            ui->treeWidget->topLevelItem(2)->child(3)->setIcon(0, warnIcon);
            ui->treeWidget->topLevelItem(2)->child(3)->setToolTip(0, "The output resolutions is finer than the Surface Input file resolution");
        }
        else
        {
            isGeoSpatialPDFFilesValid = true;
            ui->treeWidget->topLevelItem(2)->child(3)->setIcon(0, tickIcon);
            ui->treeWidget->topLevelItem(2)->child(3)->setToolTip(0, "Valid");
        }
    }
    else
    {
        isGeoSpatialPDFFilesValid = false;
        ui->treeWidget->topLevelItem(2)->child(3)->setIcon(0, bulletIcon);
        ui->treeWidget->topLevelItem(2)->child(3)->setToolTip(0, "Not selected");
    }

    updateOutputState();
}

void AppState::updateVTKFilesOutputState()
{
    if(ui->VTKFilesCheckBox->isChecked())
    {
        if(!isSurfaceInputValid)
        {
            isVTKFilesValid = false;
            ui->treeWidget->topLevelItem(2)->child(4)->setIcon(0, crossIcon);
            ui->treeWidget->topLevelItem(2)->child(4)->setToolTip(0, "Check Surface Input");
        }
        else if(!isInputValid)
        {
            isVTKFilesValid = false;
            ui->treeWidget->topLevelItem(2)->child(4)->setIcon(0, crossIcon);
            ui->treeWidget->topLevelItem(2)->child(4)->setToolTip(0, "Check Inputs");
        }
        else
        {
            isVTKFilesValid = true;
            ui->treeWidget->topLevelItem(2)->child(4)->setIcon(0, tickIcon);
            ui->treeWidget->topLevelItem(2)->child(4)->setToolTip(0, "Valid");
        }
    }
    else
    {
        isVTKFilesValid = false;
        ui->treeWidget->topLevelItem(2)->child(4)->setIcon(0, bulletIcon);
        ui->treeWidget->topLevelItem(2)->child(4)->setToolTip(0, "Not selected");
    }

    updateOutputState();
}

void AppState::updateInputState()
{
    if(!isSurfaceInputValid)
    {
        isWindInputValid = false;
        ui->treeWidget->topLevelItem(1)->child(3)->setIcon(0, crossIcon);
        ui->treeWidget->topLevelItem(1)->child(3)->setToolTip(0, "Check Surface Input");
    }
    else if(!ui->domainAverageGroupBox->isChecked() && !ui->pointInitializationGroupBox->isChecked() && !ui->weatherModelGroupBox->isChecked())
    {
        isWindInputValid = false;
        ui->treeWidget->topLevelItem(1)->child(3)->setIcon(0, crossIcon);
        ui->treeWidget->topLevelItem(1)->child(3)->setToolTip(0, "No Initialization selected");
    }
    else if(ui->domainAverageGroupBox->isChecked() && !isDomainAverageInitializationValid)
    {
        isWindInputValid = false;
        ui->treeWidget->topLevelItem(1)->child(3)->setIcon(0, crossIcon);
        ui->treeWidget->topLevelItem(1)->child(3)->setToolTip(0, "Check Domain Average Wind inputs");
    }
    else if(ui->pointInitializationGroupBox->isChecked() && !isPointInitializationValid)
    {
        isWindInputValid = false;
        ui->treeWidget->topLevelItem(1)->child(3)->setIcon(0, crossIcon);
        ui->treeWidget->topLevelItem(1)->child(3)->setToolTip(0, "Check Point Initialization inputs");
    }
    else if(ui->weatherModelGroupBox->isChecked() && !isWeatherModelInitializationValid)
    {
        isWindInputValid = false;
        ui->treeWidget->topLevelItem(1)->child(3)->setIcon(0, crossIcon);
        ui->treeWidget->topLevelItem(1)->child(3)->setToolTip(0, "Check Weather Model inputs");
    }
    else
    {
        isWindInputValid = true;
        ui->treeWidget->topLevelItem(1)->child(3)->setIcon(0, tickIcon);
        ui->treeWidget->topLevelItem(1)->child(3)->setToolTip(0, "Valid");
    }

    QVector<QString> invalidCases;
    if(ui->diurnalCheckBox->isChecked() && !isDiurnalValid)
    {
        invalidCases.append(QString("Diurnal Input"));
    }
    if(ui->stabilityCheckBox->isChecked() && !isStabilityValid)
    {
        invalidCases.append(QString("Stability Input"));
    }
    if(!isWindInputValid)
    {
        invalidCases.append(QString("Wind Input"));
    }

    if(!isSurfaceInputValid)
    {
        isInputValid = false;
        ui->treeWidget->topLevelItem(1)->setIcon(0, crossIcon);
        ui->treeWidget->topLevelItem(1)->setToolTip(0, "Check Surface Input");
    }
    else if(invalidCases.size() == 0)
    {
        isInputValid = true;
        ui->treeWidget->topLevelItem(1)->setIcon(0, tickIcon);
        ui->treeWidget->topLevelItem(1)->setToolTip(0, "Valid");
    } else
    {
        QString checkString;
        if(invalidCases.size() == 1)
        {
            checkString = QString("Check " + invalidCases[0]);
        }
        else if(invalidCases.size() == 2)
        {
            checkString = QString("Check " + invalidCases[0] + " and " + invalidCases[1]);
        }
        else
        {
            checkString = QString("Check " + invalidCases[0]);
            for(qsizetype checkIdx = 1; checkIdx < invalidCases.size() - 1; checkIdx++)
            {
                checkString = QString(checkString + ", " + invalidCases[checkIdx]);
            }
            checkString = QString(checkString + " and " + invalidCases[invalidCases.size()-1]);
        }

        isInputValid = false;
        ui->treeWidget->topLevelItem(1)->setIcon(0, crossIcon);
        ui->treeWidget->topLevelItem(1)->setToolTip(0, checkString);
    }

    updateGoogleEarthOutputState();
    updateFireBehaviorOutputState();
    updateShapeFilesOutputState();
    updateGeoSpatialPDFFilesOutputState();
    updateVTKFilesOutputState();
    updateOutputState();
    //updateOverallState();
}

void AppState::updateOutputState()
{
    if(!isSurfaceInputValid)
    {
        isOutputValid = false;
        ui->treeWidget->topLevelItem(2)->setIcon(0, crossIcon);
        ui->treeWidget->topLevelItem(2)->setToolTip(0, "Check Surface Input");
    }
    else if(!isInputValid)
    {
        isOutputValid = false;
        ui->treeWidget->topLevelItem(2)->setIcon(0, crossIcon);
        ui->treeWidget->topLevelItem(2)->setToolTip(0, "Check Inputs");
    }
    else if(!ui->googleEarthCheckBox->isChecked() && !ui->fireBehaviorGroupBox->isChecked() && !ui->shapeFilesGroupBox->isChecked() && !ui->geospatialPDFFilesGroupBox->isChecked() && !ui->VTKFilesCheckBox->isChecked())
    {
        isOutputValid = false;
        ui->treeWidget->topLevelItem(2)->setIcon(0, crossIcon);
        ui->treeWidget->topLevelItem(2)->setToolTip(0, "No outputs selected");
    }
    else
    {
        QVector<QString> invalidCases;
        if(ui->googleEarthCheckBox->isChecked() && !isGoogleEarthValid)
        {
            invalidCases.append(QString("Google Earth output"));
        }
        if(ui->fireBehaviorGroupBox->isChecked() && !isFireBehaviorValid)
        {
            invalidCases.append(QString("Fire Behavior output"));
        }
        if(ui->shapeFilesGroupBox->isChecked() && !isShapeFilesValid)
        {
            invalidCases.append(QString("Shape Files output"));
        }
        if(ui->geospatialPDFFilesGroupBox->isChecked() && !isGeoSpatialPDFFilesValid)
        {
            invalidCases.append(QString("Geospatial PDF Files output"));
        }
        if(ui->VTKFilesCheckBox->isChecked() && !isVTKFilesValid)
        {
            invalidCases.append(QString("VTK Files output"));
        }

        if(invalidCases.size() == 0)
        {
            isOutputValid = true;
            ui->treeWidget->topLevelItem(2)->setIcon(0, tickIcon);
            ui->treeWidget->topLevelItem(2)->setToolTip(0, "Valid");
        } else
        {
            QString checkString;
            if(invalidCases.size() == 1)
            {
                checkString = QString("Check " + invalidCases[0]);
            }
            else if(invalidCases.size() == 2)
            {
                checkString = QString("Check " + invalidCases[0] + " and " + invalidCases[1]);
            }
            else
            {
                checkString = QString("Check " + invalidCases[0]);
                for(qsizetype checkIdx = 1; checkIdx < invalidCases.size() - 1; checkIdx++)
                {
                    checkString = QString(checkString + ", " + invalidCases[checkIdx]);
                }
                checkString = QString(checkString + " and " + invalidCases[invalidCases.size()-1]);
            }

            isOutputValid = false;
            ui->treeWidget->topLevelItem(2)->setIcon(0, crossIcon);
            ui->treeWidget->topLevelItem(2)->setToolTip(0, checkString);
        }
    }

    updateOverallState();
}

void AppState::updateOverallState()
{
    if(isSolverMethodologyValid && isInputValid && isOutputValid)
    {
        ui->numberOfProcessorsSolveButton->setEnabled(true);
        ui->numberOfProcessorsSolveButton->setToolTip("You may start the solver");
        ui->treeWidget->topLevelItem(3)->setIcon(0, tickIcon);
        ui->treeWidget->topLevelItem(3)->setToolTip(0, "You may start the solver");
    }
    else if(!isSurfaceInputValid)
    {
        ui->numberOfProcessorsSolveButton->setEnabled(false);
        ui->numberOfProcessorsSolveButton->setToolTip("Check Surface Input");
        ui->treeWidget->topLevelItem(3)->setIcon(0, crossIcon);
        ui->treeWidget->topLevelItem(3)->setToolTip(0, "Check Surface Input");
    }
    else if(!ui->domainAverageGroupBox->isChecked() && !ui->pointInitializationGroupBox->isChecked() && !ui->weatherModelGroupBox->isChecked())
    {
        ui->numberOfProcessorsSolveButton->setEnabled(false);
        ui->numberOfProcessorsSolveButton->setToolTip("You must select an initialization method");
        ui->treeWidget->topLevelItem(3)->setIcon(0, crossIcon);
        ui->treeWidget->topLevelItem(3)->setToolTip(0, "You must select an initialization method");
    }
    else
    {
        ui->numberOfProcessorsSolveButton->setEnabled(false);
        ui->numberOfProcessorsSolveButton->setToolTip("There are errors in the inputs or outputs");
        ui->treeWidget->topLevelItem(3)->setIcon(0, crossIcon);
        ui->treeWidget->topLevelItem(3)->setToolTip(0, "There are errors in the inputs or outputs");
    }
}
