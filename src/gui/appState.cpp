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
    if (isMassSolverToggled)
    {
        isSolverMethodologyValid = true;
        ui->treeWidget->topLevelItem(0)->setIcon(0, tickIcon);
        ui->treeWidget->topLevelItem(0)->setToolTip(0, "Using Conservation of Mass Selected");
        ui->treeWidget->topLevelItem(0)->child(0)->setIcon(0, tickIcon);
        ui->treeWidget->topLevelItem(0)->child(0)->setToolTip(0, "Conservation of Mass Selected");
        ui->treeWidget->topLevelItem(0)->child(1)->setIcon(0, bulletIcon);
        ui->treeWidget->topLevelItem(0)->child(1)->setToolTip(0, "Conservation of Mass and Momentum Not Selected");
    }
    else if (isMomentumSolverToggled)
    {
        isSolverMethodologyValid = true;
        ui->treeWidget->topLevelItem(0)->setToolTip(1, "Conservation of Mass and Momentum Selected");
        ui->treeWidget->topLevelItem(0)->setIcon(0, tickIcon);
        ui->treeWidget->topLevelItem(0)->child(1)->setIcon(0, tickIcon);
        ui->treeWidget->topLevelItem(0)->child(1)->setToolTip(0, "Conservation of Mass and Momentum Selected");
        ui->treeWidget->topLevelItem(0)->child(0)->setIcon(0, bulletIcon);
        ui->treeWidget->topLevelItem(0)->child(0)->setToolTip(0, "Conservation of Mass Not Selected");
    }
    else
    {
        isSolverMethodologyValid = false;
        ui->treeWidget->topLevelItem(0)->setIcon(0, crossIcon);
        ui->treeWidget->topLevelItem(0)->setToolTip(0,"Select a Solver");
        ui->treeWidget->topLevelItem(0)->child(0)->setIcon(0, bulletIcon);
        ui->treeWidget->topLevelItem(0)->child(0)->setToolTip(0, "Conservation of Mass Not Selected");
        ui->treeWidget->topLevelItem(0)->child(1)->setIcon(0, bulletIcon);
        ui->treeWidget->topLevelItem(0)->child(1)->setToolTip(0, "Conservation of Mass and Momentum Not Selected");
    }

    updateOverallState();
}

void AppState::updateSurfaceInputState()
{
    if (ui->elevationInputFileLineEdit->text() != "")
    {
        isSurfaceInputValid = true;
        ui->treeWidget->topLevelItem(1)->child(0)->setIcon(0, tickIcon);
        ui->treeWidget->topLevelItem(1)->child(0)->setToolTip(0, "Valid");
    }
    else
    {
        isSurfaceInputValid = false;
        ui->treeWidget->topLevelItem(1)->child(0)->setIcon(0, crossIcon);
        ui->treeWidget->topLevelItem(1)->child(0)->setToolTip(0, "Input File Cannot be Detected");
    }
    updateInputState();
    updateGoogleEarthOutputState();
}

void AppState::updateDiurnalInputState()
{
    if (isDiurnalInputToggled)
    {
        ui->treeWidget->topLevelItem(1)->child(1)->setIcon(0, tickIcon);
        ui->treeWidget->topLevelItem(1)->child(1)->setToolTip(0, "Valid");
    }
    else
    {
        ui->treeWidget->topLevelItem(1)->child(1)->setIcon(0, bulletIcon);
        ui->treeWidget->topLevelItem(1)->child(1)->setToolTip(0, "No Diurnal Input");
    }
}

void AppState::updateStabilityInputState()
{
    if (isStabilityInputToggled)
    {
        ui->treeWidget->topLevelItem(1)->child(2)->setIcon(0, tickIcon);
        ui->treeWidget->topLevelItem(1)->child(2)->setToolTip(0, "Valid");
    }
    else
    {
        ui->treeWidget->topLevelItem(1)->child(2)->setIcon(0, bulletIcon);
        ui->treeWidget->topLevelItem(1)->child(2)->setToolTip(0, "No Stability Input");
    }
}

