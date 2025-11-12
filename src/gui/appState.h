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

#include "ui_mainWindow.h"
#include <QObject>


class AppState : public QObject
{
    Q_OBJECT

public:
    static AppState& instance();
    void setUi(Ui::MainWindow* mainWindowUi);
    void setState();

    bool isSolverMethodologyValid = false;
    bool isMassSolverToggled = false;
    bool isMomentumSolverToggled = false;

    bool isInputValid = false;
    bool isSurfaceInputValid = false;
    bool isDiurnalInputToggled = false;
    bool isStabilityInputToggled = false;

    bool isWindInputValid = false;
    bool isDomainAverageInitializationToggled = false;
    bool isDomainAverageWindInputTableValid = true;
    bool isDomainAverageInitializationValid = false;
    bool isPointInitializationToggled = false;
    bool isStationFileSelected = false;
    bool isStationFileSelectionValid = false;
    bool isPointInitializationValid = false;
    bool isWeatherModelInitializationToggled = false;
    bool isWeatherModelForecastValid = false;
    bool isWeatherModelInitializationValid = false;

    bool isOutputValid = false;
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

    bool isSolverReady = false;

public slots:
    void updateSolverMethodologyState();
    void updateSurfaceInputState();
    void updateDiurnalInputState();
    void updateStabilityInputState();
    void updateDomainAverageInputState();
    void updatePointInitializationInputState();
    void updateWeatherModelInputState();
    void updateGoogleEarthOutputState();
    void updateFireBehaviorOutputState();
    void updateShapeFilesOutputState();
    void updateGeoSpatialPDFFilesOutputState();
    void updateVTKFilesOutputState();

private:
    Ui::MainWindow *ui;

    QIcon tickIcon;
    QIcon crossIcon;
    QIcon bulletIcon;

    AppState();
    AppState(const AppState&) = delete;
    AppState& operator=(const AppState&) = delete;
    void updateInputState();
    void updateOutputState();
    void updateOverallState();
};

#endif // APPSTATE_H
