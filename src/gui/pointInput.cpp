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

//This is not needed for now, was the way to switch between old and new formats
//    initOpt = new QComboBox( this ); //Short for initialization Options, so we can switch the GUI between the old format and the new format
//    initOpt->addItem("Old Format"); //needs a new name
//    initOpt->addItem("New Format"); //needs a new name
//    initOpt->setCurrentIndex(1);
//    initOpt->setVisible(false);

    pointGo=false; //Very Imptortant!

//    initPages = new QStackedWidget(this); //For use in the deprecated switcher
//    oldForm = new QWidget(); //Something that was used in the deprecated switching stuff
    newForm = new QWidget();

// This was the old way of loading stations files, leaving in for now...
//    stationFileLineEdit = new QLineEdit( newForm );
//    stationFileLineEdit->setReadOnly( true );
//    stationFileLineEdit->setGeometry(QRect(10,0,141,20));
//    stationFileLineEdit->setVisible(false);

//This is the old way of setting diurnal paramters, currently need to add this in somewhere....
    dateTimeEdit = new QDateTimeEdit( newForm );
    dateTimeEdit->setDateTime( QDateTime::currentDateTime() );
    dateTimeEdit->setCalendarPopup( true );
    dateTimeEdit->setDisplayFormat( "MM/dd/yyyy HH:mm" );
    dateTimeEdit->setEnabled( false ); //This is for Old Format Diurnal Simulations
    dateTimeEdit->setVisible(false);
    dateTimeEdit->setToolTip("Set date and time for single time step diurnal/stability simulations");

    diurnalLabel = new QLabel(this);
    diurnalLabel->setText("Thermal Parameters: ");
    diurnalLabel->setVisible(false);

//Old way of reading station files, no longer needed...
//    readStationFileButton =  new QToolButton( this ); //Opens old Format Station
//    readStationFileButton->setText( tr( "Read Station File" ) );
//    readStationFileButton->setIcon( QIcon( ":weather_cloudy.png" ) );
//    readStationFileButton->setToolButtonStyle( Qt::ToolButtonTextBesideIcon );
//    readStationFileButton->setGeometry(QRect(10, 20, 151, 26));
//    readStationFileButton->setVisible(false);

    writeStationFileButton =  new QCheckBox( this ); //This writes an interpolated csv of the weather data (we might not want this)
    writeStationFileButton->setText( tr( "Write Station File" ) );
    writeStationFileButton->setIcon( QIcon( ":weather_clouds.png" ) );
    writeStationFileButton->setToolTip("Time Series: Writes an Interpolated CSV for each time step\nSingle Step: Writes a CSV of inputted weather data.");
//    writeStationFileButton->setToolButtonStyle( Qt::ToolButtonTextBesideIcon );

    writeStationKmlButton =  new QCheckBox( this ); //This writes a KML of the weather stations (1 per run)
    writeStationKmlButton->setText( tr( "Write Station Kml" ) );
    writeStationKmlButton->setIcon( QIcon( ":weather_cloudy.png" ) );
    writeStationKmlButton->setToolTip("Time Series: Writes a KML for each time step showing interpolated weather data.\nSingle Step: Writes a KML of inputted weather data.");
//    writeStationKmlButton->setToolButtonStyle( Q/t::ToolButtonTextBesideIcon );
//    writeStationKmlButton->setIcon( QIcon( ":weather_cloudy.png" ) );
//    writeStationKmlButton->setStyle(Qt::ToolButtonTextBesideIcon);
//    writeStationKmlButton->setToolButtonStyle( Qt::ToolButtonTextBesideIcon );

    widgetButton = new QToolButton( this ); //This opens the station fetch downloader Widget (formerley doTest)
    widgetButton->setText( tr( "Download Weather Stations" ));
    widgetButton->setIcon(QIcon(":world.png"));
    widgetButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    widgetButton->setToolTip("Download Weather Data from the Mesowest API");

    //Progress Bar Stuff -> Delete Later

    execProg = new QToolButton(this);
    execProg->setText("Execute Progress");
    execProg->setIcon(QIcon(":weather_sun.png"));
    execProg->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    execProg->setToolTip("This is a test...");
    execProg->setVisible(false);

    xProg = new QProgressDialog(this);
    xProg->setWindowModality(Qt::ApplicationModal);
    xProg->setAutoReset(false);
    xProg->setAutoClose(false);

//    stationTreeView = new QTreeView( this );
//    stationTreeView->setModel( &pointData ); //This is some sort of deprecated tree thing that existed before

