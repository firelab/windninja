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
    ui->fireBehaviorMeshResolutionComboBox->setItemData(0, "m");
    ui->fireBehaviorMeshResolutionComboBox->setItemData(1, "ft");
    ui->shapeFilesMeshResolutionComboBox->setItemData(0, "m");
    ui->shapeFilesMeshResolutionComboBox->setItemData(1, "ft");
    ui->geospatialPDFFilesMeshResolutionComboBox->setItemData(0, "m");
    ui->geospatialPDFFilesMeshResolutionComboBox->setItemData(1, "ft");
    ui->alternativeColorSchemeComboBox->setItemData(0, "default");
    ui->alternativeColorSchemeComboBox->setItemData(1, "ROPGW");
    ui->alternativeColorSchemeComboBox->setItemData(2, "oranges");
    ui->alternativeColorSchemeComboBox->setItemData(3, "blues");
    ui->alternativeColorSchemeComboBox->setItemData(4, "pinks");
    ui->alternativeColorSchemeComboBox->setItemData(5, "greens");
    ui->alternativeColorSchemeComboBox->setItemData(6, "magic_beans");
    ui->alternativeColorSchemeComboBox->setItemData(7, "pink_to_green");
    ui->legendComboBox->setItemData(0, "equal_interval");
    ui->legendComboBox->setItemData(1, "equal_color");

    connect(ui->googleEarthGroupBox, &QGroupBox::toggled, this, &Outputs::googleEarthGroupBoxToggled);
    connect(ui->fireBehaviorGroupBox, &QGroupBox::toggled, this, &Outputs::fireBehaviorGroupBoxToggled);
    connect(ui->shapeFilesGroupBox, &QGroupBox::toggled, this, &Outputs::shapeFilesGroupBoxToggled);
    connect(ui->geospatialPDFFilesGroupBox, &QGroupBox::toggled, this, &Outputs::geospatialPDFFilesGroupBoxToggled);
    connect(ui->VTKFilesCheckBox, &QCheckBox::clicked, this, &Outputs::VTKFilesCheckBoxClicked);
    connect(ui->googleEarthMeshResolutionGroupBox, &QGroupBox::toggled, this, &Outputs::googleEarthMeshResolutionGroupBoxToggled);
    connect(ui->fireBehaviorMeshResolutionGroupBox, &QGroupBox::toggled, this, &Outputs::fireBehaviorMeshResolutionGroupBoxToggled);
    connect(ui->shapeFilesMeshResolutionGroupBox, &QGroupBox::toggled, this, &Outputs::shapeFilesMeshResolutionGroupBoxToggled);
    connect(ui->geospatialPDFFilesMeshResolutionGroupBox, &QGroupBox::toggled, this, &Outputs::geospatialPDFFilesMeshResolutionGroupBoxToggled);
    connect(this, &Outputs::updateGoogleState, &AppState::instance(), &AppState::updateGoogleEarthOutputState);
    connect(this, &Outputs::updateFireBehaviorState, &AppState::instance(), &AppState::updateFireBehaviorOutputState);
    connect(this, &Outputs::updateShapeState, &AppState::instance(), &AppState::updateShapeFilesOutputState);
    connect(this, &Outputs::updatePDFState, &AppState::instance(), &AppState::updateGeoSpatialPDFFilesOutputState);
    connect(this, &Outputs::updateVTKState, &AppState::instance(), &AppState::updateVTKFilesOutputState);
}

void Outputs::googleEarthGroupBoxToggled(bool checked)
{
    AppState& state = AppState::instance();
    state.isGoogleEarthToggled = checked;
    emit updateGoogleState();
}

void Outputs::fireBehaviorGroupBoxToggled(bool checked)
{
    AppState& state = AppState::instance();
    state.isFireBehaviorToggled = checked;
    emit updateFireBehaviorState();
}

void Outputs::shapeFilesGroupBoxToggled(bool checked)
{
    AppState& state = AppState::instance();
    state.isShapeFilesToggled = checked;
    emit updateShapeState();
}

void Outputs::geospatialPDFFilesGroupBoxToggled(bool checked)
{
    AppState& state = AppState::instance();
    state.isGeoSpatialPDFFilesToggled = checked;
    emit updatePDFState();
}

void Outputs::VTKFilesCheckBoxClicked(bool checked)
{
    AppState& state = AppState::instance();
    state.isVTKFilesToggled = checked;
    emit updateVTKState();
}

void Outputs::googleEarthMeshResolutionGroupBoxToggled(bool checked)
{
    ui->googleEarthMeshResolutionSpinBox->setEnabled(!checked);
    ui->googleEarthMeshResolutionComboBox->setEnabled(!checked);
}

void Outputs::fireBehaviorMeshResolutionGroupBoxToggled(bool checked)
{
    ui->fireBehaviorMeshResolutionSpinBox->setEnabled(!checked);
    ui->fireBehaviorMeshResolutionComboBox->setEnabled(!checked);
}

void Outputs::shapeFilesMeshResolutionGroupBoxToggled(bool checked)
{
    ui->shapeFilesMeshResolutionSpinBox->setEnabled(!checked);
    ui->shapeFilesMeshResolutionComboBox->setEnabled(!checked);
}

void Outputs::geospatialPDFFilesMeshResolutionGroupBoxToggled(bool checked)
{
    ui->geospatialPDFFilesMeshResolutionSpinBox->setEnabled(!checked);
    ui->geospatialPDFFilesMeshResolutionComboBox->setEnabled(!checked);
}
