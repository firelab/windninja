/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja
 * Purpose:  Widget for time zone access using boost local_time.
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

#include "timeZoneWidget.h"

extern boost::local_time::tz_database globalTimeZoneDB;

/** 
 * Construct a timeZoneWidget that relies on boost for time zone
 * operations
 * 
 * @param parent parent widget
 */
timeZoneWidget::timeZoneWidget( QWidget *parent ) : QWidget( parent )
{
    tzComboBox =  new QComboBox( this );
    tzCheckBox  = new QCheckBox( tr( "Show All Zones" ), this );

    tzDetailLabel = new QLabel( tr( "\n\n\n\n" ), this );
    tzDetailCheckBox = new QCheckBox( tr( "Display time zone details" ), this );
    
    tzLayout = new QHBoxLayout();
    tzLayout->addWidget( tzComboBox );
    tzLayout->addWidget( tzCheckBox );
    tzLayout->addStretch();
    
    layout = new QVBoxLayout();
    layout->addLayout( tzLayout );
    layout->addWidget( tzDetailCheckBox );
    layout->addWidget( tzDetailLabel );

    setLayout( layout );
    
    loadTimeZones();

    createConnections();

    tzCheckBox->setChecked( true );
    tzCheckBox->setChecked( false );
}

/** 
 * Destructor
 * 
 */
timeZoneWidget::~timeZoneWidget()
{

}

/** 
 * Create connections to update the widgets
 * 
 */
void timeZoneWidget::createConnections()
{
    //connect the check box to toggle all time zones
    connect( tzCheckBox, SIGNAL( toggled( bool ) ),
	     this, SLOT( toggleAllTimeZones( bool ) ) );

    //connect the detail label visibility to the checkbox
    connect( tzDetailCheckBox, SIGNAL( toggled( bool ) ), 
	     this, SLOT( showDetails( bool ) ) );

    //connect the combo box to the label text
    connect( tzComboBox, SIGNAL( currentIndexChanged( int ) ), 
	     this, SLOT( updateDetailString( int ) ) );
}

/** 
 * Load time zone information into the QStringList.
 * From Boost Docs:
 * @verbatim
   Parameter is path to a time zone spec csv file (see Data File Details for 
   details on the contents of this file). This function populates the database 
   with time zone records found in the zone spec file. A 
   local_time::data_not_accessible exception will be thrown if 
   given zonespec file cannot be found. 
   local_time::bad_field_count exception will be thrown if the 
   number of fields in given zonespec file is incorrect.
   @endverbatim
 * 
 *
 */
void timeZoneWidget::loadTimeZones()
{
  std::vector<std::string> tz_list = globalTimeZoneDB.region_list();
  for (unsigned int i = 0; i < tz_list.size(); i++) {
    tzStringList << QString::fromStdString(tz_list[i]);
  }
}
/** 
 * Load the default time zones for US.  '~' represents standard zones.
 * 
 * - ~America/Anchorage
 * - America/Boise
 * - ~America/Chicago
 * - ~America/Denver
 * - America/Detroit
 * - America/Indiana/Indianapolis
 * - America/Indiana/Knox
 * - America/Indiana/Marengo
 * - America/Indiana/Vevay
 * - America/Indianapolis
 * - America/Juneau
 * - America/Kentucky/Louisville
 * - America/Kentucky/Monticello
 * - ~America/Los_Angeles
 * - America/Louisville
 * - ~America/New_York
 * - America/Nome
 * - America/North_Dakota/Center
 * - ~America/Phoenix
 * - ~Pacific/Honolulu
 *
 */