//####################################################
// New Format Tree Stuff                             #
//####################################################

    sfModel = new QDirModel(this); //Creates the directory model
    sfModel->setReadOnly(true); //probably can be true, but i don't know
    sfModel->setSorting(QDir::Time); //Sort by time created

    treeView = new QTreeView(this); //Creates the box where the sfModel goes
    treeView->setVisible(true); //deprecated
    treeView->setModel(sfModel); //Sets the model to the thing above
    treeView->header()->setStretchLastSection(true); //Makes it look nice
    treeView->setAnimated(true); //Fancy stuff
    treeView->setColumnHidden(1, true);
    treeView->setColumnHidden(2, true);
    treeView->setAlternatingRowColors( true );
    treeView->setSelectionMode(QAbstractItemView::MultiSelection); //Allows multiple files to be selected
//    treeView->setSelectionMode(QAbstractItemView::ExtendedSelection);


    treeLabel = new QLabel(tr("Select Weather Stations")); //Label for Tree and sfModel
    treeLabel->setToolTip("Select Weather Stations from available files, three formats are supported, old format, time series and 1 step runs");
    

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

    xvLine1 = new QFrame(this);
    xvLine1->setFrameShape(QFrame::VLine);
    xvLine1->setFrameShadow(QFrame::Sunken);

    xvLine2 = new QFrame(this);
    xvLine2->setFrameShape(QFrame::VLine);
    xvLine2->setFrameShadow(QFrame::Sunken);

    ClippyToolLayout->addStretch(); //Moves it over to the other side
    ClippyToolLayout->addWidget(refreshToolButton);
//    ClippyToolLayout->addWidget(clippit);

    //File Info Area;
    /*
     * dispaly information to the user about the file they select
     * start, stop and number of times steps
     */
    selectedFileLayout = new QHBoxLayout;
    fileStartLayout = new QVBoxLayout;
    fileEndLayout = new QVBoxLayout;
    fileStepLayout = new QVBoxLayout;

    fileStart = new QLabel;
    fileStartVal = new QLabel;
    fileStart->setText("File Start Time: ");
    fileStartVal->setText("");

    fileEnd = new QLabel;
    fileEndVal = new QLabel;
    fileEnd->setText("File End Time: ");
    fileEndVal->setText("");

    fileSteps = new QLabel;
    fileStepsVal = new QLabel;
    fileSteps->setText("Number of Time Steps: ");
    fileStepsVal->setText("");

    fileStartLayout->addWidget(fileStart);
    fileStartLayout->addWidget(fileStartVal);

    fileEndLayout->addWidget(fileEnd);
    fileEndLayout->addWidget(fileEndVal);

    fileStepLayout->addWidget(fileSteps);
    fileStepLayout->addWidget(fileStepsVal);

    fileStart->setVisible(false);
    fileEnd->setVisible(false);
    fileSteps->setVisible(false);

    fileStartVal->setVisible(false);
    fileEndVal->setVisible(false);
    fileStepsVal->setVisible(false);

    xvLine1->setVisible(false);
    xvLine2->setVisible(false);
    timeLine2->setVisible(false);

    selectedFileLayout->addLayout(fileStartLayout);
    selectedFileLayout->addWidget(xvLine1);
    selectedFileLayout->addLayout(fileEndLayout);
    selectedFileLayout->addWidget(xvLine2);
    selectedFileLayout->addLayout(fileStepLayout);

    //New Custom Layout //Deprecated...
//    treeLayout = new QHBoxLayout;
//    treeLayout->addWidget(treeView);
//    treeLayout->addWidget(refreshToolButton);

//####################################################
//End Directory. Start Timeseries Box                #
//####################################################
    startTime = new QDateTimeEdit(QDateTime::currentDateTime()); //Initializes the start and stop time boxes for timeseries
    stopTime = new QDateTimeEdit(QDateTime::currentDateTime());
    
    startTime->setDateTime(QDateTime::currentDateTime().addDays(-1)); //Set default to be yesterday
    startTime->setMaximumDateTime(QDateTime::currentDateTime().addSecs(-3600)); //Prevent time from overlapping with right now
    stopTime->setMaximumDateTime(QDateTime::currentDateTime()); //No Future simulations
    
    startTime->setDisplayFormat( "MM/dd/yyyy HH:mm" );
    stopTime->setDisplayFormat( "MM/dd/yyyy HH:mm" );
    
    enableTimeseries = new QCheckBox(this); //Initializes the timeseries checkbox
    enableTimeseries->setText(tr("Enable Timeseries"));

    labelTimeseries = new QLabel(this);
    labelTimeseries->setText("Time Series\nOptions");
    labelTimeseries->setVisible(false); //This function is probably going to go away, make invisible for now
    
    numSteps = new QSpinBox; //Number of timesteps box
    numSteps->setValue(24);
    numSteps->setMinimum(1);
    numSteps->setMaximum(99999);//Hopefully big enough
    
    startTime->setVisible(true); //Some visibility settings that probably aren't necessary anymore...
    stopTime->setVisible(true);
    enableTimeseries->setVisible(false); //Try Hiding the button without much Work

    startTime->setEnabled(false); //Disables timeseries options unless the user checks it or the system detects that the user is doing a timeseries
    stopTime->setEnabled(false);

    numSteps->setVisible(true);
    numSteps->setEnabled(false);
    
    timeBoxLayout = new QHBoxLayout; //Layout setup
    startLayout = new QVBoxLayout;
    stopLayout = new QVBoxLayout;
    stepLayout = new QVBoxLayout;
    
    startLabel = new QLabel(tr("Start Time"));
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
    
