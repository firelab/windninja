/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja Qt GUI
 * Purpose:  Main window and parent to all other widgets
 * Author:   Kyle Shannon <kyle@pobox.com>
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

#include "mainWindow.h"

mainWindow::mainWindow(QWidget *parent) 
: QMainWindow(parent)
{
    std::string tzfile = FindDataPath( "date_time_zonespec.csv" );
    if( tzfile == "" )
    {
        throw std::runtime_error( "Could not find supporting data files, " \
                                  "try setting WINDNINJA_DATA." );
    }
    lineNumber = 1;
    GDALProjRef = "";

    progressDialog = new QProgressDialog(this);
    progressDialog->setMinimumDuration(1);
    progressDialog->setAutoClose(false);
    progressDialog->setAutoReset(false);
    progressDialog->setWindowModality(Qt::ApplicationModal);

    runProgress = 0;
    totalProgress = 0;

    noGoogleCellSize = 30.0;

    //set defaults for some variables
    inputFileName = "";
    inputFileDir = "";
    inputFileType = -1;
    shortInputFileName = "";
    prjFileName = "";
    hasPrj = false;

    GDALCenterLat = GDALCenterLon = 0;
    hasGDALCenter = false;

    tree = new WindNinjaTree;

    createConsole();
    createActions();
    createMenus();

    createTimers();

    createConnections();

    setCentralWidget(tree);

    //threading
    sThread = new solveThread;

    meshCellSize = 200.0;

    QString v(VERSION);
    v = "Welcome to WindNinja " + v;


    writeToConsole(v, Qt::blue);

    //get and set working directory for open dialogs and tutorials.
    cwd = QDir::current();
    pwd.setPath(QString::fromStdString(FindNinjaBinDir()));
    //pwd = QDir::current();
    pwd.cdUp();

    writeToConsole(cwd.currentPath());
    cwd.cd("../example-files");
    //writeToConsole(cwd.absolutePath());

    readSettings();

    okToContinueCheck = false;

    computeCellSize(tree->surface->meshResComboBox->currentIndex());

    checkAllItems();
    army = NULL;
}

bool mainWindow::okToContinue()
{
  if(okToContinueCheck)
    {
      int r = QMessageBox::warning(this, tr("WindNinja"),
                   tr("Are you sure you want to exit?"),
                   QMessageBox::Yes |
                   QMessageBox::No |
                   QMessageBox::Cancel);
      if(r == QMessageBox::Yes)
    return true;
      else if(r == QMessageBox::No || r == QMessageBox::Cancel)
    return false;
      else
    return false;
    }
  else
    return true;
}

void mainWindow::closeEvent(QCloseEvent *event)
{
  if(okToContinue())
    writeSettings();
  else
    event->ignore();
}

void mainWindow::writeSettings()
{
  writeToConsole("Saving settings...");
  QSettings settings(QSettings::UserScope, "Firelab", "WindNinja");
  settings.setDefaultFormat(QSettings::IniFormat);
  //input file path
  writeToConsole(inputFileDir.absolutePath());
  settings.setValue("inputFileDir", inputFileDir.absolutePath());
  //veg choice
  settings.setValue("vegChoice", tree->surface->roughnessComboBox->
            currentIndex());
  //mesh choice
  settings.setValue("meshChoice", tree->surface->meshResComboBox->
            currentIndex());
  //mesh units
  settings.setValue("meshUnits", tree->surface->meshMetersRadioButton->
            isChecked());
  //number of processors
  settings.setValue("nProcessors", tree->solve->numProcSpinBox->value());

  //time zone
  settings.setValue("timeZone",
            tree->surface->timeZone->tzComboBox->currentText() );

  settings.setValue("pointFile", tree->point->stationFileName );

  settings.setValue("customRes", tree->surface->meshResDoubleSpinBox->value());

  writeToConsole("Settings saved.");
}

void mainWindow::readSettings()
{
  QSettings settings(QSettings::UserScope, "Firelab", "WindNinja");
  settings.setDefaultFormat(QSettings::IniFormat);
  if(settings.contains("inputFileDir"))
    {
      inputFileDir = settings.value("inputFileDir").toString();
    }
  else
    {
        std::string oTmpPath = FindNinjaRootDir();
        inputFileDir = CPLFormFilename(oTmpPath.c_str(), "etc/windninja/example-files", NULL);
    }
  if(settings.contains("vegChoice"))
    {
      tree->surface->roughnessComboBox->
    setCurrentIndex(settings.value("vegChoice").toInt());
    }
  if(settings.contains("meshChoice"))
    {
        int choice = settings.value("meshChoice").toInt();
        tree->surface->meshResComboBox->setCurrentIndex(choice);
        if(choice == 4 && settings.contains("customRes"))
        {
            double r = settings.value("customRes").toDouble();
            tree->surface->meshResDoubleSpinBox->setValue(r);
        }
    }
  if(settings.contains("meshUnits"))
    {
      if(settings.value("meshUnits").toBool())
    tree->surface->meshMetersRadioButton->setChecked(true);
      else
    tree->surface->meshFeetRadioButton->setChecked(true);
    }
  if(settings.contains("nProcessors"))
    {
    tree->solve->numProcSpinBox->
        setValue(settings.value("nProcessors").toInt());
    }
  if(settings.contains("timeZone"))
    {
    QString v = settings.value("timeZone").toString();
    int index = tree->surface->timeZone->tzComboBox->findText(v);
    if(index == -1)
        tree->surface->timeZone->tzCheckBox->setChecked( true );
    index = tree->surface->timeZone->tzComboBox->findText(v);
    if( index == 0 )
        tree->surface->timeZone->tzComboBox->setCurrentIndex(index +  1);
    tree->surface->timeZone->tzComboBox->setCurrentIndex(index);
    }
  else
    {
        tree->surface->timeZone->tzComboBox->setCurrentIndex(2);
        tree->surface->timeZone->tzComboBox->setCurrentIndex(1);
    }
  if(settings.contains("pointFile"))
    {
    QString f = settings.value("pointFile").toString();
    tree->point->stationFileName = f;
    }
}

void mainWindow::createActions()
{
  //open surface file action
  openInputFileAction = new QAction(tr("Open &Elevation Input File"), this);
  openInputFileAction->setIcon(QIcon(":folder_page.png"));
  openInputFileAction->setShortcut(tr("Ctrl+D"));
  openInputFileAction->setStatusTip(tr("Open Surface Input  File"));
  connect(openInputFileAction, SIGNAL(triggered()),
      this, SLOT(openInputFile()));

  //exitAction
  exitAction = new QAction(tr("E&xit"), this);
  exitAction->setIcon(QIcon(":cancel.png"));
  exitAction->setShortcut(tr("Alt+F4"));
  exitAction->setStatusTip(tr("Exit WindNinja"));
  connect(exitAction, SIGNAL(triggered()),
      this, SLOT(close()));

  //write console output action
  writeConsoleOutputAction = new QAction(tr("Write console output to file..."), this);
  writeConsoleOutputAction->setIcon(QIcon(":disk.png"));
  writeConsoleOutputAction->setShortcut(tr("Ctrl+W"));
  writeConsoleOutputAction->setStatusTip(tr("Write the console text to disk"));
  connect(writeConsoleOutputAction, SIGNAL(triggered()), this,
      SLOT(writeConsoleOutput()));

  //resample data action
  resampleAction = new QAction(tr("&Resample Data"), this);
  resampleAction->setIcon(QIcon(":resample.png"));
  resampleAction->setShortcut(tr("Ctrl+R"));
  resampleAction->setStatusTip(tr("Resample Existing Data"));
  connect(resampleAction, SIGNAL(triggered()), this, SLOT(resampleData()));

  //write a blank weather station file for point initialization

  writeBlankStationFileAction = new QAction(tr("Write a blank station file"),
                           this);
  writeBlankStationFileAction->setIcon(QIcon(":disk.png"));
  writeBlankStationFileAction->setShortcut(tr("Ctrl+Alt+W"));
  writeBlankStationFileAction->setStatusTip(tr("Write a blank station file for point initialization"));
  connect(writeBlankStationFileAction, SIGNAL(triggered()), this,
      SLOT(writeBlankStationFile()));

  setConfigAction = new QAction(tr("Set Configuration Option"), this);
  setConfigAction->setIcon(QIcon(":cog_go.png"));
  setConfigAction->setStatusTip(tr("Set advanced runtime configuration options"));
  connect(setConfigAction, SIGNAL(triggered()), this, SLOT(SetConfigOption()));

  //wind ninja help action
  windNinjaHelpAction = new QAction(tr("WindNinja &Help"), this);
  windNinjaHelpAction->setIcon(QIcon(":help.png"));
  windNinjaHelpAction->setShortcut(tr("Ctrl+H"));
  windNinjaHelpAction->setStatusTip(tr("Get Help with the WindNinja"));
  connect(windNinjaHelpAction, SIGNAL(triggered()), this,
      SLOT(windNinjaHelp()));

  //arcView action
  displayShapeFileViewAction = new QAction(tr("How to Display Shapefiles in ArcView"), this);
  displayShapeFileViewAction->setIcon(QIcon(":page_white_acrobat.png"));
  connect(displayShapeFileViewAction, SIGNAL(triggered()), this,
      SLOT(displayArcView()));

  //arcMap action
  displayShapeFileMapAction = new QAction(tr("How to Display Shapefiles in ArcMap"), this);
  displayShapeFileMapAction->setIcon(QIcon(":page_white_acrobat.png"));
  connect(displayShapeFileMapAction, SIGNAL(triggered()), this,
      SLOT(displayArcMap()));

  //open wind ninja tutorial 1 action
  tutorial1Action = new QAction(tr("Tutorial &1:The Basics"), this);
  tutorial1Action->setIcon(QIcon(":page_white_acrobat.png"));
  tutorial1Action->setShortcut(tr("Ctrl+1"));
  tutorial1Action->setStatusTip(tr("Get started using the WindNinja"));
  connect(tutorial1Action, SIGNAL(triggered()), this, SLOT(tutorial1()));

 //open wind ninja tutorial 2 action
  tutorial2Action = new QAction(tr("Tutorial &2: Diurnal Winds and Non-neutral Stability"), this);
  tutorial2Action->setIcon(QIcon(":page_white_acrobat.png"));
  tutorial2Action->setShortcut(tr("Ctrl+2"));
  tutorial2Action->setStatusTip(tr("Using Diurnal Winds in WindNinja"));
  connect(tutorial2Action, SIGNAL(triggered()), this, SLOT(tutorial2()));

  //open wind ninja tutorial 3 action
  tutorial3Action = new QAction(tr("Tutorial &3:Point Initialization"), this);
  tutorial3Action->setIcon(QIcon(":page_white_acrobat.png"));
  tutorial3Action->setShortcut(tr("Ctrl+3"));
  tutorial3Action->setStatusTip(tr("Using Point Initialization in WindNinja"));
  connect(tutorial3Action, SIGNAL(triggered()), this, SLOT(tutorial3()));

  //open wind ninja tutorial 4 action
  tutorial4Action = new QAction(tr("Tutorial &4:Weather Model Initialization"), this);
  tutorial4Action->setIcon(QIcon(":page_white_acrobat.png"));
  tutorial4Action->setShortcut(tr("Ctrl+4"));
  tutorial4Action->setStatusTip(tr("Using Weather Model Initialization in WindNinja"));
  connect(tutorial4Action, SIGNAL(triggered()), this, SLOT(tutorial4()));

  //dem downloader
  downloadDemAction = new QAction(tr("DEM Download Instructions"), this);
  downloadDemAction->setIcon(QIcon(":page_white_acrobat.png"));
  downloadDemAction->setStatusTip(tr("How to download DEM data with WindNinja"));
  connect(downloadDemAction, SIGNAL(triggered()), this, SLOT(demDownload()));

  //dem downloader cli
  fetchDemAction = new QAction(tr("fetch_dem Instructions"), this);
  fetchDemAction->setIcon(QIcon(":page_white_acrobat.png"));
  fetchDemAction->setStatusTip(tr("How to download DEM data with fetch_dem"));
  connect(fetchDemAction, SIGNAL(triggered()), this, SLOT(fetchDem()));


  //open wind ninja tutorial 4 action
  cliInstructionsAction = new QAction(tr("Command Line Interface Instructions"),
                                      this);
  cliInstructionsAction->setIcon(QIcon(":page_white_acrobat.png"));
  cliInstructionsAction->setShortcut(tr("Ctrl+l"));
  cliInstructionsAction->setStatusTip(tr("Using the Command Line Interface"));
  connect(cliInstructionsAction, SIGNAL(triggered()),
          this, SLOT(cliInstructions()));

  //about wn action
  aboutWindNinjaAction = new QAction(tr("&About WindNinja"), this);
  aboutWindNinjaAction->setIcon(QIcon(":help.png"));
  aboutWindNinjaAction->setShortcut(tr("Ctrl+A"));
  aboutWindNinjaAction->setStatusTip(tr("About the WindNinja"));
  connect(aboutWindNinjaAction, SIGNAL(triggered()), this,
      SLOT(aboutWindNinja()));

  //support email action
  supportEmailAction = new QAction(tr("&Email Us"), this);
  supportEmailAction->setIcon(QIcon(":email.png"));
  supportEmailAction->setShortcut(tr("Ctrl+E"));
  supportEmailAction->setStatusTip(tr("Email bugs/comments/questions to the WindNinja team"));
  connect(supportEmailAction, SIGNAL(triggered()), this,
      SLOT(supportEmail()));

  submitBugReportAction = new QAction(tr("Submit Bug Report"), this);
  submitBugReportAction->setIcon(QIcon(":bug_link.png"));
  submitBugReportAction->setShortcut(tr("Ctrl+B"));
  submitBugReportAction->setStatusTip(tr("Submit a bug report via GitHub (requires GitHub ID)"));
  connect(submitBugReportAction, SIGNAL(triggered()), this,
          SLOT(bugReport()));

  //about qt action
  aboutQtAction = new QAction(tr("About &Qt"), this);
  aboutQtAction->setStatusTip(tr("Show the Qt library's About box"));
  connect(aboutQtAction, SIGNAL(triggered()), qApp, SLOT(aboutQt()));

  //test action to test slots.
  testAction = new QAction(tr("Test"), this);
  connect(testAction, SIGNAL(triggered()), this, SLOT(test()));
}

