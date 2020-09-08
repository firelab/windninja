#include "mainwindow.h"
#include "ui_mainwindow.h"

static void panic(std::string msg) throw() {
  throw std::runtime_error(msg);
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow) {
    ui->setupUi(this);

    assert(NinjaInit() == 0);

    init();
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::init() {
    GDALAllRegister();
    setIcons();
    setConnections();
    OGRFormats();

    // Set the number of cores available.  We only have to do this once
    int cores = QThread::idealThreadCount();
    ui->availCoreLabel->setText("Available Processors: " + QString::number(cores));
    ui->availCoreSpinBox->setMaximum(cores);

    ui->timeEdit->setDateTime(QDateTime::currentDateTime());

    // Set up the wx model keys.  There is no way to programmatically get the
    // UCAR models, so these are hard coded.  The NOMADS keys are also hard
    // coded, even though there is a way to get them programmatically.  We
    // should add some sort of check to make sure the lists are in sync.

    // UCAR models
    ui->wxComboBox->addItem("UCAR-NAM-CONUS-12-KM");
    ui->wxComboBox->addItem("UCAR-NDFD-CONUS-2.5-KM");
    ui->wxComboBox->addItem("UCAR-RAP-CONUS-13-KM");
    ui->wxComboBox->addItem("UCAR-GFS-GLOBAL-1.0-DEG");

    // NOMADS models
#if defined(NOMADS_GFS_0P5DEG)
    ui->wxComboBox->addItem("NOMADS-GFS-GLOBAL-0.5-DEG");
#elif defined(NOMADS_GFS_1P0DEG)
    ui->wxComboBox->addItem("NOMADS-GFS-GLOBAL-1.0-DEG");
#else
    ui->wxComboBox->addItem("NOMADS-GFS-GLOBAL-0.25-DEG");
#endif
    ui->wxComboBox->addItem("NOMADS-HIRES-ARW-ALASKA-5-KM");
    ui->wxComboBox->addItem("NOMADS-HIRES-NMM-ALASKA-5-KM");
    ui->wxComboBox->addItem("NOMADS-HIRES-ARW-CONUS-5-KM");
    ui->wxComboBox->addItem("NOMADS-HIRES-NMM-CONUS-5-KM");
    ui->wxComboBox->addItem("NOMADS-NAM-ALASKA-11.25-KM");
    ui->wxComboBox->addItem("NOMADS-NAM-CONUS-12-KM");
    ui->wxComboBox->addItem("NOMADS-NAM-NORTH-AMERICA-32-KM");
    ui->wxComboBox->addItem("NOMADS-NAM-NEST-ALASKA-3-KM");
    ui->wxComboBox->addItem("NOMADS-NAM-NEST-CONUS-3-KM");
    ui->wxComboBox->addItem("NOMADS-HRRR-ALASKA-3-KM");
    ui->wxComboBox->addItem("NOMADS-HRRR-CONUS-3-KM");
    ui->wxComboBox->addItem("NOMADS-HRRR-CONUS-SUBHOURLY-3-KM");
    ui->wxComboBox->addItem("NOMADS-HRRR-ALASKA-SUBHOURLY-3-KM");
    ui->wxComboBox->addItem("NOMADS-RAP-CONUS-13-KM");
    ui->wxComboBox->addItem("NOMADS-RAP-NORTH-AMERICA-32-KM");

    // Use QT time zones to fill in the dialog.
    // XXX(kyle): this won't match boost::timezone
    QList<QByteArray> ids = QTimeZone::availableTimeZoneIds();
    for(int i = 0; i < ids.size(); i++) {
        ui->tzCombo->addItem(ids[i]);
    }

    // TODO(kyle): remove
    ui->tzCombo->setCurrentText("America/Boise");

    // Set up the progress bar in the status bar (insertPermanentWidget
    // re-parents progress
    progressLabel = new QLabel(this);
    statusBar()->addPermanentWidget(progressLabel, 1);
    progress = new QProgressBar(this);
    progress->setRange(0, 100);
    statusBar()->addPermanentWidget(progress, 1);
}

void MainWindow::setIcons() {
    ui->openElevButton->setIcon(QIcon(":icons/open.svg"));
    ui->downloadElevButton->setIcon(QIcon(":icons/save-as.svg"));

    ui->addDomainRunButton->setIcon(QIcon(":icons/add.svg"));

    ui->downloadForecastButton->setIcon(QIcon(":icons/save-as.svg"));
    ui->openForecastButton->setIcon(QIcon(":icons/open.svg"));
}

void MainWindow::setConnections() {
    connect(ui->openElevButton, SIGNAL(clicked()), this, SLOT(openElevation()));
    connect(ui->downloadElevButton, SIGNAL(clicked()), this, SLOT(downloadElev()));
    connect(ui->meshChoiceCombo, SIGNAL(currentIndexChanged(int)),
        this, SLOT(updateMesh(int)));
    connect(ui->treeWidget, SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)),
        this, SLOT(updateStack(QTreeWidgetItem*, QTreeWidgetItem*)));
    connect(ui->initCombo, SIGNAL(currentIndexChanged(int)),
        ui->initStack, SLOT(setCurrentIndex(int)));
    connect(ui->downloadForecastButton, SIGNAL(clicked()), this, SLOT(downloadWx()));
    connect(ui->solveButton, SIGNAL(clicked()), this, SLOT(solve()));
    connect(ui->outputButton, SIGNAL(clicked()), this, SLOT(openOutputPath()));
    connect(ui->openForecastButton, SIGNAL(clicked()), this, SLOT(openForecast()));
}