void AppState::updateDomainAverageInputState()
{
    if (isDomainAverageInitializationToggled && isDomainAverageWindInputTableValid)
    {
        ui->treeWidget->topLevelItem(1)->child(3)->child(0)->setIcon(0, tickIcon);
        ui->treeWidget->topLevelItem(1)->child(3)->child(0)->setToolTip(0, "Valid");
        isDomainAverageInitializationValid = true;
    }
    else if (isDomainAverageInitializationToggled && !isDomainAverageWindInputTableValid)
    {
        ui->treeWidget->topLevelItem(1)->child(3)->child(0)->setIcon(0, crossIcon);
        ui->treeWidget->topLevelItem(1)->child(3)->child(0)->setToolTip(0, "Bad wind inputs");
        isDomainAverageInitializationValid = false;
    }
    else
    {
        ui->treeWidget->topLevelItem(1)->child(3)->child(0)->setIcon(0, bulletIcon);
        ui->treeWidget->topLevelItem(1)->child(3)->child(0)->setToolTip(0, "Not Selected");
        isDomainAverageInitializationValid = false;
    }

    updateInputState();
}

void AppState::updatePointInitializationInputState()
{
    if (isPointInitializationToggled && isStationFileSelectionValid && isStationFileSelected)
    {
        ui->treeWidget->topLevelItem(1)->child(3)->child(1)->setIcon(0, tickIcon);
        ui->treeWidget->topLevelItem(1)->child(3)->child(1)->setToolTip(0, "Valid");
        isPointInitializationValid = true;
    }
    else if(isPointInitializationToggled && !isStationFileSelected)
    {
        ui->treeWidget->topLevelItem(1)->child(3)->child(1)->setIcon(0, crossIcon);
        ui->treeWidget->topLevelItem(1)->child(3)->child(1)->setToolTip(0, "No Station File Selected");
        isPointInitializationValid = false;
    }
    else if(isPointInitializationToggled && !isStationFileSelectionValid)
    {
        ui->treeWidget->topLevelItem(1)->child(3)->child(1)->setIcon(0, crossIcon);
        ui->treeWidget->topLevelItem(1)->child(3)->child(1)->setToolTip(0, "Conflicting Files Selected");
        isPointInitializationValid = false;
    }
    else
    {
        ui->treeWidget->topLevelItem(1)->child(3)->child(1)->setIcon(0, bulletIcon);
        ui->treeWidget->topLevelItem(1)->child(3)->child(1)->setToolTip(0, "Not Selected");
        isPointInitializationValid = false;
    }

    updateInputState();
}

void AppState::updateWeatherModelInputState()
{
    if (isWeatherModelInitializationToggled && isWeatherModelForecastValid)
    {
        isWeatherModelInitializationValid = true;
        ui->treeWidget->topLevelItem(1)->child(3)->child(2)->setIcon(0, tickIcon);
        ui->treeWidget->topLevelItem(1)->child(3)->child(2)->setToolTip(0, "Valid");
    }
    else if (isWeatherModelInitializationToggled && !isWeatherModelForecastValid)
    {
        isWeatherModelInitializationValid = false;
        ui->treeWidget->topLevelItem(1)->child(3)->child(2)->setIcon(0, crossIcon);
        ui->treeWidget->topLevelItem(1)->child(3)->child(2)->setToolTip(0, "Forecast is Invalid");
    }
    else
    {
        isWeatherModelInitializationValid = false;
        ui->treeWidget->topLevelItem(1)->child(3)->child(2)->setIcon(0, bulletIcon);
        ui->treeWidget->topLevelItem(1)->child(3)->child(2)->setToolTip(0, "Not Selected");
    }

    updateInputState();
}

void AppState::updateGoogleEarthOutputState()
{
    if(ui->googleEarthCheckBox->isChecked())
    {
        if(isSurfaceInputValid)
        {
            isGoogleEarthValid = true;
            ui->treeWidget->topLevelItem(2)->child(0)->setIcon(0, tickIcon);
            ui->treeWidget->topLevelItem(2)->setToolTip(0, "");
        }
        else
        {
            isGoogleEarthValid = false;
            ui->treeWidget->topLevelItem(2)->setIcon(0, crossIcon);
            ui->treeWidget->topLevelItem(2)->setToolTip(0, "Cannot read DEM File");
            ui->treeWidget->topLevelItem(2)->child(0)->setIcon(0, crossIcon);
            ui->treeWidget->topLevelItem(2)->child(0)->setToolTip(0, "Cannot read DEM File");
        }
    }
    else
    {
        isGoogleEarthValid = false;
        ui->treeWidget->topLevelItem(2)->child(0)->setIcon(0, bulletIcon);
        ui->treeWidget->topLevelItem(2)->setToolTip(0, "Not Selected");
    }

    updateOutputState();
}