void mainWindow::createMenus()
{
  //file menu
  fileMenu = menuBar()->addMenu(tr("&File"));
  fileMenu->addAction(openInputFileAction);
  fileMenu->addSeparator();
  fileMenu->addAction(writeConsoleOutputAction);
  fileMenu->addSeparator();
  fileMenu->addAction(exitAction);
  //options menu, used member in QDockWidget to return QAction to toggle.
  optionsMenu = menuBar()->addMenu(tr("&Options"));
  optionsMenu->addAction(console->toggleViewAction());

  //tools menu
  toolsMenu = menuBar()->addMenu(tr("&Tools"));
  //toolsMenu->addAction(resampleAction);
  toolsMenu->addAction(writeBlankStationFileAction);
  toolsMenu->addAction(setConfigAction);

  //help/tutorial menus
  helpMenu = menuBar()->addMenu(tr("&Help"));
  shapeSubMenu = helpMenu->addMenu(tr("Displaying Shapefiles"));
  shapeSubMenu->addAction(displayShapeFileViewAction);
  shapeSubMenu->addAction(displayShapeFileMapAction);
  tutorialSubMenu = helpMenu->addMenu(tr("Tutorials"));
  tutorialSubMenu->addAction(tutorial1Action);
  tutorialSubMenu->addAction(tutorial2Action);
  tutorialSubMenu->addAction(tutorial3Action);
  tutorialSubMenu->addAction(tutorial4Action);
  helpMenu->addAction(downloadDemAction);
  helpMenu->addAction(fetchDemAction);
  helpMenu->addAction(cliInstructionsAction);
  helpMenu->addAction(aboutWindNinjaAction);
  helpMenu->addAction(supportEmailAction);
  helpMenu->addAction(submitBugReportAction);

  //context menu for text edit
  //console->consoleTextEdit->addAction(writeConsoleOutputAction);
  //console->consoleTextEdit->setContextMenuPolicy(Qt::ActionsContextMenu);


  //helpMenu->addAction(aboutQtAction);
  //helpMenu->addAction(testAction);
}

/**
 * Create connections for mainWindow
 *
 */
void mainWindow::createConnections()
{
    // Connections for DEM Downloader Button
    connect(tree->surface->downloadDEMButton, SIGNAL(clicked()),
      this, SLOT(openDEMDownloader()));
    //
    connect(&fileWatcher, SIGNAL(fileChanged(QString)),
      this, SLOT(inputFileDeleted()));

  //Connect input file open button to dialog box
  connect(tree->surface->inputFileOpenToolButton, SIGNAL(clicked()),
      this, SLOT(openInputFile()));
  //connect signals/slots
  //connect combo to change mesh res
  connect(tree->surface->meshResComboBox, SIGNAL(currentIndexChanged(int)),
      this, SLOT(checkMeshCombo()));
  //also connect the radio button for feet display
  connect(tree->surface->meshMetersRadioButton, SIGNAL(toggled(bool)),
      this, SLOT(checkMeshUnits(bool)));
  //connect the mesh resolution on the surface input page to the output
  //'use mesh resolution' sections.
  connect(tree->surface->meshResDoubleSpinBox, SIGNAL(valueChanged(double)),
      this, SLOT(updateOutRes()));
  //connect the getLatLon button to the fx
  //connect(tree->location->getLatLonToolButton, SIGNAL(clicked()),
  //	  this, SLOT(getLatLon()));
  //also connect the toggle on the check box on output pages to update
  connect(tree->google->useMeshResCheckBox, SIGNAL(toggled(bool)),
      this, SLOT(updateOutRes()));
  connect(tree->fb->useMeshResCheckBox, SIGNAL(toggled(bool)),
      this, SLOT(updateOutRes()));
  connect(tree->shape->useMeshResCheckBox, SIGNAL(toggled(bool)),
      this, SLOT(updateOutRes()));
  connect(tree->diurnal->diurnalGroupBox, SIGNAL(toggled(bool)),
      tree->wind->windTable, SLOT(enableDiurnalCells(bool)));
  connect(tree->diurnal->diurnalGroupBox, SIGNAL(toggled(bool)),
      this, SLOT(enablePointDate(bool)));
#ifdef STABILITY
  connect(tree->stability->stabilityGroupBox, SIGNAL(toggled(bool)),
      this, SLOT(enablePointDate(bool)));
  connect(tree->stability->stabilityGroupBox, SIGNAL(toggled(bool)),
      tree->wind->windTable, SLOT(enableStabilityCells(bool)));
#endif
#ifdef NINJAFOAM
  connect(tree->ninjafoam->ninjafoamGroupBox, SIGNAL(toggled(bool)),
      this, SLOT(enableNinjafoamOptions(bool)));
#endif
  //connect change in tree to the checkers
  connect(tree->tree, SIGNAL(currentItemChanged(QTreeWidgetItem *,
      QTreeWidgetItem *)), this, SLOT(checkAllItems()));

  //connect the diurnalGroupBox->toggled to checkers
  connect(tree->diurnal->diurnalGroupBox, SIGNAL(toggled(bool)),
      this, SLOT(checkAllItems()));
#ifdef STABILITY
  connect(tree->stability->stabilityGroupBox, SIGNAL(toggled(bool)),
      this, SLOT(checkAllItems()));
#endif
#ifdef NINJAFOAM
  //connect the solver method check boxes for mutex
  connect(tree->ninjafoam->ninjafoamGroupBox, SIGNAL(toggled(bool)),
      this, SLOT(checkAllItems()));
  connect(tree->nativesolver->nativeSolverGroupBox, SIGNAL(toggled(bool)),
      this, SLOT(checkAllItems()));
  connect( tree->nativesolver->nativeSolverGroupBox, SIGNAL( toggled( bool ) ),
       this, SLOT( selectNativeSolver( bool ) ) );      
  connect( tree->ninjafoam->ninjafoamGroupBox, SIGNAL( toggled( bool ) ),
       this, SLOT( selectNinjafoamSolver( bool ) ) );
#endif

  //connect the speed and direction in the first row to the checkers
  connect(tree->wind->windTable->speed[0], SIGNAL(valueChanged(double)), this,
      SLOT(checkAllItems()));
  connect(tree->wind->windTable->dir[0], SIGNAL(valueChanged(int)), this,
      SLOT(checkAllItems()));

  //connect the initialization check boxes to checkers
  connect(tree->wind->windGroupBox, SIGNAL(toggled(bool)),
      this, SLOT(checkAllItems()));
  connect(tree->point->pointGroupBox, SIGNAL(toggled(bool)),
      this, SLOT(checkAllItems()));
  connect(tree->point->pointGroupBox, SIGNAL(toggled(bool)),
      this, SLOT(enablePointDate(bool)));
  connect(tree->weather->weatherGroupBox, SIGNAL(toggled(bool)),
      this, SLOT(checkAllItems()));

  //connect selection change in weather to checkers
  connect(tree->weather->treeView->selectionModel(),
      SIGNAL(currentChanged(const QModelIndex&, const QModelIndex&)),
      this, SLOT(checkAllItems()));

  //connect the output check boxes with the checkers
  connect(tree->google->googleGroupBox, SIGNAL(toggled(bool)),
      this, SLOT(checkAllItems()));
  connect(tree->fb->fbGroupBox, SIGNAL(toggled(bool)),
      this, SLOT(checkAllItems()));
  connect(tree->shape->shapeGroupBox, SIGNAL(toggled(bool)),
      this, SLOT(checkAllItems()));
  connect(tree->vtk->vtkGroupBox, SIGNAL(toggled(bool)),
      this, SLOT(checkAllItems()));

  //and the spinboxes
  connect(tree->google->googleResSpinBox, SIGNAL(valueChanged(double)),
      this, SLOT(checkAllItems()));
  connect(tree->fb->fbResSpinBox, SIGNAL(valueChanged(double)),
      this, SLOT(checkAllItems()));
  connect(tree->shape->shapeResSpinBox, SIGNAL(valueChanged(double)),
      this, SLOT(checkAllItems()));

  //check the google res, make sure not bad
  connect(tree->google->googleResSpinBox, SIGNAL(valueChanged(double)),
      this, SLOT(checkKmlLimit(double)));

  //solve button and solve()
  connect(tree->solve->solveToolButton, SIGNAL(clicked()),
      this, SLOT(solve()));

  connect(progressDialog, SIGNAL(canceled()), this, SLOT(cancelSolve()));

  connect(tree->solve->solveToolButton, SIGNAL(clicked()), progressDialog,
      SLOT(forceShow()));

  //connect double clicks on trees to main action for that item
  connect(tree->tree, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)),
      this, SLOT(treeDoubleClick(QTreeWidgetItem*, int)));

  //connect inputFileChanged to anything that wants it
  connect(this, SIGNAL(inputFileChanged(QString)),
      tree->weather, SLOT(setInputFile(QString)));
  connect(this, SIGNAL(inputFileChanged(QString)),
      tree->point, SLOT(setInputFile(QString)));

  //connect other writeToConsoles to the main writeToConsole
  connect( tree->point, SIGNAL( writeToConsole( QString ) ),
       this, SLOT( writeToConsole( QString ) ) );

  //connect timezone combo to weather model tz string
  connect( tree->surface->timeZone, SIGNAL( tzChanged( QString ) ),
       tree->weather, SLOT( updateTz( QString ) ) );

  //connect the initialization check boxes to the others for mutex
  connect( tree->wind->windGroupBox, SIGNAL( toggled( bool ) ),
       this, SLOT( selectWindInitialization( bool ) ) );
  connect( tree->point->pointGroupBox, SIGNAL( toggled( bool ) ),
       this, SLOT( selectPointInitialization( bool ) ) );
  connect( tree->weather->weatherGroupBox, SIGNAL( toggled( bool ) ),
       this, SLOT( selectWeatherInitialization( bool ) ) );

  //connect change in station file to checkAllItems
  connect( tree->point, SIGNAL( stationFileChanged() ),
       this, SLOT( checkAllItems() ) );

  //connect the solve open out path button
  connect( tree->solve->openOutputPathButton, SIGNAL( clicked() ),
       this, SLOT( openOutputPath() ) );

  connect( progressDialog, SIGNAL( canceled() ),
       this, SLOT( updateTimer() ) );
}
/**
 * Slot to catch a change in initialization method
 *
 * @param pick I am picked
 */
void mainWindow::selectWindInitialization( bool pick )
{
    if( pick ) {
    tree->point->pointGroupBox->setChecked( false );
    tree->weather->weatherGroupBox->setChecked( false );
    tree->output->wxModelOutputCheckBox->setDisabled( true );
    checkAllItems();
    }
}

void mainWindow::selectPointInitialization( bool pick )
{
    if( pick ) {
    tree->wind->windGroupBox->setChecked( false );
    tree->weather->weatherGroupBox->setChecked( false );
    tree->output->wxModelOutputCheckBox->setDisabled( true );
    checkAllItems();
    }
}

void mainWindow::selectWeatherInitialization( bool pick )
{
    if( pick ) {
    tree->wind->windGroupBox->setChecked( false );
    tree->point->pointGroupBox->setChecked( false );
    checkAllItems();
    }
    tree->output->wxModelOutputCheckBox->setEnabled( pick );
}

