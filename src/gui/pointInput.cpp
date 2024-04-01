/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja Qt GUI
 * Purpose:  Point initialization input.
 * Author:   Kyle Shannon <ksshannon@gmail.com>
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

/**
 * @file pointInput.cpp
 * @author Kyle Shannon <ksshannon@gmail.com>
 */

#include "pointInput.h"
static int wxStationFormat;

pointInput::pointInput( QWidget *parent ) : QWidget( parent )
{
    pointGroupBox = new QGroupBox( "Point Initialization", this );
    pointGroupBox->setCheckable( true );
    pointGroupBox->setChecked(false);

    pointGo=false; //Very Imptortant!
    enableTimeseries=false; //bool for mainwindow

    newForm = new QWidget();

//####################################################
// Some General Buttons                              #
//####################################################

    //dateTimeEdit is the way of setting the simulation time for old format runs
    dateTimeEdit = new QDateTimeEdit( newForm );
    dateTimeEdit->setDateTime( QDateTime::currentDateTime() );
    dateTimeEdit->setCalendarPopup( true );
    dateTimeEdit->setDisplayFormat( "MM/dd/yyyy HH:mm" );
    dateTimeEdit->setEnabled( false ); //This is for Old Format Diurnal Simulations
    dateTimeEdit->setVisible(false);
    dateTimeEdit->setToolTip("Set date and time for single time step diurnal/stability simulations");

    diurnalLabel = new QLabel(this); //A Label for dateTimeEdit
    diurnalLabel->setText("Set Simulation Time: ");
    diurnalLabel->setVisible(false);

    oneStepTimeLabel = new QLabel(this); //Label for 1 step datetime runs
    oneStepTimeLabel->setText("Simulation time set to:");
    oneStepTimeLabel->setVisible(false);

    writeStationFileButton =  new QCheckBox( this ); //This writes an interpolated csv of the weather data (we might not want this)
    writeStationFileButton->setText( tr( "Write Station File" ) );
    writeStationFileButton->setIcon( QIcon( ":weather_clouds.png" ) );
    writeStationFileButton->setToolTip("Time Series: Writes an Interpolated CSV for each time step\nSingle Step: Writes a CSV of inputted weather data.");

    writeStationKmlButton =  new QCheckBox( this ); //This writes a KML of the weather stations (1 per run)
    writeStationKmlButton->setText( tr( "Write Station Kml" ) );
    writeStationKmlButton->setIcon( QIcon( ":weather_cloudy.png" ) );
    writeStationKmlButton->setToolTip("Time Series: Writes a KML for each time step with time interpolated station data.\nSingle Step: Writes a KML of weather station data.");

    widgetButton = new QToolButton( this ); //This opens the station fetch downloader Widget (formerly doTest)
    widgetButton->setText( tr( "Download data" ));
    widgetButton->setIcon(QIcon(":server_go.png"));
    widgetButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    widgetButton->setToolTip("Download weather station data from the Mesonet API.");

//####################################################
// The Main Box and directory stuff                  #
//####################################################

    sfModel = new QDirModel(this); //Creates the directory model
    sfModel->setReadOnly(true); //probably can be true, but i don't know
    sfModel->setSorting(QDir::Time); //Sort by time created

    treeView = new QTreeView(this); //Creates the box where the sfModel goes
    treeView->setVisible(true);
    treeView->setModel(sfModel); //Sets the model to the thing above
    treeView->header()->setStretchLastSection(true); //Makes it look nice
    treeView->setAnimated(true); //Fancy stuff
    treeView->setColumnHidden(1, true);
    treeView->setColumnHidden(2, true);
    treeView->setAlternatingRowColors( false );
    treeView->setSelectionMode(QAbstractItemView::MultiSelection); //Allows multiple files to be selected
    treeView->setSelectionBehavior(QAbstractItemView::SelectRows); //Select entire row when we do select something

    treeLabel = new QLabel(tr("Select Weather Stations")); //Label for Tree and sfModel
    treeLabel->setToolTip("Select Weather Stations from available files.\n"
                          "Click a file to add it to the list of included stations\n"
                          "Click it again to remove it from the included stations.\n"
                          "Available formats are time series, one step runs with time data,\n"
                          "and one step runs without time data (old format)");
    
//####################################################
//  Tool buttons and other things                    #
//####################################################
    ClippyToolLayout = new QHBoxLayout; //Layout for info and toolbox
    
    refreshToolButton = new QToolButton(this); //This refreshes the tree so that new files will populate
    refreshToolButton->setText(tr("Refresh Weather Stations"));
    refreshToolButton->setIcon(QIcon(":arrow_rotate_clockwise.png"));
    refreshToolButton->setToolTip(tr("Refresh Stations stored on disk in the DEM directory."));
    refreshToolButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    refreshToolButton->setVisible(true);

    clippit = new QLabel(tr("")); //This gets updated with important information about the simulation as station files are chosen
    clippit->setVisible(false);

    timeLine = new QFrame(this); //Adds a horizontal line
    timeLine->setFrameShape(QFrame::HLine);
    timeLine->setFrameShadow(QFrame::Sunken);

    timeLine2 = new QFrame(this); //Adds a horizontal line
    timeLine2->setFrameShape(QFrame::HLine);
    timeLine2->setFrameShadow(QFrame::Sunken);

    ClippyToolLayout->addStretch(); //Moves it over to the other side
    ClippyToolLayout->addWidget(refreshToolButton);
//    ClippyToolLayout->addWidget(clippit);

//####################################################
//Stuff for timeseries runs...                       #
//####################################################
    startTime = new QDateTimeEdit(QDateTime::currentDateTime()); //Initializes the start and stop time boxes for timeseries
    stopTime = new QDateTimeEdit(QDateTime::currentDateTime());
    
    startTime->setDateTime(QDateTime::currentDateTime().addDays(-1)); //Set default to be yesterday
    startTime->setMaximumDateTime(QDateTime::currentDateTime().addSecs(-3600)); //Prevent time from overlapping with right now
    stopTime->setMaximumDateTime(QDateTime::currentDateTime()); //No Future simulations
    
    startTime->setDisplayFormat( "MM/dd/yyyy HH:mm" );
    stopTime->setDisplayFormat( "MM/dd/yyyy HH:mm" );

    startTime->setToolTip("Enter the simulation start time");
    stopTime->setToolTip("Enter the simulation stop time");

    startTime->setCalendarPopup(true);
    stopTime->setCalendarPopup(true);
       
    numSteps = new QSpinBox; //Number of timesteps box
    numSteps->setValue(24);
    numSteps->setMinimum(1);
    numSteps->setMaximum(99999);//Hopefully big enough
    numSteps->setToolTip("Enter the number of time steps");
    
    startTime->setVisible(true); //Some visibility settings so that the default thing the user sees is a timeseries
    stopTime->setVisible(true);

    startTime->setEnabled(false); //Disables timeseries options until the user does something
    stopTime->setEnabled(false);

    numSteps->setVisible(true); //Same as above for time step options
    numSteps->setEnabled(false);
    
    timeBoxLayout = new QHBoxLayout; //Layout setup
    startLayout = new QVBoxLayout;
    stopLayout = new QVBoxLayout;
    stepLayout = new QVBoxLayout;
    
    startLabel = new QLabel(tr("Start Time")); //Labels for timeseries
    stopLabel = new QLabel(tr("Stop Time"));
    stepLabel = new QLabel(tr("Number of Time Steps"));
    startLabel->setVisible(true);
    stopLabel->setVisible(true);
    stepLabel->setVisible(true);
    
    startLayout->addWidget(startLabel); //Labels are vertically stacked onto timeboxes and then horizontally set up
    startLayout->addWidget(startTime);
    
    stopLayout->addWidget(stopLabel);
    stopLayout->addWidget(stopTime);
    
    stepLayout->addWidget(stepLabel);
    stepLayout->addWidget(numSteps);
    
    timeBoxLayout->addLayout(startLayout); //Add all the vertial labels to a horizontal grouping
    timeBoxLayout->addLayout(stopLayout);
    timeBoxLayout->addLayout(stepLayout);
    
//-------------------------------------------------
// Add some new layouts
//-------------------------------------------------
    vTreeLayout = new QVBoxLayout; //Adds everything to a vertical layout

    hDownloaderLayout = new QHBoxLayout; //Holds the label for the tree & download button
    hDownloaderLayout->addWidget(treeLabel); //Adds label
    hDownloaderLayout->addWidget(widgetButton); //adds Download Button to top of page

    vTreeLayout->addLayout(hDownloaderLayout); //adds hDownloaderLayout sublayout to the overal system
    vTreeLayout->addWidget(treeView); //Add the big box
    vTreeLayout->addLayout(ClippyToolLayout); //Add the refresh tool button
    vTreeLayout->addWidget(timeLine); //Add a line to space it out
    vTreeLayout->addLayout(timeBoxLayout); //Add the timeseries stuff
//####################################################
//add in old format and other layout options         #
//####################################################

    buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget( writeStationFileButton );
    writeStationFileButton->setVisible( false ); //This was disabled in the original PI, leave it in for now, but the is now controlled by a config_option
    buttonLayout->addWidget( writeStationKmlButton );
    buttonLayout->addStretch();

    diurnalTimeLayout = new QHBoxLayout; //Create a layout for the old format diurnal options
    diurnalTimeLayout->addWidget(diurnalLabel); //Add all the parts
    diurnalTimeLayout->addWidget(dateTimeEdit,1);
    diurnalTimeLayout->addWidget(oneStepTimeLabel);
//    diurnalTimeLayout->addStretch(-1);

    pointLayout = new QVBoxLayout;

    pointLayout->addLayout(vTreeLayout); //add the above layout to the main window layouyt

    pointLayout->addLayout(diurnalTimeLayout);

    pointLayout->addStretch();
    pointLayout->addWidget(timeLine2); //Add another line
    pointLayout->addLayout( buttonLayout ); //Add the kml button at the bottom

    pointGroupBox->setLayout( pointLayout ); //Set the layout to all of this

    ninjafoamConflictLabel = new QLabel(tr("The point initialization option is not currently available for the momentum solver.\n"),
                                        this);
    ninjafoamConflictLabel->setHidden(true);

    layout = new QVBoxLayout;
    layout->addWidget( pointGroupBox );
    layout->addWidget(ninjafoamConflictLabel);
    setLayout( layout ); //done

    checkForModelData();//On load check for model data to filter out directories and stuff we don't want

//####################################################
// Connect Signls to Slots                           #
//####################################################
    connect( widgetButton ,SIGNAL( clicked () ), this,
             SLOT(openStationFetchWidget())); //Opens the Downloader Connector
    connect(refreshToolButton, SIGNAL(clicked()), //Refreshes new Format, deselects files
        this, SLOT(checkForModelData()));
    connect(startTime,SIGNAL(dateTimeChanged(QDateTime)),this,SLOT(updateStartTime(QDateTime))); //update time into a vector from the timeboxes
    connect(stopTime,SIGNAL(dateTimeChanged(QDateTime)),this,SLOT(updateStopTime(QDateTime)));
    connect(stopTime,SIGNAL(dateTimeChanged(QDateTime)),this,SLOT(watchStopTime()));//Watch the times to make sure the user can't do crazy things
    connect(startTime,SIGNAL(dateTimeChanged(QDateTime)),this,SLOT(watchStartTime()));
    connect(treeView->selectionModel(),
            SIGNAL(selectionChanged(const QItemSelection &,const QItemSelection &)),
            this, SLOT(readStationFiles(const QItemSelection &,const QItemSelection &))); //Update the selected stations when a user clicks something
    connect(numSteps,SIGNAL(valueChanged(int)),this,SLOT(setOneStepTimeseries())); //watch the number of steps incase it goes to 1
    stationFileName = ""; //Sets a default
}