void MainWindow::OGRFormats() {
    GDALDriverH hDrv = 0;
    const char *pszV = 0;
    const char *pszC = 0;
    QString longname, shortname, ext;
    // Requires GDAL 2.0+
    for(int i = 0; i < GDALGetDriverCount(); i++) {
        hDrv = GDALGetDriver(i);
        assert(hDrv);
        /*
        ** TODO(kyle): We need to define a short list of 'normal' vector
        ** drivers, with an option to show all writable vector formats.  This
        ** might be handled by newer versions of GDALOutput() stuff.  There may
        ** be more restrictions that need to be in place here as well (field
        ** types, etc.
        */
        pszV = GDALGetMetadataItem(hDrv, GDAL_DCAP_VECTOR, NULL);
        pszC = GDALGetMetadataItem(hDrv, GDAL_DCAP_CREATE, NULL);
        if(pszV && CPLTestBoolean(pszV) && pszC && CPLTestBoolean(pszC)) {
            longname = QString(GDALGetDriverLongName(hDrv));
            shortname = QString(GDALGetDriverShortName(hDrv));
            ext = QString(GDALGetMetadataItem(hDrv, "", GDAL_DMD_EXTENSION));
            //qDebug() << longname << "," << shortname << "," << ext;
            ui->ogrFormatCombo->addItem(longname, shortname);
        }
    }
}