#ifdef NINJAFOAM
void mainWindow::selectNativeSolver( bool pick )
{
    if( pick ) {
    tree->ninjafoam->ninjafoamGroupBox->setChecked( false );
    checkAllItems();
    }
}
void mainWindow::selectNinjafoamSolver( bool pick )
{
    if( pick ) {
    tree->nativesolver->nativeSolverGroupBox->setChecked( false );
    checkAllItems();
    }
}
#endif //NINJAFOAM


//function for finding and opening an input file.
void mainWindow::openInputFile()
{
  //writeToConsole(inputFileDir.absolutePath());
  //setCursor(Qt::WaitCursor);
  QString fileName = QFileDialog::getOpenFileName(this,
             tr("Open Elevation Input File"),
             inputFileDir.absolutePath(),

             tr("Elevation Input Files (*.asc *.lcp *.tif *.img)"));

  if(!fileName.isEmpty())
    {
      cwd = QFileInfo(fileName).dir();
      //use GDAL to check the file
      QString newFile = checkForNoData(fileName);
      if(!newFile.isEmpty())
      {
          fileName = newFile;
      }

      if(checkInputFile(fileName) < 0)
      {
          tree->surface->inputFileLineEdit->clear();
          fileName = "";
          return;
      }

      QString shortName = QFileInfo(fileName).fileName();
      if(inputFileType == LCP)
      {
        tree->surface->roughnessComboBox->setDisabled(true);
        tree->surface->roughnessComboBox->hide();
        tree->surface->roughnessLabel->show();
      }
      else
        {
            tree->surface->roughnessComboBox->setDisabled(false);
            tree->surface->roughnessComboBox->show();
            tree->surface->roughnessLabel->hide();
        }

      tree->surface->inputFileLineEdit->setText(shortName);
      
      tree->surface->meshResComboBox->setEnabled(true);

      if(inputFileName != fileName)
          emit(inputFileChanged(fileName));

      inputFileName = fileName;
      inputFileDir = QFileInfo(fileName).absolutePath();
      shortInputFileName = shortName;
      checkMeshCombo();
      checkInputItem();
    }

}

/**
 * Slot to update elevation file input with downloaded DEM file
 *
 * @param file File created from downloaded DEM
 */
void mainWindow::updateFileInput(const char* file)
{
    QString fileName(file);

    fileWatcher.addPath(fileName);

    if(!fileName.isEmpty())
    {
      cwd = QFileInfo(fileName).dir();
      //use GDAL to check the file
      if(checkInputFile(fileName) < 0)
      {
          tree->surface->inputFileLineEdit->clear();
          fileName = "";
          return;
      }

      QString shortName = QFileInfo(fileName).fileName();
      if(inputFileType == LCP)
      {
        tree->surface->roughnessComboBox->setDisabled(true);
        tree->surface->roughnessComboBox->hide();
        tree->surface->roughnessLabel->show();
      }
      else
        {
            tree->surface->roughnessComboBox->setDisabled(false);
            tree->surface->roughnessComboBox->show();
            tree->surface->roughnessLabel->hide();
        }

    tree->surface->inputFileLineEdit->setText(shortName);
    
    tree->surface->meshResComboBox->setEnabled(true);

      if(inputFileName != fileName)
    emit(inputFileChanged(fileName));

      inputFileName = fileName;
      inputFileDir = QFileInfo(fileName).absolutePath();
      shortInputFileName = shortName;
      checkMeshCombo();
      checkInputItem();
    }
}

void mainWindow::inputFileDeleted()
{
    tree->surface->inputFileLineEdit->clear();
    //emit(inputFileChanged());
    checkMeshCombo();
    checkInputItem();
}

void mainWindow::openMainWindow()
{
    this->setEnabled(true);
}

void mainWindow::createConsole()
{
  console = new ConsoleDockWidget;
  addDockWidget(Qt::BottomDockWidgetArea, console);
  console->consoleTextEdit->setReadOnly(true);

  //set prompt
  prompt = "~>";
  //orange
  orange.setRgb(255, 165, 0);

}
void mainWindow::setPrompt(QString p)
{
  prompt = p;
}
void mainWindow::writeConsoleOutput()
{
  QDateTime date(QDateTime::currentDateTime());
  QString fileName = QFileDialog::getSaveFileName
    (this, tr("Save console output as..."), "console-output.txt",
     tr("Text Files (*.txt)"));
  writeToConsole("WindNinja console output from:");
  writeToConsole(date.toString("MM/dd/yyyy hh:mm:ss"));
  if(!fileName.isEmpty())
    {
      std::ofstream fout(fileName.toStdString().c_str(), std::ios::out);
      QString text = console->consoleTextEdit->toPlainText();
      fout << text.toStdString();
      writeToConsole(QString("Console data written to " + fileName + "."), Qt::darkGreen);
      fout.close();
    }
  else
    writeToConsole(QString("Cannot open " + fileName + " for writing."), Qt::red);
}

//create timers for status bar
void mainWindow::createTimers()
{
  runTime = new QTime(0, 0, 0, 0);
  runTime->start();
}

void mainWindow::updateTimer()
{
    //elapsedRunTime = runTime->elapsed() / 1000.0;
  writeToConsole("Total Simulation time: " + QString::number(elapsedRunTime, 'f', 2) + " seconds");
}

void mainWindow::openDEMDownloader()
{
    demWidget = new WidgetDownloadDEM();
    //demWidget->setAttribute(Qt::WA_DeleteOnClose, true);
    demWidget->settingsDir.setPath(inputFileDir.absolutePath());
    connect(demWidget, SIGNAL(doneDownloading(const char*)), this, SLOT(updateFileInput(const char*)));
    connect(demWidget, SIGNAL(exitDEM()), this, SLOT(openMainWindow()));
    connect(demWidget, SIGNAL(destroyed()), this, SLOT(openMainWindow()));
    this->setEnabled(false);
}

void mainWindow::test()
{
}

void mainWindow::resampleData()
{}

void mainWindow::writeBlankStationFile()
{
    QString fileName = QFileDialog::getSaveFileName
    (this, tr("Save station file as..."), "stations.csv",
     tr("Text Files (*.csv)"));
    if(!fileName.isEmpty())
    wxStation::writeBlankStationFile( fileName.toStdString() );
    else
    return;
}

void mainWindow::windNinjaHelp()
{}

void mainWindow::tutorial1()
{
  pwd.cd("share/windninja/doc/tutorials");
  writeToConsole("Opening " + pwd.absoluteFilePath("WindNinja_tutorial1.pdf"));

  if(!QDesktopServices::openUrl(QUrl(pwd.absoluteFilePath("WindNinja_tutorial1.pdf"))))
    {

      QMessageBox::warning(this, tr("Broken Link."),
               tr("The link to the tutorial is broken, you can get to it through the Start Menu."),
               QMessageBox::Ok | QMessageBox::Default);
    }
  pwd.setPath(QString::fromStdString(FindNinjaBinDir()));
  pwd.cdUp();
}

void mainWindow::tutorial2()
{
  pwd.cd("share/windninja/doc/tutorials");
  writeToConsole("Opening " + pwd.absoluteFilePath("WindNinja_tutorial2.pdf"));

  if(!QDesktopServices::openUrl(QUrl(pwd.absoluteFilePath("WindNinja_tutorial2.pdf"))))
    {

      QMessageBox::warning(this, tr("Broken Link."),
               tr("The link to the tutorial is broken, you can get to it through the Start Menu."),
               QMessageBox::Ok | QMessageBox::Default);
    }
  pwd.setPath(QString::fromStdString(FindNinjaBinDir()));
  pwd.cdUp();
}

void mainWindow::tutorial3()
{
  pwd.cd("share/windninja/doc/tutorials");
  writeToConsole("Opening " + pwd.absoluteFilePath("WindNinja_tutorial3.pdf"));

  if(!QDesktopServices::openUrl(QUrl(pwd.absoluteFilePath("WindNinja_tutorial3.pdf"))))
    {

      QMessageBox::warning(this, tr("Broken Link."),
               tr("The link to the tutorial is broken, you can get to it through the Start Menu."),
               QMessageBox::Ok | QMessageBox::Default);
    }
  pwd.setPath(QString::fromStdString(FindNinjaBinDir()));
  pwd.cdUp();
}

void mainWindow::tutorial4()
{
  pwd.cd("share/windninja/doc/tutorials");
  writeToConsole("Opening " + pwd.absoluteFilePath("WindNinja_tutorial4.pdf"));

  if(!QDesktopServices::openUrl(QUrl(pwd.absoluteFilePath("WindNinja_tutorial4.pdf"))))
    {

      QMessageBox::warning(this, tr("Broken Link."),
               tr("The link to the tutorial is broken, you can get to it through the Start Menu."),
               QMessageBox::Ok | QMessageBox::Default);
    }
  pwd.setPath(QString::fromStdString(FindNinjaBinDir()));
  pwd.cdUp();
}

void mainWindow::demDownload()
{
  pwd.cd("share/windninja/doc");
  writeToConsole("Opening " + pwd.absoluteFilePath("download_elevation_file.pdf"));

  if(!QDesktopServices::openUrl(QUrl(pwd.absoluteFilePath("download_elevation_file.pdf"))))
    {

      QMessageBox::warning(this, tr("Broken Link."),
               tr("The link to the tutorial is broken, you can get to it through the Start Menu."),
               QMessageBox::Ok | QMessageBox::Default);
    }
  pwd.setPath(QString::fromStdString(FindNinjaBinDir()));
  pwd.cdUp();
}

void mainWindow::fetchDem()
{
  pwd.cd("share/windninja/doc");
  writeToConsole("Opening " + pwd.absoluteFilePath("fetch_dem_instructions.pdf"));

  if(!QDesktopServices::openUrl(QUrl(pwd.absoluteFilePath("fetch_dem_instructions.pdf"))))
    {

      QMessageBox::warning(this, tr("Broken Link."),
               tr("The link to the tutorial is broken, you can get to it through the Start Menu."),
               QMessageBox::Ok | QMessageBox::Default);
    }
  pwd.setPath(QString::fromStdString(FindNinjaBinDir()));
  pwd.cdUp();
}


void mainWindow::displayArcMap()
{
  pwd.cd("share/windninja/doc");
  writeToConsole("Opening " + pwd.absoluteFilePath("displaying_wind_vectors_in_ArcMap.pdf"));
  if(!QDesktopServices::openUrl(QUrl(pwd.absoluteFilePath("displaying_wind_vectors_in_ArcMap.pdf"))))
    {

      QMessageBox::warning(this, tr("Broken Link."),
               tr("The link to the tutorial is broken, you can get to it through the Start Menu."),
               QMessageBox::Ok | QMessageBox::Default);
    }
  pwd.setPath(QString::fromStdString(FindNinjaBinDir()));
  pwd.cdUp();
}
void mainWindow::displayArcView()
{
  pwd.cd("share/windninja/doc");
  writeToConsole("Opening " + pwd.absoluteFilePath("displaying_wind_vectors_in_ArcView.pdf"));
  if(!QDesktopServices::openUrl(QUrl(pwd.absoluteFilePath("displaying_wind_vectors_in_ArcView.pdf"))))
    {

      QMessageBox::warning(this, tr("Broken Link."),
               tr("The link to the tutorial is broken, you can get to it through the Start Menu."),
               QMessageBox::Ok | QMessageBox::Default);
    }
  pwd.setPath(QString::fromStdString(FindNinjaBinDir()));
  pwd.cdUp();
}

void mainWindow::cliInstructions()
{
    pwd.cd("share/windninja/doc");
    writeToConsole("Opening " + pwd.absoluteFilePath("CLI_instructions.pdf"));

    if(!QDesktopServices::openUrl(QUrl(pwd.absoluteFilePath("CLI_instructions.pdf"))))
    {

        QMessageBox::warning(this, tr("Broken Link."),
                             tr("The link to the tutorial is broken, you can get to it through the Start Menu."),
                             QMessageBox::Ok | QMessageBox::Default);
    }
  pwd.setPath(QString::fromStdString(FindNinjaBinDir()));
  pwd.cdUp();
}

void mainWindow::supportEmail()
{
  QDesktopServices::openUrl(QUrl("mailto:wind.ninja.support@gmail.com?subject=[windninja-support]"));
}

void mainWindow::bugReport()
{
  QDesktopServices::openUrl(QUrl("https://github.com/firelab/windninja/issues/new"));
}

