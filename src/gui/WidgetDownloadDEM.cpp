/******************************************************************************
 *
 * $Id: WidgetDownloadDEM.cpp 1757 2012-08-07 18:40:40Z kyle.shannon $
 *
 * Project:  WindNinja
 * Purpose:  DEM Downloader Window
 * Author:   Cody Posey <cody.posey85@gmail.com>
 *
 ******************************************************************************
 *
 * THIS SOFTWARE WAS DEVELOPED AT THE ROCKY MOUNTAIN RESEARCH STATION (RMRS)
 * MISSOULA FIRE SCIENCES LABORATORY BY EMPLOYEES OF THE FEDERAL GOVERNMENT
 * IN THE COURSE OF THEIR OFFICIAL DUTIES. PURSUANT TO TITLE 17 SECTION 105
 * OF THE UNITED STATES CODE, THIS SOFTWARE IS NOT SUBJECT TO COPYRIGHT
 * PROTECTION AND IS IN THE PUBLIC DOMAIN. RMRS MISSOULA FIRE SCIENCES
 * LABORATORY ASSUMES NO RESPONSIBILITY WHATSOEVER FOR ITS USE BY OTHER
 * PARTIES,  AND MAKES NO GUARANTEES, EXPRESSED OR IMPLIED, ABOUT ITS QUALITY,
 * RELIABILITY, OR ANY OTHER CHARACTERISTIC.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 *****************************************************************************/
 
#include "WidgetDownloadDEM.h"
//#include <vld.h>

WidgetDownloadDEM::WidgetDownloadDEM(QWidget *parent)
    : QWidget(parent)
{

    //------------provide updated SSL certs in case system ones are outdated--------------------------------//
    // qt_certs_bundle.pem generated on a windows system with the latest windows version of openssl installed
    // certutil -generateSSTFromWU roots.sst
    // certutil -split -f roots.sst
    // type NUL > qt_certs_bundle.pem
    // for %f in (*.crt) do (openssl x509 -inform der -in "%f" -out temp.pem && type temp.pem >> qt_certs_bundle.pem && del temp.pem)
    // del *.crt
    // for debugging on working systems, try disabling system certificates for testing, set it to an empty list of certificates
    //QSslSocket::setDefaultCaCertificates(QList<QSslCertificate>());
    // add the data folder SSL Ca certificates to the certificates list
    std::string pathToSslCerts = FindDataPath("qt_certs_bundle.pem");
    QString pathToCerts = QString::fromStdString(pathToSslCerts);
    QSslSocket::addDefaultCaCertificates( pathToCerts, QSsl::Pem, QRegExp::FixedString);


    wasDemFetched = false;
    elevSource = "";

    demSelected = false;
    
    setupUi(this);
    setupGM();
    initializeGoogleMapsInterface();
    connectInputs();
    this->readSettings(); // sets northBounds, southBound, eastBound, westBound initial values

    progressBar = new QProgressDialog(this);
    progressBar->setWindowModality(Qt::ApplicationModal);
    progressBar->setAutoReset(false);
    progressBar->setAutoClose(false);

    boundsError = NULL;
    bufferError = NULL;
    latlngError = NULL;

    //These are temporary values to store the points needed for DEM bounds
    double northEast[2];
    double southEast[2];
    double southWest[2];
    double northWest[2];
    
    fetcher = FetchFactory::GetSurfaceFetch(FetchFactory::SRTM,"");
    fetcher->GetCorners(northEast, southEast, southWest, northWest);
    srtm_northBound = northEast[1];
    srtm_eastBound = northEast[0];
    srtm_westBound = southWest[0];
    srtm_southBound = southWest[1];
    delete fetcher;
    
#ifdef HAVE_GMTED
    fetcher = FetchFactory::GetSurfaceFetch(FetchFactory::WORLD_GMTED,"");
    fetcher->GetCorners(northEast, southEast, southWest, northWest);
    world_gmted_northBound = northEast[1];
    world_gmted_eastBound = northEast[0];
    world_gmted_westBound = southWest[0];
    world_gmted_southBound = southWest[1];
    delete fetcher;
#endif

#ifdef WITH_LCP_CLIENT
    fetcher = FetchFactory::GetSurfaceFetch(FetchFactory::LCP,"");
    fetcher->GetCorners(northEast, southEast, southWest, northWest);
    lcp_northBound = northEast[1];
    lcp_eastBound = northEast[0];
    lcp_westBound = southWest[0];
    lcp_southBound = southWest[1];
    delete fetcher;
#endif

#ifndef HAVE_GMTED
    this->cbDEMSource->removeItem(1);
#endif 
  
#ifndef WITH_LCP_CLIENT
    this->cbDEMSource->removeItem(2);
#endif

    //cbDEMSource->setItemData(0, "US coverage Shuttle Radar Topography Mission data (SRTM) at 30 meter resolution.  Any existing holes in the data have been filled.", Qt::ToolTipRole);
    //cbDEMSource->setItemData(1, "Partial world coverage Shuttle Radar Topography Mission data (SRTM) at 90 meter resolution.  Any existing holes in the data have been filled.", Qt::ToolTipRole);
#ifdef HAVE_GMTED
    cbDEMSource->setItemData(1, "World coverage Global Multi-resolution Terrain Elevation Data 2010 (GMTED2010) at 250 meter resolution.", Qt::ToolTipRole);
#endif
#ifdef WITH_LCP_CLIENT
    cbDEMSource->setItemData(2, "Description for LCP goes here.", Qt::ToolTipRole);
#endif
    updateDEMSource(cbDEMSource->currentIndex());

    fileSize = 0;

#ifdef ENABLE_FIDDLER
    QNetworkProxy proxy;
    proxy.setType(QNetworkProxy::HttpProxy);
    proxy.setHostName("127.0.0.1");
    proxy.setPort(8888);
    QNetworkProxy::setApplicationProxy(proxy);   
#endif

    this->setWindowFlags(Qt::CustomizeWindowHint | Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint);
    this->show();
}

