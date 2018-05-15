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

    initOpt = new QComboBox( this ); //Short for initialization Options, so we can switch the GUI between the old format and the new format
    initOpt->addItem("Old Format"); //needs a new name
    initOpt->addItem("New Format"); //needs a new name
    initOpt->setCurrentIndex(1);
    initOpt->setVisible(false);

    pointGo=false;

    initPages = new QStackedWidget(this);
    oldForm = new QWidget();
    newForm = new QWidget();

    stationFileLineEdit = new QLineEdit( newForm );
    stationFileLineEdit->setReadOnly( true );
    stationFileLineEdit->setGeometry(QRect(10,0,141,20));
    stationFileLineEdit->setVisible(false);

    dateTimeEdit = new QDateTimeEdit( newForm );
    dateTimeEdit->setDateTime( QDateTime::currentDateTime() );
    dateTimeEdit->setCalendarPopup( true );
    dateTimeEdit->setDisplayFormat( "MM/dd/yyyy HH:mm" );
    dateTimeEdit->setEnabled( false ); //This is for Old Format Diurnal Simulations
    dateTimeEdit->setVisible(false);

    readStationFileButton =  new QToolButton( this ); //Opens old Format Station
    readStationFileButton->setText( tr( "Read Station File" ) );
    readStationFileButton->setIcon( QIcon( ":weather_cloudy.png" ) );
    readStationFileButton->setToolButtonStyle( Qt::ToolButtonTextBesideIcon );
    readStationFileButton->setGeometry(QRect(10, 20, 151, 26));
    readStationFileButton->setVisible(false);

    writeStationFileButton =  new QCheckBox( this ); //This writes an interpolated csv of the weather data (we might not want this)
    writeStationFileButton->setText( tr( "Write Station File" ) );
    writeStationFileButton->setIcon( QIcon( ":weather_clouds.png" ) );
//    writeStationFileButton->setToolButtonStyle( Qt::ToolButtonTextBesideIcon );

    writeStationKmlButton =  new QCheckBox( this ); //This writes a KML of the weather stations (1 per run)
    writeStationKmlButton->setText( tr( "Write Station Kml" ) );
    writeStationKmlButton->setIcon( QIcon( ":weather_cloudy.png" ) );
//    writeStationKmlButton->setToolButtonStyle( Q/t::ToolButtonTextBesideIcon );
//    writeStationKmlButton->setIcon( QIcon( ":weather_cloudy.png" ) );
//    writeStationKmlButton->setStyle(Qt::ToolButtonTextBesideIcon);
//    writeStationKmlButton->setToolButtonStyle( Qt::ToolButtonTextBesideIcon );

    doTest = new QToolButton( this ); //This opens the station fetch Widget (probably needs a new name)
    doTest->setText( tr( "Open Station Downloader " ));
    doTest->setIcon(QIcon(":world.png"));
    doTest->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

//    stationTreeView = new QTreeView( this );
//    stationTreeView->setModel( &pointData ); //This is some sort of deprecated tree thing that existed before

//####################################################
// New Format Tree Stuff                             #
//####################################################

    sfModel = new QDirModel(this); //Creates the directory model
    sfModel->setReadOnly(false); //probably can be true, but i don't know
    sfModel->setSorting(QDir::Time); //Sort by time created

    treeView = new QTreeView(this); //Creates the box
    treeView->setVisible(true); //hides it, allows it be seen when initOpt is set correctly
    treeView->setModel(sfModel);
    treeView->header()->setStretchLastSection(true);
    treeView->setAnimated(true);
    treeView->setColumnHidden(1, true);
    treeView->setColumnHidden(2, true);
    treeView->setAlternatingRowColors( true );
    treeView->setSelectionMode(QAbstractItemView::MultiSelection); //Allows multiple files to be selected