//    timeBoxLayout->addWidget(enableTimeseries);
//    timeBoxLayout->addWidget(startTime);
//    timeBoxLayout->addWidget(stopTime);
//    timeBoxLayout->addWidget(numSteps);
    timeBoxLayout->addWidget(enableTimeseries); //Adds all the parts together
    timeBoxLayout->addWidget(labelTimeseries);
    timeBoxLayout->addLayout(startLayout);
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
//    vTreeLayout->addWidget(treeLabel);
    vTreeLayout->addWidget(treeView);
//    vTreeLayout->addWidget(refreshToolButton);
    vTreeLayout->addLayout(ClippyToolLayout);
    vTreeLayout->addWidget(timeLine);
//    vTreeLayout->addLayout(selectedFileLayout);
//    vTreeLayout->addWidget(timeLine2);
    vTreeLayout->addLayout(timeBoxLayout);
//    vTreeLayout->addWidget(dateTimeEdit);
//####################################################
//END SF CUSTOM                                      #
//####################################################
//    optLayout = new QHBoxLayout; //This is deprecated...
//    optLayout->addWidget(initOpt);

//    fileLayout = new QHBoxLayout; //Old stuff for original point Initilization, leave for now
//    fileLayout->addWidget( stationFileLineEdit );
//    fileLayout->addWidget(ska);
//    fileLayout->addWidget( readStationFileButton );
//    initPages->addWidget(oldForm);
//    fileLayout->addWidget(initPages);
//    fileLayout->addWidget(treeView);
//    fileLayout->addWidget(refreshToolButton);
//    fileLayout->addWidget(jazz);

    buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget( writeStationFileButton );
//    writeStationFileButton->setVisible( false ); //This was disabled in the original PI
    buttonLayout->addWidget( writeStationKmlButton );
    buttonLayout->addWidget(execProg);
//    buttonLayout->addWidget(widgetButton); //Old Download Button Location, keep for now ->Moved to hDownloaderLayout
    buttonLayout->addStretch();

    diurnalTimeLayout = new QHBoxLayout;
    diurnalTimeLayout->addWidget(diurnalLabel);
    diurnalTimeLayout->addWidget(dateTimeEdit,1);
//    diurnalTimeLayout->addStretch(-1);

    pointLayout = new QVBoxLayout;
//    pointLayout->addWidget( stationTreeView ); //very old stuff
//    stationTreeView->setVisible( false );
//    pointLayout->addLayout(optLayout);
//    pointLayout->addLayout( fileLayout );

    pointLayout->addLayout(vTreeLayout);

    pointLayout->addLayout(diurnalTimeLayout);

    pointLayout->addStretch();
    pointLayout->addLayout( buttonLayout );

    pointGroupBox->setLayout( pointLayout );

//    cwd = QFileInfo("/home/tanner/src/wind/fs_gui_files/kmso.tif").absolutePath(); //Demo Path

//    stationTreeView->setRootIndex(model->index());

//    treeView->setModel(model);
//    treeView->header()



    ninjafoamConflictLabel = new QLabel(tr("The point initialization option is not currently available for the momentum solver.\n"),
                                        this);
    ninjafoamConflictLabel->setHidden(true);

    layout = new QVBoxLayout;
    layout->addWidget( pointGroupBox );
    layout->addWidget(ninjafoamConflictLabel);
//    layout->addStretch();

//    if (initOpt) //No Longer needed...
//    cout<<initOpt->currentIndex()<<endl;
    setLayout( layout );
//    connect( readStationFileButton, SIGNAL( clicked() ), this,
//         SLOT( readStationFile() ) ); //For Old Format (for now)
    connect( writeStationFileButton, SIGNAL( clicked() ), this,
         SLOT( writeStationFile() ) ); //Writes a CSV Connector
    connect( writeStationKmlButton, SIGNAL( clicked() ), this,
         SLOT( writeStationKml() ) ); //Writes a KML connector
    connect( widgetButton ,SIGNAL( clicked () ), this,
             SLOT(openStationFetchWidget())); //Opens the Downloader Connector
//    connect(initOpt,SIGNAL(currentIndexChanged(int)), this,SLOT(toggleUI())); //Changes to New Format and back Connector (DEPRECATED...)
    connect(refreshToolButton, SIGNAL(clicked()), //Refreshes new Format
        this, SLOT(checkForModelData()));
    connect(enableTimeseries,SIGNAL(clicked()),this,
            SLOT(toggleTimeseries()));
    connect(startTime,SIGNAL(dateTimeChanged(QDateTime)),this,SLOT(updateStartTime(QDateTime)));
    connect(stopTime,SIGNAL(dateTimeChanged(QDateTime)),this,SLOT(updateStopTime(QDateTime)));