int MainWindow::downloadUCAR(QString model, int hours, QString filename) {
    QUrl url;
    url.setScheme("https");
    url.setHost("thredds.ucar.edu");
    QMap<QString, QString>paths;
    paths["UCAR-GFS-GLOBAL-1.0-DEG"] = "/thredds/ncss/grib/NCEP/GFS/Global_0p5deg/Best";
    paths["UCAR-NAM-CONUS-12-KM"] = "/thredds/ncss/grib/NCEP/NAM/CONUS_12km/best";
    paths["UCAR-NAM-ALASKA-11-KM"] = "/thredds/ncss/grib/NCEP/NAM/Alaska_11km/best";
    paths["UCAR-NDFD-CONUS-2.5-KM"] = "/thredds/ncss/grib/NCEP/NDFD/NWS/CONUS/NOAAPORT/Best/LambertConformal_1377X2145-38p22N-95p43W";
    paths["UCAR-RAP-CONUS-13-KM"] = "/thredds/ncss/grib/NCEP/RAP/CONUS_13km/best";

    QList<QPair<QString, QString>> qItems;
    // TODO(kyle): need an API call for domain extents.
    qItems.append(QPair<QString, QString>("north", QString::number(elevInfo.maxY)));
    qItems.append(QPair<QString, QString>("west", QString::number(elevInfo.minX)));
    qItems.append(QPair<QString, QString>("east", QString::number(elevInfo.maxX)));
    qItems.append(QPair<QString, QString>("south", QString::number(elevInfo.minY)));
    qItems.append(QPair<QString, QString>("time_start", "present"));
    qItems.append(QPair<QString, QString>("time_duration", "PT" +
          QString::number(ui->wxDurSpinBox->value()) + "H"));
    qItems.append(QPair<QString, QString>("accept", "netcdf"));

    QStringList vars;
    if(model.contains("NDFD")) {
        vars.append("Maximum_temperature_height_above_ground_12_Hour_Maximum");
        vars.append("Minimum_temperature_height_above_ground_12_Hour_Minimum");
        vars.append("Total_cloud_cover_entire_atmosphere_single_layer_layer");
        vars.append("Wind_direction_from_which_blowing_height_above_ground");
        vars.append("Wind_speed_height_above_ground");
    } else {
        vars.append("v-component_of_wind_height_above_ground");
        vars.append("u-component_of_wind_height_above_ground");
        vars.append("Temperature_height_above_ground");
    }
    if(model.contains("GFS")) {
        vars.append("Total_cloud_cover_convective_cloud");
    } else {
        vars.append("Total_cloud_cover_entire_atmosphere_single_layer");
    }

    qItems.append(QPair<QString, QString>("var", vars.join(",")));

    QUrlQuery query;
    query.setQueryItems(qItems);

    url.setPath(paths[model]);
    url.setQuery(query.toString());

    /*
    QFile fout(filename);
    if(!fout.open(QIODevice::WriteOnly)) {
        qDebug() << "failed to open " << filename;
        return 1;
    }

    QNetworkRequest req;
    qDebug() << url;
    req.setUrl(url);

    QNetworkAccessManager manager;
    QNetworkReply *reply = manager.get(req);
    qDebug() << reply->isRunning();
    qDebug() << reply->isFinished();

    int i = 0;
    while(reply->isRunning()) {
        if(reply->error()) {
            qDebug() << reply->error();
            return 1;
        }
        setProgress(i, "downloading wx file...", -1);
        i++;
        qDebug() << "downloading...";
        QThread::sleep(1);
    }

    fout.write(reply->readAll());
    fout.flush();
    fout.close();
    delete reply;
    */

    QString cmd = "curl -L -o " + filename + " \"" + url.toString() + "\"";
    qDebug() << cmd;
    system(cmd.toLocal8Bit());

    return 0;
}

void MainWindow::downloadWx() {
    ui->forecastLineEdit->clear();
    ui->forecastLineEdit->setToolTip("");

    QString file = QFileDialog::getSaveFileName(this,
        tr("Save Weather Forecast File"), "./",
        tr("netCDF file (*.nc)"));
    if(file == "") {
        return;
    }

    QString model = ui->wxComboBox->currentText();
    if(model.startsWith("UCAR")) {
        int rc = downloadUCAR(model, ui->wxDurSpinBox->value(), file);
        if(rc == 0) {
            QFileInfo info(file);
            ui->forecastLineEdit->setText(info.fileName());
            ui->forecastLineEdit->setToolTip(info.absoluteFilePath());
            forecastPath = info.absoluteFilePath();
        }
    }
}

void MainWindow::openForecast() {
  ui->forecastLineEdit->clear();
  ui->forecastLineEdit->setToolTip("");
  forecastPath = "";
  QString file = QFileDialog::getOpenFileName(this,
   tr("Open Forecast File"),
   "./",
   tr("Forecast Files (*.nc *.zip)"));
  if(file == "") {
    return;
  }
  QFileInfo info = QFileInfo(file);
  ui->forecastLineEdit->setText(info.fileName());
  forecastPath = info.absoluteFilePath();
  ui->forecastLineEdit->setToolTip(elevPath);

}