/**
 * @brief Deletes all allocated memory on destruction of class object
 *
 */
WidgetDownloadDEM::~WidgetDownloadDEM()
{
    delete progressBar;
    delete fetcher;
    delete latlngError;
    delete bufferError;
    delete boundsError;
    delete gmInterface;
}

/**
 * @brief Connect all SLOTS and SIGNALS in the Qt GUI
 *
 */
void WidgetDownloadDEM::connectInputs()
{
    connect(btnDownloadDEM, SIGNAL(clicked()), this, SLOT(saveDEM()));
    connect(cbDEMSource, SIGNAL(currentIndexChanged(int)), this, SLOT(updateDEMSource(int)));
}

/**
 * @brief Initialize the google maps interface and interface object
 *
 */
void WidgetDownloadDEM::initializeGoogleMapsInterface()
{
    gmInterface = new GoogleMapsInterface();
    this->wvGoogleMaps->page()->mainFrame()->addToJavaScriptWindowObject("GMInterface", gmInterface);
    connect(gmInterface, SIGNAL(zoomExtents()), this, SLOT(zoomToMidpoint()));
}

/**
 * @brief Clears listeners when changing selection method
 *
 */
void WidgetDownloadDEM::clearListeners()
{
    this->wvGoogleMaps->page()->mainFrame()->evaluateJavaScript("clearListeners(); null");
}


/**
 * @brief Sets up the google map
 *
 */
void WidgetDownloadDEM::setupGM()
{
    this->wvGoogleMaps->settings()->setAttribute(QWebSettings::JavascriptEnabled, true);
    this->wvGoogleMaps->settings()->setAttribute(QWebSettings::DeveloperExtrasEnabled, true);
    this->wvGoogleMaps->setPage(new QWebPage());
    wvGoogleMaps->page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);
    this->wvGoogleMaps->load(QUrl(QString(CPLFormFilename("file:///", (FindDataPath("map.htm").c_str()), NULL))));

    //enable QWebInspector for degugging google maps widget
    if(CSLTestBoolean(CPLGetConfigOption("ENABLE_QWEBINSPECTOR", "NO")))
    {
        wvGoogleMaps->page()->settings()->setAttribute(QWebSettings::DeveloperExtrasEnabled, true);
        QWebInspector *i = new QWebInspector(this);
        i->setPage(wvGoogleMaps->page());
        i->setVisible(true);

        dlg.setLayout(new QVBoxLayout());
        dlg.layout()->addWidget(i);
        dlg.setModal(false);
        dlg.show();
        dlg.raise();
        dlg.activateWindow();
    }
}