//    connect(treeView,SIGNAL(QTreeView::currentChanged(const QModelIndex&, const QModelIndex&);),
//            this,SLOT(readMultipleStaitonFiles()));
    connect(treeView, SIGNAL(clicked(const QModelIndex &)),
        this, SLOT(readMultipleStaitonFiles(const QModelIndex &))); //Connects click to which files should be conencted
//    connect(sfModel, SIGNAL(/*selectionChanged*/()),this,SLOT(selChanged()));
//    connect(treeView, SIGNAL(QItemSelectionModel::selectionChanged(const QItemSelection &, const QItemSelection &)),
//        this, SLOT(readMultipleStaitonFiles(const QModelIndex &)));

    connect(execProg,SIGNAL(clicked(bool)),this,SLOT(testProg()));



    stationFileName = ""; //Sets a default
    
}

pointInput::~pointInput()
{

}

void pointInput::readStationFile() //This is the old way of loeading station files
{
    //    QString fileName;
    //    fileName = QFileDialog::getOpenFileName(this, tr("Open station file"),
    //                                             QFileInfo(stationFileName).path(),
    //                                              tr("Comma separated value files (*.csv)"));

    //    if(fileName.isEmpty() || demFileName.isEmpty()) {
    //        return;
    //    }

    //    stationFileName = fileName;
    //    stationFileLineEdit->setText(QFileInfo(fileName).fileName());
    //    cout<<stationFileName.toStdString()<<endl;
    //    emit stationFileChanged();
}
//This is all pretty good, needs to be cleaned up
//Checks how many stations there are selected
//Important for readMultipleStations
int pointInput::checkNumStations(std::string comparator, std::vector<std::string> stationVec)
{
    int cx = 0;
    for (int i=0;i<stationVec.size();i++)
    {
        if(stationVec[i]==comparator)
        {
            cx++;
        }
    }
    return cx;
}

/**
 * @brief pointInput::readMultipleStaitonFiles
 * //General Idea: Append all selections a user makes, get the unique ones, counter the number of clicks, if it is odd, keep the file, if even, don't keep it
    // This is because odd means it was selected at least once at then left alone
    // even means that it was selected and deselected
 * @param index
 */
