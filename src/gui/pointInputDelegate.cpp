/******************************************************************************
 *
 * $Id$ 
 *
 * Project:  WindNinja Qt GUI
 * Purpose:  Point initialization delegate.
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
 * @file pointInputDelegate.cpp
 * @author Kyle Shannon <ksshannon@gmail.com>
 */
#include "pointInputDelegate.h"

/**
 * Constructor for the delegate representing a weather station
 * @param parent: pointer to the parent of the widget
 * @see QItemDelegate
 */
pointInputDelegate::pointInputDelegate( QObject *parent ) : 
    QItemDelegate( parent )
{

}

/**
 * Assigns the widget to display based on the column in the table.
 * Combo boxes are used for units, spin boxes for values.  The station
 * name is defaulted to the QItemDelegate::createEditor()
 * @param parent: pointer to the parent of the widget
 * @param QStyleOptionViewItem: unused
 * @param index: index to the row(unused) and column for the editor to be
 *               placed
 * @return QWidget to place in the view based on column
 */
QWidget *pointInputDelegate::createEditor( QWidget *parent, 
					   const QStyleOptionViewItem &option,
					   const QModelIndex &index ) const
{
    if( index.column() == 1 ) {
	QComboBox *comboBox = new QComboBox( parent );
	comboBox->addItem( "PROJCS" );
	comboBox->addItem( "GEOGCS" );
	return comboBox;
    }
    else if( index.column() == 2 ) {
	QComboBox *comboBox = new QComboBox( parent );
	comboBox->addItem( "WGS84" );
	comboBox->addItem( "NAD83" );
	comboBox->addItem( "NAD27" );
	return comboBox;
    }
    else if( index.column() == 3 ) {
	QDoubleSpinBox *spinBox = new QDoubleSpinBox( parent );
	spinBox->setRange( -90.0, 90.0 );
	spinBox->setSingleStep( 5.0 );
	spinBox->setDecimals( 3 );
	return spinBox;
    }
    else if( index.column() == 4 ) {
	QDoubleSpinBox *spinBox = new QDoubleSpinBox( parent );
	spinBox->setRange( -180.0, 180.0 );
	spinBox->setSingleStep( 5.0 );
	spinBox->setDecimals( 3 );
	return spinBox;
    }
    else if( index.column() == 5 ) {
	QDoubleSpinBox *spinBox = new QDoubleSpinBox( parent );
	spinBox->setRange( 0.0, 5000.0 );
	spinBox->setSingleStep( 1.0 );
	spinBox->setDecimals( 3 );
	return spinBox;
    }
    else if( index.column() == 6 ) {
	QComboBox *comboBox = new QComboBox( parent );
	comboBox->addItem( "meters" );
	comboBox->addItem( "feet" );
	return comboBox;
    }
    else if( index.column() == 7 ) {
	QDoubleSpinBox *spinBox = new QDoubleSpinBox( parent );
	spinBox->setRange( 0.0, 500.0 );
	spinBox->setSingleStep( 1.0 );
	spinBox->setDecimals( 3 );
	return spinBox;
    }
    else if( index.column() == 8 ) {
	QComboBox *comboBox = new QComboBox( parent );
	comboBox->addItem( "mph" );
	comboBox->addItem( "kph" );
	comboBox->addItem( "mps" );
	comboBox->addItem( "kts" );
	return comboBox;
    }
    else if( index.column() == 9 ) {
	QDoubleSpinBox *spinBox = new QDoubleSpinBox( parent );
	spinBox->setRange( 0.0, 500.0 );
	spinBox->setSingleStep( 1.0 );
	spinBox->setDecimals( 3 );
	spinBox->setSuffix( "\x00B0" );
	return spinBox;
    }
    else if( index.column() == 10 ) {
	QDoubleSpinBox *spinBox = new QDoubleSpinBox( parent );
	spinBox->setRange( 0.0, 150.0 );
	spinBox->setSingleStep( 5.0 );
	spinBox->setSuffix( "\x00B0" );
	spinBox->setDecimals( 3 );
	return spinBox;
    }
    else if( index.column() == 11 ) {
	QComboBox *comboBox = new QComboBox( parent );
	comboBox->addItem( "F" );
	comboBox->addItem( "C" );
	comboBox->addItem( "K" );
	return comboBox;
    }
    else if( index.column() == 12 ) {
	QSpinBox *spinBox = new QSpinBox( parent );
	spinBox->setRange( 0, 100 );
	spinBox->setSingleStep( 5 );
	return spinBox;
    }
    else if( index.column() == 13 ) {
	QDoubleSpinBox *spinBox = new QDoubleSpinBox( parent );
	spinBox->setRange( 0.0, 50000.0 );
	spinBox->setSingleStep( 50.0 );
	return spinBox;
    }
    else if( index.column() == 14 ) {
	QComboBox *comboBox = new QComboBox( parent );
	comboBox->addItem( "miles" );
	comboBox->addItem( "feet" );
	comboBox->addItem( "meters" );
	comboBox->addItem( "km" );
	return comboBox;
    }
    else
	return QItemDelegate::createEditor( parent, option, index );
}