pointInput::~pointInput()
{

}

/**
 * @brief pointInput::readStationFiles
 * Reads the files on disk that the user selects
 *
 * x is the previously selected data
 * y is the new selected data
 *
 * These two are not used explicitly but are necessary to link up the
 * model with this function, as they trigger the checking to occur
 *
 * How this function works:
 * 1. get a list of the stations the user selects from the UI
 * 2. get what file type it is from directStationTraffic
 *      a. this will also update the user on what the station type is
 * 3. if all selected files are the same type, set the simulation type and tell mainwindow that they are good
 *    if they are not all the same type, warn the user and mainwindow and wait for the user to change something
 *
 * @param x
 * @param y
 */
void pointInput::readStationFiles(const QItemSelection &x ,const QItemSelection &y)
{
    QModelIndexList idx = x.indexes(); //Don't need these, probably delete later
    QModelIndexList idy = y.indexes();
    QModelIndexList idx0 = treeView->selectionModel()->selectedRows(); //Get the number of files selected

    std::vector<std::string> selectedStations; //The good stations
    std::vector<int> finalTypes; //What type they are
    CPLDebug("STATION_FETCH","========================================");
    CPLDebug("STATION_FETCH","NUMBER OF SELECTED STATIONS: %i",idx0.count());

    for(int i=0;i<idx0.count();i++)
    {
        if(sfModel->fileInfo(idx0[i]).isDir()==true) //If its a directory, make it so that it can't be selected
        {
            CPLDebug("STATION_FETCH","IGNORING SELECTED DIRECTORY!");
            treeView->selectionModel()->select(idx0[i],QItemSelectionModel::Deselect | QItemSelectionModel::Rows);//Deselct entire row by calling ::Rows otherwise it looks messy
        }
        else //if its not a directory, add it to the list of available files
        {
            selectedStations.push_back(sfModel->fileInfo(idx0[i]).absoluteFilePath().toStdString());
        }
    }

    for (int i=0;i<selectedStations.size();i++)
    {
        CPLDebug("STATION_FETCH","----------------------------------------");
        CPLDebug("STATION_FETCH","STATION NAME: %s",selectedStations[i].c_str());
        int stationFileType = directStationTraffic(selectedStations[i].c_str()); // Get the station data type
        CPLDebug("STATION_FETCH","Type of Station File: %i \n",stationFileType);
        finalTypes.push_back(stationFileType);
    }
    std::set<int> setInt(finalTypes.begin(),finalTypes.end()); //This is a sanity check to see if the stations that we are reading are all the same type
    std::vector<int> vecInt(setInt.begin(),setInt.end());

    CPLDebug("STATION_FETCH","Unique Data Types (bad if >1) %lu",vecInt.size());
    for(int j=0;j<vecInt.size();j++)
    {
        CPLDebug("STATION_FETCH","Selected Data Type: %i",vecInt[j]);
    }
    CPLDebug("STATION_FETCH","\n");
    stationFileList = selectedStations;
    stationFileTypes = vecInt;

    if (vecInt.size()>1) //Set the simulation type to -1 -> user selects  different file types
    {
        simType = -1;
        displayInformation(-1); //Display some information to the console
    }
    if (vecInt.size()==1) //Set the sim type to whatever it is
    {
        simType = vecInt[0];
        displayInformation(vecInt[0]);
    }
    else //we probably failed to open it because its a bad file
    {
        simType = -1;
        displayInformation(-1);
    }
}