void timeZoneWidget::loadDefaultTimeZones()
{
    QStringList tz_list, display_list;
    tz_list << "America/Anchorage"
	    << "America/Boise"
	    << "America/Chicago"
	    << "America/Denver"
	    << "America/Detroit"
	    << "America/Indiana/Indianapolis"
	    << "America/Indiana/Knox"
	    << "America/Indiana/Marengo"
	    << "America/Indiana/Vevay"
	    << "America/Indianapolis"
	    << "America/Juneau"
	    << "America/Kentucky/Louisville"
	    << "America/Kentucky/Monticello"
	    << "America/Los_Angeles"
	    << "America/Louisville"
	    << "America/New_York"
	    << "America/Nome"
	    << "America/North_Dakota/Center"
	    << "America/Phoenix"
	    << "Pacific/Honolulu";
    display_list << "America/Anchorage(Alaska Time)"
		 << "America/Boise"
		 << "America/Chicago(Central Time)"
		 << "America/Denver(Mountain Time)"
		 << "America/Detroit"
		 << "America/Indiana/Indianapolis"
		 << "America/Indiana/Knox"
		 << "America/Indiana/Marengo"
		 << "America/Indiana/Vevay"
		 << "America/Indianapolis"
		 << "America/Juneau"
		 << "America/Kentucky/Louisville"
		 << "America/Kentucky/Monticello"
		 << "America/Los_Angeles(Pacific Time)"
		 << "America/Louisville"
		 << "America/New_York(Eastern Time)"
		 << "America/Nome"
		 << "America/North_Dakota/Center"
		 << "America/Phoenix"
		 << "Pacific/Honolulu(Hawaii Time)";

    if( tz_list.size() != display_list.size() )
    	qDebug() << "diurnalInput::loadDefaultTimeZones(): Wrong list size.";

    for( int i = 0;i < tz_list.size();i++ ) {
	/*
	 * Check and make sure it's valid and on the boost list
	 */
	if( !tzStringList.contains( tz_list[i] ) )
	    qDebug() << "Time Zone does not exist!" << tz_list[i];
	else
	    tzComboBox->addItem( display_list[i], 
				       QVariant( tz_list[i] ) );
    }
}

/** 
 * Slot to show all time zones or just US time zones
 *
 * @param showAll show all the timezones
 */
void timeZoneWidget::toggleAllTimeZones( bool showAll )
{
    QString currentTimeZone = tzComboBox->currentText().split("(")[0];

    if( showAll ) {
	tzComboBox->clear();
	for( int i = 0;i < tzStringList.size();i++ ) {
	    tzComboBox->addItem( tzStringList[i], 
				       QVariant( tzStringList[i] ) );
            }
    }
    else if( !showAll ) {
	tzComboBox->clear();
	loadDefaultTimeZones();
    }
    int index = 0;
    if( !currentTimeZone.isEmpty() )
    {
        index = tzComboBox->findText( currentTimeZone, Qt::MatchStartsWith );
        if( index < 0 )
        {
            index = 0;
        }
    }
    tzComboBox->setCurrentIndex( index );
}

/** 
 * Slot to update the description string for the time zone
 * 
 * @param index combobox index after change
 */
void timeZoneWidget::updateDetailString( int index )
{
    QString tzText = tzComboBox->itemData( index ).toString();
    emit tzChanged( tzText );
    if( tzDetailCheckBox->isChecked() == false )
	return;
    else if( tzComboBox->count() == 0 )
	return;
    else if( tzText.isEmpty() )
	return;
    else {
	blt::time_zone_ptr tz;
	tz = globalTimeZoneDB.time_zone_from_region( tzText.toStdString() );
	bool has_dst = tz->has_dst();
	QString text = "Standard Name:\t\t";
	text += QString::fromStdString( tz->std_zone_name() ); 
	text += "\nDaylight Name:\t\t";
	if( has_dst )
	    text += QString::fromStdString( tz->dst_zone_name() ) + "\n";
	else
	    text += "N/A\n";
	text += "Standard Offset from UTC:\t";
	text += QString::fromStdString( bpt::to_simple_string(tz->base_utc_offset() ) );
	text += "\nDaylight Offset from UTC:\t";
	if( has_dst ) {
	    bpt::time_duration d;
	    d = tz->base_utc_offset() + tz->dst_offset();
	    text += QString::fromStdString( bpt::to_simple_string( d ) );
	}
	else
	    text += "N/A";

	tzDetailLabel->setText( text );
    }
}

/** 
 * Slot to show or hide the detailed description of the time zone
 * 
 * @param show show the label
 */
void timeZoneWidget::showDetails( bool show )
{
    if( show )
	return updateDetailString( tzComboBox->currentIndex() );
    else
	tzDetailLabel->setText( "\n" );
}