void AppState::updateFireBehaviorOutputState()
{
    if(isFireBehaviorToggled)
    {
        if(isSurfaceInputValid)
        {
            isFireBehaviorValid = true;
            ui->treeWidget->topLevelItem(2)->child(1)->setIcon(0, tickIcon);
            ui->treeWidget->topLevelItem(2)->setToolTip(0, "");
        }
        else
        {
            isFireBehaviorValid = false;
            ui->treeWidget->topLevelItem(2)->setIcon(0, crossIcon);
            ui->treeWidget->topLevelItem(2)->setToolTip(0, "Cannot read DEM File");
            ui->treeWidget->topLevelItem(2)->child(1)->setIcon(0, crossIcon);
            ui->treeWidget->topLevelItem(2)->child(1)->setToolTip(0, "Cannot read DEM File");
        }
    }
    else
    {
        isFireBehaviorValid = false;
        ui->treeWidget->topLevelItem(2)->child(1)->setIcon(0, bulletIcon);
        ui->treeWidget->topLevelItem(2)->child(1)->setToolTip(0, "Not Selected");
    }

    updateOutputState();
}

void AppState::updateShapeFilesOutputState()
{
    if(isShapeFilesToggled)
    {
        if(isSurfaceInputValid)
        {
            isShapeFilesValid = true;
            ui->treeWidget->topLevelItem(2)->child(2)->setIcon(0, tickIcon);
            ui->treeWidget->topLevelItem(2)->setToolTip(0, "");
        }
        else
        {
            isShapeFilesValid = false;
            ui->treeWidget->topLevelItem(2)->setIcon(0, crossIcon);
            ui->treeWidget->topLevelItem(2)->setToolTip(0, "Cannot read DEM File");
            ui->treeWidget->topLevelItem(2)->child(2)->setIcon(0, crossIcon);
            ui->treeWidget->topLevelItem(2)->child(2)->setToolTip(0, "Cannot read DEM File");
        }
    }
    else
    {
        isShapeFilesValid = false;
        ui->treeWidget->topLevelItem(2)->child(2)->setIcon(0, bulletIcon);
        ui->treeWidget->topLevelItem(2)->child(2)->setToolTip(0, "Not Selected");
    }

    updateOutputState();
}

void AppState::updateGeoSpatialPDFFilesOutputState()
{
    if(isGeoSpatialPDFFilesToggled)
    {
        if(isSurfaceInputValid)
        {
            isGeoSpatialPDFFilesValid = true;
            ui->treeWidget->topLevelItem(2)->child(3)->setIcon(0, tickIcon);
            ui->treeWidget->topLevelItem(2)->setToolTip(0, "");
        }
        else
        {
            isGeoSpatialPDFFilesValid = false;
            ui->treeWidget->topLevelItem(2)->setIcon(0, crossIcon);
            ui->treeWidget->topLevelItem(2)->setToolTip(0, "Cannot read DEM File");
            ui->treeWidget->topLevelItem(2)->child(3)->setIcon(0, crossIcon);
            ui->treeWidget->topLevelItem(2)->child(3)->setToolTip(0, "Cannot read DEM File");
        }
    }
    else
    {
        isGeoSpatialPDFFilesValid = false;
        ui->treeWidget->topLevelItem(2)->child(3)->setIcon(0, bulletIcon);
        ui->treeWidget->topLevelItem(2)->child(3)->setToolTip(0, "Not Selected");
    }

    updateOutputState();
}

