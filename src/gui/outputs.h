#ifndef OUTPUTS_H
#define OUTPUTS_H

#include "ui_mainWindow.h"
#include "appState.h"
#include <QObject>
#include <QStandardPaths>


class Outputs : public QObject
{
    Q_OBJECT
public:
    explicit Outputs(Ui::MainWindow *ui, QObject* parent);

signals:
    void updateGoogleState();
    void updateFireBehaviorState();
    void updateShapeState();
    void updatePDFState();
    void updateVTKState();

private slots:
    void googleEarthGroupBoxToggled(bool checked);
    void fireBehaviorGroupBoxToggled(bool checked);
    void shapeFilesGroupBoxToggled(bool checked);
    void geospatialPDFFilesGroupBoxToggled(bool checked);
    void VTKFilesCheckBoxClicked(bool checked);
    void googleEarthMeshResolutionGroupBoxToggled(bool checked);
    void fireBehaviorMeshResolutionGroupBoxToggled(bool checked);
    void shapeFilesMeshResolutionGroupBoxToggled(bool checked);
    void geospatialPDFFilesMeshResolutionGroupBoxToggled(bool checked);
    void meshResolutionSpinBoxValueChanged(double value);
    void meshResolutionUnitsComboBoxCurrentIndexChanged(int index);

private:
    Ui::MainWindow *ui;
};

#endif // OUTPUTS_H