void mainWindow::aboutWindNinja()
{
  QString aboutText = "<h2>WindNinja</h2>\n";
  aboutText.append("<p><h4>Version:</h4>" + QString(VERSION) + "</p>");

  aboutText.append("<p><h4>SVN Version:</h4>" + QString(SVN_VERSION) + "</p>");

  aboutText.append("<p><h4>Release Date:</h4>" + QString(RELEASE_DATE) + "</p>");
  aboutText.append("<p><h4>Developed by:</h4><p>Jason Forthofer<br /> " \
                                               "Kyle Shannon<br /> " \
                                               "Bret Butler<br /> " \
                                               "Natalie Wagenbrenner <br /> " \
                                               "Cody Posey<br /> " \
                                               "Levi Malott</p>");
  aboutText.append("<p>Missoula Fire Sciences Laboratory<br />");
  aboutText.append("Rocky Mountain Research Station<br />");
  aboutText.append("USDA Forest Service<br />");
  aboutText.append("5775 Highway 10 W.<br />");
  aboutText.append("Missoula, MT 59808</p>");
  aboutText.append("<h4>Sponsored By:</h4>");
  aboutText.append("US Forest Service<br />");
  aboutText.append("Center for Environmental Management of Military Lands at Colorado State University<br />");
  aboutText.append("Joint Fire Sciences Program<br />");
  aboutText.append("Washington State University</p>");

  QMessageBox::about(this, tr("About WindNinja"),
             aboutText);
}

void mainWindow::writeToConsole(QString message, QColor color)
{
  console->consoleTextEdit->setTextColor(color);
  console->consoleTextEdit->append(QString::number(lineNumber) + ": " + message);
  lineNumber++;
  //console->consoleTextEdit->append(prompt + message);
}

void mainWindow::updateOutRes()
{
  //get res from surface page and store as an int
  double resolution = tree->surface->meshResDoubleSpinBox->value();

  if(tree->google->useMeshResCheckBox->isChecked() == true)
    {
      tree->google->googleResSpinBox->setValue(resolution);
      tree->google->googleMetersRadioButton->setChecked(tree->surface->meshMetersRadioButton->isChecked());
    }
  if(tree->fb->useMeshResCheckBox->isChecked() == true)
    {
      tree->fb->fbResSpinBox->setValue(resolution);
      tree->fb->fbMetersRadioButton->setChecked(tree->surface->meshMetersRadioButton->isChecked());
    }
  if(tree->shape->useMeshResCheckBox->isChecked() == true)
    {
      tree->shape->shapeResSpinBox->setValue(resolution);
      tree->shape->shapeMetersRadioButton->setChecked(tree->surface->meshMetersRadioButton->isChecked());
    }
}

//empty fx, need to write it when help is done.
int mainWindow::openHelp(int target)
{
  return target;
}

void mainWindow::checkMeshCombo()
{
  int choice = tree->surface->meshResComboBox->currentIndex();
  double res = 200;
  //there is a splitter at 3
  if(choice == 4)
    {
      tree->surface->meshResDoubleSpinBox->setEnabled(true);
    }
  else
    {
      if(tree->surface->inputFileLineEdit->text() == "")
    {
      tree->surface->meshResDoubleSpinBox->setValue(res);
      meshCellSize = res;
      tree->surface->meshResDoubleSpinBox->setEnabled(false);
    }
      else
    {
      tree->surface->meshResDoubleSpinBox->setEnabled(false);
      res = computeCellSize(choice);
      if(tree->surface->meshFeetRadioButton->isChecked())
        res *= 3.28083989502;
      tree->surface->meshResDoubleSpinBox->setValue(res);
      meshCellSize = res;
      writeToConsole("Mesh Resolution set to " +
             QString::number(res));
    }
    }
}

void mainWindow::checkMeshUnits(bool checked)
{
  if(checked)
    checkMeshCombo();
  else
    checkMeshCombo();
}

double mainWindow::computeCellSize(int index)
{
  int coarse, medium, fine;
  double meshResolution;
  
  meshResolution = 200.0;
  
#ifdef NINJAFOAM  
  if( tree->ninjafoam->ninjafoamGroupBox->isChecked() ){
    /*ninjafoam: calculate mesh resolution of lower volume in block mesh*/
    coarse = 100000;
    medium = 500000;
    fine = 1e6;
  }
  else{
    /* use native mesh */
    coarse = 4000;
    medium = 10000;
    fine = 20000;
  }
#else
  coarse = 4000;
  medium = 10000;
  fine = 20000;
#endif //NINJAFOAM

  int targetNumHorizCells = fine;
  switch(index)
    {
    case 0:
      targetNumHorizCells = coarse;
      break;
    case 1:
      targetNumHorizCells = medium;
      break;
    case 2:
      targetNumHorizCells = fine;
      break;
    case 3:
      return meshResolution;
      break;
    case 4:
      return meshResolution;
      break;
    default:
      return meshResolution;
    }
    
#ifdef NINJAFOAM  
  if( tree->ninjafoam->ninjafoamGroupBox->isChecked() ){
    /* ninjafoam mesh */
    double XLength = (GDALXSize + 1) * GDALCellSize + 200; //100 m buffer on all sides for MDM
    double YLength = (GDALYSize + 1) * GDALCellSize + 200;
    double ZLength = 2450; //bottom face is 50 m above terrain max, top face is 2500 m above terrain max
  
    double volume1;
    double cellCount1;
    double cellVolume1;
    
    volume1 = XLength * YLength * ZLength; //volume near terrain
    cellCount1 = targetNumHorizCells * 0.5; // cell count in volume 1
    cellVolume1 = volume1/cellCount1; // volume of 1 cell in zone1
    meshResolution = std::pow(cellVolume1, (1.0/3.0)); // length of side of regular hex cell in zone1
  }
  else{
    /* native windninja mesh */
    double XLength = (GDALXSize + 1) * GDALCellSize;
    double YLength = (GDALYSize + 1) * GDALCellSize;
    double nXCells = 2 * std::sqrt((double)targetNumHorizCells) * (XLength / (XLength + YLength));
    double nYCells = 2 * std::sqrt((double)targetNumHorizCells) * (YLength / (XLength + YLength));

    double XCellSize = XLength / nXCells;
    double YCellSize = YLength / nYCells;

    meshResolution = (XCellSize + YCellSize) / 2;
  
  }
#else 
  double XLength = (GDALXSize + 1) * GDALCellSize;
  double YLength = (GDALYSize + 1) * GDALCellSize;
  double nXCells = 2 * std::sqrt((double)targetNumHorizCells) * (XLength / (XLength + YLength));
  double nYCells = 2 * std::sqrt((double)targetNumHorizCells) * (YLength / (XLength + YLength));

  double XCellSize = XLength / nXCells;
  double YCellSize = YLength / nYCells;

  meshResolution = (XCellSize + YCellSize) / 2;

  //noGoogleCellSize = std::sqrt((XLength * YLength) / noGoogleNumCells);
#endif //NINJAFOAM

  return meshResolution;
}

//open input file as a GDALDataset, return file type enum;
int mainWindow::checkInputFile(QString fileName)
{
    GDALProjRef = "";
    hasPrj = false;
    GDALDataset *poInputDS;
    double adfGeoTransform[6];
    writeToConsole("Opening dataset...");
    poInputDS = (GDALDataset*)GDALOpen(fileName.toStdString().c_str(),
                                       GA_ReadOnly);
    if(poInputDS == NULL)
    {
        writeToConsole("Cannot open the input file.", Qt::red);
        return -1;
    }

    GDALDriverName = poInputDS->GetDriver()->GetDescription();
    GDALDriverLongName = poInputDS->GetDriver()->GetMetadataItem(GDAL_DMD_LONGNAME);
    writeToConsole("Reading " + GDALDriverLongName + "...");

    //set the file type here
    if(GDALDriverName == "AAIGrid")
        inputFileType = ASC;
    if(GDALDriverName == "LCP")
        inputFileType = LCP;
    if(GDALDriverName == "GTiff")
        inputFileType = GTIFF;
    if(GDALDriverName == "IMG")
        inputFileType = IMG;

    if(inputFileType == LCP)
    {
        tree->surface->roughnessComboBox->setEnabled(false);
        tree->surface->roughnessComboBox->hide();
        tree->surface->roughnessLabel->show();
    }
      else
    {
        tree->surface->roughnessComboBox->setEnabled(true);
        tree->surface->roughnessComboBox->show();
        tree->surface->roughnessLabel->hide();
    }

    //get x and y dimension
    GDALXSize = poInputDS->GetRasterXSize();
    GDALYSize = poInputDS->GetRasterYSize();

    if(!GDALTestSRS(poInputDS))
    {
        hasPrj = false;
        writeToConsole("Invalid Spatial Reference (prj), "
                       "cannot do a simulation with the supplied DEM", red);
        QMessageBox::warning(this, tr("WindNinja"),
                             "The DEM does not contain a proper spatial reference "
                             "system. WindNinja only supports DEM files "
                             "with projected coordinate systems (e.g., UTM)",
                             QMessageBox::Ok | QMessageBox::Default);
        GDALClose((GDALDatasetH)poInputDS);
        return -1;
    }
    else
    {
        GDALProjRef = poInputDS->GetProjectionRef();
        const char *pszProjRef;
        OGRSpatialReference oSRS;
        pszProjRef = GDALProjRef.c_str();
        oSRS.importFromWkt((char**)&pszProjRef);
        if(GDALProjRef == "")
        {
            hasPrj = false;
            QMessageBox::warning(this, tr("WindNinja"),
                                 "The DEM does not contain a proper spatial reference "
                                 "system. WindNinja only supports DEM files "
                                 "with projected coordinate systems (e.g., UTM)",
                                 QMessageBox::Ok | QMessageBox::Default);
            GDALClose((GDALDatasetH)poInputDS);
            return -1;
        }
        /* Check for geographic.  Separate case as we may allow support later
         * on.
         */
        else if(oSRS.IsGeographic())
        {
            hasPrj = false;
            QMessageBox::warning(this, tr("WindNinja"),
                                 "The DEM coordinated system is in a "
                                 "geographic projection (latitude/longitude). "
                                 "WindNinja only supports projected "
                                 "coordinate systems (e.g., UTM)",
                                 QMessageBox::Ok | QMessageBox::Default);
            writeToConsole("Invalid Spatial Reference (prj), "
                           "cannot do a simulation with the supplied DEM", red);
            //tree->surface->inputFileLineEdit->setText("");
            GDALClose((GDALDatasetH)poInputDS);
            return -1;
        }
        else
        {
            hasPrj = true;
            double ll[2];
            if(GDALGetCenter(poInputDS, ll))
            {
                GDALCenterLon = ll[0];
                GDALCenterLat = ll[1];

                //set diurnal location, also set DD.DDDDD
                QString oTimeZone = FetchTimeZone(GDALCenterLon, GDALCenterLat, NULL).c_str();
                if(oTimeZone != "")
                {
                    /* Show all time zones, so we can search all time zones */
                    tree->surface->timeZone->tzCheckBox->setChecked(true);
                    int nIndex = tree->surface->timeZone->tzComboBox->findText(oTimeZone);
                    tree->surface->timeZone->tzComboBox->setCurrentIndex(nIndex);
                }

                //emit latLonChanged( ll[0], ll[1], false );
            }
        }
    }

    //check for ndv
    //if(!checkForNoData(poInputDS))
    if(GDALHasNoData(poInputDS, 1))
    {
        writeToConsole("The input file contains no data values, cannot use",
                       Qt::red);
        GDALClose((GDALDatasetH)poInputDS);
        return -1;
    }

    //get the geo-transform
    if(poInputDS->GetGeoTransform(adfGeoTransform) == CE_None)
    {
        int c1, c2;
        c1 = adfGeoTransform[1];
        c2 = adfGeoTransform[5];
        if(abs(c1) == abs(c2))
            GDALCellSize = abs(c1);
        else
        {
            writeToConsole("Invalid cell size, invalid file");
            GDALClose((GDALDatasetH)poInputDS);
            return -1;
        }
    }

    GDALClose( (GDALDatasetH)poInputDS );

    double XLength = (GDALXSize + 1) * GDALCellSize;
    double YLength = (GDALYSize + 1) * GDALCellSize;

    noGoogleCellSize = std::sqrt((XLength * YLength) / noGoogleNumCells);

    writeToConsole("File Opened.", Qt::darkGreen);
    return 0;
}

bool mainWindow::getLatLon()
{
  if(hasGDALCenter)
    {
      writeToConsole("The center of your DEM has been found.", Qt::darkGreen);
      writeToConsole("The lat/lon has been set in the diurnal inputs section", Qt::darkGreen);
      checkAllItems();
      return true;
    }
  else
    {
      writeToConsole("Cannot get Lat/Lon center", Qt::red);
      return false;
    }
}

void mainWindow::openOutputPath()
{
    if( outputPath.isEmpty() || outputPath == "!set" )
    return;
    else
    QDesktopServices::openUrl( QUrl ( "file:///" + outputPath,
                      QUrl::TolerantMode ) );
}