/**
 * @brief Figures out what format belongs to what file based on the presence of date time
 * data and header information
 * Turns options on and off for the gui based on what the file formats are, such as
 * enables timeseries when there is a timeseries. Suggests time series data ranges.
 * prevents timeseries when there is no way it is a timeseries
 * *  * 
 * Possible options to be returned from directStationTraffic:
 * a. 0  : OLD FORMAT <-1 Step
 * b. 1  : NEW FORMAT, Time Series <- Multiple Steps
 * c. 2  : NEW FORMAT, NO Time Series <- 1 Step
 * d. -1 : Something bad happened, not sure what at this point
 * @param xFileName <- the file name of the wxStation to be read from disk.
 * @return see above
 */
int pointInput::directStationTraffic(const char* xFileName)
{
    //Get the header version from a wxStation function that does this for us
    int stationHeader = wxStation::GetHeaderVersion(xFileName);
    CPLDebug("STATION_FETCH","HEADER TYPE: %i",stationHeader);
    int instant = 0;
    std::string idx3;
    stringstream ssidx;

    if(stationHeader!=1) //Station header is different than what we return
    {
        //Determine whether it is a timeseries or not...
        OGRDataSourceH hDS;
        OGRLayer *poLayer;
        OGRFeature *poFeature;
        OGRFeatureDefn *poFeatureDefn;
        OGRFieldDefn *poFieldDefn;
        OGRLayerH hLayer;

        hDS = OGROpen( xFileName, FALSE, NULL );

        if(hDS == NULL)
        {
            writeToConsole("Cannot open station file!");
            return -1; //very bad!
        }

        poLayer = (OGRLayer*)OGR_DS_GetLayer( hDS, 0 );
        hLayer=OGR_DS_GetLayer(hDS,0);
        OGR_L_ResetReading(hLayer);
        poLayer->ResetReading();

        GIntBig iBig = 1;
        GIntBig idx0 = poLayer->GetFeatureCount();
        GIntBig idx1 = idx0-iBig;
        GIntBig idx2;

        idx2 = poLayer->GetFeatureCount();

        CPLDebug("STATION_FETCH","Number of Time Entries: %llu",idx2); //How many lines are on disk
        QString qFileName = QFileInfo(xFileName).fileName();
        writeToConsole(QString(qFileName+" has: "+QString::number(idx2)+" time entries")); //tell the user in the console
        const char* emptyChair; //Muy Importante!

        poFeature = poLayer->GetFeature(iBig);
        if (poFeature==NULL)
        {
            writeToConsole("No Stations Found in file!");
            return -1; //If there are no stations in the csv!
        }
    //    startTime = poFeature->GetFieldAsString(15);
        std::string start_datetime(poFeature->GetFieldAsString(15));
        poFeature = poLayer->GetFeature(idx2);
        std::string stop_datetime(poFeature->GetFieldAsString(15));

        CPLDebug("STATION_FETCH","STATION START TIME: %s",start_datetime.c_str());
        CPLDebug("STATION_FETCH","STATION END TIME: %s",stop_datetime.c_str());

        if (start_datetime.empty()==true && stop_datetime.empty()==true)
        {
            //Means that there is not a time series
            CPLDebug("STATION_FETCH", "File cannot be used for Time Series");
            instant = 1;
        }
        if (start_datetime.empty()==false && stop_datetime.empty()==false) //Definately some sort of time series
        {
            CPLDebug("STATION_FETCH","File can be used for Times Series");
            CPLDebug("STATION_FETCH","Suggesting Potentially Reasonable Time Series Parameters...");

            readStationTime(start_datetime,stop_datetime,idx2); //Turns the Start and Stop times into local timess......
            ssidx<<idx2;
            idx3=ssidx.str();
        }
    }

    //Determine what file type we are dealing with...
    if(stationHeader == 1)
    {
        //Old Format
        //Prevent users from using time series with old format
        enableTimeseries=false;
        startTime->setEnabled(false); //Hide these things because its an old format run
        stopTime->setEnabled(false);
        numSteps->setEnabled(false);

        //Try flipping the UI
        startLabel->setVisible(false); //Hide all the timesries stuff
        stopLabel->setVisible(false);
        stepLabel->setVisible(false);

        startTime->setVisible(false);
        stopTime->setVisible(false);
        numSteps->setVisible(false);

        dateTimeEdit->setVisible(true); //show the date time box to set the sim time
        diurnalLabel->setVisible(true);
        oneStepTimeLabel->setVisible(false); // hide the 1 step time box thing for instant==1

        //Update Diurnal Time
        if (isDiurnalChecked==true)
        {
            dateTimeEdit->setEnabled(true); //enable the diurnal Box
            updateSingleTime(dateTimeEdit->dateTime()); //set the time from the diurnal box
        }


        return 0;
    }
    if (stationHeader == 2 && instant == 0)
    {
        //new Foramt w/ Time Series
        //Turn on Several options for controlling time series in the GUI
        enableTimeseries=true;
        startTime->setEnabled(true);
        stopTime->setEnabled(true);
        numSteps->setEnabled(true);
        updateStartTime(startTime->dateTime());
        updateStopTime(stopTime->dateTime());

        //try flipping the UI
        startLabel->setVisible(true);
        stopLabel->setVisible(true);
        stepLabel->setVisible(true);

        startTime->setVisible(true);
        stopTime->setVisible(true);
        numSteps->setVisible(true);

        dateTimeEdit->setVisible(false); //hide the single step stuff
        diurnalLabel->setVisible(false);
        oneStepTimeLabel->setVisible(false);
        return 1;
    }
    if (stationHeader == 2 && instant == 1)
    {
        //new Format no Time Series
        //Prevent users from using time series with 1 step
        enableTimeseries=false;
        startTime->setEnabled(false);
        stopTime->setEnabled(false);
        numSteps->setEnabled(false);

        //Try flipping the UI
        startLabel->setVisible(false);
        stopLabel->setVisible(false);
        stepLabel->setVisible(false);

        startTime->setVisible(false);
        stopTime->setVisible(false);
        numSteps->setVisible(false);

        dateTimeEdit->setVisible(false);
        diurnalLabel->setVisible(false);
        dateTimeEdit->setEnabled(false);

        const char *optChangeTime = CPLGetConfigOption("CHANGE_DATE_TIME","FALSE"); //Allow the user the change the datetime via  config option
        if(optChangeTime!="FALSE")
        {
            QString time_format = "yyyy-MM-ddTHH:mm:ss";
            QString optXTime = QString::fromAscii(optChangeTime);
            QDateTime opt_time_obj = QDateTime::fromString(optXTime,time_format);
            updateSingleTime(opt_time_obj);
            QString oneStepText = "Simulation time set to: "+optXTime;
            oneStepTimeLabel->setText(oneStepText);
            oneStepTimeLabel->setVisible(true); // to this label in the GUI
        }
        else
        {
            //Reads the sim time from the file the user provides;
            QDateTime singleRunTime = readNinjaNowName(xFileName); //Read in the date time
            updateSingleTime(singleRunTime); //update it globally
            QString runTimeText = singleRunTime.toString(); //Print it out for the user
            QString oneStepText = "Simulation time set to: "+runTimeText;
            oneStepTimeLabel->setText(oneStepText);
            oneStepTimeLabel->setVisible(true); // to this label in the GUI
        }
        return 2;

    }
    if (stationHeader == 3)
    {
        //Invalid header type for GUI run...
        return -1;
    }
    if (stationHeader == 4)
    {
        //Invalid header type for GUI run...
        return -1;
    }
    else
    {
        //Something wrong with the station...
        return -1;
    }
}
/**
 * @brief pointInput::readStationTime: Takes in times read from disk, turns them into
 * Qt datetime objects, then updates the gui with suggested ranges of times based on available disk data
 * tries to stop users from picking frivolous time ranges (maybe). Also suggests a number of timesteps based on
 * available data.
 * @param start_time
 * @param stop_time
 * @param xSteps
 */
