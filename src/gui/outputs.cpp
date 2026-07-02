#include "outputs.h"

Outputs::Outputs(Ui::MainWindow *ui,
                 QObject* parent)
    : QObject(parent),
    ui(ui)
{
    QString downloadsPath = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
    ui->outputDirectoryLineEdit->setText(downloadsPath);
    ui->outputDirectoryButton->setIcon(QIcon(":/folder.png"));

    ui->outputWindHeightUnitsComboBox->setItemData(0, "ft");
    ui->outputWindHeightUnitsComboBox->setItemData(1, "m");
    ui->meshResolutionUnitsComboBox->setItemData(0, "m");
    ui->meshResolutionUnitsComboBox->setItemData(1, "ft");
    ui->googleEarthMeshResolutionComboBox->setItemData(0, "m");
    ui->googleEarthMeshResolutionComboBox->setItemData(1, "ft");
    ui->fireBehaviorAsciiMeshResolutionComboBox->setItemData(0, "m");
    ui->fireBehaviorAsciiMeshResolutionComboBox->setItemData(1, "ft");
    ui->fireBehaviorGeoTiffMeshResolutionComboBox->setItemData(0, "m");
    ui->fireBehaviorGeoTiffMeshResolutionComboBox->setItemData(1, "ft");
    ui->shapeFilesMeshResolutionComboBox->setItemData(0, "m");
    ui->shapeFilesMeshResolutionComboBox->setItemData(1, "ft");
    ui->geospatialPDFFilesMeshResolutionComboBox->setItemData(0, "m");
    ui->geospatialPDFFilesMeshResolutionComboBox->setItemData(1, "ft");
    ui->mapVisualizationMeshResolutionComboBox->setItemData(0, "m");
    ui->mapVisualizationMeshResolutionComboBox->setItemData(1, "ft");
    ui->googleEarthAlternativeColorSchemeComboBox->setItemData(0, "default");
    ui->googleEarthAlternativeColorSchemeComboBox->setItemData(1, "ROPGW");
    ui->googleEarthAlternativeColorSchemeComboBox->setItemData(2, "oranges");
    ui->googleEarthAlternativeColorSchemeComboBox->setItemData(3, "blues");
    ui->googleEarthAlternativeColorSchemeComboBox->setItemData(4, "pinks");
    ui->googleEarthAlternativeColorSchemeComboBox->setItemData(5, "greens");
    ui->googleEarthAlternativeColorSchemeComboBox->setItemData(6, "magic_beans");
    ui->googleEarthAlternativeColorSchemeComboBox->setItemData(7, "pink_to_green");
    ui->googleEarthLegendComboBox->setItemData(0, "equal_interval");
    ui->googleEarthLegendComboBox->setItemData(1, "equal_color");
    ui->mapVisualizationAlternativeColorSchemeComboBox->setItemData(0, "default");
    ui->mapVisualizationAlternativeColorSchemeComboBox->setItemData(1, "ROPGW");
    ui->mapVisualizationAlternativeColorSchemeComboBox->setItemData(2, "oranges");
    ui->mapVisualizationAlternativeColorSchemeComboBox->setItemData(3, "blues");
    ui->mapVisualizationAlternativeColorSchemeComboBox->setItemData(4, "pinks");
    ui->mapVisualizationAlternativeColorSchemeComboBox->setItemData(5, "greens");
    ui->mapVisualizationAlternativeColorSchemeComboBox->setItemData(6, "magic_beans");
    ui->mapVisualizationAlternativeColorSchemeComboBox->setItemData(7, "pink_to_green");
    ui->mapVisualizationLegendComboBox->setItemData(0, "equal_interval");
    ui->mapVisualizationLegendComboBox->setItemData(1, "equal_color");

    connect(ui->outputWindHeightComboBox, &QComboBox::currentIndexChanged, this, &Outputs::windHeightComboBoxCurrentIndexChanged);
    connect(ui->outputWindHeightSpinBox, &QDoubleSpinBox::valueChanged, this, &Outputs::windHeightSpinBoxValueChanged);
    connect(ui->outputWindHeightUnitsComboBox, &QComboBox::currentIndexChanged, this, &Outputs::windHeightUnitsComboBoxCurrentIndexChanged);
    connect(ui->outputSpeedUnitsComboBox, &QComboBox::currentIndexChanged, this, &Outputs::windSpeedUnitsComboBoxCurrentIndexChanged);
    connect(ui->googleEarthGroupBox, &QGroupBox::toggled, this, &Outputs::googleEarthGroupBoxToggled);
    connect(ui->fireBehaviorAsciiGroupBox, &QGroupBox::toggled, this, &Outputs::fireBehaviorAsciiGroupBoxToggled);
    connect(ui->fireBehaviorAsciiAtmFileCheckBox, &QCheckBox::clicked, this, &Outputs::fireBehaviorAsciiAtmFileCheckBoxClicked);
    connect(ui->fireBehaviorGeoTiffGroupBox, &QGroupBox::toggled, this, &Outputs::fireBehaviorGeoTiffGroupBoxToggled);
    connect(ui->fireBehaviorGeoTiffAtmFileCheckBox, &QCheckBox::clicked, this, &Outputs::fireBehaviorGeoTiffAtmFileCheckBoxClicked);
    connect(ui->shapeFilesGroupBox, &QGroupBox::toggled, this, &Outputs::shapeFilesGroupBoxToggled);
    connect(ui->geospatialPDFFilesGroupBox, &QGroupBox::toggled, this, &Outputs::geospatialPDFFilesGroupBoxToggled);
    connect(ui->VTKFilesCheckBox, &QCheckBox::toggled, this, &Outputs::VTKFilesCheckBoxToggled);
    connect(ui->mapVisualizationCheckBox, &QCheckBox::toggled, this, &Outputs::mapVisualizationCheckBoxToggled);
    connect(ui->googleEarthMeshResolutionGroupBox, &QGroupBox::toggled, this, &Outputs::googleEarthMeshResolutionGroupBoxToggled);
    connect(ui->fireBehaviorAsciiMeshResolutionGroupBox, &QGroupBox::toggled, this, &Outputs::fireBehaviorAsciiMeshResolutionGroupBoxToggled);
    connect(ui->fireBehaviorGeoTiffMeshResolutionGroupBox, &QGroupBox::toggled, this, &Outputs::fireBehaviorGeoTiffMeshResolutionGroupBoxToggled);
    connect(ui->shapeFilesMeshResolutionGroupBox, &QGroupBox::toggled, this, &Outputs::shapeFilesMeshResolutionGroupBoxToggled);
    connect(ui->geospatialPDFFilesMeshResolutionGroupBox, &QGroupBox::toggled, this, &Outputs::geospatialPDFFilesMeshResolutionGroupBoxToggled);
    connect(ui->mapVisualizationMeshResolutionGroupBox, &QGroupBox::toggled, this, &Outputs::mapVisualizationMeshResolutionGroupBoxToggled);
    connect(this, &Outputs::updateGoogleState, &AppState::instance(), &AppState::updateGoogleEarthOutputState);
    connect(this, &Outputs::updateFireBehaviorAsciiState, &AppState::instance(), &AppState::updateFireBehaviorAsciiOutputState);
    connect(this, &Outputs::updateFireBehaviorGeoTiffState, &AppState::instance(), &AppState::updateFireBehaviorGeoTiffOutputState);
    connect(this, &Outputs::updateShapeState, &AppState::instance(), &AppState::updateShapeFilesOutputState);
    connect(this, &Outputs::updatePDFState, &AppState::instance(), &AppState::updateGeoSpatialPDFFilesOutputState);
    connect(this, &Outputs::updateVTKState, &AppState::instance(), &AppState::updateVTKFilesOutputState);
    connect(this, &Outputs::updateMapVisualizationState, &AppState::instance(), &AppState::updateMapVisualizationOutputState);
    connect(ui->meshResolutionSpinBox, &QDoubleSpinBox::valueChanged, this, &Outputs::meshResolutionSpinBoxValueChanged);
    connect(ui->meshResolutionUnitsComboBox, &QComboBox::currentIndexChanged, this, &Outputs::meshResolutionUnitsComboBoxCurrentIndexChanged);
    connect(ui->googleEarthMeshResolutionSpinBox, &QDoubleSpinBox::valueChanged, this, &Outputs::googleEarthMeshResolutionSpinBoxValueChanged);
    connect(ui->fireBehaviorAsciiMeshResolutionSpinBox, &QDoubleSpinBox::valueChanged, this, &Outputs::fireBehaviorAsciiMeshResolutionSpinBoxValueChanged);
    connect(ui->fireBehaviorGeoTiffMeshResolutionSpinBox, &QDoubleSpinBox::valueChanged, this, &Outputs::fireBehaviorGeoTiffMeshResolutionSpinBoxValueChanged);
    connect(ui->shapeFilesMeshResolutionSpinBox, &QDoubleSpinBox::valueChanged, this, &Outputs::shapeFilesMeshResolutionSpinBoxValueChanged);
    connect(ui->geospatialPDFFilesMeshResolutionSpinBox, &QDoubleSpinBox::valueChanged, this, &Outputs::geospatialPDFFilesMeshResolutionSpinBoxValueChanged);
    connect(ui->mapVisualizationMeshResolutionSpinBox, &QDoubleSpinBox::valueChanged, this, &Outputs::mapVisualizationMeshResolutionSpinBoxValueChanged);
}

