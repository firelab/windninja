/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja Qt GUI
 * Purpose:  Client to download and access weather data
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

#include "weatherModel.h"

/**
 * Constructor
 *
 * @param parent parent widget for the widget
 */
weatherModel::weatherModel(QWidget *parent) : QWidget(parent)
{
    try {
    tz_db.load_from_file( FindDataPath("date_time_zonespec.csv") );
    }
    catch( boost::local_time::data_not_accessible ) {
    qDebug() << "diurnalInput::loadBoostTimeZones():"
         << "caught data_not_accessible";
    //throw( new guiInitializationError( "Failed to load time zone, "
    //				   "Cannot initialize interface. " ) );
    }
    catch( boost::local_time::bad_field_count ) {
    qDebug() << "diurnalInput::loadBoostTimeZones():"
         << "caught bad_field_count";
    //throw( new guiInitializationError( "Failed to load time zone, "
    //				   "Cannot initialize interface. " ) );
    }
    //setDisabled(true);

    weatherGroupBox = new QGroupBox( tr( "Weather Model Initialization" ),
                     this );
    weatherGroupBox->setCheckable( true );
    weatherGroupBox->setChecked(false);

    downloadGroupBox = new QGroupBox(tr("Download Weather Data"));

    modelComboBox = new QComboBox(this);
    modelComboBox->setDuplicatesEnabled(false);

    hourSpinBox =  new QSpinBox(this);
    hourSpinBox->setRange(3, 84);
    hourSpinBox->setSingleStep(1);
    hourSpinBox->setSuffix(" hours");
    hourSpinBox->setValue(3);
    hourSpinBox->setAccelerated( true );
    //hourSpinBox->setVisible( false );

    downloadToolButton = new QToolButton(this);
    downloadToolButton->setText(tr("Download data"));
    downloadToolButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    downloadToolButton->setIcon(QIcon(":server_go.png"));

    //QDirModel will be deprecated in 4.7, use QFileSystemModel if upgrading
    model = new QDirModel(this);
    model->setReadOnly(true);
    model->setSorting( QDir::Time );

    forecastListLabel = new QLabel( this );
    forecastListLabel->setText( "Downloaded forecasts" );

    treeView = new QTreeView(this);
    treeView->setModel(model);
    treeView->header()->setStretchLastSection(true);
    treeView->setAnimated(true);
    treeView->setColumnHidden(1, true);
    treeView->setColumnHidden(2, true);
    //treeView->setColumnHidden(3, true);
    //treeView->setDragDropMode( QAbstractItemView::DragOnly );
    treeView->setAlternatingRowColors( true );


    statusLabel = new QLabel(this);
    statusLabel->setText(tr("Ready."));

    refreshToolButton = new QToolButton(this);
    refreshToolButton->setText(tr("Refresh Forecast List"));
    refreshToolButton->setIcon(QIcon(":arrow_rotate_clockwise.png"));
    refreshToolButton->setToolTip(tr("Refresh the forecast listing."));
    refreshToolButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

    connect(downloadToolButton, SIGNAL(clicked()),
        this, SLOT(getData()));
    connect(refreshToolButton, SIGNAL(clicked()),
        this, SLOT(checkForModelData()));
    connect(treeView, SIGNAL(clicked(const QModelIndex &)),
        this, SLOT(displayForecastTime(const QModelIndex &)));

    //clear the selection on uncheck of group box
    connect(weatherGroupBox, SIGNAL(toggled(bool)),
        this, SLOT(unselectForecast(bool)));

    //change time for given model
    connect(modelComboBox, SIGNAL(currentIndexChanged(int)),
        this, SLOT(setTimeLimits(int)));
    connect(modelComboBox, SIGNAL(currentIndexChanged(int)),
        this, SLOT(setComboToolTip(int)));

    //layout
    downloadLayout = new QHBoxLayout;
    downloadLayout->addWidget(modelComboBox);
    downloadLayout->addWidget(hourSpinBox);
    downloadLayout->addWidget(downloadToolButton);

    downloadGroupBox->setLayout(downloadLayout);

    treeLayout = new QHBoxLayout;
    treeLayout->addWidget(treeView);

    loadLayout = new QHBoxLayout;
    loadLayout->addWidget(statusLabel);
    loadLayout->addWidget(refreshToolButton);

    weatherLayout = new QVBoxLayout;
    weatherLayout->addWidget(downloadGroupBox);
    weatherLayout->addWidget(forecastListLabel);
    weatherLayout->addLayout(treeLayout);
    weatherLayout->addLayout(loadLayout);
    
    ninjafoamConflictLabel = new QLabel(tr("The weather model initialization option is not currently available for the\n"
        "momentum solver."), this);
    ninjafoamConflictLabel->setHidden(true);

    weatherGroupBox->setLayout( weatherLayout );

    layout = new QVBoxLayout;
    layout->addWidget(ninjafoamConflictLabel);
    layout->addWidget( weatherGroupBox );
    setLayout(layout);

#ifdef WITH_NOMADS_SUPPORT
    nNomadsCount = 0;
    while( apszNomadsKeys[nNomadsCount][0] != NULL )
        nNomadsCount++;
    papoNomads = new NomadsWxModel*[nNomadsCount];
    int i = 0;
    while( apszNomadsKeys[i][0] != NULL )
    {
        papoNomads[i] = new NomadsWxModel( apszNomadsKeys[i][0] );
        i++;
    }
    CPLDebug( "WINDNINJA", "Loaded %d NOMADS models", nNomadsCount );
#endif
    progressDialog = new QProgressDialog( this );
    progressDialog->setAutoClose( false );
    progressDialog->setAutoReset( false );
    progressDialog->setModal( true );

    pGlobProg = progressDialog;

    loadModelComboBox();

    checkForModelData();
}

