/******************************************************************************
 *
 * $Id: stationFetchWidget.cpp 1757 2012-08-07 18:40:40Z kyle.shannon $
 *
 * Project:  WindNinja
 * Purpose:  stationFetchWidget
 * Author:   tfinney@fs.fed.us
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
 
#include "stationFetchWidget.h"
//#include <vld.h>

stationFetchWidget::stationFetchWidget(QWidget *parent)
    : QWidget(parent)
{    
    setupUi(this);
    connectInputs();
    fixTime();
    this->setWindowFlags(Qt::CustomizeWindowHint | Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint);
    this->show();
}

/**
 * @brief Deletes all allocated memory on destruction of class object
 *
 */
stationFetchWidget::~stationFetchWidget()
{
//    delete progressBar;
//    delete fetcher;
//    delete latlngError;
//    delete bufferError;
//    delete boundsError;
//    delete gmInterface;
}
void stationFetchWidget::fixTime()
{
    startEdit->setDateTime( QDateTime::currentDateTime().addDays(-1) );
    endEdit->setDateTime(QDateTime::currentDateTime());
    
    startEdit->setMaximumDateTime(QDateTime::currentDateTime().addSecs(-3600));
    endEdit->setMaximumDateTime(QDateTime::currentDateTime());
    
    startEdit->setDisplayFormat( "MM/dd/yyyy HH:mm" );
    endEdit->setDisplayFormat( "MM/dd/yyyy HH:mm" );
    
}

/**
 * @brief Connect all SLOTS and SIGNALS in the Qt GUI
 *
 */
void stationFetchWidget::connectInputs()
{
    connect(fetchMetaButton, SIGNAL(clicked()),this, SLOT(getMetadata()));
    connect(fetchDataButton, SIGNAL(clicked()),this, SLOT(fetchStation()));
    connect(endEdit,SIGNAL(dateTimeChanged(QDateTime)),this,SLOT(watchTime()));
}

//    connect(btnDownloadDEM, SIGNAL(clicked()), this, SLOT(saveDEM()));
//    connect(cbDEMSource, SIGNAL(currentIndexChanged(int)), this, SLOT(updateDEMSource(int)));
//    connect(cbSelectionMethod, SIGNAL(currentIndexChanged(int)), this, SLOT(clearListeners()));
//    connect(ckbShowExtent, SIGNAL(stateChanged(int)), this, SLOT(displayDEMBounds(int)));
//    connect(twAdditionalData, SIGNAL(itemChanged(QTreeWidgetItem*,int)), this, SLOT(showAdditionalData(QTreeWidgetItem*,int)));
//    connect(wvGoogleMaps, SIGNAL(linkClicked(const QUrl)), this, SLOT(openGMLinks(const QUrl)));

//    connect(leGoTo, SIGNAL(returnPressed()), this, SLOT(geocoder()));
//    connect(tbGoTo, SIGNAL(clicked()), this, SLOT(geocoder()));

//    connect(stwCoordinateInputs, SIGNAL(currentChanged(int)), this, SLOT(updateGUI()));
//    connect(stwBoundCoordInputs, SIGNAL(currentChanged(int)), this, SLOT(updateGUI()));
//    connect(wdgSelectionWidgets, SIGNAL(currentChanged(int)), this, SLOT(updateGUI()));

//    connect(cbBufferUnits, SIGNAL(currentIndexChanged(int)), this, SLOT(updateBuffer()));
//    connect(cbBufferUnits2, SIGNAL(currentIndexChanged(int)), this, SLOT(updateBuffer()));

    
//    connect(sbXCoord, SIGNAL(editingFinished()), this, SLOT(plotSettings()));
//    connect(sbYCoord, SIGNAL(editingFinished()), this, SLOT(plotSettings()));
//    connect(sbXDegrees, SIGNAL(editingFinished()), this, SLOT(plotSettings()));
//    connect(sbYDegrees, SIGNAL(editingFinished()), this, SLOT(plotSettings()));
//    connect(sbXMinutes, SIGNAL(editingFinished()), this, SLOT(plotSettings()));
//    connect(sbYMinutes, SIGNAL(editingFinished()), this, SLOT(plotSettings()));
//    connect(sbXDegrees2, SIGNAL(editingFinished()), this, SLOT(plotSettings()));
//    connect(sbYDegrees2, SIGNAL(editingFinished()), this, SLOT(plotSettings()));
//    connect(sbXMinutes2, SIGNAL(editingFinished()), this, SLOT(plotSettings()));
//    connect(sbYMinutes2, SIGNAL(editingFinished()), this, SLOT(plotSettings()));
//    connect(sbXSeconds, SIGNAL(editingFinished()), this, SLOT(plotSettings()));
//    connect(sbYSeconds, SIGNAL(editingFinished()), this, SLOT(plotSettings()));