void MainWindow::downloadElev() {
  qDebug() << "download elevation...";
}

void MainWindow::openElevation() {
  ui->elevEdit->clear();
  ui->elevEdit->setToolTip("");
  ui->vegCombo->setEnabled(true);
  elevPath = "";
  QString file = QFileDialog::getOpenFileName(this,
   tr("Open Elevation Input File"),
   "./",
   tr("Elevation Input Files (*.asc *.lcp *.tif *.img)"));
  if(file == "") {
    return;
  }
  /* TODO(kyle): factor out, needs to happen in windninja */
  GDALDatasetH hDS = GDALOpen(file.toLocal8Bit().data(), GA_ReadOnly);
  if(hDS == nullptr) {
      panic("failed to open " + file.toStdString());
      return;
  }
  GDALDriverH hDrv = GDALGetDatasetDriver(hDS);
  if(hDrv == nullptr) {
      panic("failed to get driver for: " + file.toStdString());
      GDALClose(hDS);
      return;
  }
  QString fmt = GDALGetDriverShortName(hDrv);
  if(fmt == "") {
    // TODO(kyle): relay info to user about invalid input
    return;
  }
  if(fmt != "GTiff" && fmt != "HFA" && fmt != "LCP" && fmt != "VRT") {
    panic("invalid format: " + fmt.toStdString());
  }
  if(fmt == "LCP") {
    ui->vegCombo->setDisabled(true);
  }
  // Inspect the file to get some metadata.
  OGRSpatialReferenceH hFrom = OSRNewSpatialReference(GDALGetProjectionRef(hDS));
  OGRSpatialReferenceH hTo = OSRNewSpatialReference(nullptr);
  OSRImportFromEPSG(hTo, 4326);
  if(!hFrom || !hTo) {
    panic("failed to create spatial reference");
  }
#ifdef GDAL_COMPUTE_VERSION
#if GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(3,0,0)
  OSRSetAxisMappingStrategy(hTo, OAMS_TRADITIONAL_GIS_ORDER);
#endif /* GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(3,0,0) */
#endif /* GDAL_COMPUTE_VERSION */

  OGRCoordinateTransformationH hCT = OCTNewCoordinateTransformation(hFrom, hTo);
  // Start at the origin, grab the x and y coordinates
  double adfGT[6];
  GDALGetGeoTransform(hDS, adfGT);
  int nX = GDALGetRasterXSize(hDS);
  int nY = GDALGetRasterYSize(hDS);
  double *x = (double*)malloc(4 * sizeof(double));
  double *y = (double*)malloc(4 * sizeof(double));
  if(!x || !y) {
    panic("failed a tiny malloc");
  }
  // upper left
  x[0] = adfGT[0];
  y[0] = adfGT[3];
  // upper right
  x[1] = adfGT[0] + (adfGT[1] * nX);
  y[1] = adfGT[3];
  // lower right
  x[2] = adfGT[0] + (adfGT[1] * nX);
  y[2] = adfGT[3] + (adfGT[5] * nY);
  // lower left
  x[3] = adfGT[0];
  y[3] = adfGT[3] + (adfGT[5] * nY);

  int rc = OCTTransform(hCT, 4, x, y, nullptr);
  if(rc != 1) {
    panic("failed to transform point in openElevation");
  }

  elevInfo.minX = 361.0;
  elevInfo.maxX = -361.0;
  elevInfo.minY = 91.0;
  elevInfo.maxY = -91.0;

  for(int i = 0; i < 4; i++) {
    if(x[i] < elevInfo.minX) {
      elevInfo.minX = x[i];
    }
    if(x[i] > elevInfo.maxX) {
      elevInfo.maxX = x[i];
    }
    if(y[i] < elevInfo.minY) {
      elevInfo.minY = y[i];
    }
    if(y[i] > elevInfo.maxY) {
      elevInfo.maxY = y[i];
    }
  }

  qDebug() << "minX: " <<  elevInfo.minX;
  qDebug() << "maxX: " <<  elevInfo.maxX;
  qDebug() << "minY: " <<  elevInfo.minY;
  qDebug() << "maxY: " <<  elevInfo.maxY;

  free((void*)x);
  free((void*)y);

  // Set the current domain information.

  // Check file via API
  QFileInfo info = QFileInfo(file);
  ui->elevEdit->setText(info.fileName());
  elevPath = info.absoluteFilePath();
  ui->elevEdit->setToolTip(elevPath);
}

