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
    crossIcon(":/cross.png"),
    bulletIcon(":/bullet_blue.png")
{ }

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
    if (isMassSolverToggled != isMomentumSolverToggled)
    {
        isSolverMethodologyValid = true;
        ui->treeWidget->topLevelItem(0)->setIcon(0, tickIcon);
        ui->treeWidget->topLevelItem(0)->setToolTip(0, "");
    }
    else if (isMassSolverToggled && isMomentumSolverToggled)
    {
        isSolverMethodologyValid = false;
        ui->treeWidget->topLevelItem(0)->setIcon(0, crossIcon);
        ui->treeWidget->topLevelItem(0)->setToolTip(0,"Requires exactly one selection: currently too many selections.");
    }
    else
    {
        isSolverMethodologyValid = false;
        ui->treeWidget->topLevelItem(0)->setIcon(0, crossIcon);
        ui->treeWidget->topLevelItem(0)->setToolTip(0,"Requires exactly one selection: currently no selections.");
    }

    if (isMassSolverToggled)
    {
        ui->treeWidget->topLevelItem(0)->child(0)->setIcon(0, tickIcon);
    }
    else
    {
        ui->treeWidget->topLevelItem(0)->child(0)->setIcon(0, bulletIcon);
    }

    if (isMomentumSolverToggled)
    {
        ui->treeWidget->topLevelItem(0)->child(1)->setIcon(0, tickIcon);
    }
    else
    {
        ui->treeWidget->topLevelItem(0)->child(1)->setIcon(0, bulletIcon);
    }

    updateOverallState();
}

void AppState::updateSurfaceInputState()
{
    if (ui->elevationInputFileLineEdit->text() != "")
    {
        isSurfaceInputValid = true;
        ui->treeWidget->topLevelItem(1)->child(0)->setIcon(0, tickIcon);
        ui->treeWidget->topLevelItem(1)->child(0)->setToolTip(0, "");
    }
    else
    {
        isSurfaceInputValid = false;
        ui->treeWidget->topLevelItem(1)->child(0)->setIcon(0, crossIcon);
        ui->treeWidget->topLevelItem(1)->child(0)->setToolTip(0, "No DEM file detected.");
    }
    updateInputState();
}

void AppState::updateDiurnalInputState()
{
    if (isDiurnalInputToggled)
    {
        ui->treeWidget->topLevelItem(1)->child(1)->setIcon(0, tickIcon);
    }
    else
    {
        ui->treeWidget->topLevelItem(1)->child(1)->setIcon(0, bulletIcon);
    }
}

void AppState::updateStabilityInputState()
{
    if (isStabilityInputToggled)
    {
        ui->treeWidget->topLevelItem(1)->child(2)->setIcon(0, tickIcon);
    }
    else
    {
        ui->treeWidget->topLevelItem(1)->child(2)->setIcon(0, bulletIcon);
    }
}

void AppState::updateDomainAverageInputState()
{
    if (isDomainAverageInitializationToggled && isDomainAverageWindInputTableValid)
    {
        ui->treeWidget->topLevelItem(1)->child(3)->child(0)->setIcon(0, tickIcon);
        ui->treeWidget->topLevelItem(1)->child(3)->child(0)->setToolTip(0, "");
        isDomainAverageInitializationValid = true;
    }
    else if (isDomainAverageInitializationToggled && !isDomainAverageWindInputTableValid)
    {
        ui->treeWidget->topLevelItem(1)->child(3)->child(0)->setIcon(0, crossIcon);
        ui->treeWidget->topLevelItem(1)->child(3)->child(0)->setToolTip(0, "Bad wind inputs; hover over red cells for explanation.");
        isDomainAverageInitializationValid = false;
    }
    else
    {
        ui->treeWidget->topLevelItem(1)->child(3)->child(0)->setIcon(0, bulletIcon);
        ui->treeWidget->topLevelItem(1)->child(3)->child(0)->setToolTip(0, "");
        isDomainAverageInitializationValid = false;
    }

    updateInputState();
}

