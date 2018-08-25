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
    demSelected = false;
    
    setupUi(this);
    setupGM();
    initializeGoogleMapsInterface();
    connectInputs();
    this->readSettings();

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
    
    
    fetcher = FetchFactory::GetSurfaceFetch(FetchFactory::US_SRTM,"");
    fetcher->GetCorners(northEast, southEast, southWest, northWest);
    us_srtm_northBound = northEast[1];
    us_srtm_eastBound = northEast[0];
    us_srtm_westBound = southWest[0];
    us_srtm_southBound = southWest[1];
    delete fetcher;
    
    fetcher = FetchFactory::GetSurfaceFetch(FetchFactory::WORLD_SRTM,"");
    fetcher->GetCorners(northEast, southEast, southWest, northWest);
    world_srtm_northBound = northEast[1];
    world_srtm_eastBound = northEast[0];
    world_srtm_westBound = southWest[0];
    world_srtm_southBound = southWest[1];
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
    this->cbDEMSource->removeItem(2);
#endif 
  
#ifndef WITH_LCP_CLIENT
    this->cbDEMSource->removeItem(3);
#endif

    cbDEMSource->setItemData(0, "US coverage Shuttle Radar Topography Mission data (SRTM) at 30 meter resolution.  Any existing holes in the data have been filled.", Qt::ToolTipRole);
    cbDEMSource->setItemData(1, "Partial world coverage Shuttle Radar Topography Mission data (SRTM) at 90 meter resolution.  Any existing holes in the data have been filled.", Qt::ToolTipRole);
#ifdef HAVE_GMTED
    cbDEMSource->setItemData(2, "World coverage Global Multi-resolution Terrain Elevation Data 2010 (GMTED2010) at 250 meter resolution.", Qt::ToolTipRole);
#endif
#ifdef WITH_LCP_CLIENT
    cbDEMSource->setItemData(3, "Description for LCP goes here.", Qt::ToolTipRole);
#endif
    updateDEMSource(cbDEMSource->currentIndex());
    
    fileSize = 0;
    estFileSize();

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
    this->wvGoogleMaps->load(QUrl(QString(FindDataPath("map.htm").c_str())));

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
    QVariant mbr = wvGoogleMaps->page()->mainFrame()->evaluateJavaScript("mbr()");
    if(mbr.isNull()) {
        qDebug()<<"no mbr";
        return;
    }
    qDebug()<<mbr;
    QVariantList mbrl = mbr.toList();
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
        noBoundsError2.setText("Please Select an Area on the Map");
        noBoundsError2.exec();
    }
    else if(!demBoundsCheck())
    {
        QMessageBox demBoundsError;
        demBoundsError.setText("Area Is Outside Data Bounds, Please Select New Data Source or New Area, \nClick 'Show Available Data' To View Current Data Bounds");
        demBoundsError.exec();
    }
    else if((fileSize/1024) > 50)
    {
        QMessageBox demBoundsError;
        demBoundsError.setText("File Size Is Limited to 50mb, Please Select a Smaller Area");
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
            progressBar->setLabelText("An Error Occured: Please Try Again");
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
    case 0:
        //Previously "c:/src/windninja/trunk/data"
        fetcher = FetchFactory::GetSurfaceFetch(FetchFactory::US_SRTM, FindDataPath("/data"));
        northDEMBound = us_srtm_northBound;
        southDEMBound = us_srtm_southBound;
        currentResolution = (fetcher->GetXRes() * 111325);
        currentSuffix = "tif";
        currentSaveAsDesc = "Elevation files (.tif)";
        break;
    case 1:
        fetcher = FetchFactory::GetSurfaceFetch(FetchFactory::WORLD_SRTM, FindDataPath("/data"));
        northDEMBound = world_srtm_northBound;
        southDEMBound = world_srtm_southBound;
        currentResolution = (fetcher->GetXRes() * 111325);
        currentSuffix = "tif";
        currentSaveAsDesc = "Elevation files (.tif)";
        break;
#ifdef HAVE_GMTED
    case 2:
        fetcher = FetchFactory::GetSurfaceFetch(FetchFactory::WORLD_GMTED, FindDataPath("/data"));
        northDEMBound = world_gmted_northBound;
        southDEMBound = world_gmted_southBound;
        currentResolution = (fetcher->GetXRes() * 111325);
        currentSuffix = "tif";
        currentSaveAsDesc = "Elevation files (.tif)";
        break;
#endif
#ifdef WITH_LCP_CLIENT
    case 3:
        fetcher = FetchFactory::GetSurfaceFetch(FetchFactory::LCP, FindDataPath("/data"));
        northDEMBound = lcp_northBound;
        southDEMBound = lcp_southBound;
        /* this is in meters */
        currentResolution = fetcher->GetXRes();
        currentSuffix = "lcp";
        currentSaveAsDesc = "Landscape files (.lcp)";
        break;
#endif
    }
    displayDEMBounds(this->ckbShowExtent->checkState());
    estFileSize();
}