void pointInput::readMultipleStaitonFiles(const QModelIndex &index)
{

    QFileInfo fi(sfModel->fileInfo(index));
    std::vector<std::string> finalStations;
    std::vector<int> finalTypes;
    std::string filename = fi.absoluteFilePath().toStdString();
    vx.push_back(filename);

//    vy.push_back(filename);
//    cout<<filename<<endl;
//    std::vector<std::string> uniNames = std::unique(fileNameVec.begin(),fileNameVec.end());
//    std::sort(vx.begin(),vx.end());
//    vx.erase(std::unique(vx.begin(),vx.end()),vx.end());
//    std::vector<std::string>::iterator it = std::unique(fileNameVec.begin(),fileNameVec.end());
    std::set<std::string> unique(vx.begin(),vx.end());

    std::vector<std::string> uniVec(unique.begin(),unique.end());
    std::vector<int> totNum;
    for (int i=0;i<uniVec.size();i++)
    {
//        cout<<uniVec[i]<<endl;
        int single_num = checkNumStations(uniVec[i],vx);
        totNum.push_back(single_num);
        if (totNum[i]%2!=0)
        {
//          cout<<uniVec[i]<<" "<<totNum[i]<<endl;
          finalStations.push_back(uniVec[i]);
        }
      
    }
    for (int i=0;i<finalStations.size();i++)
    {
        CPLDebug("STATION_FETCH","----------------------------------------");
        CPLDebug("STATION_FETCH","STATION NAME: %s",finalStations[i].c_str());
        int stationFileType = directStationTraffic(finalStations[i].c_str()); // Get the station data type
        CPLDebug("STATION_FETCH","Type of Station File: %i \n",stationFileType);
        finalTypes.push_back(stationFileType);
    }

    //Delete repeats, get unique data types
    //We want there to be only 1 data type, so throw errors if there are more than one
    std::set<int> setInt(finalTypes.begin(),finalTypes.end());
    std::vector<int> vecInt(setInt.begin(),setInt.end());

    CPLDebug("STATION_FETCH","Unique Data Types (bad if >1) %lu",vecInt.size());
    for(int j=0;j<vecInt.size();j++)
    {
        CPLDebug("STATION_FETCH","Selected Data Type: %i",vecInt[j]);
    }
    CPLDebug("STATION_FETCH","\n");
    stationFileList = finalStations;
    stationFileTypes = vecInt;
//    if (vecInt.size()==0)
//    {
//        simType=-1;
//        displayInformation(-2);
//    }
    if (vecInt.size()>1)
    {
        simType = -1;
        displayInformation(-1);
    }
    if (vecInt.size()==1)
    {
        simType = vecInt[0];
        displayInformation(vecInt[0]);
    }
    else
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
//    QFileInfo fi(sfModel->fileInfo(index));
//    std::string xStringFileName = fi.absoluteFilePath().toStdString();
//    const char* xFileName = xStringFileName.c_str();

    //Get the header version from a wxStation function that does this for us
    int stationHeader = wxStation::GetHeaderVersion(xFileName);
    CPLDebug("STATION_FETCH","HEADER TYPE: %i",stationHeader);
    int instant = 0;
    std::string idx3;
    stringstream ssidx;

    if(stationHeader!=1) //Station header is different than what we return
    {
        //Determine whether it is a timeseries or not...
//        cout<<"READING A STATIONS TIMES"<<endl;
        OGRDataSourceH hDS;
        OGRLayer *poLayer;
        OGRFeature *poFeature;
        OGRFeatureDefn *poFeatureDefn;
        OGRFieldDefn *poFieldDefn;
        OGRLayerH hLayer;
    //    std::string stop_datetime;
    //    std::string start_datetime;
    //    hDS = OGROpen( stationFileList[0].c_str(), FALSE, NULL );
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

        CPLDebug("STATION_FETCH","Number of Time Entries: %llu",idx2);
        QString qFileName = QFileInfo(xFileName).fileName();
        writeToConsole(QString(qFileName+" has: "+QString::number(idx2)+" time entries"));
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

//        writeToConsole(QString(qFileName+"\nfirst time: "+start_datetime.c_str()));
//        writeToConsole(QString(qFileName+"\nlaste Time: "+stop_datetime.c_str()));
        writeToConsole("Start Time: "+QString(start_datetime.c_str()));
        writeToConsole("Stop Time: "+QString(stop_datetime.c_str()));


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
        enableTimeseries->setCheckable(false);
        startTime->setEnabled(false);
        stopTime->setEnabled(false);
        numSteps->setEnabled(false);

        fileStartVal->setText("N/A"); //Update the file Info Area with
        fileEndVal->setText("N/A"); //information
        fileStepsVal->setText("1"); //In this case, single step->display N/A's

        //Try flipping the UI
        labelTimeseries->setText("Single Step Options");

        startLabel->setVisible(false);
        stopLabel->setVisible(false);
        stepLabel->setVisible(false);

        startTime->setVisible(false);
        stopTime->setVisible(false);
        numSteps->setVisible(false);

        dateTimeEdit->setVisible(true);
        diurnalLabel->setVisible(true);

        return 0;
    }
    if (stationHeader == 2 && instant == 0)
    {
        //new Foramt w/ Time Series
        //Turn on Several options for controlling time series in the GUI
        enableTimeseries->setCheckable(true);
        enableTimeseries->setChecked(true);
        startTime->setEnabled(true);
        stopTime->setEnabled(true);
        numSteps->setEnabled(true);
        updateStartTime(startTime->dateTime());
        updateStopTime(stopTime->dateTime());

        //Display Some Time and Step info to the user
        fileStartVal->setText(startTime->dateTime().toString());
        fileEndVal->setText(stopTime->dateTime().toString());
        fileStepsVal->setText(QString(idx3.c_str()));


        //try flipping the UI
        labelTimeseries->setText("Time Series\nOptions");


        startLabel->setVisible(true);
        stopLabel->setVisible(true);
        stepLabel->setVisible(true);

        startTime->setVisible(true);
        stopTime->setVisible(true);
        numSteps->setVisible(true);

        dateTimeEdit->setVisible(false);
        diurnalLabel->setVisible(false);
        return 1;
    }
    if (stationHeader == 2 && instant == 1)
    {
        //new Format no Time Series
        //Prevent users from using time series with 1 step
        enableTimeseries->setCheckable(false);
        startTime->setEnabled(false);
        stopTime->setEnabled(false);
        numSteps->setEnabled(false);

        fileStartVal->setText("N/A"); //Display N/A to the user
        fileEndVal->setText("N/A");
        fileStepsVal->setText("1");

        //Try flipping the UI
        labelTimeseries->setText("Single Step Options");

        startLabel->setVisible(false);
        stopLabel->setVisible(false);
        stepLabel->setVisible(false);

        startTime->setVisible(false);
        stopTime->setVisible(false);
        numSteps->setVisible(false);

        dateTimeEdit->setVisible(true);
        diurnalLabel->setVisible(true);
        return 2;

    }
    if (stationHeader == 3)
    {
        //Invalid header type for GUI run...
        fileStartVal->setText("N/A"); //Update the file info area to be N/A
        fileEndVal->setText("N/A");
        fileStepsVal->setText("N/A");
        return -1;
    }
    else
    {
        //Something wrong with the station...
        fileStartVal->setText("N/A"); //If something is bad, don't say anything about
        fileEndVal->setText("N/A"); //the stuff inside
        fileStepsVal->setText("N/A");
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
    QString time_format = "yyyy-MM-ddTHH:mm:ssZ";
    QString start_utcX = QString::fromStdString(start_time);
    QString end_utcX = QString::fromStdString(stop_time);

    QDateTime start_qat = QDateTime::fromString(start_utcX,time_format);
    QDateTime end_qat = QDateTime::fromString(end_utcX,time_format);

    start_qat.setTimeSpec(Qt::UTC);
    end_qat.setTimeSpec(Qt::UTC); //Set the Time to UTC

    QDateTime loc_start_time = start_qat.toLocalTime(); //I hope this is robust
    QDateTime loc_end_time = end_qat.toLocalTime(); //Uses users local time on PC, rather than DEM time
//    cout<<"TIME ZONE: "<<pointInput::tzString.toStdString()<<endl;
//    cout<<"UTC DISK TIME: "<<qat.toString().toStdString()<<endl;
//    cout<<"LOCAL TIME: "<<lxTime.toString().toStdString()<<endl;

    startTime->setDateTime(loc_start_time);
    stopTime->setDateTime(loc_end_time); //Updates date time based on disk information

    //This suggests some potential options for number of timesteps to the user based on the number
    //of data points in a file.
    if(xSteps>101)//If there are an excessively large numer of times for weather data, don't suggest a lot of runs...
    {
        numSteps->setValue(24);
    }
    else
    {
        int step_update = xSteps/2;
        if (step_update<2) //don't suggest 1 timestep runs
        {
            step_update = 2;
            numSteps->setValue(step_update);
        }
        else
        {
            numSteps->setValue(step_update);
        }
    }

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
        if (isDiurnalChecked==true)
        {
            dateTimeEdit->setEnabled(true);
        }
    }
    if (dataType == -1 && stationFileList.size()==0) //Special Case
    {
        clippit->setText("No Stations Selected...");
        pointGo=false;
    }
    if (dataType == -1 && stationFileList.size()==1) //Case of 1 file that is crap
    {
        clippit->setText("No Valid Data detected in file...");

        //Set the file Info area to display nada
        fileStartVal->setText("N/A");
        fileEndVal->setText("N/A");
        fileStepsVal->setText("N/A");

        pointGo=false;
    }
    if (dataType==-1 && stationFileList.size()>=2)
    {
        clippit->setText("MULTIPLE TYPES SELECTED, CANNOT PROCEED!");

        //Set the file Info area to display nada
        fileStartVal->setText("N/A");
        fileEndVal->setText("N/A");
        fileStepsVal->setText("N/A");

        pointGo=false;
    }
//    else if(dataType == -1)
//    {
//        clippit->setText("MULTIPLE TYPES SELECTED, CANNOT PROCEED!");
//        pointGo=false;
//    }
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

void pointInput::selChanged()
{
    CPLDebug("STATION_FETCH","TEST");
}
/**
 * @brief pointInput::writeStationFile
 * Old way of writing Station Files, no longer
 * readlly neede, informs user that it will be written after sim
 */
void pointInput::writeStationFile()
{
//    writeToConsole("Interpolated CSV will be written.");
//    if (writeStationFileButton->isChecked())
//    {
//        writeToConsole("Interpolated Weather CSV will be written");
//    }
//    if( pointData.stations.empty() ) {
//    writeToConsole( "There are no stations to write" );
//    return;
//    }

//    QString fileName;
//    fileName = QFileDialog::getSaveFileName( this,
//                         tr( "Save station file" ),
//                         ".csv",
//                         tr( "Comma separated "	\
//                         "value files (*.csv)" ) );
//    //check for extension, make case insensitive test
//    if( QFileInfo( fileName ).suffix().compare( "csv", Qt::CaseInsensitive ) ) {
//    fileName += ".csv";
//    if( QFileInfo( fileName ).exists() ) {
//        int r = QMessageBox::warning( this, "WindNinja",
//                      "The file " + fileName +
//                          " exists, do you wish to" \
//                          " overwrite it?",
//                      QMessageBox::Yes |
//                      QMessageBox::No |
//                      QMessageBox::Cancel );
//        if( r == QMessageBox::No || r == QMessageBox::Cancel )
//        return;
//    }
//    }

//    if( fileName.isEmpty() || fileName == ".csv" )
//    return;
//    else
////    wxStation::writeStationFile( pointData.stations,
////                     fileName.toStdString() );
//        cout<<"this is disabled until further notice"<<endl;
}

/**
 * @brief pointInput::writeStationKml
 * Tells user that we will write the KML after simulation
 */
void pointInput::writeStationKml()
{
//    writeToConsole("KML files will be written!");
//    if (writeStationKmlButton->isChecked())
//    {
//         writeToConsole("KML files will be written!");
//    }
//    if( pointData.stations.empty() ) {
//    writeToConsole( "There are no stations to write" );
//    return;
//    }

//    QString fileName;
//    fileName = QFileDialog::getSaveFileName( this,
//                         tr( "Save station file" ),
//                         ".kml",
//                         tr( "Keyhole Markup  "	\
//                         "files (*.kml)" ) );
//    //check for extension, make case insensitive test
//    if( QFileInfo( fileName ).suffix().compare( "kml", Qt::CaseInsensitive ) ) {
//    fileName += ".kml";
//    if( QFileInfo( fileName ).exists() ) {
//        int r = QMessageBox::warning( this, "WindNinja",
//                      "The file " + fileName +
//                          " exists, do you wish to" \
//                          " overwrite it?",
//                      QMessageBox::Yes |
//                      QMessageBox::No |
//                      QMessageBox::Cancel );
//        if( r == QMessageBox::No || r == QMessageBox::Cancel )
//        return;
//    }
//    }

//    if( fileName.isEmpty() || fileName == ".kml" )
//    return;
//    else
////    wxStation::writeKmlFile( pointData.stations,
////                    fileName.toStdString() );
//        cout<<"disabled"<<endl;

/** Opens the Main Window
  *
  */
}
void pointInput::openMainWindow()
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
/** Obsolete for now, might be used later
 * Originally was intended to switch between the old format and the new format
 * @brief pointInput::toggleUI
 */

void pointInput::toggleUI()
{
//    cout<<initOpt->currentIndex()<<endl;
//    if (initOpt->currentIndex()==1)
//    {
////        jazz->setVisible(false);
//        stationFileLineEdit->setVisible(false);
//        readStationFileButton->setVisible(false);
//        dateTimeEdit->setVisible(false);
////        readStationFileButton->setVisible(true);
////        ska->setVisible(true);
//        treeView->setVisible(true);
//        refreshToolButton->setVisible(true);
//        setInputFile(demFileName);
//        checkForModelData();
        
//        enableTimeseries->setVisible(true);
//        startTime->setVisible(true);
//        stopTime->setVisible(true);
//        numSteps->setVisible(true);
//        startLabel->setVisible(true);
//        stopLabel->setVisible(true);
//        stepLabel->setVisible(true);
//    }
//    if (initOpt->currentIndex()==0)
//    {
//        stationFileLineEdit->setVisible(true);
//        dateTimeEdit->setVisible(true);
////        ska->setVisible(false);
//        treeView->setVisible(false);
//        refreshToolButton->setVisible(false);
//        readStationFileButton->setVisible(true);
        
//        enableTimeseries->setVisible(false);
//        startTime->setVisible(false);
//        stopTime->setVisible(false);
//        numSteps->setVisible(false);
//        startLabel->setVisible(false);
//        stopLabel->setVisible(false);
//        stepLabel->setVisible(false);
        

//    }
}
/** If the data is part of timeseries, ie more than one step, we need to figure out the start and stop time
 * // to do that, we need to enable them
 * @brief pointInput::toggleTimeseries
 */
void pointInput::toggleTimeseries() //If the data is part of timeseries, ie more than one step, we need to figure out the start and stop time

{
    if (enableTimeseries->isChecked()==false)
    {
        stopTime->setEnabled(false);
        startTime->setEnabled(false);
        numSteps->setEnabled(false);
    }
    if (enableTimeseries->isChecked()==true)
    {
        startTime->setEnabled(true);
        stopTime->setEnabled(true);
        numSteps->setEnabled(true);
        updateStartTime(startTime->dateTime());
        updateStopTime(stopTime->dateTime());
    }
}
/** This is obsolete, I Think...
 * @brief pointInput::pairFetchTime
 * @param xDate
 */
void pointInput::pairFetchTime(QDateTime xDate) //Obsolete
{
    QDateTime zDate = xDate;
    CPLDebug("STATION_FETCH","Pairing Fetched Time...");
    CPLDebug("STATION_FETCH","Paired Time: %s",xDate.date().toString().toStdString().c_str());
}
/** Pairs start and stop times to what the stationFetchWidget does.
 * @brief pointInput::pairStartTime
 * @param xDate
 */
void pointInput::pairStartTime(QDateTime xDate) //These functions take the start and stop time from the widget
//and populate the timeseries objects in the gUI
{
//    cout<<"PAIR START TIME"<<endl;
//    cout<<xDate.date().toString().toStdString()<<endl;
    startTime->setDateTime(xDate);    
}
void pointInput::pairStopTime(QDateTime xDate)
{
//    cout<<"PAIR STOP TIME"<<endl;
//    cout<<xDate.date().toString().toStdString()<<endl;
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
        enableTimeseries->setChecked(false);
        startTime->setEnabled(false);
        stopTime->setEnabled(false);
        numSteps->setEnabled(false);
    }
    if(curIndex==1)
    {
        enableTimeseries->setChecked(true);
        startTime->setEnabled(true);
        stopTime->setEnabled(true);
        numSteps->setEnabled(true);
    }
}
/** Updates the timeseries start time based on user requests
 * @brief pointInput::updateStartTime
 * @param xDate
 */
