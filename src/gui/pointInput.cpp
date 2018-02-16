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

pointInput::pointInput( QWidget *parent ) : QWidget( parent )
{
    pointGroupBox = new QGroupBox( "Point Initialization", this );
    pointGroupBox->setCheckable( true );
    pointGroupBox->setChecked(false);
    
    initOpt = new QComboBox( this );
    initOpt->addItem("Old Format");
    initOpt->addItem("New Format");
    
    initPages = new QStackedWidget(this);
    oldForm = new QWidget();
    newForm = new QWidget();
    
    
    
    stationFileLineEdit = new QLineEdit( newForm );
    stationFileLineEdit->setReadOnly( true );
    stationFileLineEdit->setGeometry(QRect(10,0,141,20));
//    stationFileLineEdit->setVisible(false);
    
//    ska = new QLineEdit( this );
//    ska->setReadOnly( true );
//    ska->setText( tr("SKA SKA SKA"));    
//    ska->setVisible(false);
    
//    jazz = new QLineEdit( this );
//    jazz->setReadOnly(false);
//    jazz->setText(tr("Sunflower"));
//    jazz->setVisible(true);

    dateTimeEdit = new QDateTimeEdit( newForm );
    dateTimeEdit->setDateTime( QDateTime::currentDateTime() );
    dateTimeEdit->setCalendarPopup( true );
    dateTimeEdit->setDisplayFormat( "MM/dd/yyyy HH:mm" );
    dateTimeEdit->setEnabled( false );

    readStationFileButton =  new QToolButton( this );
    readStationFileButton->setText( tr( "Read Station File" ) );
    readStationFileButton->setIcon( QIcon( ":weather_cloudy.png" ) );
    readStationFileButton->setToolButtonStyle( Qt::ToolButtonTextBesideIcon );
    readStationFileButton->setGeometry(QRect(10, 20, 151, 26));

    writeStationFileButton =  new QCheckBox( this );
    writeStationFileButton->setText( tr( "Write Station File" ) );
//    writeStationFileButton->setIcon( QIcon( ":weather_cloudy.png" ) );
//    writeStationFileButton->setToolButtonStyle( Qt::ToolButtonTextBesideIcon );

    writeStationKmlButton =  new QCheckBox( this );
    writeStationKmlButton->setText( tr( "Write Station Kml" ) );
    writeStationKmlButton->setIcon( QIcon( ":weather_clouds.png" ) );
//    writeStationKmlButton->setStyle(Qt::ToolButtonTextBesideIcon);
//    writeStationKmlButton->setToolButtonStyle( Qt::ToolButtonTextBesideIcon );

    doTest = new QToolButton( this );
    doTest->setText( tr( "Open Station Downloader " ));
    doTest->setIcon(QIcon(":world.png"));
    doTest->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    writeStationKmlButton->setIcon( QIcon( ":weather_cloudy.png" ) );
//    writeStationKmlButton->setToolButtonStyle( Q/t::ToolButtonTextBesideIcon );
    
    stationTreeView = new QTreeView( this );
    stationTreeView->setModel( &pointData ); //Ignore this shit please
    
//####################################################
//This is where I am going to work on the tree stuff # 
//####################################################    
    
    sfModel = new QDirModel(this);
    sfModel->setReadOnly(false);
    sfModel->setSorting(QDir::Time);
//    sfModel->
        
    treeView = new QTreeView(this);
    treeView->setVisible(false);
    treeView->setModel(sfModel);
    treeView->header()->setStretchLastSection(true);
    treeView->setAnimated(true);
    treeView->setColumnHidden(1, true);
    treeView->setColumnHidden(2, true);
    treeView->setAlternatingRowColors( true );
    treeView->setSelectionMode(QAbstractItemView::MultiSelection);
    
    refreshToolButton = new QToolButton(this);
    refreshToolButton->setText(tr("Refresh Weather Stations"));
    refreshToolButton->setIcon(QIcon(":arrow_rotate_clockwise.png"));
    refreshToolButton->setToolTip(tr("Refresh the station listing."));
    refreshToolButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    refreshToolButton->setVisible(false);
    
    
    //New Custom Layout
//    treeLayout = new QHBoxLayout;
//    treeLayout->addWidget(treeView);
//    treeLayout->addWidget(refreshToolButton);
    
    vTreeLayout = new QVBoxLayout;
    vTreeLayout->addWidget(treeView);
    vTreeLayout->addWidget(refreshToolButton);
    
//####################################################
//END SF CUSTOM                                      # 
//####################################################    
    connect( readStationFileButton, SIGNAL( clicked() ), this,
         SLOT( readStationFile() ) );
    connect( writeStationFileButton, SIGNAL( clicked() ), this,
         SLOT( writeStationFile() ) );
    connect( writeStationKmlButton, SIGNAL( clicked() ), this,
         SLOT( writeStationKml() ) );
    connect( doTest ,SIGNAL( clicked () ), this, 
             SLOT(openStationFetchWidget()));
    
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
    pointLayout->addWidget( stationTreeView );
    stationTreeView->setVisible( false );
    pointLayout->addLayout(optLayout);
    pointLayout->addLayout( fileLayout );
    
    pointLayout->addLayout(vTreeLayout);
//    pointLayout->addLayout(treeLayout);
//    pointLayout->addWidget( dateTimeEdit );
    
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
    connect(initOpt,SIGNAL(currentIndexChanged(int)), this,SLOT(toggleUI()));
    connect(refreshToolButton, SIGNAL(clicked()),
        this, SLOT(checkForModelData()));
    
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
    emit stationFileChanged();
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
//        readStationFileButton->setVisible(true);
//        ska->setVisible(true);
        treeView->setVisible(true);
        refreshToolButton->setVisible(true);
        setInputFile(demFileName);
        checkForModelData();
    }
    if (initOpt->currentIndex()==0)
    {
        stationFileLineEdit->setVisible(true);
//        ska->setVisible(false);
        treeView->setVisible(false);
        refreshToolButton->setVisible(false);
        readStationFileButton->setVisible(true);        
        
    }
}

void pointInput::openStationFetchWidget()
{
    xWidget = new stationFetchWidget();
    connect(xWidget, SIGNAL(exitDEM()),this, SLOT(openMainWindow()));
    this->setEnabled(false);    
    xWidget->setInputFile(demFileName);    
    xWidget->updatetz(tzString);
    
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