void Outputs::windHeightComboBoxCurrentIndexChanged(int index)
{
    switch(index)
    {
    case 0:
        ui->outputWindHeightSpinBox->setValue(20.00);
        ui->outputWindHeightSpinBox->setEnabled(false);
        ui->outputWindHeightUnitsComboBox->setCurrentIndex(0);
        break;

    case 1:
        ui->outputWindHeightSpinBox->setValue(10.00);
        ui->outputWindHeightSpinBox->setEnabled(false);
        ui->outputWindHeightUnitsComboBox->setCurrentIndex(1);
        break;

    case 2:
        ui->outputWindHeightSpinBox->setValue(0.00);
        ui->outputWindHeightSpinBox->setEnabled(true);
        ui->outputWindHeightUnitsComboBox->setEnabled(true);
        break;
    }

    emit updateFireBehaviorAsciiState();
    emit updateFireBehaviorGeoTiffState();
}

void Outputs::windHeightSpinBoxValueChanged()
{
    emit updateFireBehaviorAsciiState();
    emit updateFireBehaviorGeoTiffState();
}

void Outputs::windHeightUnitsComboBoxCurrentIndexChanged()
{
    emit updateFireBehaviorAsciiState();
    emit updateFireBehaviorGeoTiffState();
}

void Outputs::windSpeedUnitsComboBoxCurrentIndexChanged()
{
    emit updateFireBehaviorAsciiState();
    emit updateFireBehaviorGeoTiffState();
}

