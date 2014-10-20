/******************************************************************************
 *
 * $Id$
 * 
 * Project:  WindNinja Qt GUI
 * Purpose:  Handle lat/lon dd:mm:ss/dd:mm.mm/dd.dd conversions for diurnal
 *           inputs
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

#include "latLonWidget.h"
dmsWidget::dmsWidget( QWidget *parent ) : QWidget( parent )
{
  
    //latitude
    latGroupBox = new QGroupBox( tr( "Latitude" ), this );
  
    latDegrees = new QSpinBox;
    latDegrees->setRange( -90, 90 );
    latDegrees->setValue( 0 );
    latDegrees->setSuffix( "\x00B0" );
    latDegrees->setAccelerated( true );

    latMinutes = new QSpinBox;
    latMinutes->setRange( 0, 60 );
    latMinutes->setValue( 0 );
    latMinutes->setSuffix( "'" );
    latMinutes->setAccelerated( true );

    latSeconds = new QDoubleSpinBox;
    latSeconds->setDecimals( 3 );
    latSeconds->setRange( 0, 60 );
    latSeconds->setValue( 0 );
    latSeconds->setSuffix( "\"" );
    latSeconds->setAccelerated( true );

    latLayout = new QHBoxLayout;
    latLayout->addWidget( latDegrees );
    latLayout->addWidget( latMinutes );
    latLayout->addWidget( latSeconds );

    latGroupBox->setLayout( latLayout );

    //longitude
    lonGroupBox = new QGroupBox( tr( "Longitude" ) );
  
    lonDegrees = new QSpinBox;
    lonDegrees->setRange( -180, 180 );
    lonDegrees->setValue( 0 );
    lonDegrees->setSuffix( "\x00B0" );
    lonDegrees->setAccelerated( true );

    lonMinutes = new QSpinBox;
    lonMinutes->setRange( 0, 60 );
    lonMinutes->setValue( 0 );
    lonMinutes->setSuffix( "'" );
    lonMinutes->setAccelerated( true );

    lonSeconds = new QDoubleSpinBox;
    lonSeconds->setDecimals( 3 );
    lonSeconds->setRange( 0, 60 );
    lonSeconds->setValue( 0 );
    lonSeconds->setSuffix( "\"" );
    lonSeconds->setAccelerated( true );

    lonLayout = new QHBoxLayout;
    lonLayout->addWidget( lonDegrees );
    lonLayout->addWidget( lonMinutes );
    lonLayout->addWidget( lonSeconds );

    lonGroupBox->setLayout( lonLayout );
    
    layout =new QHBoxLayout;
    layout->addWidget( latGroupBox );
    layout->addWidget( lonGroupBox );
  
    setLayout( layout );
}

dmWidget::dmWidget( QWidget *parent ) : QWidget( parent )
{
    //latitude
    latGroupBox = new QGroupBox( tr( "Latitude" ) );
  
    latDegrees = new QSpinBox;
    latDegrees->setRange( -90, 90 );
    latDegrees->setValue( 0 );
    latDegrees->setSuffix( "\x00B0" );
    latDegrees->setAccelerated( true );

    latMinutes = new QDoubleSpinBox;
    latMinutes->setDecimals( 5 );
    latMinutes->setRange( 0, 60 );
    latMinutes->setValue( 0 );
    latMinutes->setSuffix( "'" );
    latMinutes->setAccelerated( true );

    latLayout = new QHBoxLayout;
    latLayout->addWidget( latDegrees );
    latLayout->addWidget( latMinutes );
  
    latGroupBox->setLayout( latLayout );

    //longitude
    lonGroupBox = new QGroupBox( tr( "Longitude" ) );
  
    lonDegrees = new QSpinBox;
    lonDegrees->setRange( -180, 180 );
    lonDegrees->setValue( 0 );
    lonDegrees->setSuffix( "\x00B0" );
    lonDegrees->setAccelerated( true );

    lonMinutes = new QDoubleSpinBox;
    lonMinutes->setDecimals( 5 );
    lonMinutes->setRange( 0, 60 );
    lonMinutes->setValue( 0 );
    lonMinutes->setSuffix( "'" );
    lonMinutes->setAccelerated( true );
  
    lonLayout = new QHBoxLayout;
    lonLayout->addWidget( lonDegrees );
    lonLayout->addWidget( lonMinutes );

    lonGroupBox->setLayout( lonLayout );  

    layout = new QHBoxLayout;
    layout->addWidget( latGroupBox );
    layout->addWidget( lonGroupBox );
  
    setLayout( layout );
}

ddWidget::ddWidget( QWidget *parent ) : QWidget( parent )
{
    //latitude
    latGroupBox = new QGroupBox( tr( "Latitude" ) );
 
    latDegrees = new QDoubleSpinBox;
    latDegrees->setDecimals( 10 );
    latDegrees->setRange( -90.0, 90.0 );
    latDegrees->setValue( 0 ); 
    latDegrees->setSuffix( "\x00B0" );
    latDegrees->setAccelerated( true );

    latLayout = new QHBoxLayout;
    latLayout->addWidget( latDegrees );

    latGroupBox->setLayout( latLayout );

    //longitude
    lonGroupBox = new QGroupBox( tr( "Longitude" ) );

    lonDegrees = new QDoubleSpinBox;
    lonDegrees->setDecimals( 10 );
    lonDegrees->setRange( -180.0, 180.0 );
    lonDegrees->setValue( 0 );
    lonDegrees->setSuffix( "\x00B0" );
    lonDegrees->setAccelerated( true );

    lonLayout = new QHBoxLayout;
    lonLayout->addWidget( lonDegrees );

    lonGroupBox->setLayout( lonLayout );

    layout = new QHBoxLayout;
    layout->addWidget( latGroupBox );
    layout->addWidget( lonGroupBox );

    setLayout( layout );
}

latLonWidget::latLonWidget( QString title, QWidget *parent ) : QWidget( parent )
{
    latitude = longitude = 0;
    
    latLonGroupBox = new QGroupBox(  title  );

    latLonFormatGroupBox = new QGroupBox(  tr(  "Lat/Lon Format"  )  );

    dmsRadio = new QRadioButton( tr( "DD MM SS.SS" ) );
    dmsRadio->setChecked( true );
    dmRadio = new QRadioButton( tr( "DD MM.MM" ) );
    ddRadio = new QRadioButton( tr( "DD.DD" ) );
    
    stackedWidget = new QStackedWidget(  this  );
    dms = new dmsWidget;
    dm = new dmWidget;
    dd = new ddWidget;

    stackedWidget->addWidget( dms );
    stackedWidget->addWidget( dm );
    stackedWidget->addWidget( dd );

    connect( dmsRadio, SIGNAL( toggled( bool ) ), 
         this, SLOT( checkLatLonFormat( ) ) );
    connect( dmRadio, SIGNAL( toggled( bool ) ),
         this, SLOT( checkLatLonFormat( ) ) );
    connect( ddRadio, SIGNAL( toggled( bool ) ), 
         this, SLOT( checkLatLonFormat( ) ) );

    //connect all location spin boxes to the update(  ) methods
    //dms lat
    connect( dms->latDegrees, SIGNAL( editingFinished(  ) ),
	     this, SLOT( updateLatLon(  ) ) );
    connect( dms->latMinutes, SIGNAL( editingFinished(  ) ),
	     this, SLOT( updateLatLon(  ) ) );
    connect( dms->latSeconds, SIGNAL( editingFinished(  ) ),
	     this, SLOT( updateLatLon(  ) ) );
    //dms lon
    connect( dms->lonDegrees, SIGNAL( editingFinished(  ) ),
	     this, SLOT( updateLatLon(  ) ) );
    connect( dms->lonMinutes, SIGNAL( editingFinished(  ) ),
	     this, SLOT( updateLatLon(  ) ) );
    connect( dms->lonSeconds, SIGNAL( editingFinished(  ) ),
	     this, SLOT( updateLatLon(  ) ) );
    //dm lat
    connect( dm->latDegrees, SIGNAL( editingFinished(  ) ),
	     this, SLOT( updateLatLon(  ) ) );
    connect( dm->latMinutes, SIGNAL( editingFinished(  ) ),
	     this, SLOT( updateLatLon(  ) ) );
    //dm lon
    connect( dm->lonDegrees, SIGNAL( editingFinished(  ) ),
	     this, SLOT( updateLatLon(  ) ) );
    connect( dm->lonMinutes, SIGNAL( editingFinished(  ) ),
	     this, SLOT( updateLatLon(  ) ) );
 
    //dd lat
    connect( dd->latDegrees, SIGNAL( editingFinished(  ) ),
	     this, SLOT( updateLatLon(  ) ) );
    //dd lon
    connect( dd->lonDegrees, SIGNAL( editingFinished(  ) ),
	     this, SLOT( updateLatLon(  ) ) );
  
    datumComboBox = new QComboBox(  this  );
    datumComboBox->addItem(  "Datum..."  );
    datumComboBox->addItem(  "WGS84"  );
    datumComboBox->addItem(  "NAD83"  );
    datumComboBox->addItem(  "NAD27"  );

    latLonFormatLayout = new QHBoxLayout;
    latLonFormatLayout->addWidget( dmsRadio );
    latLonFormatLayout->addWidget( dmRadio );
    latLonFormatLayout->addWidget( ddRadio );
    
    latLonFormatGroupBox->setLayout( latLonFormatLayout );

    latLonLayout = new QVBoxLayout;
    latLonLayout->addWidget( latLonFormatGroupBox );
    latLonLayout->addWidget( stackedWidget );
    latLonLayout->addWidget( datumComboBox );
    latLonGroupBox->setLayout( latLonLayout );
    
    layout = new QVBoxLayout;
    layout->addWidget( latLonGroupBox );
    setLayout( layout );
}

void latLonWidget::checkLatLonFormat()
{
    int index = -1;
    if( dmsRadio->isChecked(  ) )
	index = 0;
    else if( dmRadio->isChecked(  ) )
	index = 1;
    else if( ddRadio->isChecked(  ) )
	index = 2;
    else
	index = 0;

    stackedWidget->setCurrentIndex( index );
}

void latLonWidget::updateLatLon(  )
{
    //check lat/lon format
    if( dmsRadio->isChecked(  ) ) {
	latitude = dms2ddLatitude( dms->latDegrees->value(  ),
				   dms->latMinutes->value(  ),
				   dms->latSeconds->value(  ) );
	longitude = dms2ddLongitude( dms->lonDegrees->value(  ),
				     dms->lonMinutes->value(  ),
				     dms->lonSeconds->value(  ) );
	updateLatLonWidget( 0 );
    }
    else if( dmRadio->isChecked(  ) ) {
	latitude = dm2ddLatitude( dm->latDegrees->value(  ),
				  dm->latMinutes->value(  ) );
	longitude = dm2ddLongitude( dm->lonDegrees->value(  ),
				    dm->lonMinutes->value(  ) );
	updateLatLonWidget( 1 );
    }
    else if( ddRadio->isChecked(  ) ) {
	latitude = dd->latDegrees->value(  );
	longitude = dd->lonDegrees->value(  );
	updateLatLonWidget( 2 );
    }
    //latLonFoundLabel->setText( "" );
}

double latLonWidget::dms2ddLatitude( int dd, int mm, double ss )
{
    double x, xx, lat = 0.0;
    double d = dd;
    double m = mm;
    double s = ss;
    if( d >= 0 ){
	xx = s / 60.0;
	m += xx;
	x = m / 60.0;
	lat = d + x;
    }
    else {
	xx = s / 60.0;
	m += xx;
	x = m / 60.0;
	lat = d - x;
    }
    return lat;
}

double latLonWidget::dms2ddLongitude( int dd, int mm, double ss )
{
    double d = dd;
    double m = mm;
    double s = ss;
    double x, xx, lon = 0.0;
    if( d >= 0 ) {
	xx = s / 60.0;
	m += xx;
	x = m / 60.0;
	lon = d + x;
    }
    else {
	xx = s / 60.0;
	m += xx;
	x = m / 60.0;
	lon = d - x;
    }
    return lon;
}

double latLonWidget::dm2ddLatitude( int dd, double mm )
{
    double x, lat = 0.0;
    double d = dd;
    double m = mm;
  
    if( d >= 0.0 ) {
	x = m / 60.0;
	lat = d + x;
    }
    else {
	x = m / 60.0;
	lat = d - x;
    }
    return lat;
}
  
double latLonWidget::dm2ddLongitude( int dd, double mm )
{
    double x, lon = 0.0;
    double d = dd;
    double m = mm;

    if( d >= 0.0 ) {
	x = m / 60.0;
	lon = d + x;
    }
    else {
	x = m / 60.0;
	lon = d - x;
    }
    return lon;
}

void latLonWidget::updateLatLonWidget( int notMe )
{
    if( notMe == 0 ) {
	dm->latDegrees->setValue( ( int )latitude );
	dm->latMinutes->setValue( ( fabs( latitude ) - ( int )fabs( latitude ) ) * 60 );
	
	dm->lonDegrees->setValue( ( int )longitude );
	dm->lonMinutes->setValue( ( fabs( longitude ) - ( int )fabs( longitude ) ) * 60 );
	
	dd->latDegrees->setValue( latitude );
	dd->lonDegrees->setValue( longitude );
    }
    else if( notMe == 1 ) {
	double d, m, s;
	d = ( int )latitude;
	m = ( fabs( latitude ) - fabs( d ) ) * 60;
	s = ( m - ( int )m ) * 60;
	m = ( int )m;
	
	dms->latDegrees->setValue( d );
	dms->latMinutes->setValue( m );
	dms->latSeconds->setValue( s );
	
	d = ( int )longitude;
	m = ( fabs( longitude ) - fabs( d ) ) * 60;
	s = ( m - ( int )m ) * 60;
	m = ( int )m;
	
	
	dms->lonDegrees->setValue( d );
	dms->lonMinutes->setValue( m );
	dms->lonSeconds->setValue( s );
	
	dd->latDegrees->setValue( latitude );
	dd->lonDegrees->setValue( longitude );
    }
    else if( notMe == 2 )	{
	dm->latDegrees->setValue( ( int )latitude );
	dm->latMinutes->setValue( ( fabs( latitude ) - ( int )fabs( latitude ) ) * 60 );
	
	dm->lonDegrees->setValue( ( int )longitude );
	dm->lonMinutes->setValue( ( fabs( longitude ) - ( int )fabs( longitude ) ) * 60 );
	
	double d, m, s;
	d = ( int )latitude;
	m = ( fabs( latitude ) - fabs( d ) ) * 60;
	s = ( m - ( int )m ) * 60;
	m = ( int )m;
	
	dms->latDegrees->setValue( d );
	dms->latMinutes->setValue( m );
	dms->latSeconds->setValue( s );
	
	d = ( int )longitude;
	m = ( fabs( longitude ) - fabs( d ) ) * 60;
	s = ( m - ( int )m ) * 60;
	m = ( int )m;
	
	dms->lonDegrees->setValue( d );
	dms->lonMinutes->setValue( m );
	dms->lonSeconds->setValue( s );
    }
    else {
	//update all from internal lat/lon
	//degrees/minutes/seconds
	double d, m, s;
	d = ( int )latitude;
	m = ( fabs( latitude ) - fabs( d ) ) * 60;
	s = ( m - ( int )m ) * 60;
	m = ( int )m;
	
	dms->latDegrees->setValue( d );
	dms->latMinutes->setValue( m );
	dms->latSeconds->setValue( s );
	
	d = ( int )longitude;
	m = ( fabs( longitude ) - fabs( d ) ) * 60;
	s = ( m - ( int )m ) * 60;
	m = ( int )m;
	
	dms->lonDegrees->setValue( d );
	dms->lonMinutes->setValue( m );
	dms->lonSeconds->setValue( s );
	
	//degrees minutes
	dm->latDegrees->setValue( ( int )latitude );
	dm->latMinutes->setValue( ( fabs( latitude ) - ( int )fabs( latitude ) ) * 60 );
	
	dm->lonDegrees->setValue( ( int )longitude );
	dm->lonMinutes->setValue( ( fabs( longitude ) - ( int )fabs( longitude ) ) * 60 );
	
	//decimal degrees
	dd->latDegrees->setValue( latitude );
	dd->lonDegrees->setValue( longitude );
    }      
}