void pointInput::updateStartTime(QDateTime xDate)
{
    int year,month,day,hour,minute;
    year = xDate.date().year();
    month = xDate.date().month();
    day = xDate.date().day();    
    hour = xDate.time().hour();
    minute = xDate.time().minute();

//    cout<<year<<" "<<month<<" "<<day<<" "<<hour<<" "<<minute<<endl;
    CPLDebug("STATION_FETCH","UPDATED START TIME: %i %i %i %i %i",year,month,day,hour,minute);
    
//    if (type==0)
//    {
      startSeries.push_back(year);
      startSeries.push_back(month);
      startSeries.push_back(day);
      startSeries.push_back(hour);
      startSeries.push_back(minute);
//    }
//    if (type ==1)
//    {
//        endSeries.push_back(year);
//        endSeries.push_back(month);
//        endSeries.push_back(day);
//        endSeries.push_back(hour);
//        endSeries.push_back(minute);
//    }
}
/** Updates the time for stopping the simulation based on user requests
 * @brief pointInput::updateStopTime
 * @param xDate
 */
void pointInput::updateStopTime(QDateTime xDate)
{
    int year,month,day,hour,minute;
    year = xDate.date().year();
    month = xDate.date().month();
    day = xDate.date().day();    
    hour = xDate.time().hour();
    minute = xDate.time().minute();

    CPLDebug("STATION_FETCH","UPDATED STOP TIME: %i %i %i %i %i",year,month,day,hour,minute);
    
    endSeries.push_back(year);
    endSeries.push_back(month);
    endSeries.push_back(day);
    endSeries.push_back(hour);
    endSeries.push_back(minute);
}
/**
 * @brief pointInput::openStationFetchWidget
 * Opens the downloader widget to download station files
 */