void Outputs::googleEarthGroupBoxToggled()
{
    emit updateGoogleState();
}

void Outputs::fireBehaviorAsciiGroupBoxToggled()
{
    emit updateFireBehaviorAsciiState();
    emit updateFireBehaviorGeoTiffState();
}

void Outputs::fireBehaviorAsciiAtmFileCheckBoxClicked()
{
    emit updateFireBehaviorAsciiState();
    emit updateFireBehaviorGeoTiffState();
}

void Outputs::fireBehaviorGeoTiffGroupBoxToggled()
{
    emit updateFireBehaviorAsciiState();
    emit updateFireBehaviorGeoTiffState();
}

void Outputs::fireBehaviorGeoTiffAtmFileCheckBoxClicked()
{
    emit updateFireBehaviorAsciiState();
    emit updateFireBehaviorGeoTiffState();
}

void Outputs::shapeFilesGroupBoxToggled()
{
    emit updateShapeState();
}

void Outputs::geospatialPDFFilesGroupBoxToggled()
{
    emit updatePDFState();
}

void Outputs::VTKFilesCheckBoxToggled()
{
    emit updateVTKState();
}

void Outputs::mapVisualizationCheckBoxToggled()
{
    emit updateMapVisualizationState();
}

void Outputs::googleEarthMeshResolutionGroupBoxToggled(bool checked)
{
    ui->googleEarthMeshResolutionSpinBox->setEnabled(!checked);
    ui->googleEarthMeshResolutionComboBox->setEnabled(!checked);

    emit meshResolutionSpinBoxValueChanged(ui->meshResolutionSpinBox->value());
    emit meshResolutionUnitsComboBoxCurrentIndexChanged(ui->meshResolutionUnitsComboBox->currentIndex());

    emit updateGoogleState();
}

