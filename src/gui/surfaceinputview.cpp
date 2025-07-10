#include "surfaceinputview.h"
#include "ui_mainwindow.h"

SurfaceInputView::SurfaceInputView(Ui::MainWindow *ui,
                                   QWebEngineView *webView,
                                   SurfaceInput *surfaceInput,
                                   QObject* parent)
    : QObject(parent),
      ui(ui),
      webView(webView),
      surfaceInput(surfaceInput)
{
  connect(ui->boundingBoxNorthLineEdit, &QLineEdit::textChanged, this, &SurfaceInputView::boundingBoxLineEditsTextChanged);
  connect(ui->boundingBoxSouthLineEdit, &QLineEdit::textChanged, this, &SurfaceInputView::boundingBoxLineEditsTextChanged);
  connect(ui->boundingBoxEastLineEdit, &QLineEdit::textChanged, this, &SurfaceInputView::boundingBoxLineEditsTextChanged);
  connect(ui->boundingBoxWestLineEdit, &QLineEdit::textChanged, this, &SurfaceInputView::boundingBoxLineEditsTextChanged);
  connect(ui->elevationInputFileDownloadButton, &QPushButton::clicked, this, &SurfaceInputView::elevationInputFileDownloadButtonClicked);
  connect(ui->elevationInputFileOpenButton, &QPushButton::clicked, this, &SurfaceInputView::elevationInputFileOpenButtonClicked);
  connect(ui->elevationInputFileLineEdit, &QLineEdit::textChanged, this, &SurfaceInputView::elevationInputFileLineEditTextChanged);
  connect(ui->meshResolutionComboBox, &QComboBox::currentIndexChanged, this, &SurfaceInputView::meshResolutionComboBoxCurrentIndexChanged);
  connect(ui->meshResolutionUnitsComboBox, &QComboBox::currentIndexChanged, this, &SurfaceInputView::meshResolutionUnitsComboBoxCurrentIndexChanged);
  connect(ui->surfaceInputDownloadCancelButton, &QPushButton::clicked, this, &SurfaceInputView::surfaceInputDownloadCancelButtonClicked);
  connect(ui->surfaceInputDownloadButton, &QPushButton::clicked, this, &SurfaceInputView::surfaceInputDownloadButtonClicked);
  connect(ui->openElevationInputFileMenuAction, &QAction::triggered, this, &SurfaceInputView::elevationInputFileOpenButtonClicked);
  connect(ui->elevationInputTypePushButton, &QPushButton::clicked, this, &SurfaceInputView::elevationInputTypePushButtonClicked);
}


void SurfaceInputView::meshResolutionUnitsComboBoxCurrentIndexChanged(int index)
{
  switch(index)
  {
  case 0:
    ui->meshResolutionSpinBox->setValue(ui->meshResolutionSpinBox->value() * 0.3048);
    break;

  case 1:
    ui->meshResolutionSpinBox->setValue(ui->meshResolutionSpinBox->value() * 3.28084);
    break;
  }
}

void SurfaceInputView::elevationInputTypePushButtonClicked()
{
  if(ui->elevationInputTypePushButton->isChecked())
  {
    webView->page()->runJavaScript("startRectangleDrawing();");
  }
  else
  {
    webView->page()->runJavaScript("stopRectangleDrawing();");
  }
}

void SurfaceInputView::boundingBoxReceived(double north, double south, double east, double west)
{
  qDebug() << "MainWindow received bbox: "
           << north << south << east << west;

         // Block signals before updating to avoid triggering textChanged
  ui->boundingBoxNorthLineEdit->blockSignals(true);
  ui->boundingBoxEastLineEdit->blockSignals(true);
  ui->boundingBoxSouthLineEdit->blockSignals(true);
  ui->boundingBoxWestLineEdit->blockSignals(true);

  ui->boundingBoxNorthLineEdit->setText(QString::number(north));
  ui->boundingBoxEastLineEdit->setText(QString::number(east));
  ui->boundingBoxSouthLineEdit->setText(QString::number(south));
  ui->boundingBoxWestLineEdit->setText(QString::number(west));

         // Re-enable signals
  ui->boundingBoxNorthLineEdit->blockSignals(false);
  ui->boundingBoxEastLineEdit->blockSignals(false);
  ui->boundingBoxSouthLineEdit->blockSignals(false);
  ui->boundingBoxWestLineEdit->blockSignals(false);

  ui->elevationInputTypePushButton->setChecked(false);

  double pointRadius[3];
  surfaceInput->computePointRadius(north, east, south, west, pointRadius);

  ui->pointRadiusLatLineEdit->setText(QString::number(pointRadius[0]));
  ui->pointRadiusLonLineEdit->setText(QString::number(pointRadius[1]));
  ui->pointRadiusRadiusLineEdit->setText(QString::number(pointRadius[2]));
}

void SurfaceInputView::boundingBoxLineEditsTextChanged()
{
  QString north = ui->boundingBoxNorthLineEdit->text();
  QString east  = ui->boundingBoxEastLineEdit->text();
  QString south = ui->boundingBoxSouthLineEdit->text();
  QString west  = ui->boundingBoxWestLineEdit->text();

  if (!north.isEmpty() && !east.isEmpty() && !south.isEmpty() && !west.isEmpty())
  {
    QString js = QString("drawBoundingBox(%1, %2, %3, %4);").arg(north).arg(south) .arg(east).arg(west);
    webView->page()->runJavaScript(js);
  }
}