void pointInput::readStationTime(string start_time, string stop_time, int xSteps)
{
    QString q_time_format = "yyyy-MM-ddTHH:mm:ssZ";
    QString start_utcX = QString::fromStdString(start_time);
    QString end_utcX = QString::fromStdString(stop_time);

    QDateTime start_qat = QDateTime::fromString(start_utcX,q_time_format);
    QDateTime end_qat = QDateTime::fromString(end_utcX,q_time_format);

    start_qat.setTimeSpec(Qt::UTC);
    end_qat.setTimeSpec(Qt::UTC); //Set the Time to UTC
    
    
    //// convert the input start and stop times into boost local date time objects, so convert from UTC time to local time
    
    blt::time_zone_ptr tz; // Initialize time zone
    tz = globalTimeZoneDB.time_zone_from_region(tzString.toStdString()); // Get time zone from database
    
    // parse the start time into date and time parts
    int start_year = start_qat.date().year();
    int start_month = start_qat.date().month();
    int start_day = start_qat.date().day();    
    int start_hour = start_qat.time().hour();
    int start_minute = start_qat.time().minute();
    int start_sec = start_qat.time().second();
    
    // parse the start time into date and time parts
    int stop_year = end_qat.date().year();
    int stop_month = end_qat.date().month();
    int stop_day = end_qat.date().day();    
    int stop_hour = end_qat.time().hour();
    int stop_minute = end_qat.time().minute();
    int stop_sec = end_qat.time().second();
    
    // make intermediate start and stop dates for generating ptime objects
    bg::date startTime_date(start_year,start_month,start_day);
    bg::date stopTime_date(stop_year,stop_month,stop_day);
    bpt::time_duration startTime_duration(start_hour,start_minute,start_sec,0);
    bpt::time_duration stopTime_duration(stop_hour,stop_minute,stop_sec,0);
    
    // these are UTC times
    bpt::ptime startTime_ptime(startTime_date,startTime_duration);
    bpt::ptime stopTime_ptime(stopTime_date,stopTime_duration);
    
    // this constructor generates local times from UTC times
    blt::local_date_time boost_local_startTime( startTime_ptime, tz );
    blt::local_date_time boost_local_stopTime( stopTime_ptime, tz );
    
    
    //// convert the boost local date time objects into QDateTime objects
    
    std::string os_time_format = "%Y-%m-%dT%H:%M:%SZ";  // this is the ostringstream format string that replicates the above QDateTime format string
    
    blt::local_time_facet* facet;
    facet = new blt::local_time_facet();
    
    facet->format(os_time_format.c_str());
    
    
    // calculate the start_time QDate
    std::ostringstream os;
    os.imbue(std::locale(std::locale::classic(), facet));
    os << boost_local_startTime;
    QString loc_start_time_qStr = QString::fromStdString( os.str() );
    os.str("");  // reset for parsing the next time
    os.clear();
    QDateTime loc_start_time = QDateTime::fromString(loc_start_time_qStr,q_time_format);
    
    // calculate the end_time QDate
    os.imbue(std::locale(std::locale::classic(), facet));
    os << boost_local_stopTime;
    QString loc_end_time_qStr = QString::fromStdString( os.str() );
    os.str("");  // reset for parsing the next time
    os.clear();
    QDateTime loc_end_time = QDateTime::fromString(loc_end_time_qStr,q_time_format);
    
    CPLDebug("STATION_FETCH","qdate start_local = %s",loc_start_time.toString(q_time_format).toStdString().c_str());
    CPLDebug("STATION_FETCH","qdate end_local = %s",loc_end_time.toString(q_time_format).toStdString().c_str());
    
    
    //// now use the final local time QDate start and stop times
    writeToConsole("Start Time (local): "+loc_start_time.toString()); //Tell console what the
    writeToConsole("Stop Time (local): "+loc_end_time.toString()); //Local time is

    startTime->setDateTime(loc_start_time);
    stopTime->setDateTime(loc_end_time); //Updates date time based on disk information

    updateTimeSteps(); //Calculate how many steps we can do between the start and stop time
}
/**
 * @brief pointInput::displayInformation
 * Displays important stuff to the user based on what they select
 * Warns them if they select 2 types of files that are incompatible.
 * @param dataType
 */