void Outputs::fireBehaviorAsciiMeshResolutionGroupBoxToggled(bool checked)
{
    ui->fireBehaviorAsciiMeshResolutionSpinBox->setEnabled(!checked);
    ui->fireBehaviorAsciiMeshResolutionComboBox->setEnabled(!checked);

    emit meshResolutionSpinBoxValueChanged(ui->meshResolutionSpinBox->value());
    emit meshResolutionUnitsComboBoxCurrentIndexChanged(ui->meshResolutionUnitsComboBox->currentIndex());

    emit updateFireBehaviorAsciiState();
    emit updateFireBehaviorGeoTiffState();
}

void Outputs::fireBehaviorGeoTiffMeshResolutionGroupBoxToggled(bool checked)
{
    ui->fireBehaviorGeoTiffMeshResolutionSpinBox->setEnabled(!checked);
    ui->fireBehaviorGeoTiffMeshResolutionComboBox->setEnabled(!checked);

    emit meshResolutionSpinBoxValueChanged(ui->meshResolutionSpinBox->value());
    emit meshResolutionUnitsComboBoxCurrentIndexChanged(ui->meshResolutionUnitsComboBox->currentIndex());

    emit updateFireBehaviorAsciiState();
    emit updateFireBehaviorGeoTiffState();
}

void Outputs::shapeFilesMeshResolutionGroupBoxToggled(bool checked)
{
    ui->shapeFilesMeshResolutionSpinBox->setEnabled(!checked);
    ui->shapeFilesMeshResolutionComboBox->setEnabled(!checked);

    emit meshResolutionSpinBoxValueChanged(ui->meshResolutionSpinBox->value());
    emit meshResolutionUnitsComboBoxCurrentIndexChanged(ui->meshResolutionUnitsComboBox->currentIndex());

    emit updateShapeState();
}

void Outputs::geospatialPDFFilesMeshResolutionGroupBoxToggled(bool checked)
{
    ui->geospatialPDFFilesMeshResolutionSpinBox->setEnabled(!checked);
    ui->geospatialPDFFilesMeshResolutionComboBox->setEnabled(!checked);

    emit meshResolutionSpinBoxValueChanged(ui->meshResolutionSpinBox->value());
    emit meshResolutionUnitsComboBoxCurrentIndexChanged(ui->meshResolutionUnitsComboBox->currentIndex());

    emit updatePDFState();
}

void Outputs::mapVisualizationMeshResolutionGroupBoxToggled(bool checked)
{
    ui->mapVisualizationMeshResolutionSpinBox->setEnabled(!checked);
    ui->mapVisualizationMeshResolutionComboBox->setEnabled(!checked);

    emit meshResolutionSpinBoxValueChanged(ui->meshResolutionSpinBox->value());
    emit meshResolutionUnitsComboBoxCurrentIndexChanged(ui->meshResolutionUnitsComboBox->currentIndex());

    emit updateMapVisualizationState();
}

