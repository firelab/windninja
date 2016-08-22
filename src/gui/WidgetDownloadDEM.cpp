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
    cbSelectionMethod->setCurrentIndex(1);
    cbSelectionMethod->setCurrentIndex(0);
    this->readSettings();
    updateGUI();
    

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

    /*
     * Hide the 'broken' stuff
     */
    lblGoTo->hide();
    leGoTo->hide();
    tbGoTo->hide();
    gbAdditionalData->hide();

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
    connect(cbSelectionMethod, SIGNAL(currentIndexChanged(int)), this, SLOT(clearListeners()));
    connect(ckbShowExtent, SIGNAL(stateChanged(int)), this, SLOT(displayDEMBounds(int)));
    connect(twAdditionalData, SIGNAL(itemChanged(QTreeWidgetItem*,int)), this, SLOT(showAdditionalData(QTreeWidgetItem*,int)));
    connect(wvGoogleMaps, SIGNAL(linkClicked(const QUrl)), this, SLOT(openGMLinks(const QUrl)));

    connect(leGoTo, SIGNAL(returnPressed()), this, SLOT(geocoder()));
    connect(tbGoTo, SIGNAL(clicked()), this, SLOT(geocoder()));

    connect(stwCoordinateInputs, SIGNAL(currentChanged(int)), this, SLOT(updateGUI()));
    connect(stwBoundCoordInputs, SIGNAL(currentChanged(int)), this, SLOT(updateGUI()));
    connect(wdgSelectionWidgets, SIGNAL(currentChanged(int)), this, SLOT(updateGUI()));

    connect(cbBufferUnits, SIGNAL(currentIndexChanged(int)), this, SLOT(updateBuffer()));
    connect(cbBufferUnits2, SIGNAL(currentIndexChanged(int)), this, SLOT(updateBuffer()));

    
    connect(sbXCoord, SIGNAL(editingFinished()), this, SLOT(plotSettings()));
    connect(sbYCoord, SIGNAL(editingFinished()), this, SLOT(plotSettings()));
    connect(sbXDegrees, SIGNAL(editingFinished()), this, SLOT(plotSettings()));
    connect(sbYDegrees, SIGNAL(editingFinished()), this, SLOT(plotSettings()));
    connect(sbXMinutes, SIGNAL(editingFinished()), this, SLOT(plotSettings()));
    connect(sbYMinutes, SIGNAL(editingFinished()), this, SLOT(plotSettings()));
    connect(sbXDegrees2, SIGNAL(editingFinished()), this, SLOT(plotSettings()));
    connect(sbYDegrees2, SIGNAL(editingFinished()), this, SLOT(plotSettings()));
    connect(sbXMinutes2, SIGNAL(editingFinished()), this, SLOT(plotSettings()));
    connect(sbYMinutes2, SIGNAL(editingFinished()), this, SLOT(plotSettings()));
    connect(sbXSeconds, SIGNAL(editingFinished()), this, SLOT(plotSettings()));
    connect(sbYSeconds, SIGNAL(editingFinished()), this, SLOT(plotSettings()));

    connect(sbNorthDegrees1, SIGNAL(editingFinished()), this, SLOT(plotBox()));
    connect(sbSouthDegrees1, SIGNAL(editingFinished()), this, SLOT(plotBox()));
    connect(sbEastDegrees1, SIGNAL(editingFinished()), this, SLOT(plotBox()));
    connect(sbWestDegrees1, SIGNAL(editingFinished()), this, SLOT(plotBox()));
    connect(sbNorthDegrees2, SIGNAL(editingFinished()), this, SLOT(plotBox()));
    connect(sbSouthDegrees2, SIGNAL(editingFinished()), this, SLOT(plotBox()));
    connect(sbEastDegrees2, SIGNAL(editingFinished()), this, SLOT(plotBox()));
    connect(sbWestDegrees2, SIGNAL(editingFinished()), this, SLOT(plotBox()));
    connect(sbNorthDegrees3, SIGNAL(editingFinished()), this, SLOT(plotBox()));
    connect(sbSouthDegrees3, SIGNAL(editingFinished()), this, SLOT(plotBox()));
    connect(sbEastDegrees3, SIGNAL(editingFinished()), this, SLOT(plotBox()));
    connect(sbWestDegrees3, SIGNAL(editingFinished()), this, SLOT(plotBox()));
    connect(sbNorthMinutes1, SIGNAL(editingFinished()), this, SLOT(plotBox()));
    connect(sbSouthMinutes1, SIGNAL(editingFinished()), this, SLOT(plotBox()));
    connect(sbEastMinutes1, SIGNAL(editingFinished()), this, SLOT(plotBox()));
    connect(sbWestMinutes1, SIGNAL(editingFinished()), this, SLOT(plotBox()));
    connect(sbNorthMinutes2, SIGNAL(editingFinished()), this, SLOT(plotBox()));
    connect(sbSouthMinutes2, SIGNAL(editingFinished()), this, SLOT(plotBox()));
    connect(sbEastMinutes2, SIGNAL(editingFinished()), this, SLOT(plotBox()));
    connect(sbWestMinutes2, SIGNAL(editingFinished()), this, SLOT(plotBox()));
    connect(sbNorthSeconds, SIGNAL(editingFinished()), this, SLOT(plotBox()));
    connect(sbSouthSeconds, SIGNAL(editingFinished()), this, SLOT(plotBox()));
    connect(sbEastSeconds, SIGNAL(editingFinished()), this, SLOT(plotBox()));
    connect(sbWestSeconds, SIGNAL(editingFinished()), this, SLOT(plotBox()));

    connect(sbXBuffer, SIGNAL(editingFinished()), this, SLOT(plotSettings()));
    connect(sbYBuffer, SIGNAL(editingFinished()), this, SLOT(plotSettings()));
    connect(sbXBuffer2, SIGNAL(editingFinished()), this, SLOT(plotUserPoint()));
    connect(sbYBuffer2, SIGNAL(editingFinished()), this, SLOT(plotUserPoint()));

    connect(btnChoosePoint, SIGNAL(clicked()), this, SLOT(choosePoint()));
    connect(btnChooseBox, SIGNAL(clicked()), this, SLOT(chooseBox()));
    connect(btnCloseButton, SIGNAL(clicked()), this, SLOT(closeDEM()));
}