void pointInput::openStationFetchWidget()
{
    xWidget = new stationFetchWidget();
    QSignalMapper *signalMapper;
    connect(xWidget, SIGNAL(exitDEM()),this, SLOT(openMainWindow())); //Launches Widget Connector
    this->setEnabled(false);
    xWidget->setInputFile(demFileName);
    xWidget->updatetz(tzString);
    connect(xWidget, SIGNAL(exitDEM()),this, SLOT(checkForModelData())); //Launches Widget Connector    
//    checkForModelData();
//    cout<<xWidget->timeLoc->currentIndex()<<endl;
//    connect(xWidget->currentBox,SIGNAL(clicked()),this,SLOT(selChanged())); //This proves that the widget can talk to the pointInput class
    connect(xWidget->startEdit,SIGNAL(dateTimeChanged(const QDateTime)),this,SLOT(pairStartTime(const QDateTime)));
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
 */
void pointInput::checkForModelData()
{

    QDir wd(cwd);
    QStringList filters;
    filters<<"*.csv";
    sfModel->setNameFilters(filters);
    sfModel->setFilter(QDir::AllDirs | QDir::Files | QDir::NoDotAndDotDot);
    treeView->setRootIndex(sfModel->index(wd.absolutePath()));
    treeView->resizeColumnToContents(0);
}
void pointInput::testProg()
{
    writeToConsole("TESTING PROGRESS BAR");

    xProg->setLabelText("Initial Message!");
    xProg->setRange(0,0);
    xProg->setCancelButtonText("Cancel");
    xProg->reset();
    bool test = xProg->wasCanceled();

//    connect(&xFut, SIGNAL(finished()),this,SLOT(update)
    connect(&xFut,SIGNAL(finished()),this,SLOT(updateProg()));
    connect(xProg,SIGNAL(canceled()),this,SLOT(updateProg()));

    xFut.setFuture(QtConcurrent::run(this, &pointInput::progExec));

    xProg->exec();
//    xFut.setF


}

int pointInput::progExec()
{
    usleep(500000);
//    xProg->setLabelText("RUNNING EXEC!");
    return 1;
}

void pointInput::updateProg()
{
    if(xProg->wasCanceled())
    {
        xProg->setLabelText("Canceling!");
        xProg->setCancelButton(0);
        xFut.waitForFinished();
        xProg->cancel();
    }
    else
    {
        xFut.waitForFinished();
        int result = xFut.result();
        if(result==1)
        {
            xProg->setRange(0,100);
            xProg->setValue(1);
            xProg->setValue(100);
            xProg->setLabelText("Download SucessFul!");
            xProg->setCancelButtonText("Close");

//            xProg->close();
        }

    }

}


void pointInput::updateTable()
{

}