//    treeView->setSelectionMode(QAbstractItemView::);    
//    treeView->setSelectionModel(QItemSelectionModel::Toggle);

    treeLabel = new QLabel(tr("Select Weather Stations"));
    

    ClippyToolLayout = new QHBoxLayout;
    
    refreshToolButton = new QToolButton(this); //This refreshes the tree so that new files will populate
    refreshToolButton->setText(tr("Refresh Weather Stations"));
    refreshToolButton->setIcon(QIcon(":arrow_rotate_clockwise.png"));
    refreshToolButton->setToolTip(tr("Refresh the station listing."));
    refreshToolButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    refreshToolButton->setVisible(true);

    clippit = new QLabel(tr(""));

    ClippyToolLayout->addWidget(refreshToolButton);
    ClippyToolLayout->addWidget(clippit);

    //New Custom Layout
//    treeLayout = new QHBoxLayout;
//    treeLayout->addWidget(treeView);
//    treeLayout->addWidget(refreshToolButton);



//####################################################
//End Directory. Start Timeseries Box                #
//####################################################
    startTime = new QDateTimeEdit(QDateTime::currentDateTime());
    stopTime = new QDateTimeEdit(QDateTime::currentDateTime());
    
    startTime->setDateTime(QDateTime::currentDateTime().addDays(-1));
    startTime->setMaximumDateTime(QDateTime::currentDateTime().addSecs(-3600));
    stopTime->setMaximumDateTime(QDateTime::currentDateTime());
    
    startTime->setDisplayFormat( "MM/dd/yyyy HH:mm" );
    stopTime->setDisplayFormat( "MM/dd/yyyy HH:mm" );
    
    enableTimeseries = new QCheckBox(this);
    enableTimeseries->setText(tr("Enable Timeseries"));
    
    numSteps = new QSpinBox;
    numSteps->setValue(24);
    
    startTime->setVisible(true);
    stopTime->setVisible(true);
    enableTimeseries->setVisible(true);

    startTime->setEnabled(false);
    stopTime->setEnabled(false);

    numSteps->setVisible(true);
    numSteps->setEnabled(false);
    
    timeBoxLayout = new QHBoxLayout;
    startLayout = new QVBoxLayout;
    stopLayout = new QVBoxLayout;
    stepLayout = new QVBoxLayout;
    
    startLabel = new QLabel(tr("Start Time"));
    stopLabel = new QLabel(tr("Stop Time"));
    stepLabel = new QLabel(tr("Step"));
    startLabel->setVisible(true);
    stopLabel->setVisible(true);
    stepLabel->setVisible(true);
    
    startLayout->addWidget(startLabel);
    startLayout->addWidget(startTime);
    
    stopLayout->addWidget(stopLabel);
    stopLayout->addWidget(stopTime);
    
    stepLayout->addWidget(stepLabel);
    stepLayout->addWidget(numSteps);
    
//    timeBoxLayout->addWidget(enableTimeseries);
//    timeBoxLayout->addWidget(startTime);
//    timeBoxLayout->addWidget(stopTime);
//    timeBoxLayout->addWidget(numSteps);
    timeBoxLayout->addWidget(enableTimeseries);
    timeBoxLayout->addLayout(startLayout);
    timeBoxLayout->addLayout(stopLayout);
    timeBoxLayout->addLayout(stepLayout);
    
//-------------------------------------------------
// Add some new layouts
//-------------------------------------------------
    vTreeLayout = new QVBoxLayout;
    vTreeLayout->addWidget(treeLabel);
    vTreeLayout->addWidget(treeView);
//    vTreeLayout->addWidget(refreshToolButton);
    vTreeLayout->addLayout(ClippyToolLayout);
    vTreeLayout->addLayout(timeBoxLayout);
//####################################################
//END SF CUSTOM                                      #
//####################################################
    optLayout = new QHBoxLayout;
    optLayout->addWidget(initOpt);

    fileLayout = new QHBoxLayout;
    fileLayout->addWidget( stationFileLineEdit );
//    fileLayout->addWidget(ska);
    fileLayout->addWidget( readStationFileButton );