/**
 * @brief Download and save the current DEM area
 *
 */
void WidgetDownloadDEM::saveDEM()
{
    wasDemFetched = true;

    QVariant mbr = wvGoogleMaps->page()->mainFrame()->evaluateJavaScript("mbr()");
    if(mbr.isNull()) {
        qDebug()<<"no mbr";
        QMessageBox mbox;
        mbox.setText("DEM bounding box not set. Select the DEM bounding box by using the bounding box drawing tool on the upper right corner of the map, entering a point and radius, or entering the bounding box coordinates.");
        mbox.exec();
        return;
    }
    QVariantList mbrl = mbr.toList();
    if(mbrl.size() == 0)
    {
        QMessageBox mbox;
        mbox.setText("DEM bounding box not set. Select the DEM bounding box by using the bounding box drawing tool on the upper right corner of the map, entering a point and radius, or entering the bounding box coordinates.");
        mbox.exec();
        return;
    }
    for(int i=0; i < mbrl.size(); i++) {
        qDebug() << mbrl[i];
    }
    northBound = mbrl[3].toDouble();
    southBound = mbrl[2].toDouble();
    eastBound = mbrl[1].toDouble();
    westBound = mbrl[0].toDouble();
    demSelected = true;

    QString fileName;
    double boundArray[] = {this->northBound, this->eastBound, this->southBound, this->westBound};
    double *boundBox;
    boundBox = boundArray;

    if(northBound == 0.0 || southBound == 0.0 || eastBound == 0.0 || westBound == 0.0)
    {
        QMessageBox noBoundsError;
        noBoundsError.setText("Please Select an Area on the Map");
        noBoundsError.exec();
    }
    else if(!demSelected)
    {
        QMessageBox noBoundsError2;
        noBoundsError2.setText("Please select an area on the map.");
        noBoundsError2.exec();
    }
    //else if(!demBoundsCheck())
    //{
    //    QMessageBox demBoundsError;
    //    demBoundsError.setText("Area is outside data bounds. Please select new data Source or new Area.");
    //    demBoundsError.exec();
    //}
    else if((fileSize/1024) > 50)
    {
        QMessageBox demBoundsError;
        demBoundsError.setText("File size is limited to 50mb. Please select a smaller area.");
        demBoundsError.exec();
    }
    else
    {
        QFileDialog saveDialog(this, tr("Save File"), demFileDir.absolutePath(), currentSaveAsDesc);
        saveDialog.setDefaultSuffix(currentSuffix);
        saveDialog.setAcceptMode(QFileDialog::AcceptSave);
        if(saveDialog.exec())
        {
            fileName = saveDialog.selectedFiles().at(0);

            QByteArray file = fileName.toLocal8Bit();
            demFile = file.data();
 
            progressBar->setLabelText("Downloading File...");
            progressBar->setRange(0,0);
            progressBar->setCancelButtonText("Cancel");
            progressBar->reset();
            bool test = progressBar->wasCanceled();

            connect(&futureWatcher, SIGNAL(finished()), this, SLOT(updateProgress()));
            connect(progressBar, SIGNAL(canceled()), this, SLOT(updateProgress()));

            futureWatcher.setFuture(QtConcurrent::run(this, &WidgetDownloadDEM::fetchBoundBox, boundBox, demFile, currentResolution));

            progressBar->exec();
        } 
    }
}     

/**
 * @brief Updates the progress bar and fills no data values
 *
 */