/**
 * Set the editor data on click
 * @param editor the widget to populate
 * @param index row and column
 */
void pointInputDelegate::setEditorData( QWidget *editor, 
					const QModelIndex &index ) const
{
    int column = index.column();
    if( column == 1 ) {
    	QComboBox *comboBox = qobject_cast<QComboBox*>( editor );
    	QString cs = index.model()->data( index, Qt::DisplayRole ).toString();
    	if( cs == "PROJCS" )
    	    comboBox->setCurrentIndex( 0 );
    	else if( cs == "GEOGCS" )
    	    comboBox->setCurrentIndex( 1 );
    }
    else if( column == 2 ) {
    	QComboBox *comboBox = qobject_cast<QComboBox*>( editor );
    	QString d = index.model()->data( index, Qt::DisplayRole ).toString();
    	if( d == "WGS84" )
    	    comboBox->setCurrentIndex( 0 );
    	else if( d == "NAD83" )
    	    comboBox->setCurrentIndex( 1 );
    	else if( d == "NAD27" )
    	    comboBox->setCurrentIndex( 2 );
    }
    else if(  column == 6 || column == 8 || 
    	      column == 11 || column == 15 ) {
    	QComboBox *comboBox = qobject_cast<QComboBox*>( editor );
    	QString s = index.model()->data( index, Qt::DisplayRole ).toString();
    	comboBox->setEditText( s );    	
    }
    else if( column == 3 || column == 4 || column ==  5 || column == 7 ||
    	     column == 9 || column == 10 || column == 12 || column == 13 ) {
    	QDoubleSpinBox *spinBox = qobject_cast<QDoubleSpinBox*>( editor );
    	double v = index.model()->data( index, 	Qt::DisplayRole ).toDouble();
    	spinBox->setValue( v );
    }
    else
	return QItemDelegate::setEditorData( editor, index );
}

/**
 * Use the editor data to update the model behind the controller
 * @param editor widget to collect data from
 * @param model model associated with the controller
 * @param index row and column information
 */
void pointInputDelegate::setModelData( QWidget *editor, 
				       QAbstractItemModel *model, 
				       const QModelIndex &index ) const
{
    int col = index.column();
    int row = index.row();
    pointDataModel* pdm = (pointDataModel*)model;
    if( col == 1 ) {
	QComboBox *comboBox = qobject_cast<QComboBox*>( editor );
	if( comboBox->currentIndex() == 0 )
	    pdm->stations[row].set_coordType( wxStation::PROJCS );
	else if( comboBox->currentIndex() == 1 )
	    pdm->stations[row].set_coordType( wxStation::GEOGCS );
    }
    else
	QItemDelegate::setModelData( editor, model, index );
}

/**
 * Update the geomety after editing to handle resizing
 * @param editor widget information containing data
 * @param option style information
 * @param index row and column information
 */
void pointInputDelegate::updateEditorGeometry( QWidget *editor,
					       const QStyleOptionViewItem &option, 
					       const QModelIndex &index ) const
{
    editor->setGeometry( option.rect );
}