/**
 * @brief Initialize the google maps interface and interface object
 *
 */
void WidgetDownloadDEM::initializeGoogleMapsInterface()
{
    gmInterface = new GoogleMapsInterface();
    this->wvGoogleMaps->page()->mainFrame()->addToJavaScriptWindowObject("GMInterface", gmInterface);
    connect(gmInterface, SIGNAL(latlngChanged(double,double)), this, SLOT(updateLatLng(double,double)));
    connect(gmInterface, SIGNAL(latlngChangedGUI(double,double)), this, SLOT(updateLatLngGUI(double,double)));
    connect(gmInterface, SIGNAL(plotUserPoint()), this, SLOT(plotUserPoint()));
    connect(gmInterface, SIGNAL(areaSelected(bool)), this, SLOT(demSelectedUpdate(bool)));
    connect(gmInterface, SIGNAL(zoomExtents()), this, SLOT(zoomToMidpoint()));
    connect(gmInterface, SIGNAL(boundsChangedGUI(double,double,double,double)), this, SLOT(updateBoundsGUI(double,double,double,double)));
    connect(gmInterface, SIGNAL(boundsChanged(double,double,double,double)), this, SLOT(updateBounds(double,double,double,double)));
    connect(gmInterface, SIGNAL(bufferChanged()), this, SLOT(updateBuffer()));
    connect(gmInterface, SIGNAL(geocodeError()), this, SLOT(geocodeError()));
}

/**
 * @brief Updates the stored latitude and longitude values for a point
 *
 * @param lat latitude
 * @param lng longitude
 */
void WidgetDownloadDEM::updateLatLng(double lat, double lng)
{
    latitude = lat;
    longitude = lng;
}

/**
 * @brief Updates the stored latitude and longitude values for a point and updates the GUI
 *
 * @param lat latitude
 * @param lng longitude
 */
void WidgetDownloadDEM::updateLatLngGUI(double lat, double lng)
{
    latitude = lat;
    longitude = lng;
    
    updateGUI();
}

/**
 * @brief Updates the bounds of the DEM selection box
 *
 * @param north North latitude
 * @param south South latitude
 * @param east East longitude
 * @param west West longitude
 */
void WidgetDownloadDEM::updateBoundsGUI(double north, double south, double east, double west)
{
    if(north < south) 
    {
        double temp = north;
        north = south;
        south = temp;
    }

    northBound = north;
    southBound = south;
    eastBound = east;
    westBound = west;

    updateGUI();
    estFileSize();
}

/**
 * @brief Updates the bounds but does not change the GUI
 *
 * @param north North latitude
 * @param south South latitude
 * @param east East longitude
 * @param west West longitude
 */
