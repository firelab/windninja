/******************************************************************************
 *
 * $Id $ 
 *
 * Project:  WindNinja Qt GUI
 * Purpose:  Point data model.
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

#include "pointDataModel.h"

/**
 * 
 * @param parent
 */
pointDataModel::pointDataModel( QObject *parent ) : 
    QAbstractItemModel( parent )
{
    header << "Station" << "Value";
}

pointDataModel::~pointDataModel()
{

}

QVariant pointDataModel::data( const QModelIndex &index, int role ) const
{
    double dfTemp = 0;
    int row = index.row();
    int col = index.column();
    wxStation station = stations[row];
    if( role ==  Qt::DisplayRole ) {
	if( col == 0 )
	    return QVariant( station.get_stationName().c_str() );
        else if ( col == 1 ) {
	    if( station.get_coordType() == wxStation::PROJCS )
		return QVariant( "PROJCS" );
	    else if( station.get_coordType() == wxStation::GEOGCS )
		return QVariant( "GEOGCS" );
	}
	else if( col == 2 ) {
	    if( station.get_datumType() == wxStation::WGS84 )
		return QVariant( "WGS84" );
	    else if( station.get_datumType() == wxStation::NAD83 )
		return QVariant( "NAD83" );
	    else if( station.get_datumType() == wxStation::NAD27 )
		return QVariant( "NAD27" );
	}
	else if( col == 3 ) {
            if( station.get_coordType() == wxStation::GEOGCS )
                return QVariant( station.get_lat( ) );
            else if ( station.get_coordType() == wxStation::PROJCS )
                return QVariant( station.get_projYord( ) );
            else
                return QVariant();
        }
	else if( col == 4 )
	    return QVariant( station.get_lon() );
	else if( col == 5 )
        return QVariant( station.get_height(0) );
	else if( col == 6 )
	    return QVariant( "meters" );
	else if( col == 7 )
        return QVariant( station.get_speed(0) );
	else if( col == 8 )
	    return QVariant( "mps" );
	else if( col == 9 )
        return QVariant( station.get_direction(0) );
	else if( col == 10 )
        return QVariant( station.get_temperature(0) );
	else if( col == 11 )
	    return QVariant( "K" );
	else if( col == 12 )
        return QVariant( station.get_cloudCover(0) * 100 );
	else if( col ==13 ) {
        dfTemp = station.get_influenceRadius(0);
	    if( dfTemp < 0 )
		dfTemp = -1;
	    return QVariant( dfTemp );
	}
	else if( col == 14 )
	    return QVariant( "meters" );
	else
	    return QVariant();
    }
    return QVariant( );
}

QModelIndex pointDataModel::parent( const QModelIndex & index ) const
{
    return QModelIndex();
}

QModelIndex pointDataModel::index ( int row, int column, 
				    const QModelIndex & parent ) const
{
    return QModelIndex();
}

QVariant pointDataModel::headerData( int section, Qt::Orientation orientation, 
				     int role ) const
{
    if( role ==  Qt::DisplayRole ) {
	if( orientation == Qt::Horizontal )
	    return QVariant( header[section] );
	else if( orientation == Qt::Vertical )
	    return QVariant( section + 1 );
	else
	    return QVariant();
    }
    else
	return QVariant();
}

int pointDataModel::rowCount(const QModelIndex &parent ) const
{
    return stations.size();
}

int pointDataModel::columnCount(const QModelIndex &parent ) const
{
    return header.size();
}

void pointDataModel::update()
{
    reset();
}