void MainWindow::updateMesh(int index) {
    ui->meshSpinBox->setEnabled(index == 3);
    ui->meshUnitCombo->setEnabled(index == 3);
}

void MainWindow::updateStack(QTreeWidgetItem *current, QTreeWidgetItem *previous) {
    QString name = current->text(0);
    if(name == "Solver") {
        ui->stackedWidget->setCurrentIndex(0);
    } else if(name == "Site Information") {
        ui->stackedWidget->setCurrentIndex(1);
    } else if(name == "Initialization") {
        ui->stackedWidget->setCurrentIndex(2);
    } else if(name == "Output") {
        ui->stackedWidget->setCurrentIndex(3);
    } else if(name == "Run") {
        ui->stackedWidget->setCurrentIndex(4);
    } else {
        assert(0);
    }
}

void MainWindow::setProgress(int done, QString text, int timeout) {
    if(done < 0) {
        progress->reset();
        return;
    }
    progress->setValue(done);
    progressLabel->setText(text);
    // TODO(kyle): async clear the text
    if(timeout > 0) {
        QtConcurrent::run([=]() {
            QThread::sleep(timeout);
            progressLabel->setText("");
        });
    }
}

static const char * unitKey(QString txt) {
    txt = txt.toLower();
    if(txt == "celsius") {
        return "C";
    } else if(txt == "fahrenheit") {
      return "F";
    } else if(txt == "feet") {
        return "ft";
    } else if(txt == "meters") {
        return "m";
    }
    qDebug() << txt;
    assert(0);
}

void MainWindow::openOutputPath() {
    if(outputPath == "") {
        return;
    }
    QDesktopServices::openUrl( QUrl ( "file:///" + outputPath, QUrl::TolerantMode ) );
}

#define check(x, fx) if(x!=0) { \
  printf("FAIL(%s): %d\n", fx, x); \
  NinjaDestroyArmy(ninja); \
  ui->solveButton->setEnabled(true); \
  return; \
}