void pointInput::displayInformation(int dataType)
{
    CPLDebug("STATION_FETCH","Data Format: %i",dataType);
    if(dataType == 0)
    {
        clippit->setText("Run Type: Old Format");
        pointGo=true;
        if (isDiurnalChecked==true)
        {
            dateTimeEdit->setEnabled(true);
        }
        if (stationFileList.size()>1)
        {
            clippit->setText("Too many stations selected for data type");
        }
    }
    if(dataType == 1)
    {
        clippit->setText("Run Type: Time Series");
        pointGo=true;
        if (dateTimeEdit->isEnabled())
        {
            dateTimeEdit->setEnabled(false);
        }
    }
    if(dataType == 2)
    {
        clippit->setText("Run Type: Single Step");
        pointGo=true;
    }
    if (dataType == -1 && stationFileList.size()==0) //Special Case
    {
        clippit->setText("No Stations Selected...");
        pointGo=false;
    }
    if (dataType == -1 && stationFileList.size()==1) //Case of 1 file that is crap
    {
        clippit->setText("No Valid Data detected in file...");

        pointGo=false;
    }
    if (dataType==-1 && stationFileList.size()>=2)
    {
        clippit->setText("MULTIPLE TYPES SELECTED, CANNOT PROCEED!");

        pointGo=false;
    }
//    else if(dataType == -1)
//    {
//        clippit->setText("MULTIPLE TYPES SELECTED, CANNOT PROCEED!");
//        pointGo=false;
//    }
}
/**
 * @brief pointInput::readNinjaNowName
 * @param fileName
 * This is for current step new format runs
 * we need time if the user turnsl on diurnal/stability input
 * Read the Time from the date created attribute attached to the file
 *
 * @return
 */