int mainWindow::solve()
{
#ifdef NINJAFOAM
    bool useNinjaFoam = tree->ninjafoam->ninjafoamGroupBox->isChecked();
#endif
    
    //disable the open output path button
    tree->solve->openOutputPathButton->setDisabled( true );

    //dem file
    std::string demFile = inputFileName.toStdString();

    //vegetation/roughness
    int vegIndex = tree->surface->roughnessComboBox->currentIndex();
    WindNinjaInputs::eVegetation vegetation;
    if( inputFileType != LCP ) {
    //get choice from combo
    if(vegIndex == 0)
        vegetation = WindNinjaInputs::grass;
    else if( vegIndex == 1 )
        vegetation = WindNinjaInputs::brush;
    else if( vegIndex == 2 )
        vegetation = WindNinjaInputs::trees;
    }

    //mesh
    int meshIndex = tree->surface->meshResComboBox->currentIndex();
    Mesh::eMeshChoice meshChoice;
    double meshRes;
    lengthUnits::eLengthUnits meshUnits;
    bool customMesh = false;
    if( meshIndex == 0 )
    meshChoice = Mesh::coarse;
    else if( meshIndex == 1 )
    meshChoice = Mesh::medium;
    else if( meshIndex == 2 )
    meshChoice = Mesh::fine;
    else {
    meshRes = tree->surface->meshResDoubleSpinBox->value();
    customMesh = true;
    if( tree->surface->meshFeetRadioButton->isChecked() )
        meshUnits = lengthUnits::feet;
    else
        meshUnits = lengthUnits::meters;
    }
#ifdef NINJAFOAM
    WindNinjaInputs::eNinjafoamMeshChoice ninjafoamMeshChoice;
    if(useNinjaFoam){
        if( meshIndex == 0 )
            ninjafoamMeshChoice = WindNinjaInputs::coarse;
        else if( meshIndex == 1 )
            ninjafoamMeshChoice = WindNinjaInputs::medium;
        else if (meshIndex == 2)
            ninjafoamMeshChoice = WindNinjaInputs::fine;
    }
#endif

    //location

    int tzIndex = tree->surface->timeZone->tzComboBox->currentIndex();
    if(tzIndex == -1 && (tree->diurnal->diurnalGroupBox->isChecked() ||
                         tree->weather->weatherGroupBox->isChecked()
#ifdef STABILITY
                         || tree->stability->stabilityGroupBox->isChecked()
#endif
                         ))
    {
        QMessageBox::warning(this, tr("WindNinja"), tr("Could not auto-identify "
                                                      "time zone, please " 
                                                      "specify one in Surface"),
                             QMessageBox::Ok | QMessageBox::Default);
        progressDialog->setValue( 0 );
        progressDialog->cancel();
        disconnect(progressDialog, SIGNAL(canceled()), this, SLOT(cancelSolve()));
        setCursor(Qt::ArrowCursor);
        return false;
    }

    QVariant temp = tree->surface->timeZone->tzComboBox->itemData( tzIndex );
    std::string timeZone = temp.toString().toStdString();

    //diurnal
    bool useDiurnal = tree->diurnal->diurnalGroupBox->isChecked();

    //stability
#ifdef STABILITY
    bool useStability = tree->stability->stabilityGroupBox->isChecked();
#endif

    //initialization method
    WindNinjaInputs::eInitializationMethod initMethod;
    if( tree->wind->windGroupBox->isChecked() )
        initMethod = WindNinjaInputs::domainAverageInitializationFlag;
    else if( tree->point->pointGroupBox->isChecked() )
        initMethod = WindNinjaInputs::pointInitializationFlag;
    else if( tree->weather->weatherGroupBox->isChecked() )
        initMethod = WindNinjaInputs::wxModelInitializationFlag;

    //input wind height
    double inHeight = tree->wind->metaWind->inputHeightDoubleSpinBox->value();
    lengthUnits::eLengthUnits inHeightUnits;
    if(tree->wind->metaWind->feetRadioButton->isChecked())
    inHeightUnits = lengthUnits::feet;
    else
    inHeightUnits = lengthUnits::meters;

    //speed units and air temp units
    velocityUnits::eVelocityUnits inputSpeedUnits;
    if(tree->wind->windTable->inputSpeedUnits->currentIndex() == 0)
    inputSpeedUnits = velocityUnits::milesPerHour;
    else if(tree->wind->windTable->inputSpeedUnits->currentIndex() == 1)
    inputSpeedUnits = velocityUnits::metersPerSecond;
    else
    inputSpeedUnits = velocityUnits::kilometersPerHour;

    temperatureUnits::eTempUnits tempUnits;
    if(tree->wind->windTable->airTempUnits->currentIndex() == 0)
    tempUnits = temperatureUnits::F;
    else if(tree->wind->windTable->airTempUnits->currentIndex() == 1)
    tempUnits = temperatureUnits::C;

    //point initialization
    std::string pointFile = tree->point->stationFileName.toStdString();

    //model init
    std::string weatherFile;
    QModelIndex mi = tree->weather->treeView->selectionModel()->currentIndex();
    if( mi.isValid() ) {
    QFileInfo fi( tree->weather->model->fileInfo( mi ) );
    weatherFile = fi.absoluteFilePath().toStdString();
    }
    else
    weatherFile = "";

    //output height
    double outHeight = tree->output->outputHeight->outputHeightDoubleSpinBox->value();
    lengthUnits::eLengthUnits outHeightUnits;
    if(tree->output->outputHeight->feetRadioButton->isChecked())
    outHeightUnits = lengthUnits::feet;
    else
    outHeightUnits = lengthUnits::meters;

    velocityUnits::eVelocityUnits outputSpeedUnits;
    if(tree->output->outputSpeedUnitsCombo->currentIndex() == 0)
        outputSpeedUnits = velocityUnits::milesPerHour;
    else if(tree->output->outputSpeedUnitsCombo->currentIndex() == 1)
        outputSpeedUnits = velocityUnits::metersPerSecond;
    else if(tree->output->outputSpeedUnitsCombo->currentIndex() == 2)
        outputSpeedUnits = velocityUnits::kilometersPerHour;


    //clip buffer?
    int clip = tree->output->bufferSpinBox->value();

    bool writeWxOutput;
    if( tree->output->wxModelOutputCheckBox->isEnabled() )
        writeWxOutput = tree->output->wxModelOutputCheckBox->isChecked();

    //google
    bool writeGoogle = tree->google->googleGroupBox->isChecked();
    double googleRes = tree->google->googleResSpinBox->value();
    double vectorWidth = tree->google->vectorWidthDoubleSpinBox->value();
    lengthUnits::eLengthUnits googleUnits;
    KmlVector::egoogSpeedScaling googleScale;
    //bool writeLegend = tree->google->legendGroupBox->isChecked();
    if(tree->google->googleMetersRadioButton->isChecked())
    googleUnits = lengthUnits::meters;
    else
    googleUnits = lengthUnits::feet;

    if(tree->google->uniformRangeRadioButton->isChecked())
    googleScale = KmlVector::equal_interval;
    else
    googleScale = KmlVector::equal_color;

    //ascii raster fb files
    bool writeFb = tree->fb->fbGroupBox->isChecked();
    double fbRes = tree->fb->fbResSpinBox->value();
    lengthUnits::eLengthUnits fbUnits;
    if(tree->fb->fbMetersRadioButton->isChecked())
    fbUnits = lengthUnits::meters;
    else
    fbUnits = lengthUnits::feet;
    //write atmosphere file?
    bool writeAtm = tree->fb->atmFileCheckBox->isChecked();
    if(writeAtm && writeFb)
    {
        if((outHeight == 20 && outHeightUnits == lengthUnits::feet &&
            outputSpeedUnits == velocityUnits::milesPerHour) ||
           (outHeight == 10 && outHeightUnits == lengthUnits::meters &&
            outputSpeedUnits == velocityUnits::kilometersPerHour))
        {}
        else
        {
            QMessageBox::critical(this, tr("The solver cannot be run"),
                               tr("The output wind settings for atm files must "
                               "either be 10m for output height and "
                               "output speed units in kph, or "
                               "20ft for output height and "
                               "output speed units in mph."),
                               QMessageBox::Ok | QMessageBox::Default);
            progressDialog->setValue( 0 );
            progressDialog->cancel();
            disconnect(progressDialog, SIGNAL(canceled()), this, SLOT(cancelSolve()));
            setCursor(Qt::ArrowCursor);
            return false;
        }
    }

    //shape
    bool writeShape = tree->shape->shapeGroupBox->isChecked();
    double shapeRes = tree->shape->shapeResSpinBox->value();
    lengthUnits::eLengthUnits shapeUnits;
    if(tree->shape->shapeMetersRadioButton->isChecked())
    shapeUnits = lengthUnits::meters;
    else
    shapeUnits = lengthUnits::feet;

    bool writeVTK = tree->vtk->vtkGroupBox->isChecked();

    //number of processors
    int nThreads = tree->solve->numProcSpinBox->value();

    delete army;
#ifdef NINJAFOAM    
    army = new ninjaArmy(1, useNinjaFoam); // ninjafoam solver
#else
    army = new ninjaArmy(1); // native ninja solver
#endif

    //count the runs in the wind table
    if( initMethod ==  WindNinjaInputs::pointInitializationFlag )
    {
        //we can only do one run with point
        nRuns = 1;
        army->setSize( nRuns, false );
    }
    else if( initMethod == WindNinjaInputs::domainAverageInitializationFlag )
    {
        nRuns = countRuns();
        //do one run.
        if(nRuns == 0)
        {
            nRuns++;
        }
#ifdef NINJAFOAM
        army->setSize( nRuns, useNinjaFoam);
#else
        army->setSize( nRuns, false);
#endif
    }
    else if( initMethod == WindNinjaInputs::wxModelInitializationFlag )
    {
        if( !CPLCheckForFile( (char*)weatherFile.c_str(), NULL ) )
        {
             QMessageBox::critical( this, tr( "Invalid forecast file." ),
                                    tr( "The forecast file does not exist, " \
                                        "or it cannot be read." ),
                                    QMessageBox::Ok | QMessageBox::Default );
            disconnect(progressDialog, SIGNAL(canceled()), this, SLOT(cancelSolve()));
            setCursor(Qt::ArrowCursor);
            tree->weather->checkForModelData();
            progressDialog->cancel();
            return false;
        }
        /* This can throw a badForecastFile */
        try
        {
            army->makeArmy( weatherFile, timeZone );
        }
        catch( badForecastFile &e )
        {
             QMessageBox::critical( this, tr( "Invalid forecast file." ),
                                    tr( "The forecast cannot be read." ),
                                    QMessageBox::Ok | QMessageBox::Default );
            disconnect(progressDialog, SIGNAL(canceled()), this, SLOT(cancelSolve()));
            setCursor(Qt::ArrowCursor);
            tree->weather->checkForModelData();
            progressDialog->cancel();
            progressDialog->hide();
            return false;
        }
        nRuns = army->getSize();
    }

    progressDialog->setValue( 0 );
    //set progress dialog and initial value
    progressDialog->setRange(0, nRuns * 100);
    runProgress = new int[nRuns];

    //fill in the values
    for(int i = 0;i < army->getSize(); i++) 
    {

        army->setDEM( i, demFile );
        //set initialization
        if( initMethod != WindNinjaInputs::wxModelInitializationFlag )
        {
            army->setInitializationMethod( i, initMethod, true );
        }
        //set the ninjaCom
        army->setNinjaCommunication( i, i, ninjaComClass::ninjaGUICom );

        //set the input file
        //army.readInputFile( i, demFile );

        if( inputFileType != LCP )
        {
            army->setUniVegetation( i, vegetation );
        }
        if( initMethod ==  WindNinjaInputs::pointInitializationFlag )
        {
           if( NINJA_SUCCESS != army->setWxStationFilename( i, pointFile ) )
            {
                QMessageBox::critical( this, 
                                       tr("Invalid Point inititialization file" ),
                                       tr( "Invalid Point initialization file" ),
                                       QMessageBox::Ok | QMessageBox::Default );
                    disconnect(progressDialog, SIGNAL(canceled()), this, SLOT(cancelSolve()));
                    setCursor(Qt::ArrowCursor);
                    tree->weather->checkForModelData();
                    progressDialog->cancel();
                    return false;
            }
        }
        
        else if( initMethod ==  WindNinjaInputs::domainAverageInitializationFlag )
        {
            //get speed
            army->setInputSpeed( i,
                                tree->wind->windTable->speed[i]->value(),
                                inputSpeedUnits);
            //get direction
            army->setInputDirection( i, tree->wind->windTable->dir[i]->value() );
        }
        
        //set input output height
        army->setInputWindHeight ( i, inHeight, inHeightUnits );
        army->setOutputWindHeight( i, outHeight, outHeightUnits );

        //set output speed units
        army->setOutputSpeedUnits( i, outputSpeedUnits );

        //set clipping
        army->setOutputBufferClipping( i, (double) clip );

        //diurnal, if needed
        army->setDiurnalWinds( i, useDiurnal );
        if( useDiurnal == true ) 
        {
            if( initMethod == WindNinjaInputs::domainAverageInitializationFlag )
            {
            army->setDateTime( i, tree->wind->windTable->date[i]->date().year(),
                                 tree->wind->windTable->date[i]->date().month(),
                                 tree->wind->windTable->date[i]->date().day(),
                                 tree->wind->windTable->time[i]->time().hour(),
                                 tree->wind->windTable->time[i]->time().minute(),
                                 0, timeZone );
            army->setUniAirTemp( i,
                                tree->wind->windTable->airTemp[i]->value(),
                                tempUnits );
            army->setUniCloudCover( i,
                                   tree->wind->windTable->cloudCover[i]->value(),
                                   coverUnits::percent );
            army->setPosition( i, GDALCenterLat, GDALCenterLon );
            }
            else if( initMethod == WindNinjaInputs::pointInitializationFlag )
            {
                army->setDateTime( 0, tree->point->dateTimeEdit->date().year(),
                        tree->point->dateTimeEdit->date().month(),
                        tree->point->dateTimeEdit->date().day(),
                        tree->point->dateTimeEdit->time().hour(),
                        tree->point->dateTimeEdit->time().minute(),
                        0, timeZone );
                army->setPosition( i, GDALCenterLat, GDALCenterLon );
            }
            else if( initMethod == WindNinjaInputs::wxModelInitializationFlag )
            {
                army->setPosition( i );
            }
        }
        else // initMethod is wxModelInitialization or useDiurnal is false
        {
            army->setPosition( i );
        }

#ifdef STABILITY
        //stability, if needed, check for diurnal also so we don't repeat setters
        if( useStability == true && useDiurnal == false )
        {
            if( initMethod == WindNinjaInputs::domainAverageInitializationFlag )
            {
            army->setDateTime( i, tree->wind->windTable->date[i]->date().year(),
                                 tree->wind->windTable->date[i]->date().month(),
                                 tree->wind->windTable->date[i]->date().day(),
                                 tree->wind->windTable->time[i]->time().hour(),
                                 tree->wind->windTable->time[i]->time().minute(),
                                 0, timeZone );
            army->setUniAirTemp( i,
                                tree->wind->windTable->airTemp[i]->value(),
                                tempUnits );
            army->setUniCloudCover( i,
                                   tree->wind->windTable->cloudCover[i]->value(),
                                   coverUnits::percent );
            army->setPosition( i, GDALCenterLat, GDALCenterLon );
            }
            else if( initMethod == WindNinjaInputs::pointInitializationFlag )
            {
                army->setDateTime( 0, tree->point->dateTimeEdit->date().year(),
                        tree->point->dateTimeEdit->date().month(),
                        tree->point->dateTimeEdit->date().day(),
                        tree->point->dateTimeEdit->time().hour(),
                        tree->point->dateTimeEdit->time().minute(),
                        0, timeZone );
                army->setPosition( i, GDALCenterLat, GDALCenterLon );
            }
        }
        army->setStabilityFlag( i, useStability );
#endif
        //set mesh stuff
        if( customMesh )
        {
            army->setMeshResolution( i, meshRes, meshUnits );
        }
        else
        {
#ifdef NINJAFOAM
            if(useNinjaFoam)
                army->setMeshCount( i, ninjafoamMeshChoice );
            else
                army->setMeshResolutionChoice( i, meshChoice );
#else
            army->setMeshResolutionChoice( i, meshChoice );
#endif
        }

        army->setNumVertLayers( i, 20 );

        //set the input file
        //army.ninjas[i].readInputFile( demFile );
        //army->setDEM( i, demFile );
        // this is commented out?
        //army.ninjas[i].mesh.compute_domain_height();

        //set number of cpus...
        //army.setnumberCPUs(1);

        army->setGoogOutFlag     (i,writeGoogle);
        army->setGoogLineWidth   (i,vectorWidth);
        army->setGoogResolution  (i,googleRes,googleUnits);
        army->setGoogSpeedScaling(i,googleScale);
        army->setShpOutFlag      (i,writeShape); 
        army->setShpResolution   (i,shapeRes,shapeUnits);
        army->setAsciiOutFlag    (i,writeFb);    
        army->setAsciiResolution (i,fbRes,fbUnits);
        //army->setWriteAtmFile  (i,writeAtm );  
        army->setVtkOutFlag      (i,writeVTK);  
         
        if( initMethod == WindNinjaInputs::wxModelInitializationFlag &&
            writeWxOutput == true )
        {
            army->setWxModelGoogOutFlag( i, writeGoogle );
            army->setWxModelShpOutFlag( i, writeShape );
            army->setWxModelAsciiOutFlag( i, writeFb );
        }

        //army.setOutputFilenames();
        army->setNinjaComNumRuns( i, nRuns );
    }


    army->set_writeFarsiteAtmFile( writeAtm );

    for( unsigned int i = 0; i < army->getSize(); i++ )
    {
        runProgress[i] = 0;
    }

    totalProgress = 0;

    progressDialog->setLabelText("Solving...");

    /*
    connect(army.Com, SIGNAL(sendMessage(QString, QColor)),
            this, SLOT(writeToConsole(QString, QColor)),
            Qt::AutoConnection);
    */
    for( unsigned int i = 0; i < army->getSize(); i++ ) 
    {
        connect( army->getNinjaCom( i ),
                 SIGNAL( sendProgress( int, int) ), this,
                 SLOT( updateProgress( int, int ) ), Qt::AutoConnection );

        connect( army->getNinjaCom( i ),
                 SIGNAL( sendMessage(QString, QColor)),
                 this, SLOT(writeToConsole(QString, QColor ) ),
                 Qt::AutoConnection );
    }
    writeToConsole(QString::number( army->getSize() ) + " runs initialized. Starting solver...");
    //sThread->start();

    progressDialog->setValue( 0 );
    runTime->restart();
    connect( progressDialog, SIGNAL(canceled() ),
         this, SLOT( cancelSolve() ) );

    progressDialog->setCancelButtonText( "Cancel" );

    setCursor( Qt::WaitCursor );

    progressDialog->setLabelText( "Initializing runs..." );

    writeToConsole( "Initializing runs..." );

    bool ninjaSuccess = false;
    //ninjaSuccess = sThread->run( nThreads, army );
    //start the army
    try {
        ninjaSuccess = army->startRuns( nThreads );
    }
    catch (bad_alloc& e)
    {
        progressDialog->cancel();
        QMessageBox::warning(this, tr("Exception Caught"),
                             tr("WindNinja may have run out of memory. This may be caused by too fine of a mesh resolution."),
                             QMessageBox::Ok | QMessageBox::Default);

        disconnect(progressDialog, SIGNAL(canceled()), this, SLOT(cancelSolve()));
        setCursor(Qt::ArrowCursor);
        return false;
    }
    catch (cancelledByUser& e)
    {
        progressDialog->cancel();
        QMessageBox::warning(this, tr("Exception Caught"),
                             tr(e.what()),
                             QMessageBox::Ok | QMessageBox::Default);
        disconnect(progressDialog, SIGNAL(canceled()), this, SLOT(cancelSolve()));
        setCursor(Qt::ArrowCursor);
        return false;
    }
    catch (exception& e)
    {
        progressDialog->cancel();
        QMessageBox::warning(this, tr("Exception Caught"),
                             tr(e.what()),
                             QMessageBox::Ok | QMessageBox::Default);
        disconnect(progressDialog, SIGNAL(canceled()), this, SLOT(cancelSolve()));
        setCursor(Qt::ArrowCursor);
        return false;
    }
    catch (...)
    {
        progressDialog->cancel();
        QMessageBox::warning(this, tr("Exception Caught"),
                             tr("Unknown Exception"),
                             QMessageBox::Ok | QMessageBox::Default);
        disconnect(progressDialog, SIGNAL(canceled()), this, SLOT(cancelSolve()));
        setCursor(Qt::ArrowCursor);
        return false;
    }

    disconnect(progressDialog, SIGNAL(canceled()), this, SLOT(cancelSolve()));

    writeToConsole("Finished with simulations", Qt::darkGreen);
    //updateTimer();

    elapsedRunTime = runTime->elapsed() / 1000.0;

    progressDialog->setLabelText("Simulations finished");
    progressDialog->setValue(progressDialog->maximum());
    progressDialog->setCancelButtonText("Close");


     //Everything went okay? enable output path button
    tree->solve->openOutputPathButton->setEnabled( true );
    outputPath = QString::fromStdString( army->getOutputPath( 0 ) );

    //clear the army
    army->reset();

    setCursor(Qt::ArrowCursor);

    return ninjaSuccess;
}