void SurfaceInputView::surfaceInputDownloadCancelButtonClicked()
{
  int currentIndex = ui->inputsStackedWidget->currentIndex();
  ui->inputsStackedWidget->setCurrentIndex(currentIndex-1);

  ui->boundingBoxNorthLineEdit->clear();
  ui->boundingBoxEastLineEdit->clear();
  ui->boundingBoxSouthLineEdit->clear();
  ui->boundingBoxWestLineEdit->clear();

  ui->pointRadiusLatLineEdit->clear();
  ui->pointRadiusLonLineEdit->clear();
  ui->pointRadiusRadiusLineEdit->clear();

  webView->page()->runJavaScript("stopRectangleDrawing();");
}

void SurfaceInputView::surfaceInputDownloadButtonClicked()
{
  double boundingBox[4];
  switch(ui->elevationInputTypeStackedWidget->currentIndex())
  {
  case 0:
    boundingBox[0] = ui->boundingBoxNorthLineEdit->text().toDouble();
    boundingBox[1] = ui->boundingBoxEastLineEdit->text().toDouble();
    boundingBox[2] = ui->boundingBoxSouthLineEdit->text().toDouble();
    boundingBox[3] = ui->boundingBoxWestLineEdit->text().toDouble();
    break;
  case 1:
    double centerLat = ui->pointRadiusLatLineEdit->text().toDouble();
    double centerLon = ui->pointRadiusLonLineEdit->text().toDouble();
    double radius = ui->pointRadiusRadiusLineEdit->text().toDouble();
    surfaceInput->computeBoundingBox(centerLat, centerLon, radius, boundingBox);
    break;
  }

  double resolution = 30;

  QString defaultName = "";
  QString downloadsPath = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
  QDir dir(downloadsPath);
  QString fullPath = dir.filePath(defaultName);
  QString demFilePath = QFileDialog::getSaveFileName(ui->centralwidget, "Save DEM File", fullPath, "TIF Files (*.tif)");
  if (demFilePath.isEmpty()) {
    return;
  }
  if (!demFilePath.endsWith(".tif", Qt::CaseInsensitive)) {
    demFilePath += ".tif";
  }
  currentDemFilePath = demFilePath;
  std::string demFile = demFilePath.toStdString();

  std::string fetchType;
  switch(ui->elevationFileTypeComboBox->currentIndex())
  {
  case 0:
    fetchType = "srtm";
    break;
  case 1:
    fetchType = "gmted";
    break;
  case 2:
    fetchType = "lcp";
    break;
  }

  int result = surfaceInput->fetchDEMFile(boundingBox, demFile, resolution, fetchType);

  ui->elevationInputFileLineEdit->setText(QFileInfo(demFilePath).fileName());
  int currentIndex = ui->inputsStackedWidget->currentIndex();
  ui->inputsStackedWidget->setCurrentIndex(currentIndex-1);
}

void SurfaceInputView::elevationInputFileDownloadButtonClicked()
{
  int currentIndex = ui->inputsStackedWidget->currentIndex();
  ui->inputsStackedWidget->setCurrentIndex(currentIndex+1);
}

void SurfaceInputView::meshResolutionComboBoxCurrentIndexChanged(int index)
{
  if (index == 3) {
    ui->meshResolutionSpinBox->setEnabled(true);
  } else {
    ui->meshResolutionSpinBox->setEnabled(false);
  }
  ui->meshResolutionSpinBox->setValue(surfaceInput->computeMeshResolution(ui->meshResolutionComboBox->currentIndex(), ui->momentumSolverCheckBox->isChecked()));
}

void SurfaceInputView::elevationInputFileLineEditTextChanged(const QString &arg1)
{
  surfaceInput->computeDEMFile(currentDemFilePath);
  surfaceInput->computeMeshResolution(ui->meshResolutionComboBox->currentIndex(), ui->momentumSolverCheckBox->isChecked());

  ui->meshResolutionSpinBox->setValue(surfaceInput->computeMeshResolution(ui->meshResolutionComboBox->currentIndex(), ui->momentumSolverCheckBox->isChecked()));
  emit requestRefresh();
}

void SurfaceInputView::elevationInputFileOpenButtonClicked()
{
  QString directoryPath;
  if(!currentDemFilePath.isEmpty())
  {
    directoryPath = currentDemFilePath;
  }
  else {
    directoryPath = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
  }
  QString demFilePath = QFileDialog::getOpenFileName(ui->centralwidget, "Select a file", directoryPath, "(*.tif);;All Files (*)");

  if (demFilePath.isEmpty()) {
    if (!currentDemFilePath.isEmpty()) {
      ui->elevationInputFileLineEdit->setText(QFileInfo(currentDemFilePath).fileName());
      ui->elevationInputFileLineEdit->setToolTip(currentDemFilePath);
    }
    return;
  }

  currentDemFilePath = demFilePath;
  ui->elevationInputFileLineEdit->setText(QFileInfo(demFilePath).fileName());
  ui->elevationInputFileLineEdit->setToolTip(demFilePath);
}

