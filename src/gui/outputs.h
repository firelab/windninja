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
    void meshResolutionSpinBoxValueChanged(double value);
    void meshResolutionUnitsComboBoxCurrentIndexChanged(int index);
    void googleEarthMeshResolutionSpinBoxValueChanged();
    void fireBehaviorAsciiMeshResolutionSpinBoxValueChanged();
    void fireBehaviorGeoTiffMeshResolutionSpinBoxValueChanged();
    void shapeFilesMeshResolutionSpinBoxValueChanged();
    void geospatialPDFFilesMeshResolutionSpinBoxValueChanged();

private:
    Ui::MainWindow *ui;
};

#endif // OUTPUTS_H