void Outputs::meshResolutionSpinBoxValueChanged(double value)
{
    if(ui->googleEarthMeshResolutionGroupBox->isChecked())
    {
        ui->googleEarthMeshResolutionSpinBox->setValue(value);
        emit updateGoogleState();
    }

    if(ui->fireBehaviorAsciiMeshResolutionGroupBox->isChecked())
    {
        ui->fireBehaviorAsciiMeshResolutionSpinBox->setValue(value);
        emit updateFireBehaviorAsciiState();
        emit updateFireBehaviorGeoTiffState();
    }

    if(ui->fireBehaviorGeoTiffMeshResolutionGroupBox->isChecked())
    {
        ui->fireBehaviorGeoTiffMeshResolutionSpinBox->setValue(value);
        emit updateFireBehaviorAsciiState();
        emit updateFireBehaviorGeoTiffState();
    }

    if(ui->shapeFilesMeshResolutionGroupBox->isChecked())
    {
        ui->shapeFilesMeshResolutionSpinBox->setValue(value);
        emit updateShapeState();
    }

    if(ui->geospatialPDFFilesMeshResolutionGroupBox->isChecked())
    {
        ui->geospatialPDFFilesMeshResolutionSpinBox->setValue(value);
        emit updatePDFState();
    }

    if(ui->mapVisualizationMeshResolutionGroupBox->isChecked())
    {
        ui->mapVisualizationMeshResolutionSpinBox->setValue(value);
        emit updateMapVisualizationState();
    }
}

void Outputs::meshResolutionUnitsComboBoxCurrentIndexChanged(int index)
{
    if(ui->googleEarthMeshResolutionGroupBox->isChecked())
    {
        ui->googleEarthMeshResolutionComboBox->setCurrentIndex(index);
    }

    if(ui->fireBehaviorAsciiMeshResolutionGroupBox->isChecked())
    {
        ui->fireBehaviorAsciiMeshResolutionComboBox->setCurrentIndex(index);
    }

    if(ui->fireBehaviorGeoTiffMeshResolutionGroupBox->isChecked())
    {
        ui->fireBehaviorGeoTiffMeshResolutionComboBox->setCurrentIndex(index);
    }

    if(ui->shapeFilesMeshResolutionGroupBox->isChecked())
    {
        ui->shapeFilesMeshResolutionComboBox->setCurrentIndex(index);
    }

    if(ui->geospatialPDFFilesMeshResolutionGroupBox->isChecked())
    {
        ui->geospatialPDFFilesMeshResolutionComboBox->setCurrentIndex(index);
    }

    if(ui->mapVisualizationMeshResolutionGroupBox->isChecked())
    {
        ui->mapVisualizationMeshResolutionComboBox->setCurrentIndex(index);
    }
}

void Outputs::googleEarthMeshResolutionSpinBoxValueChanged()
{
    emit updateGoogleState();
}

void Outputs::fireBehaviorAsciiMeshResolutionSpinBoxValueChanged()
{
    emit updateFireBehaviorAsciiState();
    emit updateFireBehaviorGeoTiffState();
}

void Outputs::fireBehaviorGeoTiffMeshResolutionSpinBoxValueChanged()
{
    emit updateFireBehaviorAsciiState();
    emit updateFireBehaviorGeoTiffState();
}

void Outputs::shapeFilesMeshResolutionSpinBoxValueChanged()
{
    emit updateShapeState();
}

void Outputs::geospatialPDFFilesMeshResolutionSpinBoxValueChanged()
{
    emit updatePDFState();
}

void Outputs::mapVisualizationMeshResolutionSpinBoxValueChanged()
{
    emit updateMapVisualizationState();
}