void mainWindow::updateProgress(int run, int progress)
{
  runProgress[run] = progress;
  totalProgress = 0;

  for(int i = 0;i < nRuns;i++)
    totalProgress += runProgress[i];

  progressDialog->setValue(totalProgress);
}

int mainWindow::countRuns()
{
  int runs = 0;

  while(tree->wind->windTable->speed[runs]->value() != 0 ||
    tree->wind->windTable->dir[runs]->value() != 0)
      runs++;

  return runs;
}

int mainWindow::checkAllItems()
{
  //check and see if the objects have been visited before changing
  eInputStatus status = green;
#ifdef NINJAFOAM
  checkSolverMethodItem();
  checkMeshCombo();
#endif
  checkInputItem();
  checkOutputItem();
  checkSolveItem();

  return status;
}

#ifdef NINJAFOAM
int mainWindow::checkSolverMethodItem()
{
    eInputStatus status = blue;
    
    checkNativeSolverItem();
    checkNinjafoamItem();

    if(checkNativeSolverItem() == green)
    {
        tree->solverMethodItem->setIcon(0, tree->check);
        tree->solverMethodItem->setToolTip(0, "Using conservation of mass solver");
        checkNinjafoamItem();
        status = green;
    }
    else if(checkNinjafoamItem() == green)
    {
        tree->solverMethodItem->setIcon(0, tree->check);
        tree->solverMethodItem->setToolTip(0, "Using conservation of mass and momentum solver");
        checkNativeSolverItem();
        status = green;
    }
    else
    {
        tree->solverMethodItem->setIcon(0, tree->cross);
        tree->solverMethodItem->setToolTip(0, "Select a solver");
        status = red;
    }
    return status;
}

int mainWindow::checkNativeSolverItem()
{
    eInputStatus status = green;
    if(!tree->nativesolver->nativeSolverGroupBox->isChecked())
    {
        tree->nativeSolverItem->setIcon(0, tree->radio);
        tree->nativeSolverItem->setToolTip(0, "Conservation of Mass not selected");
        status = blue;
    }
    else
    {
        tree->nativeSolverItem->setIcon(0, tree->check);
        tree->nativeSolverItem->setToolTip(0, "Conservation of Mass selected");
        status = green;
    }

    return status;
}
int mainWindow::checkNinjafoamItem()
{
    eInputStatus status = green;
    if(!tree->ninjafoam->ninjafoamGroupBox->isChecked())
    {
        tree->ninjafoamItem->setIcon(0, tree->radio);
        tree->ninjafoamItem->setToolTip(0, "Conservation of Mass and Momentum not selected");
        status = blue;
    }
    else
    {
        tree->ninjafoamItem->setIcon(0, tree->check);        
        tree->ninjafoamItem->setToolTip(0, "Conservation of Mass and Momentum selected");
        status = green;
    }

    return status;
}
#endif //NINJAFOAM

int mainWindow::checkInputItem()
{  
  eInputStatus status = red;
  
  checkWindItem();
  
#ifdef STABILITY
  if(checkSurfaceItem() == red && checkWindItem() == red && checkDiurnalItem() == red || checkStabilityItem() == red)
    { 
      tree->inputItem->setIcon(0, tree->cross);
      tree->inputItem->setToolTip(0, "Check surface input, wind input, stability input, and diurnal input");
      status = red;
    }
#else
  if(checkSurfaceItem() == red && checkWindItem() == red && checkDiurnalItem() == red )
    { 
      tree->inputItem->setIcon(0, tree->cross);
      tree->inputItem->setToolTip(0, "Check surface input, wind input and diurnal input");
      status = red;
    }
#endif
  else if(checkSurfaceItem() == red)
    { 
      tree->inputItem->setIcon(0, tree->cross);
      tree->inputItem->setToolTip(0, "Check surface input");
      status = red;
    }
  else if(checkDiurnalItem() == red)
    { 
      tree->inputItem->setIcon(0, tree->caution);
      tree->inputItem->setToolTip(0, "Check diurnal input");
      status = red;
    }
#ifdef STABILITY
  else if(checkStabilityItem() == red)
    {
      tree->inputItem->setIcon(0, tree->caution);
      tree->inputItem->setToolTip(0, "Check stability input");
      status = red;
    }
#endif
  else if(checkWindItem() == red)
    {
      tree->inputItem->setIcon(0, tree->cross);
      tree->inputItem->setToolTip(0, "Check wind input");
      status = red;
    }
  else
    {
      tree->inputItem->setIcon(0, tree->check);
      tree->inputItem->setToolTip(0, "Valid");
      status = green;
    }
  return status;
}

