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
    void updateFireBehaviorState();
    void updateGeoTiffState();
    void updateShapeState();
    void updatePDFState();
    void updateVTKState();

private slots:
    void windHeightComboBoxCurrentIndexChanged(int index);
    void windHeightSpinBoxValueChanged();
    void windHeightUnitsComboBoxCurrentIndexChanged();
    void windSpeedUnitsComboBoxCurrentIndexChanged();
    void googleEarthCheckBoxToggled();
    void fireBehaviorGroupBoxToggled();
    void fireBehaviorAsciiCheckBoxClicked();
    void fireBehaviorGeoTiffCheckBoxClicked();
    void fireBehaviorAtmFileCheckBoxClicked();
    void geoTiffFilesCheckBoxClicked();
    void shapeFilesGroupBoxToggled();
    void geospatialPDFFilesGroupBoxToggled();
    void VTKFilesCheckBoxClicked();
    void googleEarthMeshResolutionGroupBoxToggled(bool checked);
    void fireBehaviorMeshResolutionGroupBoxToggled(bool checked);
    void shapeFilesMeshResolutionGroupBoxToggled(bool checked);
    void geospatialPDFFilesMeshResolutionGroupBoxToggled(bool checked);
    void meshResolutionSpinBoxValueChanged(double value);
    void meshResolutionUnitsComboBoxCurrentIndexChanged(int index);
    void googleEarthMeshResolutionSpinBoxValueChanged();
    void fireBehaviorMeshResolutionSpinBoxValueChanged();
    void shapeFilesMeshResolutionSpinBoxValueChanged();
    void geospatialPDFFilesMeshResolutionSpinBoxValueChanged();

private:
    Ui::MainWindow *ui;
};

#endif // OUTPUTS_H