//    initPages->addWidget(oldForm);
//    fileLayout->addWidget(initPages);
//    fileLayout->addWidget(treeView);
//    fileLayout->addWidget(refreshToolButton);
//    fileLayout->addWidget(jazz);

    buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget( writeStationFileButton );
//    writeStationFileButton->setVisible( false );
    buttonLayout->addWidget( writeStationKmlButton );
    buttonLayout->addWidget(doTest);
    buttonLayout->addStretch();



    pointLayout = new QVBoxLayout;
//    pointLayout->addWidget( stationTreeView ); //This is shit as far as I know?
//    stationTreeView->setVisible( false );
    pointLayout->addLayout(optLayout);
    pointLayout->addLayout( fileLayout );

    pointLayout->addLayout(vTreeLayout);
//    pointLayout->addLayout(treeLayout);
    pointLayout->addWidget( dateTimeEdit );

    pointLayout->addStretch();
    pointLayout->addLayout( buttonLayout );

    pointGroupBox->setLayout( pointLayout );

    cwd = QFileInfo("/home/tanner/src/wind/fs_gui_files/kmso.tif").absolutePath();

//    stationTreeView->setRootIndex(model->index());

//    treeView->setModel(model);
//    treeView->header()



    ninjafoamConflictLabel = new QLabel(tr("The point initialization option is not currently available for the momentum solver.\n"
        ), this);
    ninjafoamConflictLabel->setHidden(true);

    layout = new QVBoxLayout;
    layout->addWidget( pointGroupBox );
    layout->addWidget(ninjafoamConflictLabel);
//    layout->addStretch();

//    if (initOpt)
    cout<<initOpt->currentIndex()<<endl;
    setLayout( layout );
    connect( readStationFileButton, SIGNAL( clicked() ), this,
         SLOT( readStationFile() ) ); //For Old Format (for now)
    connect( writeStationFileButton, SIGNAL( clicked() ), this,
         SLOT( writeStationFile() ) ); //Writes a CSV Connector
    connect( writeStationKmlButton, SIGNAL( clicked() ), this,
         SLOT( writeStationKml() ) ); //Writes a KML connector
    connect( doTest ,SIGNAL( clicked () ), this,
             SLOT(openStationFetchWidget())); //Opens the Downloader Connector
    connect(initOpt,SIGNAL(currentIndexChanged(int)), this,SLOT(toggleUI())); //Changes to New Format and back Connector
    connect(refreshToolButton, SIGNAL(clicked()), //Refreshes new Format
        this, SLOT(checkForModelData()));
    connect(enableTimeseries,SIGNAL(clicked()),this,
            SLOT(toggleTimeseries()));
    connect(startTime,SIGNAL(dateTimeChanged(QDateTime)),this,SLOT(updateStartTime(QDateTime)));
    connect(stopTime,SIGNAL(dateTimeChanged(QDateTime)),this,SLOT(updateStopTime(QDateTime)));
//    connect(

//    connect(treeView,SIGNAL(QTreeView::currentChanged(const QModelIndex&, const QModelIndex&);),
//            this,SLOT(readMultipleStaitonFiles()));
    connect(treeView, SIGNAL(clicked(const QModelIndex &)),
        this, SLOT(readMultipleStaitonFiles(const QModelIndex &))); //Connects click to which files should be conencted
//    connect(treeView, SIGNAL(clicked(const QModelIndex &)),this,SLOT(readStationTime(const QModelIndex &)));


//    connect(sfModel, SIGNAL(/*selectionChanged*/()),this,SLOT(selChanged()));
//    connect(treeView, SIGNAL(QItemSelectionModel::selectionChanged(const QItemSelection &, const QItemSelection &)),
//        this, SLOT(readMultipleStaitonFiles(const QModelIndex &)));



    stationFileName = "";
    
}

pointInput::~pointInput()
{

}