int mainWindow::checkSurfaceItem()
{
  eInputStatus status = red;
  if(inputFileName == "" || !QFile::exists(inputFileName))
    {
      tree->surfaceItem->setIcon(0, tree->cross);
      tree->surfaceItem->setToolTip(0, "The input file cannot be opened");
      status = red;
    }
  else if(!hasPrj)
    {
      tree->surfaceItem->setIcon(0, tree->caution);
      tree->surfaceItem->setToolTip(0, "No projection information");
      status = red;
    }
  else
    {
      tree->surfaceItem->setIcon(0, tree->check);
      tree->surfaceItem->setToolTip(0, "Valid");
      status = green;
    }
  return status;
}

int mainWindow::checkDiurnalItem()
{ 
  eInputStatus status = green;
  if(!tree->diurnal->diurnalGroupBox->isChecked()) {
      tree->diurnalItem->setIcon(0, tree->blue);
      tree->diurnalItem->setToolTip(0, "No Diurnal Input");
      status = blue;
  }
  else
  {
        tree->diurnalItem->setIcon(0, tree->check);
        status = green;
  }
  return status;
}

#ifdef STABILITY
int mainWindow::checkStabilityItem()
{   
    eInputStatus status = green;
    if(!tree->stability->stabilityGroupBox->isChecked())
    {
        tree->stabilityItem->setIcon(0, tree->blue);
        tree->stabilityItem->setToolTip(0, "No Stability Input");
        status = blue;
    }
    else
    {
        tree->stabilityItem->setIcon(0, tree->check);
        status = green;
    }
    return status;
}
#endif

int mainWindow::checkWindItem()
{
    eInputStatus status = blue;

    //check all once, just to update icons
    checkSpdDirItem();
    checkPointItem();
    checkWeatherItem();

    if( checkSpdDirItem() == blue && checkPointItem() == blue
    && checkWeatherItem() == blue ) {
    tree->windItem->setIcon(0, tree->cross);
    tree->windItem->setToolTip(0, "No initialization selected");
    status = red;
    }
    else if( checkSpdDirItem() == red ) {
    tree->windItem->setIcon(0, tree->cross);
    tree->windItem->setToolTip(0, "Check speed and direction");
    status = red;
    }
    else if( checkSpdDirItem() == amber ) {
    tree->windItem->setIcon(0, tree->caution);
    tree->windItem->setToolTip(0, "No runs have been added, one run will be done at speed = 0, dir = 0");
    status = amber;
    }
    else if( checkPointItem() == red ) {
    tree->windItem->setIcon(0, tree->cross);
    tree->windItem->setToolTip(0, "Check point initialization");
    status = red;
    }
    else if( checkWeatherItem() == red ) {
    tree->windItem->setIcon(0, tree->cross);
    tree->windItem->setToolTip(0, "Check weather model initialization");
    status = red;
    }
    else {
    tree->windItem->setIcon(0, tree->check);
    tree->windItem->setToolTip(0, "Valid");
    status = green;
    }
    return status;
}

int mainWindow::checkSpdDirItem()
{
    int runs = countRuns();
    eInputStatus status = blue;
    if( tree->wind->windGroupBox->isChecked() ) {
    if(checkSurfaceItem() != red) {
        if(runs == 0 && tree->diurnal->diurnalGroupBox->isChecked() == false) {
        tree->spdDirItem->setIcon(0, tree->cross);
        tree->spdDirItem->setToolTip(0, "No runs have been added, diurnal is not active");
        status = red;
        }
        else if(runs == 0 && tree->diurnal->diurnalGroupBox->isChecked() == true) {
        tree->spdDirItem->setIcon(0, tree->caution);
        tree->spdDirItem->setToolTip(0, "No runs have been added, one run will be done at speed = 0, dir = 0");
        status = amber;
        }
        else {
        tree->spdDirItem->setIcon(0, tree->check);
        tree->spdDirItem->setToolTip(0, QString::number(runs) + " runs");
        status = green;
        }
    }
    else {
        tree->spdDirItem->setIcon(0, tree->cross);
        tree->spdDirItem->setToolTip(0, "Cannot read input file");
        status = red;
    }
    }
    else {
    tree->spdDirItem->setIcon(0, tree->radio);
    tree->spdDirItem->setToolTip(0, "Not used");
    status = blue;
    }
    return status;
}
int mainWindow::checkPointItem()
{
    eInputStatus status = blue;
    if( tree->point->pointGroupBox->isChecked() ) {
    QFileInfo fi( tree->point->stationFileName );
    if( !fi.exists() || !fi.isFile() ) {
        status = red;
        tree->pointItem->setIcon( 0, tree->cross );
        tree->pointItem->setToolTip( 0, "Cannot read or open point input file" );
    }
    else {
        status = green;
        tree->pointItem->setIcon( 0, tree->check );
        tree->pointItem->setToolTip( 0, "Valid" );
    }
    }
    else {
    status = blue;
    tree->pointItem->setIcon( 0, tree->radio );
    tree->pointItem->setToolTip( 0, "Not used" );
    }
    return status;
}

int mainWindow::checkWeatherItem()
{
    eInputStatus status = blue;
    wxModelInitialization* model = NULL;
    if( tree->weather->weatherGroupBox->isChecked() ) {
    QFileInfo fi;
    QModelIndex mi = tree->weather->treeView->selectionModel()->currentIndex();
    if( mi.isValid() ) {
        fi = tree->weather->model->fileInfo( mi );
        std::string filename = fi.absoluteFilePath().toStdString();
        char *p, *q;
        p = strdup( filename.c_str() );
        q = strrchr( p, '/' );
        int n = 0;
        if( !q )
            q = strrchr( p, '\\' );
        if( q )
        {
            if( strlen( q ) > 1 )
                q++;
            if( strlen( q ) > 5 )
                *(q + 4) = '\0';
            n = atoi( q );
        }
        else
            n = atoi( p );
        free( p );
        if( fi.isDir() && n < 2000 )
        {
            status = red;
            tree->modelItem->setIcon( 0, tree->cross );
            tree->modelItem->setToolTip( 0, "Forecast is invalid" );
            return status;
        }
        try {
            model = wxModelInitializationFactory::makeWxInitialization(filename);
        }
        catch( ... ) {
            status = red;
            tree->modelItem->setIcon( 0, tree->cross );
            tree->modelItem->setToolTip( 0, "Forecast is invalid" );
            delete model;
            return status;
        }

        if( !fi.exists() ) {
        status = red;
        tree->modelItem->setIcon( 0, tree->cross );
        tree->modelItem->setToolTip( 0, "Forecast does not exist" );
        }
        else {
        status = green;
        tree->modelItem->setIcon( 0, tree->check );
        tree->modelItem->setToolTip( 0, "Valid" );
        }
    }
    else {
        status = red;
        tree->modelItem->setIcon( 0, tree->cross );
        tree->modelItem->setToolTip( 0, "You must select a valid forecast file or path" );
    }
    }
    else {
    tree->modelItem->setIcon( 0, tree->radio );
    tree->modelItem->setToolTip( 0, "Not used" );
    }
    delete model;
    return status;
}

int mainWindow::checkOutputItem()
{
  eInputStatus status = green;
  if(checkSurfaceItem() == red)
    {
      tree->outputItem->setIcon(0, tree->cross);
      tree->outputItem->setToolTip(0, "Cannot read input file");
      status = red;
    }
  if(checkGoogleItem() == blue && checkFbItem() == blue && checkShapeItem() == blue && checkVtkItem() == blue)
    {
      tree->outputItem->setIcon(0, tree->cross);
      tree->outputItem->setToolTip(0, "No outputs selected");
      status = red;
    }
  if(checkGoogleItem() == amber || checkFbItem() == amber || checkShapeItem() == amber || checkVtkItem() == amber)
    {
      if(checkGoogleItem() == amber)
    {
      tree->outputItem->setIcon(0, tree->caution);
      tree->outputItem->setToolTip(0, "Check Google ouput");
      status = amber;
    }
      if(checkFbItem() == amber)
    {
      tree->outputItem->setIcon(0, tree->check);
      tree->outputItem->setToolTip(0, "Check fire behavior ouput");
      status = amber;
    }
      if(checkShapeItem() == amber)
    {
      tree->outputItem->setIcon(0, tree->check);
      tree->outputItem->setToolTip(0, "Check shape file ouput");
      status = amber;
    }
      if(checkVtkItem() == amber)
    {
      tree->outputItem->setIcon(0, tree->check);
      tree->outputItem->setToolTip(0, "Check vtk file ouput");
      status = amber;
    }
    }
  if(status == green)
    {
      tree->outputItem->setIcon(0, tree->check);
      tree->outputItem->setToolTip(0, "Valid");
      status = green;
    }
  return status;
}

int mainWindow::checkGoogleItem()
{
  eInputStatus status = red;

  if(!tree->google->googleGroupBox->isChecked())
    {
      tree->googleItem->setIcon(0, tree->blue);
      tree->googleItem->setToolTip(0, "No output");
      status = blue;
    }
  else
    {
      if(checkSurfaceItem() != red)
    {
      if((int)noGoogleCellSize > tree->google->googleResSpinBox->value())
        {
          tree->googleItem->setIcon(0, tree->caution);
          tree->googleItem->setToolTip(0, "The resolution of the google file may be too fine.");
          status = amber;
        }
      else if((int)GDALCellSize > tree->google->googleResSpinBox->value())
        {
          tree->googleItem->setIcon(0, tree->caution);
          tree->googleItem->setToolTip(0, "The output resolution is finer than the DEM resolution");
          status = amber;
        }
      /*
      else if((int)meshCellSize > tree->google->googleResSpinBox->value())
        {
          tree->googleItem->setIcon(0, tree->check);
          tree->googleItem->setToolTip(0, "The output resolutions is finer than the computational mesh");
          status = amber;
        }
      */
      else
        {
          tree->googleItem->setIcon(0, tree->check);
          tree->googleItem->setToolTip(0, "Valid");
          status = green;
        }
    }
      else
    {
      tree->googleItem->setIcon(0, tree->cross);
      tree->googleItem->setToolTip(0, "Cannot read input file");
      status = red;
    }
    }
  return status;
}

int mainWindow::checkFbItem()
{
  eInputStatus status = red;
  if(!tree->fb->fbGroupBox->isChecked())
    {
      tree->fbItem->setIcon(0, tree->blue);
      tree->fbItem->setToolTip(0, "No output");
      status = blue;
    }
  else
    {
      if(checkSurfaceItem() == green || checkSurfaceItem() == amber)
    {
      if((int)GDALCellSize > tree->fb->fbResSpinBox->value())
        {
          tree->fbItem->setIcon(0, tree->caution);
          tree->fbItem->setToolTip(0, "The output resolutions is finer than the DEM resolution");
          status = amber;
        }

      else
        {
          tree->fbItem->setIcon(0, tree->check);
          tree->fbItem->setToolTip(0, "Valid");
          status = green;
        }
    }
      else
    {
      tree->fbItem->setIcon(0, tree->cross);
      tree->fbItem->setToolTip(0, "Cannot read input file");
      status = red;
    }
    }
  return status;
}

int mainWindow::checkShapeItem()
{
  eInputStatus status = red;
  if(!tree->shape->shapeGroupBox->isChecked())
    {
      tree->shapeItem->setIcon(0, tree->blue);
      tree->shapeItem->setToolTip(0, "No output");
      status = blue;
    }
  else
    {
      if(checkSurfaceItem() == green || checkSurfaceItem() == amber)
    {
      /*
      if((int)meshCellSize > tree->shape->shapeResSpinBox->value())
        {
          tree->shapeItem->setIcon(0, tree->check);
          tree->shapeItem->setToolTip(0, "The output resolutions is finer than the computational mesh");
          status = amber;
        }
      */
      if((int)GDALCellSize > tree->shape->shapeResSpinBox->value())
        {
          tree->shapeItem->setIcon(0, tree->caution);
          tree->shapeItem->setToolTip(0, "The output resolutions is finer than the DEM resolution");
          status = amber;
        }
      else
        {
          tree->shapeItem->setIcon(0, tree->check);
          tree->shapeItem->setToolTip(0, "Valid");
          status = green;
        }
    }
      else
    {
      tree->shapeItem->setIcon(0, tree->cross);
      tree->shapeItem->setToolTip(0, "Cannot read input file") ;
      status = red;
    }
    }
  return status;
}

