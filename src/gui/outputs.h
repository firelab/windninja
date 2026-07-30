/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja Qt GUI
 * Purpose:  Handles functions of output pages
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

#ifndef OUTPUTS_H
#define OUTPUTS_H

#include "ui_mainWindow.h"
#include "appState.h"
#include <QObject>
#include <QStandardPaths>
#include <QFileDialog>


class Outputs : public QObject
{
    Q_OBJECT
public:
    explicit Outputs(Ui::MainWindow *ui, QObject* parent);

signals:
    void updateGoogleState();
    void updateFireBehaviorAsciiState();
    void updateFireBehaviorGeoTiffState();
    void updateShapeState();
    void updatePDFState();
    void updateVTKState();
    void updateMapVisualizationState();

private slots:
    void windHeightComboBoxCurrentIndexChanged(int index);
    void windHeightSpinBoxValueChanged();
    void windHeightUnitsComboBoxCurrentIndexChanged();
    void windSpeedUnitsComboBoxCurrentIndexChanged();
    void googleEarthGroupBoxToggled();
    void fireBehaviorAsciiGroupBoxToggled();
    void fireBehaviorAsciiAtmFileCheckBoxClicked();
    void fireBehaviorGeoTiffGroupBoxToggled();
    void fireBehaviorGeoTiffAtmFileCheckBoxClicked();
    void shapeFilesGroupBoxToggled();
    void geospatialPDFFilesGroupBoxToggled();
    void VTKFilesCheckBoxToggled();
    void mapVisualizationCheckBoxToggled();
    void googleEarthMeshResolutionGroupBoxToggled(bool checked);
    void fireBehaviorAsciiMeshResolutionGroupBoxToggled(bool checked);
    void fireBehaviorGeoTiffMeshResolutionGroupBoxToggled(bool checked);
    void shapeFilesMeshResolutionGroupBoxToggled(bool checked);
    void geospatialPDFFilesMeshResolutionGroupBoxToggled(bool checked);
    void mapVisualizationMeshResolutionGroupBoxToggled(bool checked);
    void meshResolutionSpinBoxValueChanged(double value);
    void meshResolutionUnitsComboBoxCurrentIndexChanged(int index);
    void googleEarthMeshResolutionSpinBoxValueChanged();
    void fireBehaviorAsciiMeshResolutionSpinBoxValueChanged();
    void fireBehaviorGeoTiffMeshResolutionSpinBoxValueChanged();
    void shapeFilesMeshResolutionSpinBoxValueChanged();
    void geospatialPDFFilesMeshResolutionSpinBoxValueChanged();
    void mapVisualizationMeshResolutionSpinBoxValueChanged();

private:
    Ui::MainWindow *ui;
};

#endif // OUTPUTS_H