/**
 * Destructor.  Closes files.`
 */
weatherModel::~weatherModel()
{
#ifdef WITH_NOMADS_SUPPORT
    for( int i = 0; i < nNomadsCount; i++ )
        delete papoNomads[i];
    delete [] papoNomads;
#endif
}

/**
 * Populate the combobox with available models from the host_list.xml
 *
 * @return false if population fails
 */
void weatherModel::loadModelComboBox()
{
    modelComboBox->addItem(  QString::fromStdString(ndfd.getForecastIdentifier() ) );
    modelComboBox->addItem( QString::fromStdString(nam.getForecastIdentifier() ) );
    modelComboBox->addItem( QString::fromStdString(rap.getForecastIdentifier() ) );
    //modelComboBox->addItem( QString::fromStdString(dgex.getForecastIdentifier() ) );
    modelComboBox->addItem( QString::fromStdString(namAk.getForecastIdentifier() ) );
    modelComboBox->addItem( QString::fromStdString(gfs.getForecastIdentifier() ) );
#ifdef WITH_NOMADS_SUPPORT
    /* Nomads */
    QString s;
    for( int i = 0; i < nNomadsCount; i++ )
    {
        s = QString::fromStdString( papoNomads[i]->getForecastReadable( '-' ) );
        s = s.toUpper();
        modelComboBox->addItem( s );
    }
#endif
}

void weatherModel::setTimeLimits( int index )
{
    if( index == 0 )
        hourSpinBox->setRange( ndfd.getStartHour(), ndfd.getEndHour() );
    else if( index == 1 )
        hourSpinBox->setRange( nam.getStartHour(), nam.getEndHour() );
    else if( index == 2 )
        hourSpinBox->setRange( rap.getStartHour(), rap.getEndHour() );
    else if( index == 3 )
        hourSpinBox->setRange( namAk.getStartHour(), namAk.getEndHour() );
    else if( index == 4 )
        hourSpinBox->setRange( gfs.getStartHour(), gfs.getEndHour() );
    else
    {
#ifdef WITH_NOMADS_SUPPORT
        int n = index - 5;
        hourSpinBox->setRange( papoNomads[n]->getStartHour(),
                               papoNomads[n]->getEndHour() );
#endif
        return;
    }
}