int mainWindow::checkVtkItem()
{
  eInputStatus status = red;
  if(!tree->vtk->vtkGroupBox->isChecked())
    {
      tree->vtkItem->setIcon(0, tree->blue);
      tree->vtkItem->setToolTip(0, "No output");
      status = blue;
    }
  else
    {
      if(checkSurfaceItem() == green || checkSurfaceItem() == amber)
    {
      tree->vtkItem->setIcon(0, tree->check);
      tree->vtkItem->setToolTip(0, "Valid");
      status = green;
    }
      else
    {
      tree->vtkItem->setIcon(0, tree->cross);
      tree->vtkItem->setToolTip(0, "Cannot read input file") ;
      status = red;
    }
    }
  return status;
}

int mainWindow::checkSolveItem()
{
  eInputStatus status = red;
  if(checkInputItem() != red && checkOutputItem() != red)
    {
      tree->solveItem->setIcon(0, tree->check);
      tree->solveItem->setToolTip(0, "You may start the solver");
      tree->solve->solveToolButton->setEnabled(true);
      status = green;
    }
  //handle if nothing is checked for initialization
  else if( !tree->wind->windGroupBox->isChecked() &&
       !tree->point->pointGroupBox->isChecked() &&
       !tree->weather->weatherGroupBox->isChecked () ) {
      tree->solveItem->setIcon(0, tree->cross);
      tree->solveItem->setToolTip(0, "You must select and initialization method");
      tree->solve->solveToolButton->setEnabled(false);
      status = red;
  }
  else
    {
      tree->solveItem->setIcon(0, tree->cross);
      tree->solveItem->setToolTip(0, "There are errors in the inputs or outputs");
      tree->solve->solveToolButton->setEnabled(false);
      status = red;
    }
  return status;
}

int mainWindow::checkKmlLimit(double xx)
{
  eInputStatus status = amber;
  if((int)noGoogleCellSize > tree->google->googleResSpinBox->value())
    {
      writeToConsole("The resolution of the google file may be too fine.", orange);
      status = amber;
    }
  return amber;
}

void mainWindow::cancelSolve()
{
  progressDialog->setAutoClose(true);
  progressDialog->setLabelText("Canceling...");
  army->cancel();  
}

void mainWindow::treeDoubleClick(QTreeWidgetItem *item, int column)
{
  if(item == tree->surfaceItem)
    openInputFile();
  else if(item == tree->diurnalItem)
    {
      if(tree->diurnal->diurnalGroupBox->isChecked())
    tree->diurnal->diurnalGroupBox->setChecked(false);
      else
    tree->diurnal->diurnalGroupBox->setChecked(true);
    }
#ifdef NINJAFOAM
  else if(item == tree->nativeSolverItem)
  {
      if(tree->nativesolver->nativeSolverGroupBox->isChecked())
          tree->nativesolver->nativeSolverGroupBox->setChecked(false);
      else{
          tree->nativesolver->nativeSolverGroupBox->setChecked(true);
      }
  }
  else if(item == tree->ninjafoamItem)
  {
      if(tree->ninjafoam->ninjafoamGroupBox->isChecked())
          tree->ninjafoam->ninjafoamGroupBox->setChecked(false);
      else{
          tree->ninjafoam->ninjafoamGroupBox->setChecked(true);
      }
  }
#endif
#ifdef STABILITY
  else if(item == tree->stabilityItem)
  {
      if(tree->stability->stabilityGroupBox->isChecked())
          tree->stability->stabilityGroupBox->setChecked(false);
      else
          tree->stability->stabilityGroupBox->setChecked(true);
  }
#endif
  else if( item == tree->spdDirItem ) {
      if( tree->wind->windGroupBox->isChecked() )
      tree->wind->windGroupBox->setChecked( false );
      else
      tree->wind->windGroupBox->setChecked( true );
  }
  else if( item == tree->pointItem ) {
      if( tree->point->pointGroupBox->isChecked() )
      tree->point->pointGroupBox->setChecked( false );
      else
      tree->point->pointGroupBox->setChecked( true );
  }
  else if( item == tree->modelItem ) {
      if( tree->weather->weatherGroupBox->isChecked() )
      tree->weather->weatherGroupBox->setChecked( false );
      else
      tree->weather->weatherGroupBox->setChecked( true );
  }
  else if(item == tree->googleItem)
    {
      if(tree->google->googleGroupBox->isChecked())
    tree->google->googleGroupBox->setChecked(false);
      else
    tree->google->googleGroupBox->setChecked(true);
    }
  else if(item == tree->fbItem)
    {
      if(tree->fb->fbGroupBox->isChecked())
    tree->fb->fbGroupBox->setChecked(false);
      else
    tree->fb->fbGroupBox->setChecked(true);
    }
  else if(item == tree->shapeItem)
    {
      if(tree->shape->shapeGroupBox->isChecked())
    tree->shape->shapeGroupBox->setChecked(false);
      else
    tree->shape->shapeGroupBox->setChecked(true);
    }
  else if(item == tree->vtkItem)
    {
      if(tree->vtk->vtkGroupBox->isChecked())
    tree->vtk->vtkGroupBox->setChecked(false);
      else
    tree->vtk->vtkGroupBox->setChecked(true);
    }
  checkAllItems();
  return;
}

/**
 * \brief Check the input dem for no data and optionally fill
 *
 * Note that if we fail to fix things, we have to close the ds.
 *
 * \return
 *
 */
QString mainWindow::checkForNoData( QString inputFile )
{
    QString fileName;
    QString newFile = inputFile;
    GDALDataset *poDS = (GDALDataset*)GDALOpen( inputFile.toStdString().c_str(),
                                                GA_ReadOnly );
    if( GDALHasNoData( poDS, 1 ) )
    {
        int r = QMessageBox::warning( this, tr ("WindNinja" ),
                     tr( "The input dataset contains pixels with no data. "
                         "These datasets cannot be used by WindNinja, "
                         "would you like to attempt to fill those pixels?" ),
                     QMessageBox::Yes |
                     QMessageBox::No |
                     QMessageBox::Cancel);
        if( r == QMessageBox::Yes )
        {
            fileName = QFileDialog::getSaveFileName( this,
                          tr( "Open Elevation Input File" ),
                          inputFileDir.absolutePath(),
                          tr( "GeoTiff (*.tif)" ) );
            if( fileName.isEmpty() )
                return QString("");

            if( !fileName.endsWith( ".tif", Qt::CaseInsensitive ) )
                fileName += ".tif";
            GDALDriverH hDriver = GDALGetDriverByName( "GTiff" );
            GDALDatasetH hNewDS;
            hNewDS = GDALCreateCopy( hDriver, fileName.toStdString().c_str(),
                                     (GDALDriverH)poDS, FALSE, NULL, NULL,
                                     NULL );
            GDALRasterBandH hSrcBand, hDstBand;
            hSrcBand = GDALGetRasterBand( (GDALDatasetH)poDS, 1 );
            hDstBand = GDALGetRasterBand( hNewDS, 1 );
            int nSuccess;
            GDALSetRasterNoDataValue( hDstBand,
                    GDALGetRasterNoDataValue( hSrcBand, &nSuccess ) );
            int nNoData = GDALFillBandNoData( (GDALDataset*)hNewDS, 1, 100 );
            if( nNoData )
            {
                QMessageBox::warning( this, tr ("WindNinja" ),
                   tr( "Could not fill no data pixels, too many pixels "
                       "were invalid." ),
                   QMessageBox::Ok );
                GDALClose( hNewDS );
                GDALClose( (GDALDatasetH)poDS );
                VSIUnlink( fileName.toStdString().c_str() );
                newFile = "";
            }
            else
            {
                GDALFlushCache( hNewDS );
                GDALClose( hNewDS );
                GDALClose( (GDALDatasetH)poDS );
                newFile = fileName;
            }
        }
    }
    else
        GDALClose( (GDALDatasetH)poDS );
    return newFile;
}

void mainWindow::enablePointDate(bool enable)
{
    (void)enable;
    if( tree->point->pointGroupBox->isChecked() )
    {
        if( tree->diurnal->diurnalGroupBox->isChecked()
#ifdef STABILITY
            || tree->stability->stabilityGroupBox->isChecked()
#endif
          )
        {
            tree->point->dateTimeEdit->setEnabled( true );
        }
        else
        {
            tree->point->dateTimeEdit->setEnabled( false );
        }
    }
}

#ifdef NINJAFOAM
void mainWindow::enableNinjafoamOptions(bool enable)
{
    (void)enable;
    if( tree->ninjafoam->ninjafoamGroupBox->isChecked() )
    {
        //tree->diurnal->diurnalGroupBox->setCheckable( false );
        //tree->diurnal->diurnalGroupBox->setChecked( false );
        //tree->diurnal->diurnalGroupBox->setHidden( true );
        //tree->diurnal->ninjafoamConflictLabel->setHidden( false );

        #ifdef STABILITY
        tree->stability->stabilityGroupBox->setCheckable( false );
        tree->stability->stabilityGroupBox->setChecked( false );
        tree->stability->stabilityGroupBox->setHidden( true );
        tree->stability->ninjafoamConflictLabel->setHidden( false );
        #endif
         
        tree->wind->windTable->enableDiurnalCells( false ); 
        
        tree->point->pointGroupBox->setCheckable( false );
        tree->point->pointGroupBox->setChecked( false );
        tree->point->pointGroupBox->setHidden( true );
        tree->point->ninjafoamConflictLabel->setHidden( false );
        
        tree->weather->weatherGroupBox->setCheckable( false );
        tree->weather->weatherGroupBox->setChecked( false );
        tree->weather->weatherGroupBox->setHidden( true );
        tree->weather->ninjafoamConflictLabel->setHidden( false );
        
        tree->surface->timeZoneGroupBox->setHidden( true );
        tree->surface->meshResComboBox->removeItem(4);
        tree->surface->ninjafoamConflictLabel->setHidden( false );
        
        tree->vtk->ninjafoamConflictLabel->setHidden( false );
        tree->vtk->vtkLabel->setHidden( true );
        tree->vtk->vtkWarningLabel->setHidden( true );
        
    }
    else{
        tree->diurnal->diurnalGroupBox->setCheckable( true );
        tree->diurnal->diurnalGroupBox->setChecked( false );
        tree->diurnal->diurnalGroupBox->setHidden( false );
        tree->diurnal->ninjafoamConflictLabel->setHidden( true );
        
        #ifdef STABILITY
        tree->stability->stabilityGroupBox->setCheckable( true );
        tree->stability->stabilityGroupBox->setChecked( false );
        tree->stability->stabilityGroupBox->setHidden( false );
        tree->stability->ninjafoamConflictLabel->setHidden( true );
        #endif
        
        tree->point->pointGroupBox->setCheckable( true );
        tree->point->pointGroupBox->setChecked( false );
        tree->point->pointGroupBox->setHidden( false );
        tree->point->ninjafoamConflictLabel->setHidden( true );
        
        tree->weather->weatherGroupBox->setCheckable( true );
        tree->weather->weatherGroupBox->setChecked( false );
        tree->weather->weatherGroupBox->setHidden( false );
        tree->weather->ninjafoamConflictLabel->setHidden( true );
        
        tree->surface->timeZoneGroupBox->setHidden( false );
        tree->surface->meshResComboBox->addItem("Custom", 4);
        tree->surface->ninjafoamConflictLabel->setHidden( true );
        
        tree->vtk->ninjafoamConflictLabel->setHidden( true );
        tree->vtk->vtkLabel->setHidden( false );
        tree->vtk->vtkWarningLabel->setHidden( false );
        
    }
}
#endif

void mainWindow::SetConfigOption()
{
    QString key, val;
    int rc;
    SetConfigDialog dialog;
    rc = dialog.exec();
    if( rc == QDialog::Rejected )
        return;
    const char *pszKey, *pszVal;
    key = dialog.GetKey();
    val = dialog.GetVal();
    if( key == "" )
        return;
    if( val == "" )
        pszVal = NULL;
    else
        pszVal = CPLSPrintf( "%s", (char*)val.toLocal8Bit().data() );
    qDebug() << "Setting config option " << key << "to" << val;
    pszKey = CPLSPrintf( "%s", (char*)key.toLocal8Bit().data() );
    CPLSetConfigOption( pszKey, pszVal );
}

