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

void AppState::setUi(Ui::MainWindow* mainUi)
{
    ui = mainUi;
}

/*
 * Helper function to refresh the ui state of the app
 * Called on every user input action
 */
void AppState::refreshUI()
{
    AppState& state = AppState::instance();

    QIcon tickIcon(":/tick.png");
    QIcon xIcon(":/cross.png");
    QIcon bulletIcon(":/bullet_blue.png");

    ui->treeWidget->setMouseTracking(true);

    // Update Solver Methodology UI
    if (state.isMassSolverToggled != state.isMomentumSolverToggled) {
        state.isSolverMethodologyValid = true;
        ui->treeWidget->topLevelItem(0)->setIcon(0, tickIcon);
        ui->treeWidget->topLevelItem(0)->setToolTip(0, "");
    } else if (state.isMassSolverToggled && state.isMomentumSolverToggled) {
        state.isSolverMethodologyValid = false;
        ui->treeWidget->topLevelItem(0)->setIcon(0, xIcon);
        ui->treeWidget->topLevelItem(0)->setToolTip(0,"Requires exactly one selection: currently too many selections.");
    } else {
        state.isSolverMethodologyValid = false;
        ui->treeWidget->topLevelItem(0)->setIcon(0, xIcon);
        ui->treeWidget->topLevelItem(0)->setToolTip(0,"Requires exactly one selection: currently no selections.");
    }

    if (state.isMassSolverToggled) {
        ui->treeWidget->topLevelItem(0)->child(0)->setIcon(0, tickIcon);
    } else {
        ui->treeWidget->topLevelItem(0)->child(0)->setIcon(0, bulletIcon);
    }

    if (state.isMomentumSolverToggled) {
        ui->treeWidget->topLevelItem(0)->child(1)->setIcon(0, tickIcon);
    } else {
        ui->treeWidget->topLevelItem(0)->child(1)->setIcon(0, bulletIcon);
    }

    // Update surface input state
    if (ui->elevationInputFileLineEdit->text() != "") {
        state.isSurfaceInputValid = true;
        ui->treeWidget->topLevelItem(1)->child(0)->setIcon(0, tickIcon);
        ui->treeWidget->topLevelItem(1)->child(0)->setToolTip(0, "");
    } else {
        state.isSurfaceInputValid = false;
        ui->treeWidget->topLevelItem(1)->child(0)->setIcon(0, xIcon);
        ui->treeWidget->topLevelItem(1)->child(0)->setToolTip(0, "No DEM file detected.");
    }

    // Update diurnal input state
    if (state.isDiurnalInputToggled) {
        ui->treeWidget->topLevelItem(1)->child(1)->setIcon(0, tickIcon);
    } else {
        ui->treeWidget->topLevelItem(1)->child(1)->setIcon(0, bulletIcon);
    }

    // Update stability input state
    if (state.isStabilityInputToggled) {
        ui->treeWidget->topLevelItem(1)->child(2)->setIcon(0, tickIcon);
    } else {
        ui->treeWidget->topLevelItem(1)->child(2)->setIcon(0, bulletIcon);
    }

    // Update domain average initialization
    if (state.isDomainAverageInitializationToggled && state.isDomainAverageWindInputTableValid) {
        ui->treeWidget->topLevelItem(1)->child(3)->child(0)->setIcon(0, tickIcon);
        ui->treeWidget->topLevelItem(1)->child(3)->child(0)->setToolTip(0, "");
        state.isDomainAverageInitializationValid = true;
    } else if (state.isDomainAverageInitializationToggled && !state.isDomainAverageWindInputTableValid){
        ui->treeWidget->topLevelItem(1)->child(3)->child(0)->setIcon(0, xIcon);
        ui->treeWidget->topLevelItem(1)->child(3)->child(0)->setToolTip(0, "Bad wind inputs; hover over red cells for explanation.");
        state.isDomainAverageInitializationValid = false;
    } else {
        ui->treeWidget->topLevelItem(1)->child(3)->child(0)->setIcon(0, bulletIcon);
        ui->treeWidget->topLevelItem(1)->child(3)->child(0)->setToolTip(0, "");
        state.isDomainAverageInitializationValid = false;
    }

    // Update point initialization
    if (state.isPointInitializationToggled && state.isStationFileSelectionValid && state.isStationFileSelected) {
        ui->treeWidget->topLevelItem(1)->child(3)->child(1)->setIcon(0, tickIcon);
        ui->treeWidget->topLevelItem(1)->child(3)->child(1)->setToolTip(0, "");
        state.isPointInitializationValid = true;
    } else if(state.isPointInitializationToggled && !state.isStationFileSelected) {
        ui->treeWidget->topLevelItem(1)->child(3)->child(1)->setIcon(0, xIcon);
        ui->treeWidget->topLevelItem(1)->child(3)->child(1)->setToolTip(0, "No station file selected.");
        state.isPointInitializationValid = false;
    } else if(state.isPointInitializationToggled && !state.isStationFileSelectionValid){
        ui->treeWidget->topLevelItem(1)->child(3)->child(1)->setIcon(0, xIcon);
        ui->treeWidget->topLevelItem(1)->child(3)->child(1)->setToolTip(0, "Conflicting files selected.");
        state.isPointInitializationValid = false;
    } else {
        ui->treeWidget->topLevelItem(1)->child(3)->child(1)->setIcon(0, bulletIcon);
        ui->treeWidget->topLevelItem(1)->child(3)->child(1)->setToolTip(0, "");
        state.isPointInitializationValid = false;
    }

    // Update weather model initialization
    if (state.isWeatherModelInitializationToggled && state.isWeatherModelForecastValid) {
        ui->treeWidget->topLevelItem(1)->child(3)->child(2)->setIcon(0, tickIcon);
        state.isWeatherModelInitializationValid = true;
    } else if (state.isWeatherModelInitializationToggled && !state.isWeatherModelForecastValid) {
        ui->treeWidget->topLevelItem(1)->child(3)->child(2)->setIcon(0, xIcon);
        ui->treeWidget->topLevelItem(1)->child(3)->child(2)->setToolTip(0, "Forecast is Invalid");
        state.isWeatherModelInitializationValid = false;
    } else {
        ui->treeWidget->topLevelItem(1)->child(3)->child(2)->setIcon(0, bulletIcon);
        state.isWeatherModelInitializationValid = false;
    }

    //  Update wind input
    if (state.isDomainAverageInitializationValid || state.isPointInitializationValid || state.isWeatherModelInitializationValid) {
        ui->treeWidget->topLevelItem(1)->child(3)->setIcon(0, tickIcon);
        state.isWindInputValid = true;
    } else {
        ui->treeWidget->topLevelItem(1)->child(3)->setIcon(0, xIcon);
        state.isWindInputValid = false;
    }

    // Update overall input UI state
    if (state.isSurfaceInputValid && state.isWindInputValid) {
        state.isInputsValid = true;
        ui->treeWidget->topLevelItem(1)->setIcon(0, tickIcon);
        ui->treeWidget->topLevelItem(1)->setToolTip(0, "");
    } else if (!state.isSurfaceInputValid && !state.isWindInputValid) {
        state.isInputsValid = false;
        ui->treeWidget->topLevelItem(1)->setIcon(0, xIcon);
        ui->treeWidget->topLevelItem(1)->setToolTip(0, "Bad surface and wind inputs.");
    } else if (!state.isSurfaceInputValid) {
        state.isInputsValid = false;
        ui->treeWidget->topLevelItem(1)->setIcon(0, xIcon);
        ui->treeWidget->topLevelItem(1)->setToolTip(0, "Bad surface input.");
    } else if (!state.isWindInputValid) {
        state.isInputsValid = false;
        ui->treeWidget->topLevelItem(1)->setIcon(0, xIcon);
        ui->treeWidget->topLevelItem(1)->setToolTip(0, "Bad wind input.");
    }

    if(state.isGoogleEarthToggled)
    {
        if(state.isSurfaceInputValid)
        {
            state.isGoogleEarthValid = true;
            ui->treeWidget->topLevelItem(2)->child(0)->setIcon(0, tickIcon);
            ui->treeWidget->topLevelItem(2)->setToolTip(0, "");
        }
        else
        {
            state.isGoogleEarthValid = false;
            ui->treeWidget->topLevelItem(2)->setIcon(0, xIcon);
            ui->treeWidget->topLevelItem(2)->setToolTip(0, "Cannot read DEM File");
            ui->treeWidget->topLevelItem(2)->child(0)->setIcon(0, xIcon);
            ui->treeWidget->topLevelItem(2)->child(0)->setToolTip(0, "Cannot read DEM File");
        }
    }
    else
    {
        state.isGoogleEarthValid = false;
        ui->treeWidget->topLevelItem(2)->child(0)->setIcon(0, bulletIcon);
    }
    if(state.isFireBehaviorToggled)
    {
        if(state.isSurfaceInputValid)
        {
            state.isFireBehaviorValid = true;
            ui->treeWidget->topLevelItem(2)->child(1)->setIcon(0, tickIcon);
            ui->treeWidget->topLevelItem(2)->setToolTip(0, "");
        }
        else
        {
            state.isFireBehaviorValid = false;
            ui->treeWidget->topLevelItem(2)->setIcon(0, xIcon);
            ui->treeWidget->topLevelItem(2)->setToolTip(0, "Cannot read DEM File");
            ui->treeWidget->topLevelItem(2)->child(1)->setIcon(0, xIcon);
            ui->treeWidget->topLevelItem(2)->child(1)->setToolTip(0, "Cannot read DEM File");
        }
    }
    else
    {
        state.isFireBehaviorValid = false;
        ui->treeWidget->topLevelItem(2)->child(1)->setIcon(0, bulletIcon);
    }
    if(state.isShapeFilesToggled)
    {
        if(state.isSurfaceInputValid)
        {
            state.isShapeFilesValid = true;
            ui->treeWidget->topLevelItem(2)->child(2)->setIcon(0, tickIcon);
            ui->treeWidget->topLevelItem(2)->setToolTip(0, "");
        }
        else
        {
            state.isShapeFilesValid = false;
            ui->treeWidget->topLevelItem(2)->setIcon(0, xIcon);
            ui->treeWidget->topLevelItem(2)->setToolTip(0, "Cannot read DEM File");
            ui->treeWidget->topLevelItem(2)->child(2)->setIcon(0, xIcon);
            ui->treeWidget->topLevelItem(2)->child(2)->setToolTip(0, "Cannot read DEM File");
        }
    }
    else
    {
        state.isShapeFilesValid = false;
        ui->treeWidget->topLevelItem(2)->child(2)->setIcon(0, bulletIcon);
    }
    if(state.isGeoSpatialPDFFilesToggled)
    {
        if(state.isSurfaceInputValid)
        {
            state.isGeoSpatialPDFFilesValid = true;
            ui->treeWidget->topLevelItem(2)->child(3)->setIcon(0, tickIcon);
            ui->treeWidget->topLevelItem(2)->setToolTip(0, "");
        }
        else
        {
            state.isGeoSpatialPDFFilesValid = false;
            ui->treeWidget->topLevelItem(2)->setIcon(0, xIcon);
            ui->treeWidget->topLevelItem(2)->setToolTip(0, "Cannot read DEM File");
            ui->treeWidget->topLevelItem(2)->child(3)->setIcon(0, xIcon);
            ui->treeWidget->topLevelItem(2)->child(3)->setToolTip(0, "Cannot read DEM File");
        }
    }
    else
    {
        state.isGeoSpatialPDFFilesValid = false;
        ui->treeWidget->topLevelItem(2)->child(3)->setIcon(0, bulletIcon);
    }
    if(state.isVTKFilesToggled)
    {
        if(state.isSurfaceInputValid)
        {
            state.isVTKFilesValid = true;
            ui->treeWidget->topLevelItem(2)->child(4)->setIcon(0, tickIcon);
            ui->treeWidget->topLevelItem(2)->setToolTip(0, "");
        }
        else
        {
            state.isVTKFilesValid = false;
            ui->treeWidget->topLevelItem(2)->setIcon(0, xIcon);
            ui->treeWidget->topLevelItem(2)->setToolTip(0, "Cannot read DEM File");
            ui->treeWidget->topLevelItem(2)->child(4)->setIcon(0, xIcon);
            ui->treeWidget->topLevelItem(2)->child(4)->setToolTip(0, "Cannot read DEM File");
        }
    }
    else
    {
        state.isVTKFilesValid = false;
        ui->treeWidget->topLevelItem(2)->child(4)->setIcon(0, bulletIcon);
    }


    if(state.isGoogleEarthValid || state.isFireBehaviorValid || state.isShapeFilesValid || state.isGeoSpatialPDFFilesValid || state.isVTKFilesValid)
    {
        state.isOutputsValid = true;
        ui->treeWidget->topLevelItem(2)->setIcon(0, tickIcon);
        ui->treeWidget->topLevelItem(2)->setToolTip(0, "");
    }
    else
    {
        ui->treeWidget->topLevelItem(2)->setIcon(0, xIcon);
        ui->treeWidget->topLevelItem(2)->setToolTip(0, "No Output Selected");
    }

    if (state.isSolverMethodologyValid && state.isInputsValid && state.isOutputsValid) {
        ui->solveButton->setEnabled(true);
        ui->numberOfProcessorsSolveButton->setEnabled(true);
        ui->solveButton->setToolTip("");
        ui->numberOfProcessorsSolveButton->setToolTip("");
        ui->treeWidget->topLevelItem(3)->setIcon(0, tickIcon);
        ui->treeWidget->topLevelItem(3)->setToolTip(0, "");
    } else {
        ui->solveButton->setEnabled(false);
        ui->numberOfProcessorsSolveButton->setEnabled(false);
        ui->solveButton->setToolTip("Solver Methodology and Inputs must be passing to solve.");
        ui->numberOfProcessorsSolveButton->setToolTip("Solver Methodology and Inputs must be passing to solve.");
        ui->treeWidget->topLevelItem(3)->setIcon(0, xIcon);
        ui->treeWidget->topLevelItem(3)->setToolTip(0, "There are errors in the inputs or outputs");
    }
}