static int UpdateProgress( double dfProgress, const char *pszMessage,
                           void *pProgressArg )
{
    (void*)pProgressArg;
    if( pszMessage )
        pGlobProg->setLabelText( pszMessage );
    pGlobProg->setValue( dfProgress * 100 );
    QCoreApplication::processEvents();
    if( pGlobProg->wasCanceled() )
        return 1;
    return 0;
}

/**
 * Retrieve the forecast from the UCAR thredds server based on user input
 *
 */
void weatherModel::getData()
{
    statusLabel->setText( "Downloading data...");
    setCursor(Qt::WaitCursor);
    int modelChoice = modelComboBox->currentIndex();
    int hours = hourSpinBox->value();

    //change length to 4 days;
    //days = 20;

    wxModelInitialization *model;

    if( inputFile.isEmpty() ) {
    statusLabel->setText( "No input dem file specified" );
    setCursor(Qt::ArrowCursor);
    return;
    }
    if( modelChoice == 0 )
        model = new ncepNdfdInitialization( ndfd );
    else if( modelChoice == 1 )
        model = new ncepNamSurfInitialization( nam );
    else if( modelChoice == 2 )
        model = new ncepRapSurfInitialization( rap );
    //else if( modelChoice == 3 )
    //	model = new ncepDgexSurfInitialization( dgex );
    else if( modelChoice == 3 )
        model = new ncepNamAlaskaSurfInitialization( namAk );
    else if( modelChoice == 4 )
        model = new ncepGfsSurfInitialization( gfs );
    else
    {
#ifdef WITH_NOMADS_SUPPORT
        model = papoNomads[modelChoice - 5];
        /*
        ** Disable progress on 32-bit windows as we segfault.
        */
#if defined(WIN32) && defined(NINJA_32BIT)
        model->SetProgressFunc( NULL );
        QCoreApplication::processEvents();
#else /* defined(WIN32) && !defined(NINJA_64BIT) */
        progressDialog->reset();
        progressDialog->setRange( 0, 100 );
        model->SetProgressFunc( (GDALProgressFunc)&UpdateProgress );
        progressDialog->show();
        progressDialog->setCancelButtonText( "Cancel" );
#endif /* defined(WIN32) && defined(NINJA_32BIT) */
#endif /* WITH_NOMADS_SUPPORT */
    }

    try {
        model->fetchForecast( inputFile.toStdString(), hours );
    }
    catch( badForecastFile &e ) {
        progressDialog->close();
        QMessageBox::warning( this, "WindNinja",
                              QString::fromStdString( e.what() ),
                              QMessageBox::Ok );
        setCursor(Qt::ArrowCursor);
        return;
    }
    catch( std::runtime_error &e ) {
        progressDialog->close();
        QMessageBox::warning( this, "WindNinja",
                              QString::fromStdString( e.what() ),
                              QMessageBox::Ok );
        setCursor(Qt::ArrowCursor);
        checkForModelData();
        return;
    }

#if defined(NINJA_64BIT)
    if( modelChoice > 4 )
    {
        progressDialog->setRange( 0, 100 );
        progressDialog->setValue( 100 );
        progressDialog->setLabelText( "Done" );
        progressDialog->setCancelButtonText( "Close" );
    }
#endif /* defined(NINJA_64BIT) */

    checkForModelData();
    setCursor(Qt::ArrowCursor);

    //connect with thread::finished()?
    if( modelChoice < 5 )
        delete model;
}

/**
 * Check the working directory for a weather model data directory
 *
 */
