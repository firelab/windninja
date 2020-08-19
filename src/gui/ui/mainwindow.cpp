#include "mainwindow.h"
#include "ui_mainwindow.h"

static void panic(std::string msg) throw() {
  throw std::runtime_error(msg);
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow) {
    ui->setupUi(this);

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
    connect(ui->solveButton, SIGNAL(clicked()), this, SLOT(solve()));
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

void MainWindow::openElevation() {
  ui->elevEdit->clear();
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
    rc = NinjaSetElevationFile(ninja, 0, ui->elevEdit->text().toLocal8Bit());
    check(rc, "NinjaSetElevationFile");
    rc = NinjaSetNumVertLayers(ninja, 0, 20);
    rc = NinjaSetInitializationMethod(ninja, 0, "domain_average");
    check(rc, "NinjaSetInitializationMethod");
    rc = NinjaSetInputSpeed(ninja, 0, ui->speedSpinBox->value(), "mph");
    check(rc, "NinjaSetInputSpeed");
    rc = NinjaSetInputDirection(ninja, 0, ui->dirSpinBox->value());
    check(rc, "NinjaSetInputDirection");
    rc = NinjaSetInputWindHeight(ninja, 0, ui->inHeightSpinBox->value(), "m");
    check(rc, "NinjaSetInputWindHeight");
    rc = NinjaSetOutputWindHeight(ninja, 0, 10.0, "m");
    check(rc, "NinjaSetOutputWindHeight");
    rc = NinjaSetOutputSpeedUnits(ninja, 0, "mph");
    check(rc, "NinjaSetOutputSpeedUnits");
    rc = NinjaSetDiurnalWinds(ninja, 0, 0);
    check(rc, "NinjaSetDiurnalWinds");
    rc = NinjaSetUniVegetation( ninja, 0, "grass");
    check(rc, "NinjaSetUniVegetation");
    rc = NinjaSetMeshResolutionChoice(ninja, 0, "coarse");
    check(rc, "NinjaSetMeshResolutionChoice");
    rc = NinjaSetAsciiOutFlag(ninja, 0, 1);
    check(rc, "NinjaSetAsciiOutFlag");
    rc = NinjaStartRuns(ninja, ui->availCoreSpinBox->value());
    check(rc, "NinjaStartRuns");
    NinjaDestroyArmy(ninja);
    ui->solveButton->setEnabled(true);
}