void WidgetDownloadDEM::updateProgress()
{
    disconnect(progressBar, SIGNAL(canceled()), this, SLOT(updateProgress()));
    disconnect(&futureWatcher, SIGNAL(finished()), this, SLOT(updateProgress()));

    if(progressBar->wasCanceled())
    {
        progressBar->setLabelText("Canceling...");
        progressBar->setCancelButton(0);
        futureWatcher.waitForFinished();
        VSIUnlink(demFile);
        progressBar->cancel();
    }
    else
    {
        futureWatcher.waitForFinished();
        int result = futureWatcher.result();

        if(result < 0)
        {
            progressBar->setLabelText("The surface data download failed. \nThis can happen when either the data source doesn't cover your region or the server that provides the surface data is down or under high usage. \nPlease try again later or try a different data source.");
            progressBar->setRange(0,1);
            progressBar->setValue( 0 );
            progressBar->setCancelButtonText("Close");
            VSIUnlink(demFile);                
        }
        else 
        {
            if(result)
                this->fillNoDataValues(demFile);

            progressBar->setValue(1);
            progressBar->setRange(0,100);
            progressBar->setValue(100);
            progressBar->setLabelText("Download Successful");
            progressBar->setCancelButtonText("Close");

            this->doneDownloading(demFile);
        }         
    }
}
        

/**
 * @brief Closes the DEM downloader
 *
 */
void WidgetDownloadDEM::closeDEM()
{
    this->close();
}

/**
 * @brief Fetches bounding box for DEM
 *
 * @param boundsBox Lat/Lon Bounds
 * @param fileName Name of saved DEM file
 * @param resolution Resolution for DEM
 */
int WidgetDownloadDEM::fetchBoundBox(double *boundsBox, const char *fileName, double resolution)
{
    int result = fetcher->FetchBoundingBox(boundsBox, resolution, fileName, NULL);

    return result;
}          

/**
 * @brief Updates the DEM Source
 *
 * @param index Current index of the DEM source combo box
 */
void WidgetDownloadDEM::updateDEMSource(int index)
{
    switch(index){
    case 0: //SRTM
        elevSource = "srtm";
        fetcher = FetchFactory::GetSurfaceFetch(FetchFactory::SRTM, FindDataPath("/data"));
        northDEMBound = srtm_northBound;
        southDEMBound = srtm_southBound;
        currentResolution = fetcher->GetXRes();
        currentSuffix = "tif";
        currentSaveAsDesc = "Elevation files (*.tif)";
        break;
#ifdef HAVE_GMTED
    case 1: //GMTED
        elevSource = "gmted";
        fetcher = FetchFactory::GetSurfaceFetch(FetchFactory::WORLD_GMTED, FindDataPath("/data"));
        northDEMBound = world_gmted_northBound;
        southDEMBound = world_gmted_southBound;
        currentResolution = (fetcher->GetXRes() * 111325);
        currentSuffix = "tif";
        currentSaveAsDesc = "Elevation files (*.tif)";
        break;
#endif
#ifdef WITH_LCP_CLIENT
    case 2: //LCP
        elevSource = "lcp";
        fetcher = FetchFactory::GetSurfaceFetch(FetchFactory::LCP, FindDataPath("/data"));
        northDEMBound = lcp_northBound;
        southDEMBound = lcp_southBound;
        /* this is in meters */
        currentResolution = fetcher->GetXRes();
        currentSuffix = "tif";
        currentSaveAsDesc = "Landscape files (*.tif)";
        break;
#endif
    }
}

/**
 * @brief Checks whether the current selected area is withing the current DEM bounds
 *
 */
bool WidgetDownloadDEM::demBoundsCheck()
{ 
    QString checkBounds;
    QString wktPoly;
    QByteArray byteArray;
    int contains;
    //std::string oFile = FindDataPath( "us_srtm_region.shp" );
    std::string oFile = FindDataPath( "srtm_region.geojson" );
    const char *pszWkt;
    switch(cbDEMSource->currentIndex()){
        case 0: //SRTM
            wktPoly =
                QString("POLYGON( (%1 %2, %3 %4, ").arg(westBound).arg(northBound).arg(eastBound).arg(northBound) +
                QString("%1 %2, %3 %4, ").arg(eastBound).arg(southBound).arg(westBound).arg(southBound) +
                QString("%1 %2) )").arg(westBound).arg(northBound);
            byteArray = wktPoly.toUtf8();
            pszWkt = byteArray.constData();
            contains = NinjaOGRContain(pszWkt, oFile.c_str(), NULL);
            if(contains)
            {
                return true;
            }
            else
                return false; 
        case 1: //GMTED
            if(northDEMBound < northBound || southDEMBound > southBound)
                return false;
            else
                return true;
        break;
        case 2: //LCP
            if(northDEMBound < northBound || southDEMBound > southBound)
                return false;
            else
                return true;
        break;
    }
}

