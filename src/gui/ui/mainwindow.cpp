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


















