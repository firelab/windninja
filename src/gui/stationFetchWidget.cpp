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

    currentBox->setVisible(false);

    stationFetchProgress = new QProgressDialog(this); //Sets up a mediocre progress bar that kind of works
    stationFetchProgress->setModal(true); //Needs some improvements...
    stationFetchProgress->setAutoReset(false); //Displays how far along the download process is
    stationFetchProgress->setAutoClose(false);
    stationFetchProgress->setRange(0,100);

}

/**
 * @brief Deletes all allocated memory on destruction of class object
 *
 */
stationFetchWidget::~stationFetchWidget()
{
    delete stationFetchProgress;
//    delete progressBar;
//    delete fetcher;
//    delete latlngError;
//    delete bufferError;
//    delete boundsError;
//    delete gmInterface;
}
/**
 * @brief stationFetchWidget::fixTime
 * Sets the time correctly in the downloader widget
 * Default is start time to be yesterday and
 * end time to be now
 */
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
    connect(closeButton,SIGNAL(clicked()),this,SLOT(close())); //closes stationFetchWidget
}

void stationFetchWidget::updatetz(QString tz) //Updates the Time Zone
{
    tzString = tz;
}
void stationFetchWidget::setInputFile(QString file) //Gets the DEM
{
    demFileName = file;    
}

void stationFetchWidget::watchTime() //Makes sure that the start time never goes farther into the future and the end time
{
//    cout<<"end TIME CHANGED!"<<endl;
//    endEdit->dateTime()
    startEdit->setMaximumDateTime(endEdit->dateTime().addSecs(-3600));    
    if (endEdit->dateTime()<startEdit->dateTime())
    {
        writeToConsole("Start Time is greater than End Time!, reverting...");
        CPLDebug("STATION_FETCH","START TIME > END TIME, FIXING!");
        startEdit->setDateTime(endEdit->dateTime().addSecs(-3600));
    }
}

void stationFetchWidget::updateGeoFetch()
{
    
}

void stationFetchWidget::updateTimeFetch()
{
    
}

std::string stationFetchWidget::removeWhiteSpace(std::string str) //Cleans up spaces in text
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

std::string stationFetchWidget::demButcher()//Cleans up the DEM for use in the downloader
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
/**
 * @brief stationFetchWidget::fetchStation
 * Fetches data from the Mesowest API based on GUI request
 */