void AppState::updatePointInitializationInputState()
{
    if (isPointInitializationToggled && isStationFileSelectionValid && isStationFileSelected)
    {
        ui->treeWidget->topLevelItem(1)->child(3)->child(1)->setIcon(0, tickIcon);
        ui->treeWidget->topLevelItem(1)->child(3)->child(1)->setToolTip(0, "");
        isPointInitializationValid = true;
    }
    else if(isPointInitializationToggled && !isStationFileSelected)
    {
        ui->treeWidget->topLevelItem(1)->child(3)->child(1)->setIcon(0, crossIcon);
        ui->treeWidget->topLevelItem(1)->child(3)->child(1)->setToolTip(0, "No station file selected.");
        isPointInitializationValid = false;
    }
    else if(isPointInitializationToggled && !isStationFileSelectionValid)
    {
        ui->treeWidget->topLevelItem(1)->child(3)->child(1)->setIcon(0, crossIcon);
        ui->treeWidget->topLevelItem(1)->child(3)->child(1)->setToolTip(0, "Conflicting files selected.");
        isPointInitializationValid = false;
    }
    else
    {
        ui->treeWidget->topLevelItem(1)->child(3)->child(1)->setIcon(0, bulletIcon);
        ui->treeWidget->topLevelItem(1)->child(3)->child(1)->setToolTip(0, "");
        isPointInitializationValid = false;
    }

    updateInputState();
}

void AppState::updateWeatherModelInputState()
{
    // Update weather model initialization
    if (isWeatherModelInitializationToggled && isWeatherModelForecastValid) {
        ui->treeWidget->topLevelItem(1)->child(3)->child(2)->setIcon(0, tickIcon);
        isWeatherModelInitializationValid = true;
    } else if (isWeatherModelInitializationToggled && !isWeatherModelForecastValid) {
        ui->treeWidget->topLevelItem(1)->child(3)->child(2)->setIcon(0, crossIcon);
        ui->treeWidget->topLevelItem(1)->child(3)->child(2)->setToolTip(0, "Forecast is Invalid");
        isWeatherModelInitializationValid = false;
    } else {
        ui->treeWidget->topLevelItem(1)->child(3)->child(2)->setIcon(0, bulletIcon);
        isWeatherModelInitializationValid = false;
    }

    updateInputState();
}

void AppState::updateGoogleEarthOutputState()
{
    if(isGoogleEarthToggled)
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
    }

    updateOutputState();
}

void AppState::updateInputState()
{
    if (isDomainAverageInitializationValid || isPointInitializationValid || isWeatherModelInitializationValid)
    {
        ui->treeWidget->topLevelItem(1)->child(3)->setIcon(0, tickIcon);
        isWindInputValid = true;
    }
    else
    {
        ui->treeWidget->topLevelItem(1)->child(3)->setIcon(0, crossIcon);
        isWindInputValid = false;
    }

    if (isSurfaceInputValid && isWindInputValid)
    {
        isInputValid = true;
        ui->treeWidget->topLevelItem(1)->setIcon(0, tickIcon);
        ui->treeWidget->topLevelItem(1)->setToolTip(0, "");
    }
    else if (!isSurfaceInputValid && !isWindInputValid)
    {
        isInputValid = false;
        ui->treeWidget->topLevelItem(1)->setIcon(0, crossIcon);
        ui->treeWidget->topLevelItem(1)->setToolTip(0, "Bad surface and wind inputs.");
    }
    else if (!isSurfaceInputValid)
    {
        isInputValid = false;
        ui->treeWidget->topLevelItem(1)->setIcon(0, crossIcon);
        ui->treeWidget->topLevelItem(1)->setToolTip(0, "Bad surface input.");
    }
    else if (!isWindInputValid)
    {
        isInputValid = false;
        ui->treeWidget->topLevelItem(1)->setIcon(0, crossIcon);
        ui->treeWidget->topLevelItem(1)->setToolTip(0, "Bad wind input.");
    }

    updateOverallState();
}

void AppState::updateOutputState()
{
    if(isGoogleEarthValid || isFireBehaviorValid || isShapeFilesValid || isGeoSpatialPDFFilesValid || isVTKFilesValid)
    {
        isOutputValid = true;
        ui->treeWidget->topLevelItem(2)->setIcon(0, tickIcon);
        ui->treeWidget->topLevelItem(2)->setToolTip(0, "");
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
        ui->solveButton->setEnabled(true);
        ui->numberOfProcessorsSolveButton->setEnabled(true);
        ui->solveButton->setToolTip("");
        ui->numberOfProcessorsSolveButton->setToolTip("");
        ui->treeWidget->topLevelItem(3)->setIcon(0, tickIcon);
        ui->treeWidget->topLevelItem(3)->setToolTip(0, "");
    }
    else
    {
        ui->solveButton->setEnabled(false);
        ui->numberOfProcessorsSolveButton->setEnabled(false);
        ui->solveButton->setToolTip("Solver Methodology and Inputs must be passing to solve.");
        ui->numberOfProcessorsSolveButton->setToolTip("Solver Methodology and Inputs must be passing to solve.");
        ui->treeWidget->topLevelItem(3)->setIcon(0, crossIcon);
        ui->treeWidget->topLevelItem(3)->setToolTip(0, "There are errors in the inputs or outputs");
    }
}