void MainWindow::solve() {
    ui->solveButton->setDisabled(true);

    setProgress(-1);

    // Start with the initialization method, this governs how we create the set
    // of ninjas.
    NinjaH *ninja = 0;

    NinjaErr rc = 0;

    int nr = 1;

    // Start with the first run, fill in the common data values

    // Start with the init method that is simplest
    if(ui->initCombo->currentIndex() == 2) {
#ifdef NINJAFOAM
        ninja = NinjaCreateArmy(1, ui->momentumRadio->isChecked(), nullptr);
#else
        ninja = NinjaCreateArmy(1, nullptr);
#endif
      qDebug() << elevPath;
      rc = NinjaSetElevationFile(ninja, 0, elevPath.toLocal8Bit());
      check(rc, "NinjaSetElevationFile");

      rc = NinjaSetNumVertLayers(ninja, 0, 20);
      check(rc, "NinjaSetNumVertLayers");

      if(ui->meshSpinBox->isEnabled()) {
          rc = NinjaSetMeshResolution(ninja, 0, ui->meshSpinBox->value(),
                 unitKey(ui->meshUnitCombo->currentText()));
      } else {
          QString mc = ui->meshChoiceCombo->currentText();
          rc = NinjaSetMeshResolutionChoice(ninja, 0, mc.toLower().toLocal8Bit());
      }
      check(rc, "NinjaSetMeshResolution");

      if(ui->vegCombo->isEnabled()) {
          rc = NinjaSetUniVegetation(ninja, 0,
              ui->vegCombo->currentText().toLower().toLocal8Bit());
          check(rc, "NinjaSetUniVegetation");
      }

      rc = NinjaSetInitializationMethod(ninja, 0, "wxmodel");
      check(rc, "NinjaSetInitializationMethod");

      if(ui->diurnalCheck->isChecked()) {
          qDebug() << "setting diurnal...";
          rc = NinjaSetDiurnalWinds(ninja, 0, 1);
          check(rc, "NinjaSetDiurnalWinds");
      }
      rc = NinjaSetOutputWindHeight(ninja, 0, 10.0, "m");
      check(rc, "NinjaSetOutputWindHeight");

      rc = NinjaSetOutputSpeedUnits(ninja, 0, "mph");
      check(rc, "NinjaSetOutputSpeedUnits");

      rc = NinjaSetAsciiOutFlag(ninja, 0, 1);
      check(rc, "NinjaSetAsciiOutFlag");
      nr = NinjaMakeArmy(ninja, forecastPath.toLocal8Bit(),
          ui->tzCombo->currentText().toLocal8Bit(), 0);
      if(nr <= 0) {
          qDebug() << "INVALID FORECAST";
          return;
      }
      // Set the rest of the values?
      for(int i = 0; i < nr; i++) {
          rc = NinjaSetElevationFile(ninja, i, elevPath.toLocal8Bit());
          check(rc, "NinjaSetElevationFile");

          rc = NinjaSetNumVertLayers(ninja, i, 20);
          check(rc, "NinjaSetNumVertLayers");

          if(ui->meshSpinBox->isEnabled()) {
              rc = NinjaSetMeshResolution(ninja, i, ui->meshSpinBox->value(),
                     unitKey(ui->meshUnitCombo->currentText()));
          } else {
              QString mc = ui->meshChoiceCombo->currentText();
              rc = NinjaSetMeshResolutionChoice(ninja, i, mc.toLower().toLocal8Bit());
          }
          check(rc, "NinjaSetMeshResolution*");

          if(ui->vegCombo->isEnabled()) {
              rc = NinjaSetUniVegetation(ninja, i,
                  ui->vegCombo->currentText().toLower().toLocal8Bit());
              check(rc, "NinjaSetUniVegetation");
          }

          rc = NinjaSetInitializationMethod(ninja, i, "wxmodel");
          check(rc, "NinjaSetInitializationMethod");

          if(ui->diurnalCheck->isChecked()) {
              qDebug() << "setting diurnal...";
              rc = NinjaSetDiurnalWinds(ninja, i, 1);
              check(rc, "NinjaSetDiurnalWinds");
          }
          rc = NinjaSetOutputWindHeight(ninja, i, 10.0, "m");
          check(rc, "NinjaSetOutputWindHeight");

          rc = NinjaSetOutputSpeedUnits(ninja, i, "mph");
          check(rc, "NinjaSetOutputSpeedUnits");

          rc = NinjaSetAsciiOutFlag(ninja, i, 1);
          check(rc, "NinjaSetAsciiOutFlag");

      }
      ui->solveButton->setEnabled(false);
      NinjaErr rc = NinjaStartRuns(ninja, ui->availCoreSpinBox->value());
      //check(rc, "NinjaStartRuns");
      const char *p = NinjaGetOutputPath(ninja, 0);
      outputPath = QString(p);
      free((void*)p);

      assert(outputPath != "");
      NinjaDestroyArmy(ninja);
      ui->solveButton->setEnabled(true);
      ui->outputButton->setEnabled(true);
      return;
    }

#ifdef NINJAFOAM
    ninja = NinjaCreateArmy(nr, ui->momentumRadio->isChecked(), nullptr);
#else
    ninja = NinjaCreateArmy(nr, nullptr);
#endif
    // Suface information, needed for all runs
    rc = NinjaSetElevationFile(ninja, 0, elevPath.toLocal8Bit());
    check(rc, "NinjaSetElevationFile");

    rc = NinjaSetNumVertLayers(ninja, 0, 20);
    check(rc, "NinjaSetNumVertLayers");

    if(ui->meshSpinBox->isEnabled()) {
        rc = NinjaSetMeshResolution(ninja, 0, ui->meshSpinBox->value(),
               unitKey(ui->meshUnitCombo->currentText()));
    } else {
        QString mc = ui->meshChoiceCombo->currentText();
        rc = NinjaSetMeshResolutionChoice(ninja, 0, mc.toLower().toLocal8Bit());
    }
    check(rc, "NinjaSetMeshResolution*");

    if(ui->vegCombo->isEnabled()) {
        rc = NinjaSetUniVegetation(ninja, 0,
            ui->vegCombo->currentText().toLower().toLocal8Bit());
        check(rc, "NinjaSetUniVegetation");
    }

    rc = NinjaSetInitializationMethod(ninja, 0, "domain_average");
    check(rc, "NinjaSetInitializationMethod");

    // domain average information
    // input wind height and units
    rc = NinjaSetInputWindHeight(ninja, 0, ui->inHeightSpinBox->value(),
        unitKey(ui->inHeightUnitCombo->currentText()));
    check(rc, "NinjaSetInputWindHeight");

    // input speed and direction
    rc = NinjaSetInputSpeed(ninja, 0, ui->speedSpinBox->value(),
        ui->inSpeedUnitCombo->currentText().toLocal8Bit());
    check(rc, "NinjaSetInputSpeed");

    rc = NinjaSetInputDirection(ninja, 0, ui->dirSpinBox->value());
    check(rc, "NinjaSetInputDirection");

    // diurnal info, if needed
    if(ui->diurnalCheck->isChecked()) {
        qDebug() << "setting diurnal...";
        rc = NinjaSetDiurnalWinds(ninja, 0, 1);
        check(rc, "NinjaSetDiurnalWinds");
        rc = NinjaSetUniAirTemp(ninja, 0, ui->tempSpinBox->value(),
            unitKey(ui->tempCombo->currentText()));
        check(rc, "NinjaSetUniAirTemp");
        rc = NinjaSetUniCloudCover(ninja, 0, ui->cloudSpinBox->value(), "percent");
        check(rc, "NinjaSetUniCloudCover");
        QDate d = ui->timeEdit->date();
        QTime t = ui->timeEdit->time();
        rc = NinjaSetDateTime(ninja, 0, d.year(), d.month(), d.day(),
            t.hour(), t.minute(), t.second(),
            ui->tzCombo->currentText().toLocal8Bit());
    }

    rc = NinjaSetOutputWindHeight(ninja, 0, 10.0, "m");
    check(rc, "NinjaSetOutputWindHeight");

    rc = NinjaSetOutputSpeedUnits(ninja, 0, "mph");
    check(rc, "NinjaSetOutputSpeedUnits");

    if(ui->fbOutCheckbox->isChecked()) {
      rc = NinjaSetAsciiOutFlag(ninja, 0, 1);
      check(rc, "NinjaSetAsciiOutFlag");
    }
    if(ui->googleOutCheckbox->isChecked()) {
      rc = NinjaSetGoogOutFlag(ninja, 0, 1);
      check(rc, "NinjaSetGoogOutFlag");
    }
    if(ui->shapeOutCheckbox->isChecked()) {
      rc = NinjaSetShpOutFlag(ninja, 0, 1);
      check(rc, "NinjaSetShpOutFlag");
    }

    ui->solveButton->setEnabled(false);
    rc = NinjaStartRuns(ninja, ui->availCoreSpinBox->value());
    //check(rc, "NinjaStartRuns");
    const char *p = NinjaGetOutputPath(ninja, 0);
    outputPath = QString(p);
    free((void*)p);

    assert(outputPath != "");
    NinjaDestroyArmy(ninja);
    ui->solveButton->setEnabled(true);
    ui->outputButton->setEnabled(true);
}

