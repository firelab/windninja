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

    stationFileLineEdit = new QLineEdit( this );
    stationFileLineEdit->setReadOnly( true );

    dateTimeEdit = new QDateTimeEdit( this );
    dateTimeEdit->setDateTime( QDateTime::currentDateTime() );
    dateTimeEdit->setCalendarPopup( true );
    dateTimeEdit->setDisplayFormat( "MM/dd/yyyy HH:mm" );
    dateTimeEdit->setEnabled( false );

    readStationFileButton =  new QToolButton( this );
    readStationFileButton->setText( tr( "Read Station File" ) );
    readStationFileButton->setIcon( QIcon( ":weather_cloudy.png" ) );
    readStationFileButton->setToolButtonStyle( Qt::ToolButtonTextBesideIcon );

    writeStationFileButton =  new QToolButton( this );
    writeStationFileButton->setText( tr( "Write Station File" ) );
    writeStationFileButton->setIcon( QIcon( ":weather_cloudy.png" ) );
    writeStationFileButton->setToolButtonStyle( Qt::ToolButtonTextBesideIcon );

    writeStationKmlButton =  new QToolButton( this );
    writeStationKmlButton->setText( tr( "Write Station Kml" ) );
    writeStationKmlButton->setIcon( QIcon( ":weather_cloudy.png" ) );
    writeStationKmlButton->setToolButtonStyle( Qt::ToolButtonTextBesideIcon );

    stationTreeView = new QTreeView( this );
    stationTreeView->setModel( &pointData );

    connect( readStationFileButton, SIGNAL( clicked() ), this,
         SLOT( readStationFile() ) );
    connect( writeStationFileButton, SIGNAL( clicked() ), this,
         SLOT( writeStationFile() ) );
    connect( writeStationKmlButton, SIGNAL( clicked() ), this,
         SLOT( writeStationKml() ) );

    fileLayout = new QHBoxLayout;
    fileLayout->addWidget( stationFileLineEdit );
    fileLayout->addWidget( readStationFileButton );

    buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget( writeStationFileButton );
    writeStationFileButton->setVisible( false );
    buttonLayout->addWidget( writeStationKmlButton );
    buttonLayout->addStretch();

    pointLayout = new QVBoxLayout;
    pointLayout->addWidget( stationTreeView );
    stationTreeView->setVisible( false );
    pointLayout->addLayout( fileLayout );
    pointLayout->addWidget( dateTimeEdit );
    pointLayout->addStretch();
    pointLayout->addLayout( buttonLayout );

    pointGroupBox->setLayout( pointLayout );
    
    ninjafoamConflictLabel = new QLabel(tr("The point initialization option is not currently available for the momentum solver.\n"
        ), this);
    ninjafoamConflictLabel->setHidden(true);

    layout = new QVBoxLayout;
    layout->addWidget( pointGroupBox );
    layout->addWidget(ninjafoamConflictLabel);
    //layout->addStretch();

    setLayout( layout );

    stationFileName = "";
}

pointInput::~pointInput()
{

}

void pointInput::readStationFile()
{
    std::vector<wxStation>readStations;
    pointData.stations.clear();

    QString fileName;
    fileName = QFileDialog::getOpenFileName(this, tr("Open station file"),
                                             QFileInfo(stationFileName).path(),
                                             tr("Comma separated value files (*.csv)"));

    if(fileName.isEmpty() || demFileName.isEmpty()) {
        return;
    }
    else {
        try {
            readStations = wxStation::readStationFile(fileName.toStdString(),
                                                      demFileName.toStdString());
        }
        catch(std::domain_error &e) {
            QMessageBox::warning(this, tr("WindNinja"), tr(e.what()),
                                 QMessageBox::Ok);
            stationFileLineEdit->clear();
            return;
        }
        catch( std::runtime_error &e ) {
            QMessageBox::warning(this, tr("WindNinja"), tr(e.what()),
                                 QMessageBox::Ok);
            stationFileLineEdit->clear();
            return;
        }
    }
    if(!readStations.empty()) {
    for(unsigned int i = 0;i < readStations.size();i++)
        pointData.stations.push_back(readStations[i]);
    }
    stationFileName = fileName;
    stationFileLineEdit->setText(QFileInfo(fileName).fileName());
    emit stationFileChanged();
    pointData.update();
}

void pointInput::writeStationFile()
{
    if( pointData.stations.empty() ) {
    writeToConsole( "There are no stations to write" );
    return;
    }

    QString fileName;
    fileName = QFileDialog::getSaveFileName( this,
                         tr( "Save station file" ),
                         ".csv",
                         tr( "Comma separated "	\
                         "value files (*.csv)" ) );
    //check for extension, make case insensitive test
    if( QFileInfo( fileName ).suffix().compare( "csv", Qt::CaseInsensitive ) ) {
    fileName += ".csv";
    if( QFileInfo( fileName ).exists() ) {
        int r = QMessageBox::warning( this, "WindNinja",
                      "The file " + fileName +
                          " exists, do you wish to" \
                          " overwrite it?",
                      QMessageBox::Yes |
                      QMessageBox::No |
                      QMessageBox::Cancel );
        if( r == QMessageBox::No || r == QMessageBox::Cancel )
        return;
    }
    }

    if( fileName.isEmpty() || fileName == ".csv" )
    return;
    else
    wxStation::writeStationFile( pointData.stations,
                     fileName.toStdString() );
}

void pointInput::writeStationKml()
{
    if( pointData.stations.empty() ) {
    writeToConsole( "There are no stations to write" );
    return;
    }

    QString fileName;
    fileName = QFileDialog::getSaveFileName( this,
                         tr( "Save station file" ),
                         ".kml",
                         tr( "Keyhole Markup  "	\
                         "files (*.kml)" ) );
    //check for extension, make case insensitive test
    if( QFileInfo( fileName ).suffix().compare( "kml", Qt::CaseInsensitive ) ) {
    fileName += ".kml";
    if( QFileInfo( fileName ).exists() ) {
        int r = QMessageBox::warning( this, "WindNinja",
                      "The file " + fileName +
                          " exists, do you wish to" \
                          " overwrite it?",
                      QMessageBox::Yes |
                      QMessageBox::No |
                      QMessageBox::Cancel );
        if( r == QMessageBox::No || r == QMessageBox::Cancel )
        return;
    }
    }

    if( fileName.isEmpty() || fileName == ".kml" )
    return;
    else
    wxStation::writeKmlFile( pointData.stations,
                    fileName.toStdString() );
}

void pointInput::setInputFile( QString file )
{
    demFileName = file;
}

void pointInput::updateTable()
{

}