void AppState::updateVTKFilesOutputState()
{
    if(isVTKFilesToggled)
    {
        if(isSurfaceInputValid)
        {
            isVTKFilesValid = true;
            ui->treeWidget->topLevelItem(2)->child(4)->setIcon(0, tickIcon);
            ui->treeWidget->topLevelItem(2)->setToolTip(0, "");
        }
        else
        {
            isVTKFilesValid = false;
            ui->treeWidget->topLevelItem(2)->setIcon(0, crossIcon);
            ui->treeWidget->topLevelItem(2)->setToolTip(0, "Cannot read DEM File");
            ui->treeWidget->topLevelItem(2)->child(4)->setIcon(0, crossIcon);
            ui->treeWidget->topLevelItem(2)->child(4)->setToolTip(0, "Cannot read DEM File");
        }
    }
    else
    {
        isVTKFilesValid = false;
        ui->treeWidget->topLevelItem(2)->child(4)->setIcon(0, bulletIcon);
        ui->treeWidget->topLevelItem(2)->child(4)->setToolTip(0, "Not Selected");
    }

    updateOutputState();
}

void AppState::updateInputState()
{
    if (isDomainAverageInitializationValid || isPointInitializationValid || isWeatherModelInitializationValid)
    {
        isWindInputValid = true;
        ui->treeWidget->topLevelItem(1)->child(3)->setIcon(0, tickIcon);
        ui->treeWidget->topLevelItem(1)->child(3)->setToolTip(0, "Valid");
    }
    else
    {
        isWindInputValid = false;
        ui->treeWidget->topLevelItem(1)->child(3)->setIcon(0, crossIcon);
        ui->treeWidget->topLevelItem(1)->child(3)->setToolTip(0, "No Initialization Method Selected");
    }

    if (isSurfaceInputValid && isWindInputValid)
    {
        isInputValid = true;
        ui->treeWidget->topLevelItem(1)->setIcon(0, tickIcon);
        ui->treeWidget->topLevelItem(1)->setToolTip(0, "Valid");
    }
    else if (!isSurfaceInputValid && !isWindInputValid)
    {
        isInputValid = false;
        ui->treeWidget->topLevelItem(1)->setIcon(0, crossIcon);
        ui->treeWidget->topLevelItem(1)->setToolTip(0, "Check Surface Input");
    }
    else if (!isSurfaceInputValid)
    {
        isInputValid = false;
        ui->treeWidget->topLevelItem(1)->setIcon(0, crossIcon);
        ui->treeWidget->topLevelItem(1)->setToolTip(0, "Check Surface Input");
    }
    else if (!isWindInputValid)
    {
        isInputValid = false;
        ui->treeWidget->topLevelItem(1)->setIcon(0, crossIcon);
        ui->treeWidget->topLevelItem(1)->setToolTip(0, "Check Wind Input");
    }

    updateOverallState();
}

void AppState::updateOutputState()
{
    if(isGoogleEarthValid || isFireBehaviorValid || isShapeFilesValid || isGeoSpatialPDFFilesValid || isVTKFilesValid)
    {
        isOutputValid = true;
        ui->treeWidget->topLevelItem(2)->setIcon(0, tickIcon);
        ui->treeWidget->topLevelItem(2)->setToolTip(0, "Valid");
    }
    else
    {
        ui->treeWidget->topLevelItem(2)->setIcon(0, crossIcon);
        ui->treeWidget->topLevelItem(2)->setToolTip(0, "No Output Selected");
    }

    updateOverallState();
}

void AppState::updateOverallState()
{
    if (isSolverMethodologyValid && isInputValid && isOutputValid)
    {
        ui->numberOfProcessorsSolveButton->setEnabled(true);
        ui->numberOfProcessorsSolveButton->setToolTip("");
        ui->treeWidget->topLevelItem(3)->setIcon(0, tickIcon);
        ui->treeWidget->topLevelItem(3)->setToolTip(0, "");
    }
    else
    {
        ui->numberOfProcessorsSolveButton->setEnabled(false);
        ui->numberOfProcessorsSolveButton->setToolTip("Solver Methodology and Inputs must be passing to solve.");
        ui->treeWidget->topLevelItem(3)->setIcon(0, crossIcon);
        ui->treeWidget->topLevelItem(3)->setToolTip(0, "There are errors in the inputs or outputs");
    }
}
