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
 
/** @file stationFetchWidget.cpp
  *
  * Fetch stations from the Mesonet API
  *
  */

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
    fetchMetaButton->setVisible(false); //Hide the metadata button from the gui

    stationFetchProgress = new QProgressDialog(this); //Sets up a mediocre progress bar that kind of works
    stationFetchProgress->setWindowModality(Qt::ApplicationModal);
    stationFetchProgress->setAutoReset(false); //Displays how far along the download process is
    stationFetchProgress->setAutoClose(false);
}

/**
 * @brief Deletes all allocated memory on destruction of class object
 *
 */
stationFetchWidget::~stationFetchWidget()
{
    delete stationFetchProgress;
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
    connect(fetchMetaButton, SIGNAL(clicked()),this, SLOT(getMetadata())); //Gets the metadata, indirectly used in mainwindow via config option
    connect(fetchDataButton,SIGNAL(clicked()),this,SLOT(executeFetchStation())); //Fetches the data and
    connect(endEdit,SIGNAL(dateTimeChanged(QDateTime)),this,SLOT(watchStopTime())); //Watches the time to make sure we don't go over
    connect(startEdit,SIGNAL(dateTimeChanged(QDateTime)),this,SLOT(watchStartTime())); //Watches the time to make sure we don't go under
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

/**
 * @brief stationFetchWidget::watchStartTime
 * Watches the start time that the user puts in, if
 * its bigger than the end time, correct the end time by adding 1 hour
 * to the start time
 */
void stationFetchWidget::watchStartTime()
{
    if (endEdit->dateTime()<startEdit->dateTime())
    {
        writeToConsole("Start Time is greater than End Time!, reverting...");
        CPLDebug("STATION_FETCH","START TIME > END TIME, FIXING END TIME");
        endEdit->setDateTime(startEdit->dateTime().addSecs(3600));
    }
}
/**
 * @brief stationFetchWidget::watchStopTime
 * watch the end time the user puts in, if it is smaller
 * than the start time
 * correct the start time by subtracting 1 hour from the end time
 */
void stationFetchWidget::watchStopTime()
{
    if (endEdit->dateTime()<startEdit->dateTime())
    {
        writeToConsole("Start Time is greater than End Time!, reverting...");
        CPLDebug("STATION_FETCH","START TIME > END TIME, FIXING START TIME");
        startEdit->setDateTime(endEdit->dateTime().addSecs(-3600));
    }
}

/**
 * @brief stationFetchWidget::updateFetchProgress
 * Updates the Progress Bar and tells the GUI
 * when station fetch is done downloading, or if the user
 * cancels the request
 *
 * Kills the request once it is downloaded
 *
 */
void stationFetchWidget::updateFetchProgress()
{
    if (stationFetchProgress->wasCanceled()) //If the user hits the cancel button
    {
        stationFetchProgress->setLabelText("Canceling!");
        stationFetchProgress->setCancelButton(0);
        stationFutureWatcher.waitForFinished();
        stationFetchProgress->cancel();
        setCursor(Qt::ArrowCursor);
    }
    else
    {
        stationFutureWatcher.waitForFinished();
        int result = stationFutureWatcher.result(); //Get the result, 1 good, -1 bad

        if (result==-1) //Means that we failed to get data, the error_msg should tell the user
        { //What happened
            stationFetchProgress->setLabelText(QString(pointInitialization::error_msg.c_str()));
            stationFetchProgress->setRange(0,1);
            stationFetchProgress->setValue(0);
            stationFetchProgress->setCancelButtonText("Close");
            setCursor(Qt::ArrowCursor);
        }
        else if (result==-2) //Special type of error that we catch in the GUI
        {
            stationFetchProgress->setLabelText("ERROR: Selected Time Range is greater than 1 year! Input custom API KEY to remove limits");
            stationFetchProgress->setRange(0,1);
            stationFetchProgress->setValue(0);
            stationFetchProgress->setCancelButtonText("Close");
            setCursor(Qt::ArrowCursor);
        }
        else //IT WORKED!
        {
            stationFetchProgress->setRange(0,100);
            stationFetchProgress->setValue(1);
            stationFetchProgress->setValue(100);
            stationFetchProgress->setLabelText("Data Downloaded Sucessfully!");
            stationFetchProgress->setCancelButtonText("Close");
            setCursor(Qt::ArrowCursor); //set the cursor back to normal
        }
    }
}

/**
 * @brief stationFetchWidget::executeFetchStation
 * Executes fetchStation using Qt stuff
 * based on widgetDownloadDEM
 *
 * so that we can see a progress bar
 * and prevent program hanging
 *
 */
void stationFetchWidget::executeFetchStation()
{
    stationFetchProgress->setLabelText("Downloading Station Data!");
    stationFetchProgress->setRange(0,0); //make it bounce back and forth
    stationFetchProgress->setCancelButtonText("Cancel");
    stationFetchProgress->reset(); //Set the progress bar back to its basic state

    connect(&stationFutureWatcher,SIGNAL(finished()),this,SLOT(updateFetchProgress()));
    connect(stationFetchProgress,SIGNAL(canceled()),this,SLOT(updateFetchProgress()));

     /* Note on Concurrent Processing:
     * You can't update the GUI from a spawned process on another thread
     * If you do:
     * It will segfault or throw X11 errors at you and freeze
     * This was encountered when updating the cursor was set inside fetchStation.
     * Always update the UI from outside the spawned thread...
     */

    stationFutureWatcher.setFuture(QtConcurrent::run(this,&stationFetchWidget::fetchStation)); //Run the
    //actual fetching
    setCursor(Qt::WaitCursor);//Make the cursor spinny
    stationFetchProgress->exec(); //Execute the progress bar to do its thing
//    stationFutureWatcher.cancel(); //commented for now, probably can be deleted...

}
/**
 * @brief stationFetchWidget::removeWhiteSpace
 * //Cleans up spaces in text
 * if the user types in a station name and then puts a space
 * strip it out
 * @param str
 * @return
 */
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
/**
 * @brief stationFetchWidget::demButcher
 * get rid of some crap in the dem so that the file name looks nice
 *
 * @return
 */
std::string stationFetchWidget::demButcher()//Cleans up the DEM for use in the downloader
{
    std::string demPath = std::string(CPLGetDirname(demFileName.toStdString().c_str()));
//    std::string demRaw = demFileName.toStdString();
//    size_t lastDot=demRaw.find_last_of("/");
//    if (lastDot==std::string::npos)
//    {
//        return demRaw;
//    }
//    std::string demBetter=demRaw.substr(0,lastDot)+"/";
    return demPath;
}
/**
 * @brief stationFetchWidget::fetchStation
 * Fetches data from the Mesowest API based on GUI request
 */
int stationFetchWidget::fetchStation()
{
    writeToConsole("Downloading Station Data...");
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

    //Custom API_KEY STUFF
    const char *api_key_conf_opt = CPLGetConfigOption("CUSTOM_API_KEY","FALSE");
    if(api_key_conf_opt!="FALSE")
    {
        std::ostringstream api_stream;
        api_stream<<api_key_conf_opt;
        pointInitialization::setCustomAPIKey(api_stream.str());
    }
    //End Custom API_KEY STUFF

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

    // This means DEM and Current Data 1 step
    if (terrainPart==0 && timePart==0)
    {
        CPLDebug("STATION_FETCH","Fetch Params: DEM and Current Data");
        buffer=bufferSpin->text().toDouble();
        bufferUnits=buffUnits->currentText().toStdString();
        fetchNow=true;
        
        //Set the Station Buffer
        pointInitialization::setStationBuffer(buffer,bufferUnits);
        //Generates the directory to store the file names, because current data is on, don't specify time zone        
        stationPathName=pointInitialization::generatePointDirectory(demFileName.toStdString(),demUse,eTimeList,true);
        pointInitialization::SetRawStationFilename(stationPathName);
        result = pointInitialization::fetchStationFromBbox(demFileName.toStdString(),eTimeList,tzString.toStdString(),fetchNow);

        CPLDebug("STATION_FETCH","Return: %i",result);
    }
    //DEM and Time series
    if (terrainPart==0 && timePart==1)
    {
        CPLDebug("STATION_FETCH","Fetch Params: DEM and Time series");
        buffer=bufferSpin->text().toDouble();
        bufferUnits=buffUnits->currentText().toStdString();
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
        
        std::vector<boost::posix_time::ptime> timeList;
        timeList=pointInitialization::getTimeList(sY,sMo,sD,sH,sMi,eY,eMo,eD,eH,eMi,
                                                  numSteps,tzString.toStdString());
        int duration_check = pointInitialization::checkFetchTimeDuration(timeList); //Check the timelist duration
        if(duration_check==-2) //Means that they select too much and we have to quit
        {
            return duration_check;
        }

        //Set station Buffer
        pointInitialization::setStationBuffer(buffer,bufferUnits);

        //Generate Station directory, because timeseries is on, specify what time zone the stations will be
        //downloaded in, based on DEM time zone settings, or user specified.
        stationPathName=pointInitialization::generatePointDirectory(demFileName.toStdString(),demUse,timeList,false);
        pointInitialization::SetRawStationFilename(stationPathName);
        result = pointInitialization::fetchStationFromBbox(demFileName.toStdString(),timeList,
                                                           tzString.toStdString(),false);       
        CPLDebug("STATION_FETCH","Return: %i",result);       
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

        CPLDebug("STATION_FETCH","Return: %i",result);
        
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
                
        std::vector<boost::posix_time::ptime> timeList;
        timeList=pointInitialization::getTimeList(sY,sMo,sD,sH,sMi,eY,eMo,eD,eH,eMi,
                                                  numSteps,tzString.toStdString());
        int duration_check = pointInitialization::checkFetchTimeDuration(timeList); //Check the timelist duration
        if(duration_check==-2) //Means that they select too much and we have to quit
        {
            return duration_check;
        }

        stationPathName=pointInitialization::generatePointDirectory(demFileName.toStdString(),demUse,timeList,false);  //As we keep working on the GUI, need to get change eTimeList to timeList for timeseries
        pointInitialization::SetRawStationFilename(stationPathName);
        result = pointInitialization::fetchStationByName(stid,timeList,tzString.toStdString(),fetchNow);

        //timeseries, so specify time zone in path name.
        
        CPLDebug("STATION_FETCH","Return: %i",result);
    }
    if(result==false) //If there are no stations, tell the user
    {
        pointInitialization::removeBadDirectory(stationPathName);
        writeToConsole("Could not read station File: Possibly no stations exist for request");
        return -1;
    }
    else
    {
        if(fetchNow==false)
        {
            pointInitialization::start_and_stop_times.clear(); //Need to clear these times to allow multiple downloads
        }
        return 1;
    }
}

/**
 * @brief stationFetchWidget::getMetadata
 *
 * GUI wrapper for metadata fetcher,
 * we don't currently use this
 * and instead have a config option
 * that fetches metadata at runtime
 *
 * leave this in incase we decide to add in a
 * metadata button later
 */
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
    exitWidget();
    event->accept();
}