void WidgetDownloadDEM::updateBounds(double north, double south, double east, double west)
{
    if(north < south) 
    {
        double temp = north;
        north = south;
        south = temp;
    }

    northBound = north;
    southBound = south;
    eastBound = east;
    westBound = west;

    estFileSize();
}

/**
 * @brief Updates the buffer distances
 *
 */
void WidgetDownloadDEM::updateBuffer()
{
    double xBuffer, yBuffer;
    xBuffer = (northBound - southBound);
    if(eastBound < 0 && westBound < 0)
    {
        yBuffer = (eastBound - westBound);
    }
    else if(eastBound > 0 && westBound > 0)
    {
        yBuffer = (eastBound - westBound);
    }
    else if(eastBound > 0 && westBound < 0)
    {
        yBuffer = (eastBound - westBound);
    }
    else if(eastBound < 0 && westBound > 0)
    {
        double temp;
        temp = (180 + eastBound);
        temp = (temp + (180 - eastBound));
        yBuffer = temp;
    }
    else if(eastBound == 0 && westBound == 0)
    {
        yBuffer = 0;
    }

    int index = wdgSelectionWidgets->currentIndex();

    if(index == 2)
    {
        switch(cbBufferUnits2->currentIndex()){
        case 0:
            sbXBuffer2->setValue(xBuffer * 111325);
            sbYBuffer2->setValue(yBuffer * ((cos(((latitude * PI)/180)))*111325));
            break;
        case 1:
            sbXBuffer2->setValue(xBuffer * 365228);
            sbYBuffer2->setValue(yBuffer * ((cos(((latitude * PI)/180)))*365228));
            break;
        case 2:
            sbXBuffer2->setValue(xBuffer * 111.325);
            sbYBuffer2->setValue(yBuffer * ((cos(((latitude * PI)/180)))*111.325));
            break;
        case 3:
            sbXBuffer2->setValue(xBuffer * 69.172);
            sbYBuffer2->setValue(yBuffer * ((cos(((latitude * PI)/180)))*69.172));
            break;
        }
    }
    else if(index == 3)
    {
        switch(cbBufferUnits->currentIndex()){
        case 0:
            sbXBuffer->setValue(xBuffer * 111325);
            sbYBuffer->setValue(yBuffer * ((cos(((latitude * PI)/180)))*111325));
            break;
        case 1:
            sbXBuffer->setValue(xBuffer * 365228);
            sbYBuffer->setValue(yBuffer * ((cos(((latitude * PI)/180)))*365228));
            break;
        case 2:
            sbXBuffer->setValue(xBuffer * 111.325);
            sbYBuffer->setValue(yBuffer * ((cos(((latitude * PI)/180)))*111.325));
            break;
        case 3:
            sbXBuffer->setValue(xBuffer * 69.172);
            sbYBuffer->setValue(yBuffer * ((cos(((latitude * PI)/180)))*69.172));
            break;
        }
    }
}
        

/**
 * @brief Calls javascript method that allows user to select a box on the map
 *
 */
void WidgetDownloadDEM::chooseBox()
{
    this->wvGoogleMaps->page()->mainFrame()->evaluateJavaScript("clearOverlays(); null");
    this->wvGoogleMaps->page()->mainFrame()->evaluateJavaScript("userBounds();");
}

/**
 * @brief Plots the user designated centroid point with a buffer box around it
 *
 */
void WidgetDownloadDEM::plotUserPoint()
{
    this->wvGoogleMaps->page()->mainFrame()->evaluateJavaScript("clearOverlays(); null");

    double bufferLat;
    double bufferLng;
    convertBuffer(bufferLat, bufferLng);

    if(checkLatLng(latitude, longitude, bufferLat, bufferLng))
    {
        double s = latitude-bufferLat;
        double n = latitude+bufferLat;
        double temp,e,w;

        if(longitude-bufferLng < -180)
        {
            temp = (-180 - longitude);
            temp = (bufferLng + temp);
            w = (180 - temp);
        }
        else
        {
            w = (longitude - bufferLng);
        }
        if(longitude+bufferLng > 180)
        {
            temp = (180 - longitude);
            temp = (bufferLng - temp);
            e = (-180 + temp);
        }
        else
        {
            e = (longitude + bufferLng);
        }
        
        QString args = QString("plotCentroidBuffer(%1,%2,%3,%4,%5,%6);").arg(s).arg(w).arg(n).arg(e).arg(latitude).arg(longitude);
        this->wvGoogleMaps->page()->currentFrame()->documentElement().evaluateJavaScript(args);
        this->wvGoogleMaps->page()->currentFrame()->documentElement().evaluateJavaScript("clearListeners();");
        zoomToMidpoint();
        demSelectedUpdate(true);   
    }
    else
    {
        if(latlngError)
        {
            return;
        }
        else
        {
            latlngError = new QMessageBox(this);
            latlngError->setText("An Error Occured: Please Check Your Latitude and Longitude");
            progressBar->setRange(0,1);
            progressBar->setValue( 0 );
            latlngError->setIcon(QMessageBox::Warning);
            if(latlngError->exec())
            {
                updateGUI();
                delete latlngError;
                latlngError = NULL;
            }  
        }
    }
}
/**
 * @brief Plots the current GUI settings on the map including the point and the buffer box around it 
 *
 */
