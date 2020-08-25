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
    connect(ui->meshChoiceCombo, SIGNAL(currentIndexChanged(int)),
        this, SLOT(updateMesh(int)));
    connect(ui->treeWidget, SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)),
        this, SLOT(updateStack(QTreeWidgetItem*, QTreeWidgetItem*)));
    connect(ui->downloadForecastButton, SIGNAL(clicked()), this, SLOT(downloadWx()));
    connect(ui->solveButton, SIGNAL(clicked()), this, SLOT(solve()));
    connect(ui->outputButton, SIGNAL(clicked()), this, SLOT(openOutputPath()));
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
    qItems.append(QPair<QString, QString>("north", "44.0312153"));
    qItems.append(QPair<QString, QString>("west", "-113.7496934"));
    qItems.append(QPair<QString, QString>("east", "-113.4634461"));
    qItems.append(QPair<QString, QString>("south", "43.7769564"));
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
    QString file = QFileDialog::getSaveFileName(this,
        tr("Open Elevation Input File"), "./",
        tr("netCDF file (*.nc)"));
    if(file == "") {
        return;
    }

    QString model = ui->wxComboBox->currentText();
    if(model.startsWith("UCAR")) {
        int rc = downloadUCAR(model, ui->wxDurSpinBox->value(), file);
        qDebug() << rc;
    }
}

void MainWindow::openElevation() {
  ui->elevEdit->clear();
  ui->elevEdit->setToolTip("");
  ui->vegCombo->setEnabled(true);
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
  // Check file via API
  QFileInfo info = QFileInfo(file);
  ui->elevEdit->setText(info.fileName());
  ui->elevEdit->setToolTip(info.absoluteFilePath());
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

void MainWindow::solve() {
    ui->solveButton->setDisabled(true);

    setProgress(-1);

    // Start with the initialization method, this governs how we create the set
    // of ninjas.
    NinjaH *ninja = 0;

    int nr = 1;

    // Start from the first set of inputs, then work our way down.
#ifdef NINJAFOAM
    ninja = NinjaCreateArmy(nr, ui->momentumRadio->isChecked(), nullptr);
#else
    ninja = NinjaCreateArmy(nr, nullptr);
#endif

#define check(x, fx) if(x!=0) { \
  printf("FAIL(%s): %d\n", fx, x); \
  NinjaDestroyArmy(ninja); \
  ui->solveButton->setEnabled(true); \
  return; \
}

    NinjaErr rc = 0;

    // Suface information, needed for all runs
    rc = NinjaSetElevationFile(ninja, 0, ui->elevEdit->text().toLocal8Bit());
    check(rc, "NinjaSetElevationFile");

    rc = NinjaSetNumVertLayers(ninja, 0, 20);
    check(rc, "NinjaSetNumVertLayers");

    if(ui->meshSpinBox->isEnabled()) {
        const char *u = 0;
        QString txt = ui->meshUnitCombo->currentText();
        if(txt == "meters") {
            u = "m";
        } else if(txt == "feet") {
            u = "ft";
        } else {
            assert(0);
        }
        rc = NinjaSetMeshResolution(ninja, 0, ui->meshSpinBox->value(), u);
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

    rc = NinjaSetAsciiOutFlag(ninja, 0, 1);
    check(rc, "NinjaSetAsciiOutFlag");

    ui->solveButton->setEnabled(false);
    QtConcurrent::run([=]() {
        NinjaErr rc = NinjaStartRuns(ninja, ui->availCoreSpinBox->value());
        //check(rc, "NinjaStartRuns");
        // This isn't working
        outputPath = NinjaGetOutputPath(ninja, 0);
        if(outputPath == "") {
            qDebug() << "BUG: output path should be set";
        }
        NinjaDestroyArmy(ninja);
        ui->solveButton->setEnabled(true);
        ui->outputButton->setEnabled(true);
    });
}