void weatherModel::checkForModelData()
{
    QDir wd(cwd);

    QStringList filters;
    /* ndfd */
    filters << QString::fromStdString( ndfd.getForecastIdentifier() )
               + "-" + QFileInfo( inputFile ).fileName();
    /* nam suface */
    filters << QString::fromStdString( nam.getForecastIdentifier() )
               + "-" + QFileInfo( inputFile ).fileName();
    /* rap surface */
    filters << QString::fromStdString( rap.getForecastIdentifier() )
            + "-" + QFileInfo( inputFile ).fileName();
    /* dgex surface */
    //filters << QString::fromStdString( dgex.getForecastIdentifier() )
    //	+ "-" + QFileInfo( inputFile ).fileName();
    /* nam alaska surface */
    filters << QString::fromStdString( namAk.getForecastIdentifier() )
               + "-" + QFileInfo( inputFile ).fileName();
    /* gfs */
    filters << QString::fromStdString( gfs.getForecastIdentifier() )
               + "-" + QFileInfo( inputFile ).fileName();

#ifdef WITH_NOMADS_SUPPORT
    int i;
    for( i = 0; i < nNomadsCount; i++ )
    {
        filters << 
            (QString::fromStdString(papoNomads[i]->getForecastReadable('-') )
                    + "-" + QFileInfo( inputFile ).fileName()).toUpper();
    }
    filters << "20*.zip";
#endif
    //filter to see the folder in utc time
    filters << "20*T*";
    filters << "*.nc";
    model->setNameFilters(filters);
    model->setFilter(QDir::Files | QDir::Dirs);
    treeView->setRootIndex(model->index(wd.absolutePath()));
    treeView->resizeColumnToContents(0);
    statusLabel->setText( "" );

    unselectForecast(false);
    // QModelIndex index = treeView->indexBelow( treeView->rootIndex() );
    // treeView->setExpanded( index, true );
    // index = treeView->indexBelow( index );
    // treeView->setExpanded( index, true );
}

/**
 * Slot to catch any time the dem file changes
 *
 * @param newFile new dem file name
 */
void weatherModel::setInputFile(QString newFile)
{
    inputFile = newFile;
    cwd = QFileInfo(newFile).absolutePath();
    checkForModelData();
}

/**
 * Display the first forecast time for a given file
 *
 * @param index index in the tree view.
 */
void weatherModel::displayForecastTime( const QModelIndex &index )
{
    QFileInfo fi( model->fileInfo( index ) );

    if( !fi.exists() ) {
        statusLabel->setText( "" );
        return;
    }
    std::string filename = fi.absoluteFilePath().toStdString();
    wxModelInitialization* model = NULL;
    std::vector<blt::local_date_time> timelist;
    try {
        model = wxModelInitializationFactory::makeWxInitialization(filename);
        timelist = model->getTimeList(tzString.toStdString());
    }
    catch( ... ) {
        statusLabel->setText( "" );
        delete model;
        return;
    }

    delete model;

    blt::local_time_facet* facet;
    facet = new blt::local_time_facet();
    std::ostringstream os;
    os.imbue(std::locale(std::locale::classic(), facet));
    facet->format("%a %b %d %H:%M %z");
    os << timelist[0];
    QString dateTime = QString::fromStdString( os.str() );
    dateTime.prepend( "First forecast time: " );

    statusLabel->setText( dateTime );
}

/**
 * Slot to recieve a change in timezone from location
 *
 * @param tz timezone string
 */
void weatherModel::updateTz( QString tz )
{
    tzString = tz;
    checkForModelData();
}

/**
 * Unselect the forecast if the group box is disabled.  This is for
 * mainWindow check...() fx(s)
 *
 * @param checked
 */
void weatherModel::unselectForecast( bool checked )
{
    if( checked )
    return;
    else
    treeView->selectionModel()->clear();
}
const char * weatherModel::ExpandDescription( const char *pszReadable )
{
    if( pszReadable == NULL )
        return NULL;
    const char *pszDesc = NULL;
    const char *pszTmp =  NULL;
    int i;
    char **papszKeys = NULL;
    papszKeys = CSLTokenizeString2( pszReadable, "- ", 0 );
    i = 0;
    while( papszKeys[i] != NULL )
    {
        {
            pszTmp = CSLFetchNameValue( (char**)apszWxModelGlossary, papszKeys[i] );
            if( pszTmp != NULL )
            {
                if( pszDesc == NULL )
                    pszDesc = CPLSPrintf( "%s", pszTmp );
                else
                    pszDesc = CPLSPrintf( "%s, %s", pszDesc, pszTmp );
            }
        }
        i++;
    }
    return pszDesc;
}

void weatherModel::setComboToolTip(int)
{
    QString s = modelComboBox->currentText();
    s = ExpandDescription( s.toLocal8Bit().data() );
    modelComboBox->setToolTip( s );
}