QDateTime pointInput::readNinjaNowName(const char *fileName)
{
    CPLDebug("STATION_FETCH","Reading 1 step Station start Time");
    QDateTime qxDate = QFileInfo(fileName).created(); //Get when the file was created because that is when the simulation time is
    return qxDate;
}
/**
 * @brief pointInput::setOneStepTimeseries
 * If one step is set, diable stop time
 * and only use start time
 */
void pointInput::setOneStepTimeseries()
{
    if(numSteps->value()==1)
    {
        CPLDebug("STATION_FETCH","One Step Set for Timeseries, greying out stop time!");
        stopTime->setEnabled(false);
        stopTime->setToolTip("Stop time is disabled for 1 time step simulations");
    }
    else
    {
        stopTime->setEnabled(true);
        stopTime->setToolTip("Enter the simulation stop time");
    }
}


/**
 * @brief pointInput::setWxStationFormat
 * sets the format of the wxStation
 * not used right now
 * @param format
 */
void pointInput::setWxStationFormat(int format)
{
       wxStationFormat = format;
}

void pointInput::selChanged(const QItemSelection &x, const QItemSelection &y) //Generic test function, delete once everything is good
{
    CPLDebug("STATION_FETCH","TEST");
}

void pointInput::openMainWindow() //This is for opening and closing the station-fetch widget
{
    this->setEnabled(true);
}