/**
 * @brief Pans to lat/lng point
 *
 */
void WidgetDownloadDEM::zoomToMidpoint()
{
    QString midPoint = QString("zoomFitBounds(%1,%2,%3,%4);").arg(southBound).arg(westBound).arg(northBound).arg(eastBound);
    wvGoogleMaps->page()->mainFrame()->evaluateJavaScript(midPoint);
}

/**
 * @brief Updates area selected for Est File Size and for downloading a DEM
 *
 * @param selected Whether an area has been selected
 */
void WidgetDownloadDEM::demSelectedUpdate(bool selected)
{
    demSelected = selected;
}

/**
 * @brief Uses GDAL utilities to fill any no data values in the file
 *
 * @param file The file containing DEM data
 */
void WidgetDownloadDEM::fillNoDataValues(const char* file)
{
    GDALDataset *poDS;
    poDS = (GDALDataset*)GDALOpen(demFile, GA_Update);
    int nNoDataValues;
    if(GDALHasNoData(poDS, 1))
    {
        nNoDataValues = GDALFillBandNoData(poDS, 1, 100);
        /* Should we fill again? */
        if(nNoDataValues)
        {}
    }
    GDALClose((GDALDatasetH)poDS);
}

/**
 * @brief Saves the current DEM settings
 *
 */
void WidgetDownloadDEM::writeSettings()
{
    QSettings settings(QSettings::UserScope, "Firelab", "WindNinja");
    settings.setDefaultFormat(QSettings::IniFormat);
    //Saves DEM Source to settings
    settings.setValue("demSource", cbDEMSource->currentIndex());
    //Saves Current Lat/Lon Positions
    settings.setValue("North", northBound);
    settings.setValue("South", southBound);
    settings.setValue("East", eastBound);
    settings.setValue("West", westBound);
    settings.setValue("Latitude", latitude);
    settings.setValue("Longitude", longitude);
    //Saves Download Path to settings
    settings.setValue("downloadPath", settingsDir.absolutePath());
}

/**
 * @brief Reads previous DEM settings
 *
 */
void WidgetDownloadDEM::readSettings()
{
  QSettings settings(QSettings::UserScope, "Firelab", "WindNinja");
  settings.setDefaultFormat(QSettings::IniFormat);
  
    if(settings.contains("demSource"))
    {
        cbDEMSource->setCurrentIndex(settings.value("demSource").toInt());
    }
    if(settings.contains("inputFileDir"))
    {
      demFileDir = settings.value("inputFileDir").toString();
    }

    if(settings.contains("North"))
    {
      northBound = settings.value("North").toDouble();
    }

    else 
        northBound = 44.0249023401036;

    if(settings.contains("South"))
    {
      southBound = settings.value("South").toDouble();
    }
    else
        southBound = 43.7832152227745;
  
    if(settings.contains("East"))
    {
      eastBound = settings.value("East").toDouble();
    }
    else
        eastBound = -113.463446144564;
    if(settings.contains("West"))
    {
      westBound = settings.value("West").toDouble();
    }
    else
        westBound = -113.749693430469;
    
    if(settings.contains("Latitude"))
    {
        latitude = settings.value("Latitude").toDouble();
    }
    else
        latitude = -113.613611;
    if(settings.contains("Longitude"))
    {
        longitude = settings.value("Longitude").toDouble();
    }
    else
        longitude = 43.911944;
}

bool WidgetDownloadDEM::get_wasDemFetched()
{
    return wasDemFetched;
}

std::string WidgetDownloadDEM::get_elevSource()
{
    return elevSource;
}

double WidgetDownloadDEM::get_northBound()
{
    return northBound;
}

double WidgetDownloadDEM::get_southBound()
{
    return southBound;
}

double WidgetDownloadDEM::get_eastBound()
{
    return eastBound;
}

double WidgetDownloadDEM::get_westBound()
{
    return westBound;
}

/**
 * @brief Handles close events of GUI such as clicking X close icon
 *
 */
void WidgetDownloadDEM::closeEvent(QCloseEvent *event)
{
    event->ignore();
    this->writeSettings();
    exitDEM();
    event->accept();
}