void stationFetchWidget::fetchStation()
{
    stationFetchProgress->setValue(0);
    stationFetchProgress->setVisible(true);
    stationFetchProgress->show();
    CPLDebug("STATION_FETCH","Fetch Station GUI Function");
    CPLDebug("STATION_FETCH","---------------------------------------");
    CPLDebug("STATION_FETCH","DEM FILE NAME: %s",demFileName.toStdString().c_str());
    CPLDebug("STATION_FETCH","TIME ZONE: %s",tzString.toStdString().c_str());
    CPLDebug("STATION_FETCH","geoLoc: %i",geoLoc->currentIndex());
    CPLDebug("STATION_FETCH","timeLoc: %i",timeLoc->currentIndex());
    CPLDebug("STATION_FETCH","---------------------------------------");
    std::string stid;
    double buffer;
    std::string bufferUnits;
    bool fetchNow;
    std::string blank="blank";
    std::string demUse=demButcher();
    std::string stationPathName;
    CPLDebug("STATION_FETCH","USING DEM: %s",demUse.c_str());

    int terrainPart=geoLoc->currentIndex();
    int timePart=timeLoc->currentIndex();
//Debugging code
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
    
    
    //pointInitialization::SetRawStationFilename(demUse);
    //This is a little different than in the CLI, because there is on option for a custom output path
    //Instead, the "raw File", demFileName is the dem file, like it would be in the LCI
    //and then demUse, which the is just the path to the dem acts as the output path
    //So the directory storing the weather csvs is always the same level as the DEM
    //The Path name has a time zone in it
    // This is because the station file names have time in them and to specify to the user
    // where they downloaded the times at
    // This is only necessary for timeseries however, because the other options, such as 1 step/current data
    //are time  naive and require the user to specify a time for what is current.
    //These two lines of code are the old way for which files were generated, leaving them in for debugging for now...
//    stationPathName=pointInitialization::generatePointDirectory(demFileName.toStdString(),demUse,eTimeList,true);  //As we keep working on the GUI, need to get change eTimeList to timeList for timeseries
//    pointInitialization::SetRawStationFilename(stationPathName);
       
    // This means DEM and Current Data 1 step
    if (terrainPart==0 && timePart==0)
    {
        CPLDebug("STATION_FETCH","Fetch Params: DEM and Current Data");
        buffer=bufferSpin->text().toDouble();
        bufferUnits=bufferSpin->text().toStdString();
        fetchNow=true;        
        
//        cout<<bufferSpin->text().toDouble()<<endl;
//        cout<<buffUnits->currentText().toStdString()<<endl;
//        cout<<currentBox->isChecked()<<endl;
        //Generates the directory to store the file names, because current data is on, don't specify time zone
        stationPathName=pointInitialization::generatePointDirectory(demFileName.toStdString(),demUse,eTimeList,true);
        pointInitialization::SetRawStationFilename(stationPathName);
        result = pointInitialization::fetchStationFromBbox(demFileName.toStdString(),eTimeList,tzString.toStdString(),fetchNow);

//        pointInitialization::writeStationLocationFile(stationPathName,demFileName.toStdString());
        CPLDebug("STATION_FETCH","Return: %i",result);
    }
    //DEM and Time series
    if (terrainPart==0 && timePart==1)
    {
        CPLDebug("STATION_FETCH","Fetch Params: DEM and Time series");
        buffer=bufferSpin->text().toDouble();
        bufferUnits=bufferSpin->text().toStdString();
        fetchNow=false;
        
        int sY,sMo,sD,sH,sMi;
        int eY,eMo,eD,eH,eMi;
        int numSteps=10; //make up a number for now.... It really doesn't matter at this point
        //Just need to generate a timelist for fetching purposes
        
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
//        cout<<timeList.size()<<endl;
        //Generate Station directory, because timeseries is on, specify what time zone the stations will be
        //downloaded in, based on DEM time zone settings, or user specified.

        stationPathName=pointInitialization::generatePointDirectory(demFileName.toStdString(),demUse,timeList,false);
        pointInitialization::SetRawStationFilename(stationPathName);
        result = pointInitialization::fetchStationFromBbox(demFileName.toStdString(),timeList,
                                                           tzString.toStdString(),false);

//        pointInitialization::writeStationLocationFile(stationPathName,demFileName.toStdString());

        
        CPLDebug("STATION_FETCH","Return: %i",result);
//        cout<<bufferSpin->text().toDouble()<<endl;
//        cout<<buffUnits->currentText().toStdString()<<endl;
//        cout<<startEdit->text().toStdString()<<endl;
//        cout<<endEdit->text().toStdString()<<endl;
        
        
    }
    if (terrainPart==1 && timePart==0) //STation ID and 1 step
    {
        CPLDebug("STATION_FETCH","STID and Current Data");
        stid=removeWhiteSpace(idLine->text().toStdString());
        fetchNow=true;
        //Fetch now is on, don't specify time zone in station path       
        stationPathName=pointInitialization::generatePointDirectory(demFileName.toStdString(),demUse,eTimeList,true);
        pointInitialization::SetRawStationFilename(stationPathName);

        result = pointInitialization::fetchStationByName(stid,eTimeList,tzString.toStdString(),fetchNow);

//        pointInitialization::writeStationLocationFile(stationPathName,demFileName.toStdString());
        CPLDebug("STATION_FETCH","Return: %i",result);

//        cout<<stid<<endl;        
//        cout<<currentBox->isChecked()<<endl;        
        
    }
    if (terrainPart==1 && timePart==1) //STATION ID and timeseries
    {
        CPLDebug("STATION_FETCH","STID and Timeseries");
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

        stationPathName=pointInitialization::generatePointDirectory(demFileName.toStdString(),demUse,timeList,false);  //As we keep working on the GUI, need to get change eTimeList to timeList for timeseries
        pointInitialization::SetRawStationFilename(stationPathName);
        result = pointInitialization::fetchStationByName(stid,timeList,tzString.toStdString(),fetchNow);

        //timeseries, so specify time zone in path name.
        
//        pointInitialization::writeStationLocationFile(stationPathName,demFileName.toStdString());
        CPLDebug("STATION_FETCH","Return: %i",result);

//        cout<<stid<<endl;        
//        cout<<startEdit->text().toStdString()<<endl;
//        cout<<endEdit->text().toStdString()<<endl;
    }
    if(result==false) //If there are no stations, tell the user
    {
        pointInitialization::removeBadDirectory(stationPathName);
        writeToConsole("Could not read station File: Possibly no stations exist for request");
        stationFetchProgress->setValue(100);
        stationFetchProgress->setLabelText("No Station Data Found!");
        stationFetchProgress->setCancelButtonText("OK!");
    }
    else
    {
        stationFetchProgress->setValue(100);
        stationFetchProgress->setLabelText("Download Succesful!");
        stationFetchProgress->setCancelButtonText("OK!");
        writeToConsole("Data Downlaoded Successfully");
    }
}

void stationFetchWidget::getMetadata()
{
    QString fileName;    
    CPLDebug("STATION_FETCH","METADATA DOWNLOADER FOR STATIONS IN DEM: %s",demFileName.toStdString().c_str());

    //the third param: QFileInfo sets the metadata
    //save widget to the current directory and then
    //appends .csv to the name to tell the user to save the metadata with
    //that extension. This may have some bugs, needs testing 5/24/2018
    fileName = QFileDialog::getSaveFileName(this,
                                            tr("Save Domain Metadata File"),
                                            QFileInfo(demFileName).absoluteDir().absolutePath()+"/.csv",
                                            tr("Comma Separated files (*.csv"));

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