/**
 * @brief Displays the current DEM bounds on the map
 *
 * @param state A state of 0 indicates the Show Extent box in unchecked, a state of 2 indicates that it is checked
 */
void WidgetDownloadDEM::displayDEMBounds(int state)
{
    if(state == 0)
    {
        wvGoogleMaps->page()->mainFrame()->evaluateJavaScript("clearDEMBounds(); null");
    }
    else if(state == 2)
    {
        int currentIndex = cbDEMSource->currentIndex();
        QString bounds;
        QString box;

        wvGoogleMaps->page()->mainFrame()->evaluateJavaScript("clearDEMBounds(); null");
        switch(currentIndex){
        case 0:
            // US SRTM
            bounds =
                QString("us_srtm_kml.setMap(map);");
            qDebug() << bounds;

            this->wvGoogleMaps->page()->currentFrame()->documentElement().evaluateJavaScript(bounds);
            break;
        case 1:
            // World SRTM
            bounds =
                QString("new google.maps.LatLng(%1,%2),").arg(world_srtm_southBound).arg(world_srtm_westBound) +
                QString("new google.maps.LatLng(%1,%2)").arg(world_srtm_northBound).arg(world_srtm_eastBound);

            box = 
                QString("var demBox = new google.maps.Rectangle({") +
                QString("bounds: new google.maps.LatLngBounds(%1),").arg(bounds) +
                QString("clickable: false,") +
                QString("editable: false,") +
                QString("strokeColor: \"#0000EE\",") +
                QString("strokeWeight: 2,") +
                QString("fillColor: \"#0000EE\",") +
                QString("fillOpacity: 0.1,") +
                QString("map: map,") +
                QString("});") +
                QString("demBounds.push(demBox);");
            qDebug() << box;

            this->wvGoogleMaps->page()->currentFrame()->documentElement().evaluateJavaScript(box);
            break;
    #ifdef HAVE_GMTED
        case 2:
            // GMTED
            bounds =
                QString("new google.maps.LatLng(%1,%2),").arg(world_gmted_southBound).arg(world_gmted_westBound) +
                QString("new google.maps.LatLng(%1,%2)").arg(world_gmted_northBound).arg(world_gmted_eastBound);

            box = 
                QString("var demBox = new google.maps.Rectangle({") +
                QString("bounds: new google.maps.LatLngBounds(%1),").arg(bounds) +
                QString("clickable: false,") +
                QString("editable: false,") +
                QString("strokeColor: \"#1400F0\",") +
                QString("strokeWeight: 2,") +
                QString("fillColor: \"#1400F0\",") +
                QString("fillOpacity: 0.1,") +
                QString("map: map,") +
                QString("});") +
                QString("demBounds.push(demBox);");
            qDebug() << box;

            this->wvGoogleMaps->page()->currentFrame()->documentElement().evaluateJavaScript(box);
            break;
    #endif
    #ifdef WITH_LCP_CLIENT
        case 3:
            // LCP
            bounds =
                QString("new google.maps.LatLng(%1,%2),").arg(lcp_southBound).arg(lcp_westBound) +
                QString("new google.maps.LatLng(%1,%2)").arg(lcp_northBound).arg(lcp_eastBound);

            box = 
                QString("var demBox = new google.maps.Rectangle({") +
                QString("bounds: new google.maps.LatLngBounds(%1),").arg(bounds) +
                QString("clickable: false,") +
                QString("editable: false,") +
                QString("strokeColor: \"#1400F0\",") +
                QString("strokeWeight: 2,") +
                QString("fillColor: \"#1400F0\",") +
                QString("fillOpacity: 0.1,") +
                QString("map: map,") +
                QString("});") +
                QString("demBounds.push(demBox);");
            qDebug() << box;

            this->wvGoogleMaps->page()->currentFrame()->documentElement().evaluateJavaScript(box);
            break;
    #endif
        }
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
    std::string oFile = FindDataPath( "us_srtm_region.shp" );
    //oFile = "/vsizip/" + oFile + "/us_srtm_region.shp";
    const char *pszWkt;
    switch(cbDEMSource->currentIndex()){
        case 0:
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
        case 1:
            if(northDEMBound < northBound || southDEMBound > southBound)
                return false;
            else
                return true;
        break;
        case 2:
            if(northDEMBound < northBound || southDEMBound > southBound)
                return false;
            else
                return true;
        break;
        case 3:
            if(northDEMBound < northBound || southDEMBound > southBound)
                return false;
            else
                return true;
        break;
    }
}
    
/**
* @brief Checks the tree for checked checkboxes and displays the corresponding kml link
*
* @param item The item that was checked/unchecked
* @param column The status of the checkbox
*/
void WidgetDownloadDEM::showAdditionalData(QTreeWidgetItem *item, int column)
{
    int num = column;
    QString itemText = item->text(0);
    int checkState = item->checkState(num);
    QTreeWidgetItem *parent = item->parent();
    QString parentText = parent->text(0);
    
    if(parentText.toStdString() == "Fire Locations")
    {
        if(itemText.toStdString() == "Sit Report(RSAC)")
        {
            if(checkState == 2)
            {
                QString data = 
                    QString("var fireKML_RSAC = new google.maps.KmlLayer('http://wfas.net/google-earth/large_fires_rsac.kmz', { preserveViewport: true });") +
                    QString("fireKML_RSAC.setMap(map);");
                qDebug() << data;

                this->wvGoogleMaps->page()->currentFrame()->documentElement().evaluateJavaScript(data);
            }
            else if(checkState == 0)
            {
                QString removeData =
                    QString("fireKML_RSAC.setMap(null);");
                qDebug() << removeData;

                this->wvGoogleMaps->page()->currentFrame()->documentElement().evaluateJavaScript(removeData);
            }
        }
        else if(itemText.toStdString() == "Sit Report(GeoMac)")
        {
            if(checkState == 2)
            {
                QString data = 
                    QString("var fireKML_GeoMac = new google.maps.KmlLayer('http://rmgsc.cr.usgs.gov/outgoing/GeoMAC/ActiveFirePerimeters.kml', { preserveViewport: true });") +
                    QString("fireKML_GeoMac.setMap(map);");
                qDebug() << data;

                this->wvGoogleMaps->page()->currentFrame()->documentElement().evaluateJavaScript(data);
            }
            else if(checkState == 0)
            {
                QString removeData =
                    QString("fireKML_GeoMac.setMap(null);");
                qDebug() << removeData;

                this->wvGoogleMaps->page()->currentFrame()->documentElement().evaluateJavaScript(removeData);
            }
        }
    }
    else if(parentText.toStdString() == "Contiguous US")
    {
        if(itemText.toStdString() == "MODIS")
        {
            if(checkState == 2)
            {
                QString data = 
                    QString("var modisKML = new google.maps.KmlLayer('http://wfas.net/google-earth/modis_conus.kmz', { preserveViewport: true });") +
                    QString("modisKML.setMap(map);");
                qDebug() << data;

                this->wvGoogleMaps->page()->currentFrame()->documentElement().evaluateJavaScript(data);
            }
            else if(checkState == 0)
            {
                QString removeData =
                    QString("modisKML.setMap(null);");
                qDebug() << removeData;

                this->wvGoogleMaps->page()->currentFrame()->documentElement().evaluateJavaScript(removeData);
            }  
        }
        else if(itemText.toStdString() == "VIIRS")
        {
            if(checkState == 2)
            {
                QString data = 
                    QString("var viirsKML = new google.maps.KmlLayer('http://wfas.net/google-earth/viirs_conus.kmz', { preserveViewport: true });") +
                    QString("viirsKML.setMap(map);");
                qDebug() << data;

                this->wvGoogleMaps->page()->currentFrame()->documentElement().evaluateJavaScript(data);
            }
            else if(checkState == 0)
            {
                QString removeData =
                    QString("viirsKML.setMap(null);");
                qDebug() << removeData;

                this->wvGoogleMaps->page()->currentFrame()->documentElement().evaluateJavaScript(removeData);
            }
        }
    }
    else if(parentText.toStdString() == "Alaska")
    {
        if(itemText.toStdString() == "MODIS")
        {
            if(checkState == 2)
            {
                QString data = 
                    QString("var modisKML = new google.maps.KmlLayer('http://wfas.net/google-earth/modis_alaska.kmz', { preserveViewport: true });") +
                    QString("modisKML.setMap(map);");
                qDebug() << data;

                this->wvGoogleMaps->page()->currentFrame()->documentElement().evaluateJavaScript(data);
            }
            else if(checkState == 0)
            {
                QString removeData =
                    QString("modisKML.setMap(null);");
                qDebug() << removeData;

                this->wvGoogleMaps->page()->currentFrame()->documentElement().evaluateJavaScript(removeData);
            }  
        }
        else if(itemText.toStdString() == "VIIRS")
        {
            if(checkState == 2)
            {
                QString data = 
                    QString("var viirsKML = new google.maps.KmlLayer('http://wfas.net/google-earth/viirs_alaska.kmz', { preserveViewport: true });") +
                    QString("viirsKML.setMap(map);");
                qDebug() << data;

                this->wvGoogleMaps->page()->currentFrame()->documentElement().evaluateJavaScript(data);
            }
            else if(checkState == 0)
            {
                QString removeData =
                    QString("viirsKML.setMap(null);");
                qDebug() << removeData;

                this->wvGoogleMaps->page()->currentFrame()->documentElement().evaluateJavaScript(removeData);
            }
        }
    }
    else if(parentText.toStdString() == "Hawaii")
    {
        if(itemText.toStdString() == "MODIS")
        {
            if(checkState == 2)
            {
                QString data = 
                    QString("var modisKML = new google.maps.KmlLayer('http://wfas.net/google-earth/modis_hawaii.kmz', { preserveViewport: true });") +
                    QString("modisKML.setMap(map);");
                qDebug() << data;

                this->wvGoogleMaps->page()->currentFrame()->documentElement().evaluateJavaScript(data);
            }
            else if(checkState == 0)
            {
                QString removeData =
                    QString("modisKML.setMap(null);");
                qDebug() << removeData;

                this->wvGoogleMaps->page()->currentFrame()->documentElement().evaluateJavaScript(removeData);
            }  
        }
        else if(itemText.toStdString() == "VIIRS")
        {
            if(checkState == 2)
            {
                QString data = 
                    QString("var viirsKML = new google.maps.KmlLayer('http://wfas.net/google-earth/viirs_hawaii.kmz', { preserveViewport: true });") +
                    QString("viirsKML.setMap(map);");
                qDebug() << data;

                this->wvGoogleMaps->page()->currentFrame()->documentElement().evaluateJavaScript(data);
            }
            else if(checkState == 0)
            {
                QString removeData =
                    QString("viirsKML.setMap(null);");
                qDebug() << removeData;

                this->wvGoogleMaps->page()->currentFrame()->documentElement().evaluateJavaScript(removeData);
            }
        }
    }
    else if(parentText.toStdString() == "Canada")
    {
        if(itemText.toStdString() == "MODIS")
        {
            if(checkState == 2)
            {
                QString data = 
                    QString("var modisKML = new google.maps.KmlLayer('http://wfas.net/google-earth/modis_canada.kmz', { preserveViewport: true });") +
                    QString("modisKML.setMap(map);");
                qDebug() << data;

                this->wvGoogleMaps->page()->currentFrame()->documentElement().evaluateJavaScript(data);
            }
            else if(checkState == 0)
            {
                QString removeData =
                    QString("modisKML.setMap(null);");
                qDebug() << removeData;

                this->wvGoogleMaps->page()->currentFrame()->documentElement().evaluateJavaScript(removeData);
            }  
        }
        else if(itemText.toStdString() == "VIIRS")
        {
            if(checkState == 2)
            {
                QString data = 
                    QString("var viirsKML = new google.maps.KmlLayer('http://wfas.net/google-earth/viirs_canada.kmz', { preserveViewport: true });") +
                    QString("viirsKML.setMap(map);");
                qDebug() << data;

                this->wvGoogleMaps->page()->currentFrame()->documentElement().evaluateJavaScript(data);
            }
            else if(checkState == 0)
            {
                QString removeData =
                    QString("viirsKML.setMap(null);");
                qDebug() << removeData;

                this->wvGoogleMaps->page()->currentFrame()->documentElement().evaluateJavaScript(removeData);
            }
        }
    }
    else if(parentText.toStdString() == "NASA EOS")
    {
        if(itemText.toStdString() == "Alaska")
        {
            if(checkState == 2)
            {
                QString data = 
                    QString("var eos_Alaska = new google.maps.KmlLayer('http://firms.modaps.eosdis.nasa.gov/active_fire/kml/Alaska_24h.kml', { preserveViewport: true });") +
                    QString("eos_Alaska.setMap(map);");
                qDebug() << data;

                this->wvGoogleMaps->page()->currentFrame()->documentElement().evaluateJavaScript(data);
            }
            else if(checkState == 0)
            {
                QString removeData =
                    QString("eos_Alaska.setMap(null);");
                qDebug() << removeData;

                this->wvGoogleMaps->page()->currentFrame()->documentElement().evaluateJavaScript(removeData);
            }
        }
        else if(itemText.toStdString() == "Australia and New Zealand")
        {
            if(checkState == 2)
            {
                QString data = 
                    QString("var eos_Australia = new google.maps.KmlLayer('http://firms.modaps.eosdis.nasa.gov/active_fire/kml/Australia_and_New_Zealand_24h.kml', { preserveViewport: true });") +
                    QString("eos_Australia.setMap(map);");
                qDebug() << data;

                this->wvGoogleMaps->page()->currentFrame()->documentElement().evaluateJavaScript(data);
            }
            else if(checkState == 0)
            {
                QString removeData =
                    QString("eos_Australia.setMap(null);");
                qDebug() << removeData;

                this->wvGoogleMaps->page()->currentFrame()->documentElement().evaluateJavaScript(removeData);
            }
        }
        else if(itemText.toStdString() == "Canada")
        {
            if(checkState == 2)
            {
                QString data = 
                    QString("var eos_Canada = new google.maps.KmlLayer('http://firms.modaps.eosdis.nasa.gov/active_fire/kml/Canada_24h.kml', { preserveViewport: true });") +
                    QString("eos_Canada.setMap(map);");
                qDebug() << data;

                this->wvGoogleMaps->page()->currentFrame()->documentElement().evaluateJavaScript(data);
            }
            else if(checkState == 0)
            {
                QString removeData =
                    QString("eos_Canada.setMap(null);");
                qDebug() << removeData;

                this->wvGoogleMaps->page()->currentFrame()->documentElement().evaluateJavaScript(removeData);
            }
        }
        else if(itemText.toStdString() == "Central America")
        {
            if(checkState == 2)
            {
                QString data = 
                    QString("var eos_centralAmerica = new google.maps.KmlLayer('http://firms.modaps.eosdis.nasa.gov/active_fire/kml/Central_America_24h.kml', { preserveViewport: true });") +
                    QString("eos_centralAmerica.setMap(map);");
                qDebug() << data;

                this->wvGoogleMaps->page()->currentFrame()->documentElement().evaluateJavaScript(data);
            }
            else if(checkState == 0)
            {
                QString removeData =
                    QString("eos_centralAmerica.setMap(null);");
                qDebug() << removeData;

                this->wvGoogleMaps->page()->currentFrame()->documentElement().evaluateJavaScript(removeData);
            }
        }
        else if(itemText.toStdString() == "Europe")
        {
            if(checkState == 2)
            {
                QString data = 
                    QString("var eos_Europe = new google.maps.KmlLayer('http://firms.modaps.eosdis.nasa.gov/active_fire/kml/Europe_24h.kml', { preserveViewport: true });") +
                    QString("eos_Europe.setMap(map);");
                qDebug() << data;

                this->wvGoogleMaps->page()->currentFrame()->documentElement().evaluateJavaScript(data);
            }
            else if(checkState == 0)
            {
                QString removeData =
                    QString("eos_Europe.setMap(null);");
                qDebug() << removeData;

                this->wvGoogleMaps->page()->currentFrame()->documentElement().evaluateJavaScript(removeData);
            }
        }
        else if(itemText.toStdString() == "Northern and Central Africa")
        {
            if(checkState == 2)
            {
                QString data = 
                    QString("var eos_northernAfrica = new google.maps.KmlLayer('http://firms.modaps.eosdis.nasa.gov/active_fire/kml/Northern_and_Central_Africa_24h.kml', { preserveViewport: true });") +
                    QString("eos_northernAfrica.setMap(map);");
                qDebug() << data;

                this->wvGoogleMaps->page()->currentFrame()->documentElement().evaluateJavaScript(data);
            }
            else if(checkState == 0)
            {
                QString removeData =
                    QString("eos_northernAfrica.setMap(null);");
                qDebug() << removeData;

                this->wvGoogleMaps->page()->currentFrame()->documentElement().evaluateJavaScript(removeData);
            }
        }
        else if(itemText.toStdString() == "Russia and Asia")
        {
            if(checkState == 2)
            {
                QString data = 
                    QString("var eos_Russia = new google.maps.KmlLayer('http://firms.modaps.eosdis.nasa.gov/active_fire/kml/Russia_and_Asia_24h.kml', { preserveViewport: true });") +
                    QString("eos_Russia.setMap(map);");
                qDebug() << data;

                this->wvGoogleMaps->page()->currentFrame()->documentElement().evaluateJavaScript(data);
            }
            else if(checkState == 0)
            {
                QString removeData =
                    QString("eos_Russia.setMap(null);");
                qDebug() << removeData;

                this->wvGoogleMaps->page()->currentFrame()->documentElement().evaluateJavaScript(removeData);
            }
        }
        else if(itemText.toStdString() == "South America")
        {
            if(checkState == 2)
            {
                QString data = 
                    QString("var eos_southAmerica = new google.maps.KmlLayer('http://firms.modaps.eosdis.nasa.gov/active_fire/kml/South_America_24h.kml', { preserveViewport: true });") +
                    QString("eos_southAmerica.setMap(map);");
                qDebug() << data;

                this->wvGoogleMaps->page()->currentFrame()->documentElement().evaluateJavaScript(data);
            }
            else if(checkState == 0)
            {
                QString removeData =
                    QString("eos_southAmerica.setMap(null);");
                qDebug() << removeData;

                this->wvGoogleMaps->page()->currentFrame()->documentElement().evaluateJavaScript(removeData);
            }
        }
        else if(itemText.toStdString() == "South Asia")
        {
            if(checkState == 2)
            {
                QString data = 
                    QString("var eos_southAsia = new google.maps.KmlLayer('http://firms.modaps.eosdis.nasa.gov/active_fire/kml/South_Asia_24h.kml', { preserveViewport: true });") +
                    QString("eos_southAsia.setMap(map);");
                qDebug() << data;

                this->wvGoogleMaps->page()->currentFrame()->documentElement().evaluateJavaScript(data);
            }
            else if(checkState == 0)
            {
                QString removeData =
                    QString("eos_southAsia.setMap(null);");
                qDebug() << removeData;

                this->wvGoogleMaps->page()->currentFrame()->documentElement().evaluateJavaScript(removeData);
            }
        }
        else if(itemText.toStdString() == "South East Asia")
        {
            if(checkState == 2)
            {
                QString data = 
                    QString("var eos_southEastAsia = new google.maps.KmlLayer('http://firms.modaps.eosdis.nasa.gov/active_fire/kml/SouthEast_Asia_24h.kml', { preserveViewport: true });") +
                    QString("eos_southEastAsia.setMap(map);");
                qDebug() << data;

                this->wvGoogleMaps->page()->currentFrame()->documentElement().evaluateJavaScript(data);
            }
            else if(checkState == 0)
            {
                QString removeData =
                    QString("eos_southEastAsia.setMap(null);");
                qDebug() << removeData;

                this->wvGoogleMaps->page()->currentFrame()->documentElement().evaluateJavaScript(removeData);
            }
        }
        else if(itemText.toStdString() == "Southern Africa")
        {
            if(checkState == 2)
            {
                QString data = 
                    QString("var eos_southAfrica = new google.maps.KmlLayer('http://firms.modaps.eosdis.nasa.gov/active_fire/kml/Southern_Africa_24h.kml', { preserveViewport: true });") +
                    QString("eos_southAfrica.setMap(map);");
                qDebug() << data;

                this->wvGoogleMaps->page()->currentFrame()->documentElement().evaluateJavaScript(data);
            }
            else if(checkState == 0)
            {
                QString removeData =
                    QString("eos_southAfrica.setMap(null);");
                qDebug() << removeData;

                this->wvGoogleMaps->page()->currentFrame()->documentElement().evaluateJavaScript(removeData);
            }
        }
        else if(itemText.toStdString() == "U.S.A (Conterminous) and Hawaii")
        {
            if(checkState == 2)
            {
                QString data = 
                    QString("var eos_USA = new google.maps.KmlLayer('http://firms.modaps.eosdis.nasa.gov/active_fire/kml/USA_contiguous_and_Hawaii_24h.kml', { preserveViewport: true });") +
                    QString("eos_USA.setMap(map);");
                qDebug() << data;

                this->wvGoogleMaps->page()->currentFrame()->documentElement().evaluateJavaScript(data);
            }
            else if(checkState == 0)
            {
                QString removeData =
                    QString("eos_USA.setMap(null);");
                qDebug() << removeData;

                this->wvGoogleMaps->page()->currentFrame()->documentElement().evaluateJavaScript(removeData);
            }
        }
    }
    else if(parentText.toStdString() == "Weather Stations")
    {
        if(itemText.toStdString() == "RAWS")
        {
            if(checkState == 2)
            {
                QString data = 
                    QString("var ws_RAWS = new google.maps.KmlLayer('http://wfas.net/google-earth/raws.kmz', { preserveViewport: true });") +
                    QString("ws_RAWS.setMap(map);");
                qDebug() << data;

                this->wvGoogleMaps->page()->currentFrame()->documentElement().evaluateJavaScript(data);
            }
            else if(checkState == 0)
            {
                QString removeData =
                    QString("ws_RAWS.setMap(null);");
                qDebug() << removeData;

                this->wvGoogleMaps->page()->currentFrame()->documentElement().evaluateJavaScript(removeData);
            }
        }
        else if(itemText.toStdString() == "NOAA/FAA")
        {
            if(checkState == 2)
            {
                QString data = 
                    QString("var ws_NWS = new google.maps.KmlLayer('http://wfas.net/google-earth/nws.kmz', { preserveViewport: true });") +
                    QString("ws_NWS.setMap(map);");
                qDebug() << data;

                this->wvGoogleMaps->page()->currentFrame()->documentElement().evaluateJavaScript(data);
            }
            else if(checkState == 0)
            {
                QString removeData =
                    QString("ws_NWS.setMap(null);");
                qDebug() << removeData;

                this->wvGoogleMaps->page()->currentFrame()->documentElement().evaluateJavaScript(removeData);
            }
        }
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
 * @brief Estimates the DEM file size
 *
 */
void WidgetDownloadDEM::estFileSize()
{
    if(demSelected)
    {
        double xDegrees, yDegrees;
        if(northBound < southBound)
        {
            double temp = northBound;
            northBound = southBound;
            southBound = temp;
            xDegrees = (northBound - southBound);
        }
        else
            xDegrees = (northBound - southBound);

        if(eastBound < 0 && westBound < 0)
        {
            yDegrees = (eastBound - westBound);
        }
        if(eastBound > 0 && westBound > 0)
        {
            yDegrees = (eastBound - westBound);
        }
        if(eastBound > 0 && westBound < 0)
        {
            yDegrees = (eastBound - westBound);
        }
        if(eastBound < 0 && westBound > 0)
        {
            double temp;
            temp = (180 + eastBound);
            temp = (temp + (180 - eastBound));
            yDegrees = temp;
        }
        if(eastBound == 0 || westBound == 0)
        {
            yDegrees = eastBound - westBound;
        }

        double xMeters, yMeters, numCells;
        xMeters = xDegrees*111325;
        yMeters = yDegrees*((cos(((latitude * PI)/180)))*111325);
        numCells = ((xMeters * yMeters)/(currentResolution * currentResolution));
        fileSize = (numCells * 4)/1024; //4 bytes per float

        QString fileSizeMessage;
        if(fileSize > 1024)
        {
            //fileSize = fileSize/1024;
            if((fileSize/1024) > 50)
            {
                fileSizeMessage = 
                    QString("Est. File Size: File Too Large, Adjust Size");
            }
            else
            {
                fileSizeMessage = 
                    QString("Est. File Size: %1mb").arg((int)(fileSize/1024));
            }
        }
        else
        {
            fileSizeMessage = 
                QString("Est. File Size: %1kb").arg((int)fileSize);
        }

        this->lblFileSize->setText(fileSizeMessage);
    }
    else
    {
        this->lblFileSize->setText(QString(""));
    }
}

/**
 * @brief Updates area selected for Est File Size and for downloading a DEM
 *
 * @param selected Whether an area has been selected
 */
void WidgetDownloadDEM::demSelectedUpdate(bool selected)
{
    demSelected = selected;
    estFileSize();
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