void pointInput::readStationFile()
{
    QString fileName;
    fileName = QFileDialog::getOpenFileName(this, tr("Open station file"),
                                             QFileInfo(stationFileName).path(),
                                              tr("Comma separated value files (*.csv)"));

    if(fileName.isEmpty() || demFileName.isEmpty()) {
        return;
    }

    stationFileName = fileName;
    stationFileLineEdit->setText(QFileInfo(fileName).fileName());
    cout<<stationFileName.toStdString()<<endl;
    emit stationFileChanged();
}
//This is all pretty good, needs to be cleaned up
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
//General Idea: Append all selections a user makes, get the unique ones, counter the number of clicks, if it is odd, keep the file, if even, don't keep it
// This is because odd means it was selected at least once at then left alone
// even means that it was selected and deselected
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
        cout<<finalStations[i]<<endl;
        int stationFileType = directStationTraffic(finalStations[i].c_str()); // Get the station data type
        cout<<"Type of Station File: "<<stationFileType<<endl;
        cout<<"\n"<<endl;
        finalTypes.push_back(stationFileType);
    }

    //Delete repeats, get unique data types
    //We want there to be only 1 data type, so throw errors if there are more than one
    std::set<int> setInt(finalTypes.begin(),finalTypes.end());
    std::vector<int> vecInt(setInt.begin(),setInt.end());

    cout<<"Number of Unique Data Types: "<<vecInt.size()<<endl;
    for(int j=0;j<vecInt.size();j++)
    {
        cout<<"D-type: "<<vecInt[j]<<endl;
    }
    
    cout<<"\n"<<endl;
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
 * enables timeseries when there is a timeseries
 * prevents timeseries when there is no way it is a timeseries
 *
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
    cout<<"STATION HEADER TYPE: "<<stationHeader<<endl;
    int instant = 0;

    if(stationHeader!=1)
    {
        //Determine whether it is a timeseries or not...
        cout<<"READING A STATIONS TIMES"<<endl;
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
        cout<<"Number of Entries in Station: "<<idx2<<endl;
        const char* emptyChair; //Muy Importante!

        poFeature = poLayer->GetFeature(iBig);
    //    startTime = poFeature->GetFieldAsString(15);
        std::string start_datetime(poFeature->GetFieldAsString(15));
    //    cout<<"STATION START TIME:"<<poFeature->GetFieldAsString(15)<<endl;
        poFeature = poLayer->GetFeature(idx2);
        std::string stop_datetime(poFeature->GetFieldAsString(15));
    //    cout<<"STATION STOP TIME:"<<poFeature->GetFieldAsString(15)<<endl;
        cout<<"STATION START TIME: "<<start_datetime<<endl;
        cout<<"STATION END TIME: "<<stop_datetime<<endl;


        if (start_datetime.empty()==true && stop_datetime.empty()==true)
        {
            //Means that there is not a time series
            cout<<"No Time Series"<<endl;
            instant = 1;
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
        return 2;

    }
    if (stationHeader == 3)
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

void pointInput::displayInformation(int dataType)
{
    cout<<dataType<<endl;
    if(dataType == 0)
    {
        clippit->setText("Run Type: Old Format");
        pointGo=true;
    }
    if(dataType == 1)
    {
        clippit->setText("Run Type: Time Series");
        pointGo=true;
    }
    if(dataType == 2)
    {
        clippit->setText("Run Type: Single Step");
        pointGo=true;
    }
    if (dataType == -1 && stationFileList.size()==0)
    {
        clippit->setText("No Stations Selected...");
        pointGo=false;
    }
    if (dataType == -1 && stationFileList.size()==0) //Special Case
    {
        clippit->setText("No Stations Selected...");
        pointGo=false;
    }
    else if(dataType == -1)
    {
        clippit->setText("MULTIPLE TYPES SELECTED, CANNOT PROCEED!");
        pointGo=false;
    }



}

void pointInput::setWxStationFormat(int format)
{
       wxStationFormat = format;
}

void pointInput::selChanged()
{
    cout<<"TEST"<<endl;
}

void pointInput::writeStationFile()
{
    writeToConsole("Written after sim");
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

void pointInput::writeStationKml()
{
    writeToConsole("kml will be written after simulation!");
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
}
void pointInput::openMainWindow()
{
    this->setEnabled(true);
}

void pointInput::updateTz(QString tz)
{
    tzString = tz;
}

void pointInput::toggleUI()
{
    cout<<initOpt->currentIndex()<<endl;
    if (initOpt->currentIndex()==1)
    {
//        jazz->setVisible(false);
        stationFileLineEdit->setVisible(false);
        readStationFileButton->setVisible(false);
        dateTimeEdit->setVisible(false);
//        readStationFileButton->setVisible(true);
//        ska->setVisible(true);
        treeView->setVisible(true);
        refreshToolButton->setVisible(true);
        setInputFile(demFileName);
        checkForModelData();
        
        enableTimeseries->setVisible(true);
        startTime->setVisible(true);
        stopTime->setVisible(true);
        numSteps->setVisible(true);
        startLabel->setVisible(true);
        stopLabel->setVisible(true);
        stepLabel->setVisible(true);
    }
    if (initOpt->currentIndex()==0)
    {
        stationFileLineEdit->setVisible(true);
        dateTimeEdit->setVisible(true);
//        ska->setVisible(false);
        treeView->setVisible(false);
        refreshToolButton->setVisible(false);
        readStationFileButton->setVisible(true);
        
        enableTimeseries->setVisible(false);
        startTime->setVisible(false);
        stopTime->setVisible(false);
        numSteps->setVisible(false);
        startLabel->setVisible(false);
        stopLabel->setVisible(false);
        stepLabel->setVisible(false);
        

    }
}
void pointInput::toggleTimeseries() //If the data is part of timeseries, ie more than one step, we need to figure out the start and stop time
                                    // to do that, we need to enable them
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
void pointInput::pairFetchTime(QDateTime xDate) //Obsolete
{
    QDateTime zDate = xDate;
    cout<<"PAIR FETCH TIME"<<endl;
    cout<<xDate.date().toString().toStdString()<<endl;
}
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

void pointInput::updateStartTime(QDateTime xDate)
{
    int year,month,day,hour,minute;
    year = xDate.date().year();
    month = xDate.date().month();
    day = xDate.date().day();    
    hour = xDate.time().hour();
    minute = xDate.time().minute();

    cout<<year<<" "<<month<<" "<<day<<" "<<hour<<" "<<minute<<endl;
    
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
void pointInput::updateStopTime(QDateTime xDate)
{
    int year,month,day,hour,minute;
    year = xDate.date().year();
    month = xDate.date().month();
    day = xDate.date().day();    
    hour = xDate.time().hour();
    minute = xDate.time().minute();

    cout<<year<<" "<<month<<" "<<day<<" "<<hour<<" "<<minute<<endl;

    
    endSeries.push_back(year);
    endSeries.push_back(month);
    endSeries.push_back(day);
    endSeries.push_back(hour);
    endSeries.push_back(minute);
}

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
    connect(xWidget->currentBox,SIGNAL(clicked()),this,SLOT(selChanged())); //This proves that the widget can talk to the pointInput class
    connect(xWidget->startEdit,SIGNAL(dateTimeChanged(const QDateTime)),this,SLOT(pairStartTime(const QDateTime)));
    connect(xWidget->endEdit,SIGNAL(dateTimeChanged(const QDateTime)),this,SLOT(pairStopTime(const QDateTime)));
    connect(xWidget->timeLoc,SIGNAL(currentIndexChanged(int)),this,SLOT(pairTimeSeries(int))); //Connects What the user does in the widget
            //to what the timeseries checkbox does

    
}

void pointInput::setInputFile( QString file )
{
    demFileName = file;
    cwd = QFileInfo(file).absolutePath();
}
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


void pointInput::updateTable()
{

}
