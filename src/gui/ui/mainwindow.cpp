#include "mainwindow.h"
#include "ui_mainwindow.h"

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

    ui->toolBox->setCurrentIndex(0);
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
}

void MainWindow::OGRFormats() {
    GDALDriverH hDrv = 0;
    const char *pszM = 0;
    QString longname, shortname, ext;
    for(int i = 0; i < GDALGetDriverCount(); i++) {
        hDrv = GDALGetDriver(i);
        assert(hDrv);
        // Check for DCAP_VECTOR
        longname = QString(GDALGetDriverLongName(hDrv));
        shortname = QString(GDALGetDriverShortName(hDrv));
        ext = QString(GDALGetMetadataItem(hDrv, "", GDAL_DMD_EXTENSIONS));
        qDebug() << longname << "," << shortname << "," << ext;
    }
}

void MainWindow::openElevation() {
    QString file = QFileDialog::getOpenFileName(this,
     tr("Open Elevation Input File"),
     "./",
     tr("Elevation Input Files (*.asc *.lcp *.tif *.img)"));
    if(file == "") {
      return;
    }
    // Check file via API
    QFileInfo info = QFileInfo(file);
    ui->elevEdit->setText(info.fileName());
}

void MainWindow::updateMesh(int index) {
    ui->meshSpinBox->setEnabled(index == 3);
    ui->meshUnitCombo->setEnabled(index == 3);
}




