//    connect(sbNorthDegrees1, SIGNAL(editingFinished()), this, SLOT(plotBox()));
//    connect(sbSouthDegrees1, SIGNAL(editingFinished()), this, SLOT(plotBox()));
//    connect(sbEastDegrees1, SIGNAL(editingFinished()), this, SLOT(plotBox()));
//    connect(sbWestDegrees1, SIGNAL(editingFinished()), this, SLOT(plotBox()));
//    connect(sbNorthDegrees2, SIGNAL(editingFinished()), this, SLOT(plotBox()));
//    connect(sbSouthDegrees2, SIGNAL(editingFinished()), this, SLOT(plotBox()));
//    connect(sbEastDegrees2, SIGNAL(editingFinished()), this, SLOT(plotBox()));
//    connect(sbWestDegrees2, SIGNAL(editingFinished()), this, SLOT(plotBox()));
//    connect(sbNorthDegrees3, SIGNAL(editingFinished()), this, SLOT(plotBox()));
//    connect(sbSouthDegrees3, SIGNAL(editingFinished()), this, SLOT(plotBox()));
//    connect(sbEastDegrees3, SIGNAL(editingFinished()), this, SLOT(plotBox()));
//    connect(sbWestDegrees3, SIGNAL(editingFinished()), this, SLOT(plotBox()));
//    connect(sbNorthMinutes1, SIGNAL(editingFinished()), this, SLOT(plotBox()));
//    connect(sbSouthMinutes1, SIGNAL(editingFinished()), this, SLOT(plotBox()));
//    connect(sbEastMinutes1, SIGNAL(editingFinished()), this, SLOT(plotBox()));
//    connect(sbWestMinutes1, SIGNAL(editingFinished()), this, SLOT(plotBox()));
//    connect(sbNorthMinutes2, SIGNAL(editingFinished()), this, SLOT(plotBox()));
//    connect(sbSouthMinutes2, SIGNAL(editingFinished()), this, SLOT(plotBox()));
//    connect(sbEastMinutes2, SIGNAL(editingFinished()), this, SLOT(plotBox()));
//    connect(sbWestMinutes2, SIGNAL(editingFinished()), this, SLOT(plotBox()));
//    connect(sbNorthSeconds, SIGNAL(editingFinished()), this, SLOT(plotBox()));
//    connect(sbSouthSeconds, SIGNAL(editingFinished()), this, SLOT(plotBox()));
//    connect(sbEastSeconds, SIGNAL(editingFinished()), this, SLOT(plotBox()));
//    connect(sbWestSeconds, SIGNAL(editingFinished()), this, SLOT(plotBox()));

//    connect(sbXBuffer, SIGNAL(editingFinished()), this, SLOT(plotSettings()));
//    connect(sbYBuffer, SIGNAL(editingFinished()), this, SLOT(plotSettings()));
//    connect(sbXBuffer2, SIGNAL(editingFinished()), this, SLOT(plotUserPoint()));
//    connect(sbYBuffer2, SIGNAL(editingFinished()), this, SLOT(plotUserPoint()));

//    connect(btnChoosePoint, SIGNAL(clicked()), this, SLOT(choosePoint()));
//    connect(btnChooseBox, SIGNAL(clicked()), this, SLOT(chooseBox()));
//    connect(btnCloseButton, SIGNAL(clicked()), this, SLOT(closeDEM()));
//}
void stationFetchWidget::updatetz(QString tz)
{
    tzString = tz;
}
void stationFetchWidget::setInputFile(QString file)
{
    demFileName = file;    
}

void stationFetchWidget::watchTime()
{
//    cout<<"end TIME CHANGED!"<<endl;
//    endEdit->dateTime()
    startEdit->setMaximumDateTime(endEdit->dateTime().addSecs(-3600));    
    if (endEdit->dateTime()<startEdit->dateTime())
    {
        cout<<"start is bigger than end! Fixing..."<<endl;
        startEdit->setDateTime(endEdit->dateTime().addSecs(-3600));
    }
}

void stationFetchWidget::updateGeoFetch()
{
    
}

void stationFetchWidget::updateTimeFetch()
{
    
}

std::string stationFetchWidget::removeWhiteSpace(std::string str)
{
    std::string tofind=" ";
    std::string toreplace="";
    size_t position = 0;
    for ( position = str.find(tofind); position != std::string::npos; position = str.find(tofind,position) )
    {
            str.replace(position ,1, toreplace);
    }
    return(str);
    
}

std::string stationFetchWidget::demButcher()
{
    std::string demRaw = demFileName.toStdString();
    size_t lastDot=demRaw.find_last_of("/");
    if (lastDot==std::string::npos)
    {
        return demRaw;
    }
    std::string demBetter=demRaw.substr(0,lastDot)+"/";
    return demBetter;    
}

