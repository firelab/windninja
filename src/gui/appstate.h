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

#ifndef APPSTATE_H
#define APPSTATE_H

class AppState
{
public:
    static AppState& instance() {
        static AppState s;
        return s;
    }

    // Solver Methodology input states
    bool isSolverMethodologyValid = false;
    bool isMassSolverToggled = false;
    bool isMomentumSolverToggled = false;

    // Input states
    bool isInputsValid = false;
    bool isSurfaceInputValid = false;
    bool isDiurnalInputToggled = false;
    bool isStabilityInputToggled = false;

    // Wind Input States
    bool isWindInputValid = false;
    bool isDomainAverageInitializationToggled = false;
    bool isDomainAverageWindInputTableValid = true;
    bool isDomainAverageInitializationValid = false;
    bool isPointInitializationToggled = false;
    bool isPointInitializationValid = false;
    bool isWeatherModelInitializationToggled = false;
    bool isWeatherModelInitializationValid = false;
    bool isShowAllTimeZonesSelected = false;
    bool isDisplayTimeZoneDetailsSelected = false;

    //Output States
    bool isOutputsValid = false;
    bool isGoogleEarthToggled = false;
    bool isGoogleEarthValid = false;
    bool isFireBehaviorToggled = false;
    bool isFireBehaviorValid = false;
    bool isShapeFilesToggled = false;
    bool isShapeFilesValid = false;
    bool isGeoSpatialPDFFilesToggled = false;
    bool isGeoSpatialPDFFilesValid = false;
    bool isVTKFilesToggled = false;
    bool isVTKFilesValid = false;

    // All Inputs Ok
    bool isSolverReady = false;


private:
    AppState() {}                         // private constructor
    AppState(const AppState&) = delete;  // prevent copying
    AppState& operator=(const AppState&) = delete;
};

#endif // APPSTATE_H