/** Allows mainwindow to update the timezone from the DEM or as provided by the user
 * @brief pointInput::updateTz
 * @param tz
 */

void pointInput::updateTz(QString tz)
{
    tzString = tz;
}

/** Pairs start and stop times to what the stationFetchWidget does.
 * @brief pointInput::pairStartTime
 * @param xDate
 */
void pointInput::pairStartTime(QDateTime xDate) //These functions take the start and stop time from the widget
//and populate the timeseries objects in the gUI
{
    startTime->setDateTime(xDate);    
}
void pointInput::pairStopTime(QDateTime xDate)
{
    stopTime->setDateTime(xDate);
}
/** Groups all the different timseries parts to be enabled/disabled at the same time based on
 * user options
 * @brief pointInput::pairTimeSeries
 * @param curIndex
 */
void pointInput::pairTimeSeries(int curIndex)
{
    if(curIndex==0)
    {
        enableTimeseries=true;
        startTime->setEnabled(false);
        stopTime->setEnabled(false);
        numSteps->setEnabled(false);
    }
    if(curIndex==1)
    {
        enableTimeseries=true;
        startTime->setEnabled(true);
        stopTime->setEnabled(true);
        numSteps->setEnabled(true);
    }
}
/**
 * @brief pointInput::watchStopTime
 * corrects user selecting stop time behind start time!
 */
void pointInput::watchStopTime() //Stop time cannot be farther in the future than the stop time
{
    if(stopTime->dateTime()<startTime->dateTime())
    {
        writeToConsole("Start Time is greater than Stop Time!");
        CPLDebug("STATION_FETCH","START TIME > END TIME, FIXING START TIME!");
        startTime->setDateTime(stopTime->dateTime().addSecs(-3600));
    }
}
/**
 * @brief pointInput::watchStartTime
 * corrects the stop time if the user picks a start time farther in the future than the stop
 * time
 */
void pointInput::watchStartTime() //Stop time cannot be farther in the future than the stop time
{
    if(stopTime->dateTime()<startTime->dateTime())
    {
        writeToConsole("Start Time is greater than Stop Time!");
        CPLDebug("STATION_FETCH","START TIME > END TIME, FIXING STOP TIME!");
        stopTime->setDateTime(startTime->dateTime().addSecs(3600));
    }
}
/** Updates the timeseries start time based on user requests
 * @brief pointInput::updateStartTime
 * @param xDate
 */
void pointInput::updateStartTime(QDateTime xDate)
{
    startSeries.clear(); //delete whatever is stored in the vector
    int year,month,day,hour,minute;
    year = xDate.date().year();
    month = xDate.date().month();
    day = xDate.date().day();    
    hour = xDate.time().hour();
    minute = xDate.time().minute();

    updateTimeSteps(); //when we change the time, update the math on the time steps
    CPLDebug("STATION_FETCH","UPDATED START TIME: %i %i %i %i %i",year,month,day,hour,minute);

    startSeries.push_back(year);
    startSeries.push_back(month);
    startSeries.push_back(day);
    startSeries.push_back(hour);
    startSeries.push_back(minute);
}
/** Updates the time for stopping the simulation based on user requests
 * @brief pointInput::updateStopTime
 * @param xDate
 */
void pointInput::updateStopTime(QDateTime xDate)
{
    endSeries.clear(); //delete whatever is stored in the vector
    int year,month,day,hour,minute;
    year = xDate.date().year();
    month = xDate.date().month();
    day = xDate.date().day();    
    hour = xDate.time().hour();
    minute = xDate.time().minute();

    updateTimeSteps(); //when we change the time, update the math on the time steps

    CPLDebug("STATION_FETCH","UPDATED STOP TIME: %i %i %i %i %i",year,month,day,hour,minute);
    
    endSeries.push_back(year);
    endSeries.push_back(month);
    endSeries.push_back(day);
    endSeries.push_back(hour);
    endSeries.push_back(minute);
}
/**
 * @brief pointInput::updateSingleTime
 * @param xDate
 *
 * For single step runs, set the simulation time based on the read station file time,
 * see also:
 * readNinjaNowName()
 *
 */