void WidgetDownloadDEM::plotSettings()
{
    /*
    bool focus;
    QObject *name = new QObject();
    name = QObject::sender();
    QString objName = name->objectName();
    QDoubleSpinBox *sender = qobject_cast<QDoubleSpinBox *>(QObject::sender());
    if(sender)
    {
        focus = sender->hasFocus();
    }
    else
    {
        QSpinBox *sender = qobject_cast<QSpinBox *>(QObject::sender());
        if(sender)
        {
            focus = sender->hasFocus();
        }
    }
    */
        
    this->wvGoogleMaps->page()->mainFrame()->evaluateJavaScript("clearOverlays(); null");

    double tempLat, tempLng;
    convertLatLng(tempLat, tempLng);

    double bufferLat;
    double bufferLng;
    convertBuffer(bufferLat, bufferLng);

    if(checkLatLng(tempLat, tempLng, bufferLat, bufferLng))
    {
        double s = latitude-bufferLat;
        double n = latitude+bufferLat;
        double temp,e,w;

        if(longitude-bufferLng < -180)
        {
            temp = (-180 - longitude);
            temp = (bufferLng + temp);
            w = (180 - temp);
        }
        else
        {
            w = (longitude - bufferLng);
        }
        if(longitude+bufferLng > 180)
        {
            temp = (180 - longitude);
            temp = (bufferLng - temp);
            e = (-180 + temp);
        }
        else
        {
            e = (longitude + bufferLng);
        }
        
        QString args = QString("plotCentroidBuffer(%1,%2,%3,%4,%5,%6);").arg(s).arg(w).arg(n).arg(e).arg(latitude).arg(longitude);
        this->wvGoogleMaps->page()->currentFrame()->documentElement().evaluateJavaScript(args);
        this->wvGoogleMaps->page()->currentFrame()->documentElement().evaluateJavaScript("clearListeners();");
        zoomToMidpoint();
        demSelectedUpdate(true);   
    }
    else
    {
        if(bufferError)
        {
            return;
        }
        else
        {
            bufferError = new QMessageBox(this);
            bufferError->setText("An Error Occured: Please Check Your Bounds, Latitude and Longitude");
            progressBar->setRange(0,1);
            progressBar->setValue( 0 );
            bufferError->setIcon(QMessageBox::Warning);
            if(bufferError->exec())
            {
                updateGUI();
                delete bufferError;
                bufferError = NULL;
            }  
        }
    }
    
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
 * @brief Allows the user to choose a point on the map
 *
 */
void WidgetDownloadDEM::choosePoint()
{
    this->wvGoogleMaps->page()->mainFrame()->evaluateJavaScript("clearOverlays(); null");
    this->wvGoogleMaps->page()->mainFrame()->evaluateJavaScript("userPoint();");
}

/**
 * @brief Plots the current bounds settings on the map
 *
 */
void WidgetDownloadDEM::plotBox()
{
    /*
    bool focus;
    QObject *name = new QObject();
    name = QObject::sender();
    QString objName = name->objectName();
    QDoubleSpinBox *sender = qobject_cast<QDoubleSpinBox *>(QObject::sender());
    if(sender)
    {
        focus = sender->hasFocus();
    }
    else
    {
        QSpinBox *sender = qobject_cast<QSpinBox *>(QObject::sender());
        if(sender)
        {
            focus = sender->hasFocus();
        }
    }
    */

    this->wvGoogleMaps->page()->mainFrame()->evaluateJavaScript("clearOverlays(); null");

    double tempNorth, tempSouth, tempEast, tempWest;
    convertBounds(tempNorth, tempSouth, tempEast, tempWest);

    if(checkBounds(tempNorth, tempSouth, tempEast, tempWest))
    {
        QString args = QString("plotBox(%1,%2,%3,%4);").arg(southBound).arg(westBound).arg(northBound).arg(eastBound);
        this->wvGoogleMaps->page()->currentFrame()->documentElement().evaluateJavaScript(args);
        zoomToMidpoint();
        //demSelectedUpdate(true);
    }
    else
    {
        if(boundsError)
        {
            return;
        }
        else
        {
            boundsError = new QMessageBox(this);
            boundsError->setText("An Error Occured: Please Check Your Bounds");
            progressBar->setRange(0,1);
            progressBar->setValue( 0 );
            boundsError->setIcon(QMessageBox::Warning);
            if(boundsError->exec())
            {
                updateGUI();
                delete boundsError;
                boundsError = NULL;
            }  
        } 
    }
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
}

/**
 * @brief Converts the buffer units into degrees for plotting the buffer
 *
 * @param &lat Passes the latitude value of the buffer by reference
 * @param &lng Passes the longitude value of the buffer by reference
 */
void WidgetDownloadDEM::convertBuffer(double &lat, double &lng)
{
    double xBuf = this->sbXBuffer->value();
    double yBuf = this->sbYBuffer->value();
    double xBuf2 = this->sbXBuffer2->value();
    double yBuf2 = this->sbYBuffer2->value();

    switch(wdgSelectionWidgets->currentIndex()) {
    case 2:
        if(cbBufferUnits2->currentText() == "Meters")
        {
            lat = (xBuf2 / 111325)/2;
            lng = (yBuf2 / ((cos(((latitude * PI)/180)))*111325))/2;
        }
        else if(cbBufferUnits2->currentText() == "Kilometers")
        {
            lat = (xBuf2 / 111.325)/2;
            lng = (yBuf2 / ((cos(((latitude * PI)/180)))*111.325))/2;
        }
        else if(cbBufferUnits2->currentText() == "Miles")
        {
            lat = (xBuf2 / 69.172)/2;
            lng = (yBuf2 / ((cos(((latitude * PI)/180)))*69.172))/2;
        }
        else if(cbBufferUnits2->currentText() == "Feet")
        {
            lat = (xBuf2 / 365228)/2;
            lng = (yBuf2 / ((cos(((latitude * PI)/180)))*365228))/2;
        }
        break;
    case 3:
        if(cbBufferUnits->currentText() == "Meters")
        {
            lat = (xBuf / 111325)/2;
            lng = (yBuf / ((cos(((latitude * PI)/180)))*111325))/2;
        }
        else if(cbBufferUnits->currentText() == "Kilometers")
        {
            lat = (xBuf / 111.325)/2;
            lng = (yBuf / ((cos(((latitude * PI)/180)))*111.325))/2;
        }
        else if(cbBufferUnits->currentText() == "Miles")
        {
            lat = (xBuf / 69.172)/2;
            lng = (yBuf / ((cos(((latitude * PI)/180)))*69.172))/2;
        }
        else if(cbBufferUnits->currentText() == "Feet")
        {
            lat = (xBuf / 365228)/2;
            lng = (yBuf / ((cos(((latitude * PI)/180)))*365228))/2;
        }
        break;
    }

}

/**
 * @brief Converts the current bounds settings into degree format
 *
 * @param &north Converts the north bound and passes it by reference
 * @param &south Converts the south bound and passes it by reference
 * @param &east Converts the east bound and passes it by reference
 * @param &west Converts the west bound and passes it by reference
 */
void WidgetDownloadDEM::convertBounds(double &north, double &south, double &east, double &west)
{
    if(stwBoundCoordInputs->currentIndex() == 0)
    {
        north = sbNorthDegrees1->value();
        south = sbSouthDegrees1->value();
        east = sbEastDegrees1->value();
        west = sbWestDegrees1->value();
    }
    else if(stwBoundCoordInputs->currentIndex() == 1)
    {
        if(sbNorthDegrees2->value() < 0)
        {
            north = (double)sbNorthDegrees2->value() - (sbNorthMinutes1->value() / 60);
        }
        else if(sbNorthDegrees2->value() >= 0)
        {
            north = (double)sbNorthDegrees2->value() + (sbNorthMinutes1->value() / 60);
        }

        if(sbSouthDegrees2->value() < 0)
        {
            south = (double)sbSouthDegrees2->value() - (sbSouthMinutes1->value() / 60);
        }
        else if(sbSouthDegrees2->value() >= 0)
        {
            south = (double)sbSouthDegrees2->value() + (sbSouthMinutes1->value() / 60);
        }

        if(sbEastDegrees2->value() < 0)
        {
            east = (double)sbEastDegrees2->value() - (sbEastMinutes1->value() / 60);
        }
        else if(sbEastDegrees2->value() >= 0)
        {
            east = (double)sbEastDegrees2->value() + (sbEastMinutes1->value() / 60);
        }

        if(sbWestDegrees2->value() < 0)
        {
            west = (double)sbWestDegrees2->value() - (sbWestMinutes1->value() / 60);
        }
        else if(sbWestDegrees2->value() >= 0)
        {  
            west = (double)sbWestDegrees2->value() + (sbWestMinutes1->value() / 60);
        }
    }
    else if(stwBoundCoordInputs->currentIndex() == 2)
    {
        if(sbNorthDegrees3->value() < 0)
        {
            north = (double)sbNorthDegrees3->value() - (((double)sbNorthMinutes2->value() + (sbNorthSeconds->value() / 60)) / 60);
        }
        else
        {
            north = (double)sbNorthDegrees3->value() + (((double)sbNorthMinutes2->value() + (sbNorthSeconds->value() / 60)) / 60);
        }
        if(sbSouthDegrees3->value() < 0)
        {
            south = (double)sbSouthDegrees3->value() - (((double)sbSouthMinutes2->value() + (sbSouthSeconds->value() / 60)) / 60);
        }
        else
        {
            south = (double)sbSouthDegrees3->value() + (((double)sbSouthMinutes2->value() + (sbSouthSeconds->value() / 60)) / 60);
        }
        if(sbEastDegrees3->value() < 0)
        {
            east = (double)sbEastDegrees3->value() - (((double)sbEastMinutes2->value() + (sbEastSeconds->value() / 60)) / 60);
        }
        else
        {
            east = (double)sbEastDegrees3->value() + (((double)sbEastMinutes2->value() + (sbEastSeconds->value() / 60)) / 60);
        }
        if(sbWestDegrees3->value() < 0)
        {
            west = (double)sbWestDegrees3->value() - (((double)sbWestMinutes2->value() + (sbWestSeconds->value() / 60)) / 60);
        }
        else
        {  
            west = (double)sbWestDegrees3->value() + (((double)sbWestMinutes2->value() + (sbWestSeconds->value() / 60)) / 60);
        }
    }
}

/**
 * @brief Checks the bounds and fixes them if they get reversed, this is due to a Google Maps API v3 issue that sometimes reverses the bounds
 *
 */
bool WidgetDownloadDEM::checkBounds(double north, double south, double east, double west)
{
    if(south > north)
    {
        return false;
    }
    if(west > east)
    {
        return false;
    }
    else
    {
        northBound = north;
        southBound = south;
        eastBound = east;
        westBound = west;
        return true;
    }
}

/**
 * @brief Converts the latitude and longitude values on the GUI into degrees format
 *
 * @param &lat Converts latitude to degrees and passes by reference
 * @param &lng Converts longitude to degrees and passes by reference
 */
void WidgetDownloadDEM::convertLatLng(double &lat, double &lng)
{
    if(this->stwCoordinateInputs->currentIndex() == 0)
    {
        lat = sbXCoord->value();
        lng = sbYCoord->value();
    }
    else if(this->stwCoordinateInputs->currentIndex() == 1)
    {
        if(sbXDegrees->value() < 0)
        {
            lat = (double)sbXDegrees->value() - (sbXMinutes->value() / 60);
        }
        else if(sbXDegrees->value() >= 0)
        {
            lat = (double)sbXDegrees->value() + (sbXMinutes->value() / 60);
        }

        if(sbYDegrees->value() < 0)
        {
            lng = (double)sbYDegrees->value() - (sbYMinutes->value() / 60);
        }
        else if(sbYDegrees->value() >= 0)
        {
            lng = (double)sbYDegrees->value() + (sbYMinutes->value() / 60);
        }
    }
    else if(this->stwCoordinateInputs->currentIndex() == 2)
    {
        if(sbXDegrees2->value() < 0)
        {
            lat = (double)sbXDegrees2->value() - (((double)sbXMinutes2->value() + (sbXSeconds->value() / 60)) / 60);
        }
        else if(sbXDegrees2->value() >= 0)
        {
            lat = (double)sbXDegrees2->value() + (((double)sbXMinutes2->value() + (sbXSeconds->value() / 60)) / 60);
        }
        if(sbYDegrees2->value() < 0)
        {
            lng = (double)sbYDegrees2->value() - (((double)sbYMinutes2->value() + (sbYSeconds->value() / 60)) / 60);
        }
        else if(sbYDegrees2->value() >= 0)
        {
            lng = (double)sbYDegrees2->value() + (((double)sbYMinutes2->value() + (sbYSeconds->value() / 60)) / 60);
        }
    }
}

/**
 * @brief Checks the latitiude and longitude values
 *
 * @param &lat Checks the latitude value and passes it back by reference if needed
 * @param &lng Checks the longitude value and passes it back by reference if needed
 * @param bufLat Uses the latitude buffer for checking
 * @param bufLng Uses the longitude buffer for checking
 */
bool WidgetDownloadDEM::checkLatLng(double &lat, double &lng, double bufLat, double bufLng)
{
    if((lat + bufLat) > 90 || (lat - bufLat) < -90 || lat > 90 || lat < -90)
    {
        return false;
    }
    if(lng > 180 || lng < -180)
    {
        return false;
    }
    else 
    {
        latitude = lat;
        longitude = lng;
        return true;
    }
}

/**
 * @brief Updates stwCoordinateInputs
 *
 * @param index Current index of STWCoordinateInputs
 */
void WidgetDownloadDEM::updateSTWCoordinateInputs()
{
    double decPartLat, decPartLng, minPartLat, minPartLng, secPartLat, secPartLng, minPartLat2, minPartLng2;

    switch(stwCoordinateInputs->currentIndex()){
    case 0:
        sbXCoord->setValue(latitude);
        sbYCoord->setValue(longitude);
        break;
    case 1:
        minPartLat = modf(latitude, &decPartLat);
        minPartLng = modf(longitude, &decPartLng);
        sbXDegrees->setValue((int)decPartLat);
        sbYDegrees->setValue((int)decPartLng);
        if(minPartLat < 0)
        {
            minPartLat *= -1;
        }
        if(minPartLng < 0)
        {
            minPartLng *= -1;
        }
        sbXMinutes->setValue((minPartLat * 60));
        sbYMinutes->setValue((minPartLng * 60));
        break;
    case 2:
        minPartLat = modf(latitude, &decPartLat);
        minPartLng = modf(longitude, &decPartLng);
        sbXDegrees2->setValue((int)decPartLat);
        sbYDegrees2->setValue((int)decPartLng);
        if(minPartLat < 0)
        {
            minPartLat *= -1;
        }
        if(minPartLng < 0)
        {
            minPartLng *= -1;
        } 
        minPartLat *= 60;
        minPartLng *= 60;
        secPartLat = modf(minPartLat, &minPartLat2);
        secPartLng = modf(minPartLng, &minPartLng2);
        sbXMinutes2->setValue((int)minPartLat2);
        sbYMinutes2->setValue((int)minPartLng2);
        if(secPartLat < 0)
        {
            secPartLat *= -1;
        }
        if(secPartLng < 0)
        {
            secPartLng *= -1;
        }
        secPartLat *= 60;
        secPartLng *= 60;
        if(secPartLat < 0.001)
        {
            secPartLat = 0.000;
        }
        if(secPartLat < 60 && secPartLat > 59.9990)
        {
            secPartLat = 59.999;
        }

        if(secPartLng < 0.001)
        {
            secPartLng = 0.000;
        }
        if(secPartLng < 60 && secPartLng > 59.9990)
        {
            secPartLng = 59.999;
        }
        sbXSeconds->setValue(secPartLat);
        sbYSeconds->setValue(secPartLng);
        break;
    }
}

/**
 * @brief Updates stwBoundCoordInputs
 *
 * @param index Current index of STWBoundCoordInputs
 */
void WidgetDownloadDEM::updateSTWBoundCoordInputs()
{
    double decPart, minPart, secPart;

    switch(stwBoundCoordInputs->currentIndex()){
    case 0:
        sbNorthDegrees1->setValue(northBound);
        sbSouthDegrees1->setValue(southBound);
        sbEastDegrees1->setValue(eastBound);
        sbWestDegrees1->setValue(westBound);
        break;
    case 1:
        minPart = modf(northBound, &decPart);
        sbNorthDegrees2->setValue((int)decPart);
        if(minPart < 0)
        {
            minPart *= -1;
        }
        sbNorthMinutes1->setValue((minPart * 60));

        minPart = modf(southBound, &decPart);
        sbSouthDegrees2->setValue((int)decPart);
        if(minPart < 0)
        {
            minPart *= -1;
        }
        sbSouthMinutes1->setValue((minPart * 60));

        minPart = modf(eastBound, &decPart);
        sbEastDegrees2->setValue((int)decPart);
        if(minPart < 0)
        {
            minPart *= -1;
        }
        sbEastMinutes1->setValue((minPart * 60));

        minPart = modf(westBound, &decPart);
        sbWestDegrees2->setValue((int)decPart);
        if(minPart < 0)
        {
            minPart *= -1;
        }
        sbWestMinutes1->setValue((minPart * 60));
        break;
    case 2:
        minPart = modf(northBound, &decPart);
        sbNorthDegrees3->setValue((int)decPart);
        secPart = modf((minPart*60), &minPart);
        if(minPart < 0)
        {
            minPart *= -1;
        }
        sbNorthMinutes2->setValue((int)minPart);
        if(secPart < 0)
        {
            secPart *= -1;
        }
        secPart *= 60;
        if(secPart < 0.001)
        {
            secPart = 0.000;
        }
        if(secPart < 60 && secPart > 59.9990)
        {
            secPart = 59.999;
        }
        sbNorthSeconds->setValue(secPart);

        minPart = modf(southBound, &decPart);
        sbSouthDegrees3->setValue((int)decPart);
        minPart *= 60;
        secPart = modf(minPart, &minPart);
        if(minPart < 0)
        {
            minPart *= -1;
        }
        sbSouthMinutes2->setValue((int)minPart);
        if(secPart < 0)
        {
            secPart *= -1;
        }
        secPart *= 60;
        if(secPart < 0.001)
        {
            secPart = 0.000;
        }
        if(secPart < 60 && secPart > 59.9990)
        {
            secPart = 59.999;
        }
        sbSouthSeconds->setValue(secPart);

        minPart = modf(eastBound, &decPart);
        sbEastDegrees3->setValue((int)decPart);
        secPart = modf((minPart*60), &minPart);
        if(minPart < 0)
        {
            minPart *= -1;
        }
        sbEastMinutes2->setValue((int)minPart);
        if(secPart < 0)
        {
            secPart *= -1;
        }
        secPart *= 60;
        if(secPart < 0.001)
        {
            secPart = 0.000;
        }
        if(secPart < 60 && secPart > 59.9990)
        {
            secPart = 59.999;
        }
        sbEastSeconds->setValue(secPart);

        minPart = modf(westBound, &decPart);
        sbWestDegrees3->setValue((int)decPart);
        secPart = modf((minPart*60), &minPart);
        if(minPart < 0)
        {
            minPart *= -1;
        }
        sbWestMinutes2->setValue((int)minPart);
        if(secPart < 0)
        {
            secPart *= -1;
        }
        secPart *= 60;
        if(secPart < 0.001)
        {
            secPart = 0.000;
        }
        if(secPart < 60 && secPart > 59.9990)
        {
            secPart = 59.999;
        }
        sbWestSeconds->setValue(secPart);
        break;
    }
}

/**
 * @brief Download and save the current DEM area
 *
 */
void WidgetDownloadDEM::saveDEM()
{
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
 * @brief Opens Google Maps links in default browser
 *
 */
void WidgetDownloadDEM::openGMLinks(const QUrl url)
{
    QString name = url.toString();
    bool r = QDesktopServices::openUrl(QUrl::fromUserInput(name));
}

/**
 * @brief Geocoder lookup
 *
 */
void WidgetDownloadDEM::geocoder()
{
    QString address = leGoTo->text();
    QString geocode = QString("geocode('%1');").arg(address);
    wvGoogleMaps->page()->mainFrame()->evaluateJavaScript(geocode);
}

/**
 * @brief Geocoder error message
 *
 */
void WidgetDownloadDEM::geocodeError()
{
    QMessageBox geocoderError;
    geocoderError.setText("Error: Unable to locate.");
    geocoderError.exec();
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
    //Saves Selection Method to settings
    settings.setValue("selectionMethod", cbSelectionMethod->currentIndex());
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
    if(settings.contains("selectionMethod"))
    {
        cbSelectionMethod->setCurrentIndex(settings.value("selectionMethod").toInt());
        wdgSelectionWidgets->setCurrentIndex(settings.value("selectionMethod").toInt());
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

/**
 * @brief Updates the portion of the GUI currently being viewed
 *
 */
void WidgetDownloadDEM::updateGUI()
{
    switch(wdgSelectionWidgets->currentIndex())
    {
        case 0:
            break;
        case 1:
            updateSTWBoundCoordInputs();
            break;
        case 2:
            updateBuffer();
            break;
        case 3:
            updateSTWCoordinateInputs();
            updateBuffer();
            break;
    }
}