void stationFetchWidget::fetchStation()
{
    cout<<"Fetch Station Func"<<endl;
    cout<<"---------------------------------------"<<endl;
    cout<<demFileName.toStdString()<<endl;
    cout<<tzString.toStdString()<<endl;
    cout<<"geoLoc: "<<geoLoc->currentIndex()<<endl;
    cout<<"TimeLoc: "<<timeLoc->currentIndex()<<endl;
    cout<<"---------------------------------------"<<endl;
    std::string stid;
    double buffer;
    std::string bufferUnits;
    bool fetchNow;
    std::string blank="blank";
    std::string demUse=demButcher();
    std::string stationPathName;
    cout<<demFileName.toStdString()<<endl;
    cout<<demUse<<endl;
    
    int terrainPart=geoLoc->currentIndex();
    int timePart=timeLoc->currentIndex();
    
//    if (geoLoc->currentIndex()==0)
//    {
//        cout<<"DEM VALUES"<<endl;
//        cout<<bufferSpin->text().toDouble()<<endl;
//        cout<<buffUnits->currentText().toStdString()<<endl;
//    }
//    if (geoLoc->currentIndex()==1)
//    {
//        cout<<"STID VALUES"<<endl;
//        cout<<idLine->text().toStdString()<<endl;
//    }
    
//    if(timeLoc->currentIndex()==0)
//    {
//        cout<<"Current Values"<<endl;
//        cout<<currentBox->isChecked()<<endl;
//    }
//    if(timeLoc->currentIndex()==1)
//    {
//        cout<<"Time Series Values"<<endl;
//        cout<<startEdit->text().toStdString()<<endl;
//        cout<<endEdit->text().toStdString()<<endl;
//    }
    std::vector<boost::posix_time::ptime> eTimeList;
    boost::posix_time::ptime noTime;
    eTimeList.push_back(noTime); // Create an Empty Time list for options that don't need it.
    bool result;
    
    
//    pointInitialization::SetRawStationFilename(demUse);
    //This is a little different than in the CLI, because there is on option for a custom output path
    //Instead, the "raw File", demFileName is the dem file, like it would be in the LCI
    //and then demUse, which the is just the path to the dem acts as the output path
    //So the directory storing the weather csvs is always the same level as the DEM
    stationPathName=pointInitialization::generatePointDirectory(demFileName.toStdString(),demUse,eTimeList,true);  //As we keep working on the GUI, need to get change eTimeList to timeList for timeseries
    pointInitialization::SetRawStationFilename(stationPathName);              
    
    
    // This means DEM and Current Data
    if (terrainPart==0 && timePart==0)
    {
        cout<<"DEM and Current Data"<<endl;
        buffer=bufferSpin->text().toDouble();
        bufferUnits=bufferSpin->text().toStdString();
        fetchNow=true;        
        
//        cout<<bufferSpin->text().toDouble()<<endl;
//        cout<<buffUnits->currentText().toStdString()<<endl;
//        cout<<currentBox->isChecked()<<endl;   
        
        result = pointInitialization::fetchStationFromBbox(demFileName.toStdString(),eTimeList,tzString.toStdString(),fetchNow);
//        pointInitialization::writeStationLocationFile(stationPathName,demFileName.toStdString());
        cout<<"Return: "<<result<<endl;
    }
    if (terrainPart==0 && timePart==1)
    {
        cout<<"DEM and Timeseries"<<endl;
        buffer=bufferSpin->text().toDouble();
        bufferUnits=bufferSpin->text().toStdString();
        fetchNow=false;
        
        int sY,sMo,sD,sH,sMi;
        int eY,eMo,eD,eH,eMi;
        int numSteps=10; //make up a number for now.... It really doesn't matter at this point
        
        std::string StartTime=startEdit->text().toStdString();
        std::string EndTime=endEdit->text().toStdString();
                
        istringstream(StartTime.substr(0,2))>>sMo;
        istringstream(StartTime.substr(3,2))>>sD;
        istringstream(StartTime.substr(6,4))>>sY;
        istringstream(StartTime.substr(11,2))>>sH;
        istringstream(StartTime.substr(14,2))>>sMi;

        istringstream(EndTime.substr(0,2))>>eMo;
        istringstream(EndTime.substr(3,2))>>eD;
        istringstream(EndTime.substr(6,4))>>eY;
        istringstream(EndTime.substr(11,2))>>eH;
        istringstream(EndTime.substr(14,2))>>eMi;
        

//        cout<<sY<<sMo<<sD<<sH<<sMi<<endl;
//        cout<<eY<<eMo<<eD<<eH<<eMi<<endl;
        std::vector<boost::posix_time::ptime> timeList;
        timeList=pointInitialization::getTimeList(sY,sMo,sD,sH,sMi,eY,eMo,eD,eH,eMi,
                                                  numSteps,tzString.toStdString());
        cout<<timeList.size()<<endl;
        result = pointInitialization::fetchStationFromBbox(demFileName.toStdString(),timeList,
                                                           tzString.toStdString(),false);
//        pointInitialization::writeStationLocationFile(stationPathName,demFileName.toStdString());

        
        cout<<"Return: "<<result<<endl;
//        cout<<bufferSpin->text().toDouble()<<endl;
//        cout<<buffUnits->currentText().toStdString()<<endl;
//        cout<<startEdit->text().toStdString()<<endl;
//        cout<<endEdit->text().toStdString()<<endl;
        
        
    }
    if (terrainPart==1 && timePart==0)
    {
        cout<<"STID and Current DATA"<<endl;
        stid=removeWhiteSpace(idLine->text().toStdString());
        fetchNow=true;
        
        result = pointInitialization::fetchStationByName(stid,eTimeList,tzString.toStdString(),fetchNow);
//        pointInitialization::writeStationLocationFile(stationPathName,demFileName.toStdString());
        cout<<"Return: "<<result<<endl;       
        
//        cout<<stid<<endl;        
//        cout<<currentBox->isChecked()<<endl;        
        
    }
    if (terrainPart==1 && timePart==1)
    {
        cout<<"STID and Time series"<<endl;
        stid=removeWhiteSpace(idLine->text().toStdString());
        fetchNow=false;
        int sY,sMo,sD,sH,sMi;
        int eY,eMo,eD,eH,eMi;
        int numSteps=10; //make up a number for now.... It really doesn't matter at this point
        
        std::string StartTime=startEdit->text().toStdString();
        std::string EndTime=endEdit->text().toStdString();
                
        istringstream(StartTime.substr(0,2))>>sMo;
        istringstream(StartTime.substr(3,2))>>sD;
        istringstream(StartTime.substr(6,4))>>sY;
        istringstream(StartTime.substr(11,2))>>sH;
        istringstream(StartTime.substr(14,2))>>sMi;

        istringstream(EndTime.substr(0,2))>>eMo;
        istringstream(EndTime.substr(3,2))>>eD;
        istringstream(EndTime.substr(6,4))>>eY;
        istringstream(EndTime.substr(11,2))>>eH;
        istringstream(EndTime.substr(14,2))>>eMi;
        

//        cout<<sY<<sMo<<sD<<sH<<sMi<<endl;
//        cout<<eY<<eMo<<eD<<eH<<eMi<<endl;
        
        std::vector<boost::posix_time::ptime> timeList;
        timeList=pointInitialization::getTimeList(sY,sMo,sD,sH,sMi,eY,eMo,eD,eH,eMi,
                                                  numSteps,tzString.toStdString());
        
        result = pointInitialization::fetchStationByName(stid,timeList,tzString.toStdString(),fetchNow);
//        pointInitialization::writeStationLocationFile(stationPathName,demFileName.toStdString());
        cout<<"Return: "<<result<<endl;       
        
//        cout<<stid<<endl;        
//        cout<<startEdit->text().toStdString()<<endl;
//        cout<<endEdit->text().toStdString()<<endl;
    }
//    if (geoLoc->currentIndex())

    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
}

void stationFetchWidget::getMetadata()
{
    QString fileName;    
    cout<<"LINKED METADATA FUNCTION: below is DEM"<<endl;
    cout<<demFileName.toStdString()<<endl;
    fileName = QFileDialog::getSaveFileName(this, tr("Save Domain Metadata File"), ".csv", tr("Comma Separated " \
    "files (*.csv")); 
    if (QFileInfo(fileName).suffix().compare("csv", Qt::CaseInsensitive))
    {
        fileName += ".csv";
        if (QFileInfo(fileName).exists())
        {
            int r = QMessageBox::warning(this, "WindNinja",
                                         "The file " + fileName +
                                         " exists, do you wish to" \
                          " overwrite it?",
                                         QMessageBox::Yes |
                                         QMessageBox::No |
                                         QMessageBox::Cancel);
            if (r == QMessageBox::No || r == QMessageBox::Cancel)
                return;
        }
    }
            else
                pointInitialization::fetchMetaData(fileName.toStdString(), demFileName.toStdString(), true);
    
    
}



void stationFetchWidget::closeDEM()
{
    this->close();
}



void stationFetchWidget::closeEvent(QCloseEvent *event)
{
    event->ignore();
//    this->writeSettings();
    exitDEM();
    event->accept();
}