void pointInput::updateSingleTime(QDateTime xDate)
{
    int year,month,day,hour,minute;
    year = xDate.date().year();
    month = xDate.date().month();
    day = xDate.date().day();
    hour = xDate.time().hour();
    minute = xDate.time().minute();

    CPLDebug("STATION_FETCH","UPDATED SINGLE STEP TIME: %i %i %i %i %i",year,month,day,hour,minute);

    diurnalTimeVec.push_back(year);
    diurnalTimeVec.push_back(month);
    diurnalTimeVec.push_back(day);
    diurnalTimeVec.push_back(hour);
    diurnalTimeVec.push_back(minute);

}
/**
 * @brief pointInput::updateTimeSteps
 * Suggest a number of time steps based on the
 * start and stop time
 *
 * this function sets the number of time steps to the number of
 * hours between the start and stop time
 *
 * if its less than 2 hours,
 * suggest two steps
 *
 * the user can then change this if they really want
 *
 */
void pointInput::updateTimeSteps()
{
    CPLDebug("STATION_FETCH","Updating Suggested Time steps....");
    int u_start = startTime->dateTime().toTime_t();
    int u_stop = stopTime->dateTime().toTime_t();
    
    if ( u_start == u_stop ) {
        
        numSteps->setValue(1);
        
        // additionally, disable gui's ability to edit the values of the start and stop times, and the num steps, it is a single time
        
        // Prevent users from using time series with 1 step
        // enableTimeseries=false;  // this one breaks it
        startTime->setEnabled(false);
        stopTime->setEnabled(false);
        numSteps->setEnabled(false);
        
        // looks like the above worked, I was surprised that this didn't need to get set at the very end of direct station traffic instead of here to work, 
        // using an if statement check on the number of timesteps to trigger if numSteps was ever equal to 1
        
    } else {
        
        int u_diff = (u_stop - u_start)/(3600); //calculate the number of hours between the start and stop times

        if(u_diff<=2) //if its less than 2 hours, make it 2
        {
            u_diff = 2;
        }

        numSteps->setValue(u_diff);
    
    }
}
/**
 * @brief pointInput::openStationFetchWidget
 * Opens the downloader widget to download station files
 */
void pointInput::openStationFetchWidget()
{
    xWidget = new stationFetchWidget();
    QSignalMapper *signalMapper;
    connect(xWidget, SIGNAL(exitWidget()),this, SLOT(openMainWindow())); //Launches Widget Connector
    connect(xWidget, SIGNAL(destroyed()),this,SLOT(openMainWindow())); //Some sort of deconstructor thing
    this->setEnabled(false); //disable the main window
    xWidget->setInputFile(demFileName); //give the widget the dem file
    xWidget->updatetz(tzString); //give the widget the time zone as a string
    connect(xWidget, SIGNAL(exitWidget()),this, SLOT(checkForModelData())); //Launches Widget Connector
    connect(xWidget->startEdit,SIGNAL(dateTimeChanged(const QDateTime)),this,SLOT(pairStartTime(const QDateTime))); //connect the various time boxes to eachother
    connect(xWidget->endEdit,SIGNAL(dateTimeChanged(const QDateTime)),this,SLOT(pairStopTime(const QDateTime)));
    connect(xWidget->timeLoc,SIGNAL(currentIndexChanged(int)),this,SLOT(pairTimeSeries(int))); //Connects What the user does in the widget
            //to what the timeseries checkbox does   
}
/** Allows mainwindow to update pointInput with changes to the DEM
 * @brief pointInput::setInputFile
 * @param file
 */
void pointInput::setInputFile( QString file )
{
    demFileName = file;
    cwd = QFileInfo(file).absolutePath();
}
/** Allows mainWindow to update pointInput with changes in diurnal/stability options
 * @brief pointInput::setDiurnalParam
 * @param diurnalCheck
 */
void pointInput::setDiurnalParam(bool diurnalCheck)
{
    isDiurnalChecked = diurnalCheck;//Note that this works for stability too
    CPLDebug("STATION_FETCH","DIURNAL/STABILITY STATUS: %i",isDiurnalChecked);
}


/**
 * *@brief pointInput::checkForModelData
 * Applies filters to the tree
 * Also clears selection when the user clicks it
 */
void pointInput::checkForModelData()
{
    stationFileList.clear(); //Clear the list
    treeView->selectionModel()->clear(); //Clear the models selections
    pointGo = false; //Set the pointInput bool to false just to be extra explicit
    QDir wd(cwd);
    QStringList filters;
    filters<<"*.csv"; //Only show CSV
    filters<<"WXSTATIONS-*"; //Add downloadable directories to filters    
    sfModel->setNameFilters(filters);
    sfModel->setFilter(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot); //QDir::Dir specifies to add filters to directories
    treeView->setRootIndex(sfModel->index(wd.absolutePath()));
    treeView->resizeColumnToContents(0);
}
