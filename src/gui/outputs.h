#ifndef OUTPUTS_H
#define OUTPUTS_H

#include "ui_mainWindow.h"
#include "appState.h"
#include <QObject>
#include <QStandardPaths>
#include <QFileDialog>
#include <QtWebEngineWidgets/qwebengineview.h>



class Outputs : public QObject
{
    Q_OBJECT
public:
    explicit Outputs(Ui::MainWindow *ui, QWebEngineView *webEngineView, QObject* parent);

signals:
    void updateGoogleState();
    void updateFireBehaviorState();
    void updateShapeState();
    void updatePDFState();
    void updateVTKState();

private slots:
    void windHeightComboBoxCurrentIndexChanged(int index);
    void googleEarthCheckBoxToggled(bool checked);
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
    void kmzOutputOpenFileButtonClicked();
    void kmzOutputClearButtonClicked();

private:
    Ui::MainWindow *ui;
    QWebEngineView *webEngineView;

};

#endif // OUTPUTS_H
