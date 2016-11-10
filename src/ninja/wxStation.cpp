/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja
 * Purpose:  A class for storing point weather station data
 * Author:   Jason Forthofer <jforthofer@gmail.com>
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
 * @file wxStation.cpp
 * @author Jason Forthofer <jforthofer@gmail.com>
 * @author Kyle Shannon <ksshannon@gmail.com>
 */
#include "wxStation.h"



/**
 * Default constructor for wxStation class populated with default values
 * @see wxStation::initialize
 */




wxStation::wxStation()
{
    initialize();
}

/**
 * Copy Constructor for wxStation
 * @param m right hand side wxStation object
 */
wxStation::wxStation( wxStation const& m )
{
    stationName = m.stationName;
    lat = m.lat;
    lon = m.lon;
    projXord = m.projXord;
    projYord = m.projYord;
    xord = m.xord;
    yord = m.yord;
    height = m.height;
    heightUnits = m.heightUnits;
    speed = m.speed;
    inputSpeedUnits = m.inputSpeedUnits;
    direction = m.direction;
    w_speed = m.w_speed;
    w_speedUnits = m.w_speedUnits;
    temperature = m.temperature;
    tempUnits = m.tempUnits;
    cloudCover = m.cloudCover;
    cloudCoverUnits = m.cloudCoverUnits;
    influenceRadius = m.influenceRadius;
    influenceRadiusUnits = m.influenceRadiusUnits;
    datumType = m.datumType;
    coordType = m.coordType;
    datetime = m.datetime;
    localDateTime=m.localDateTime;
    curStep=m.curStep;

}

/**
 * Equals operator for wxStation
 * @param m right hand side wxStation object
 * @return reference to a new wxStation object
 */
wxStation& wxStation::operator= ( wxStation const& m )
{
    if( &m != this ) {
    stationName = m.stationName;
    lat = m.lat;
    lon = m.lon;
    projXord = m.projXord;
    projYord = m.projYord;
    xord = m.xord;
    yord = m.yord;
    height = m.height;
    heightUnits = m.heightUnits;
    speed = m.speed;
    inputSpeedUnits = m.inputSpeedUnits;
    direction = m.direction;
    w_speed = m.w_speed;
    w_speedUnits = m.w_speedUnits;
    temperature = m.temperature;
    tempUnits = m.tempUnits;
    cloudCover = m.cloudCover;
    cloudCoverUnits = m.cloudCoverUnits;
    influenceRadius = m.influenceRadius;
    influenceRadiusUnits = m.influenceRadiusUnits;
    datumType = m.datumType;
    coordType = m.coordType;
    datetime =m.datetime;
    localDateTime=m.localDateTime;
    curStep=m.curStep;
    }
    return *this;
}
/**
 * Destructor
 */
wxStation::~wxStation()
{

}
/**
 *@brief Fetches station data from a known station id
 *@param station_id Known station id
 *@param nHours Duration of data to fetch in hours
 *@return void
 */




/**
 * Initialize a wxStation object with default values
 */
void wxStation::initialize()
{
    stationName = "";
    lat = lon = 0.0;
    datumType = wxStation::WGS84;
    coordType = wxStation::GEOGCS;
    height = zero;
    heightUnits = lengthUnits::meters;
    speed = zero;
    inputSpeedUnits = velocityUnits::metersPerSecond;
    direction = zero;
    w_speed = 0.0;
    w_speedUnits = velocityUnits::metersPerSecond;
    temperature = zero;
    tempUnits = temperatureUnits::K;
    cloudCover = zero;
    cloudCoverUnits = coverUnits::fraction;
    influenceRadius = zero;
    influenceRadiusUnits = lengthUnits::meters;
    datumType = WGS84;
    coordType = GEOGCS;
    datetime;
    localDateTime;
    curStep;
}

/**
 * Fetch the height of the wxStation.  Default height is meters
 * @param units to retrieve the height in
 * @return height of station
 */
double wxStation::get_height(lengthUnits::eLengthUnits units)
{
//    double h = height[idx];
//    lengthUnits::fromBaseUnits( h, units );
//    return h;


    double h;
    lengthUnits::fromBaseUnits( h, units );
    boost::local_time::local_date_time cur=get_currentTimeStep();
    for (int i=0;i<datetime.size();i++)
    {
        if (cur==localDateTime[i])
        {
            h=height[i];
            break;
        }
    }
    return h;

}

/**
 * Fetch the station speed, default unit is mps
 * @param units to retrieve speed in
 * @return speed
 */
//double wxStation::get_speed( int idx, velocityUnits::eVelocityUnits units)
//{
//    double s = speed[idx];
//    velocityUnits::fromBaseUnits( s, units );
//    return s;
//}

double wxStation::get_speed(velocityUnits::eVelocityUnits units)
{
    double s;
    velocityUnits::fromBaseUnits( s, units );
    boost::local_time::local_date_time cur=get_currentTimeStep();
    for (int i=0;i<datetime.size();i++)
    {
        if (cur==localDateTime[i])
        {
//            cout<<i<<endl;
//            cout<<speed[i]<<endl;
            s=speed[i];
            break;
        }
    }
    return s;

}


double wxStation::get_direction()
{
    double d;

    boost::local_time::local_date_time cur=get_currentTimeStep();
    for (int i=0;i<datetime.size();i++)
    {
        if (cur==localDateTime[i])
        {
            d=direction[i];
            break;
        }
    }
    return d;
}

/**
 * Fetch the vertical speed for the station
 * @param units to retrieved the vertical speed in, default is mps
 * @return vertical speed
 */
double wxStation::get_w_speed( velocityUnits::eVelocityUnits units)
{
    double w = w_speed;
    velocityUnits::fromBaseUnits( w, units );
    return w;
}
/**
 * Fetch temperature for the station
 * @param units to retrieve the temperature in, default is K
 * @return temperature
 */
double wxStation::get_temperature(temperatureUnits::eTempUnits units)
{
//    double t = temperature[idx];
//    temperatureUnits::fromBaseUnits( t, units );
//    return t;
    double t;
    temperatureUnits::fromBaseUnits( t, units );
    boost::local_time::local_date_time cur=get_currentTimeStep();
    for (int i=0;i<datetime.size();i++)
    {
        if (cur==localDateTime[i])
        {
            t=temperature[i];
            break;
        }
    }
    return t;
}
/**
 * Fetch the cloud cover for the station
 * @param units to retrieve the cover in, default is fraction
 * @return cloud cover
 */
double wxStation::get_cloudCover(coverUnits::eCoverUnits units)
{
//    double c = cloudCover[idx];
//    coverUnits::fromBaseUnits( c, units );
//    return c;
    double c;
    coverUnits::fromBaseUnits( c, units );
    boost::local_time::local_date_time cur=get_currentTimeStep();
    for (int i=0;i<datetime.size();i++)
    {
        if (cur==localDateTime[i])
        {
            c=cloudCover[i];
            break;
        }
    }
    return c;
}
/**
 * Fetch the radius of influence of the station
 * @param units to retrieve the radius in, default is meters
 * @return radius of infuence
 */
double wxStation::get_influenceRadius(lengthUnits::eLengthUnits units)
{
//    double i = influenceRadius[idx];
//    lengthUnits::fromBaseUnits( i, units );
//    return i;

    double i;
    lengthUnits::fromBaseUnits( i, units );
    boost::local_time::local_date_time cur=get_currentTimeStep();
    for (int k=0;k<datetime.size();k++)
    {
        if (cur==localDateTime[k])
        {
            i=influenceRadius[k];
            break;
        }
    }
    return i;


}
/**
 * Set the weather station name
 * @param Name representation of the station name.
 */
void wxStation::set_stationName( std::string Name )
{
    stationName = Name;
}
/**
 * Set the location of the station in lat/lon based on projected coordinates
 * @param Xord x coordinate in system
 * @param Yord y coordinate in system
 * @param demFile name of the dem file to create the coordinate transformation
 * from
 */
void wxStation::set_location_projected( double Xord, double Yord,
                    std::string demFile )
{
    projXord = Xord;
    projYord = Yord;


    if( demFile.empty() || demFile == "" )
    return;

    //get llcorner to subtract
    GDALDataset *poDS = (GDALDataset*) GDALOpen( demFile.c_str(), GA_ReadOnly );
    if( poDS == NULL ){
    xord = Xord;
    yord = Yord;
    return;
    }
    double adfGeoTransform[6];
    if( poDS->GetGeoTransform( adfGeoTransform ) != CE_None ) {
    xord = Xord;
    yord = Yord;
    return;
    }
    double llx = 0.0;
    double lly = 0.0;
    llx = adfGeoTransform[0];
    lly = adfGeoTransform[3] + ( adfGeoTransform[5] * poDS->GetRasterYSize() );

    xord = Xord - llx;
    yord = Yord - lly;

    double lonx = projXord;
    double laty = projYord;

    GDALPointToLatLon( lonx, laty, poDS, "WGS84" );
    datumType = WGS84;
    coordType = PROJCS;

    lon = lonx;
    lat = laty;

    GDALClose( (GDALDatasetH)poDS );
}
/**
 * Set location of the station based on latitude and longitude.  Datum
 * transformation info may need to occur
 * @param Lat latitude of the station
 * @param Lon longitude of the station
 * @param demFile file name of the input dem, for coord tranformation
 * @param datum datum to convert to.
 */
void wxStation::set_location_LatLong( double Lat, double Lon,
                                      const std::string demFile,
                      const char *datum )
{
    lon = Lon;
    lat = Lat;

    if( demFile.empty() || demFile == "" )
    return;

    GDALDataset *poDS = (GDALDataset*)GDALOpen( demFile.c_str(), GA_ReadOnly );
    if( poDS == NULL )
    return;

    double projX = lon;
    double projY = lat;

    GDALPointFromLatLon( projX, projY, poDS, datum );

    projXord = projX;
    projYord = projY;

    //still assume dem is projected.
    double llx = 0.0;
    double lly = 0.0;
    double adfGeoTransform[6];
     if( poDS->GetGeoTransform( adfGeoTransform ) != CE_None ) {
    xord = projXord;
    yord = projYord;
    return;
    }
    llx = adfGeoTransform[0];
    lly = adfGeoTransform[3] + ( adfGeoTransform[5] * poDS->GetRasterYSize() );

    xord = projXord - llx;
    yord = projYord - lly;

    if( EQUAL( datum, "WGS84" ) )
    datumType = WGS84;
    else if( EQUAL( datum, "NAD83" ) )
    datumType = NAD83;
    else if ( EQUAL( datum, "NAD27" ) )
    datumType = NAD27;

    coordType = GEOGCS;

    GDALClose( (GDALDatasetH)poDS );
}

/**
 * Set the height of the station
 * @param Height height of the station
 * @param units units of the Height parameter
 * @see ninjaUnits
 */
void wxStation::set_height( double Height, lengthUnits::eLengthUnits units )
{
    lengthUnits::toBaseUnits( Height, units );
    height.push_back(Height);
    heightUnits = units;
}
/** Set the speed for the station
 *
 *
 * @param Speed
 * @param units
 */
void wxStation::set_speed( double Speed, velocityUnits::eVelocityUnits units )
{
    velocityUnits::toBaseUnits( Speed, units );
    speed.push_back(Speed);
    inputSpeedUnits = units;
}

void wxStation::set_direction( double Direction )
{
    direction.push_back(Direction);
}

void wxStation::set_w_speed( double W_Speed, velocityUnits::eVelocityUnits units )
{
    velocityUnits::toBaseUnits( W_Speed, units );
    w_speed = W_Speed;
    w_speedUnits = units;
}

void wxStation::set_temperature( double Temperature,
                 temperatureUnits::eTempUnits units )
{
    temperatureUnits::toBaseUnits( Temperature, units );
    temperature.push_back(Temperature);
    tempUnits = units;
}

void wxStation::set_cloudCover( double CloudCover,
                coverUnits::eCoverUnits units )
{
    coverUnits::toBaseUnits( CloudCover, units );
    cloudCover.push_back(CloudCover);
    cloudCoverUnits = units;
}

void wxStation::set_influenceRadius( double InfluenceRadius,
                     lengthUnits::eLengthUnits units )
{
    lengthUnits::toBaseUnits( InfluenceRadius, units );
    influenceRadius.push_back(InfluenceRadius);
    influenceRadiusUnits = units;
}

void wxStation::set_datetime(boost::posix_time::ptime timedata)
{
    datetime.push_back(timedata);
}
boost::posix_time::ptime wxStation::get_datetime(int idx)
{
    boost::posix_time::ptime time=datetime[idx];
    return time;
}
void wxStation::set_localDateTime(boost::local_time::local_date_time timedata)
{
    localDateTime.push_back(timedata);
}

void wxStation::assign_direction(double Direction, int index)// Used for Match Points
{
    direction[index]=Direction;
}
void wxStation::assign_speed(double Speed, velocityUnits::eVelocityUnits units, int index)// Used for Match Points
{
    velocityUnits::toBaseUnits( Speed, units );
    speed[index]=Speed;
    inputSpeedUnits = units;
}

int wxStation::get_listSize()
{
    int size=speed.size();
    return size;
}

boost::local_time::local_date_time wxStation::get_localDateTime(int idx)
{
    boost::local_time::local_date_time time=localDateTime[idx];
    return time;
}

boost::local_time::local_date_time wxStation::get_currentTimeStep()
{
    boost::local_time::local_date_time current=curStep[0];

    return current;
}

void wxStation::set_currentTimeStep(boost::local_time::local_date_time step)
{
//    cout<<step<<endl;
    curStep.assign(1,step);
//    cout<<step<<endl;
//    cout<<curStep[0]<<endl;
//    cout<<curStep.size()<<endl;

}



bool wxStation::check_station(wxStation station)
{
    if(station.stationName.empty() || station.stationName == "")
    {
        cout<<"failed Name Check"<<endl;
        return false;
    }

    if(station.coordType != GEOGCS && station.coordType != PROJCS)
    {
        cout<<"failed Coord Check"<<endl;
        return false;
    }

    if(station.datumType != WGS84 && station.datumType != NAD83 && station.datumType !=NAD27)
    {
        cout<<"failed datum Check"<<endl;
        return false;
    }

    if(station.lat < -90.0 || station.lat > 90.0)
    {
        cout<<"failed lat Check"<<endl;
        return false;
    }

    if(station.lon < -180.0 || station.lon > 360.0)
    {
        cout<<"failed lon Check"<<endl;
        return false;
    }
    for (int i=0;i<station.height.size();i++)
    {
        if(station.height[i] < 0.0)
            {
                cout<<"failed height Check on "<<i<<endl;
                cout<<station.height[i]<<endl;
                return false;
            }

        if(station.speed[i] < 0.0)
            {
                cout<<"failed speed Check on "<<i<<endl;
                cout<<station.speed[i]<<endl;
                return false;
            }

        if(station.direction[i] < 0.0 || station.direction[i] > 360.0)
            {
                cout<<"failed direction Check on "<<i<<endl;
                cout<<station.direction[i]<<endl;
                return false;
            }
        if(station.temperature[i]< 173.15 || station.temperature[i] > 330.00)
            {
                cout<<"failed temperature Check on "<<i<<endl;
                cout<<station.temperature[i]<<endl;
                return false;
            }
        if(station.cloudCover[i]<0.0||station.cloudCover[i]>1.10)
            {
                cout<<"failed cloud check on "<<i<<endl;
                cout<<station.cloudCover[i]<<endl;
                return false;
            }

    }

    if(station.w_speed < 0.0)
    {
        cout<<"failed vert_speed Check"<<endl;
        return false;
    }

    return true;
}


/**
 * Write CSV and KML files and stuff used to be here, I don't know what do to/if they are needed
 */

void wxStation::stationViewer(wxStation station)
{

}


/**
 * Fetch a valid header associated with values contained in wxStationList.
 * This is for displaying data and contains extra tags that represent
 * choices in units and other quantities.
 * @return array of strings representing the header
 */

char **wxStation::oldGetValidHeader()
{
    char** papszList = NULL;
    papszList = CSLAddString( papszList, "Station_Name" );
    papszList = CSLAddString( papszList,
                    "Coord_Sys(PROJCS,GEOGCS)" );
    papszList = CSLAddString( papszList, "Datum(WGS84,NAD83,NAD27)" );
    papszList = CSLAddString( papszList, "Lat/YCoord" );
    papszList = CSLAddString( papszList, "Lon/XCoord" );
    papszList = CSLAddString( papszList, "Height" );
    papszList = CSLAddString( papszList, "Height_Units(meters,feet)" );
    papszList = CSLAddString( papszList, "Speed" );
    papszList = CSLAddString( papszList,
                    "Speed_Units(mph,kph,mps)" );
    papszList = CSLAddString( papszList, "Direction(degrees)" );
    papszList = CSLAddString( papszList, "Temperature" );
    papszList = CSLAddString( papszList,
                    "Temperature_Units(F,C)" );
    papszList = CSLAddString( papszList, "Cloud_Cover(%)" );
    papszList = CSLAddString( papszList, "Radius_of_Influence" );
    papszList = CSLAddString( papszList,
                    "Radius_of_Influence_Units(miles,feet,meters,km)" );
    return papszList;
}
char **wxStation::getValidHeader()
{
    char** papszList = NULL;
    papszList = CSLAddString( papszList, "Station_Name" );
    papszList = CSLAddString( papszList,
                    "Coord_Sys(PROJCS,GEOGCS)" );
    papszList = CSLAddString( papszList, "Datum(WGS84,NAD83,NAD27)" );
    papszList = CSLAddString( papszList, "Lat/YCoord" );
    papszList = CSLAddString( papszList, "Lon/XCoord" );
    papszList = CSLAddString( papszList, "Height" );
    papszList = CSLAddString( papszList, "Height_Units(meters,feet)" );
    papszList = CSLAddString( papszList, "Speed" );
    papszList = CSLAddString( papszList,
                    "Speed_Units(mph,kph,mps)" );
    papszList = CSLAddString( papszList, "Direction(degrees)" );
    papszList = CSLAddString( papszList, "Temperature" );
    papszList = CSLAddString( papszList,
                    "Temperature_Units(F,C)" );
    papszList = CSLAddString( papszList, "Cloud_Cover(%)" );
    papszList = CSLAddString( papszList, "Radius_of_Influence" );
    papszList = CSLAddString( papszList,
                    "Radius_of_Influence_Units(miles,feet,meters,km)" );
    papszList= CSLAddString(papszList,"date_time");
    return papszList;
}
/**Write a csv file with no data, just a header
 * @param outFileName file to write
 */
void wxStation::writeBlankStationFile( std::string outFileName )
{
    if( outFileName.empty() || outFileName == "" )
    return;
    FILE *fout;
    fout = fopen( outFileName.c_str(), "w" );
    if( fout == NULL )
    return;
    char** papszHeader = NULL;
    papszHeader = wxStation::getValidHeader();
    for( int i = 0;i < CSLCount( papszHeader ) - 1;i++ )
    fprintf( fout, "\"%s\",", papszHeader[i] );
    fprintf( fout, "\"%s\"\n", papszHeader[CSLCount( papszHeader ) - 1] );
    fclose( fout );
    CSLDestroy( papszHeader );
}

/** Write a csv file representing interpolated wxStation data
 * @param StationVect std::vector of wxStation objects
 * @param outFileName output file name as std::string
 */
void wxStation::writeStationFile( std::vector<wxStation>StationVect,
                                  std::string outFileName )
{
    std::string outFileNameStamp;
    std::string outFileNameMod;
    boost::local_time::local_date_time stamp=StationVect[0].get_currentTimeStep();

    boost::local_time::local_time_facet* timeOutputFacet;
    timeOutputFacet = new boost::local_time::local_time_facet();
    std::ostringstream timestream;
    timestream.imbue(std::locale(std::locale::classic(), timeOutputFacet));
    timeOutputFacet->format("%m-%d-%Y_%H%M");

    timestream<<stamp;

    if (outFileName.substr(outFileName.size()-4,4)==".csv")
    {
//        cout<<outFileName.substr(0,outFileName.size()-4)<<endl;
        outFileNameMod=outFileName.substr(0,outFileName.size()-4);
    }
    else
    {
        outFileNameMod=outFileName;
    }
//    cout<<stringStamp.str()<<endl;
    outFileNameStamp=outFileNameMod+"-"+timestream.str()+".csv";
//    cout<<outFileNameStamp<<endl;

    if( outFileNameStamp.empty() || outFileNameStamp == "" )
    return;
    if( StationVect.empty() ) {
    writeBlankStationFile( outFileNameStamp );
    return;
    }

    FILE *fout;
    fout = fopen( outFileNameStamp.c_str(), "w" );
    if( fout == NULL )
    return;
    char** papszHeader = NULL;
    papszHeader = wxStation::getValidHeader();
    for( int i = 0;i < CSLCount( papszHeader ) - 1;i++ )
    fprintf( fout, "\"%s\",", papszHeader[i] );
    fprintf( fout, "\"%s\"\n", papszHeader[CSLCount( papszHeader ) - 1] );

    for( unsigned int i = 0;i < StationVect.size();i++ ) {
    fprintf( fout, "\"%s\",", StationVect[i].get_stationName().c_str() );
    fprintf( fout, "\"GEOGCS\"," );
    fprintf( fout, "\"WGS84\"," );
    fprintf( fout, "%.10lf,", StationVect[i].get_lat() );
    fprintf( fout, "%.10lf,", StationVect[i].get_lon() );
    fprintf( fout, "%.2lf,", StationVect[i].get_height() );
    fprintf( fout, "\"meters\"," );
    fprintf( fout, "%.2lf,", StationVect[i].get_speed() );
    fprintf( fout, "\"mps\"," );
    fprintf( fout, "%.1lf,", StationVect[i].get_direction() );
    fprintf( fout, "%.1lf,", StationVect[i].get_temperature() );
    fprintf( fout, "\"K\"," );
    fprintf( fout, "%.1lf,", StationVect[i].get_cloudCover() * 100 );
    if( StationVect[i].get_influenceRadius() < 0.0 )
        fprintf( fout, "%.2lf,", -1.0 );
    else
        fprintf( fout, "%.2lf,", StationVect[i].get_influenceRadius() );
    fprintf( fout, "\"meters\"," );
    fprintf( fout, "\"%s\",", timestream.str().c_str() );

    fprintf( fout, "\n" );
    }

    fclose( fout );
    CSLDestroy( papszHeader );
}
/** Write a point kml file for all the weather stations in stations vector.
 * @param stations A vector of wxStationList objects
 * @param outFileName A string for output file
 * @return void
 */
void wxStation::writeKmlFile( std::vector<wxStation> stations,
        const std::string outFileName )
{
    std::string outFileNameStamp;
    std::string outFileNameMod;
    boost::local_time::local_date_time stamp=stations[0].get_currentTimeStep();

    boost::local_time::local_time_facet* timeOutputFacet;
    timeOutputFacet = new boost::local_time::local_time_facet();
    std::ostringstream timestream;
    timestream.imbue(std::locale(std::locale::classic(), timeOutputFacet));
    timeOutputFacet->format("%m-%d-%Y_%H%M");

    timestream<<stamp;

    if (outFileName.substr(outFileName.size()-4,4)==".kml")
    {
//        cout<<outFileName.substr(0,outFileName.size()-4)<<endl;
        outFileNameMod=outFileName.substr(0,outFileName.size()-4);
    }
    else
    {
        outFileNameMod=outFileName;
    }
//    cout<<stringStamp.str()<<endl;
    outFileNameStamp=outFileNameMod+"-"+timestream.str()+".kml";
//    cout<<outFileNameStamp<<endl;
//    exit(1);
    if( stations.size() == 0 )
        return;
    FILE *fout = fopen( outFileNameStamp.c_str(), "w" );
    if( fout == NULL ) {
        printf( "Cannot open kml file: %s for writing.", outFileNameStamp.c_str() );
        return;
    }

    double heightTemp, speedTemp, directionTemp, ccTemp, temperatureTemp, radOfInflTemp;


    fprintf( fout, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n" );
    fprintf( fout, "<kml>\n" );
    fprintf( fout, "  <Document>\n" );
    fprintf( fout, "    <description>Weather Stations:%s</description>\n",
            outFileNameStamp.c_str() );
    for( unsigned int i = 0;i < stations.size();i++ ) {

        heightTemp = stations[i].get_height();
        speedTemp = stations[i].get_speed();
        directionTemp = stations[i].get_direction();
        ccTemp = stations[i].get_cloudCover();
        temperatureTemp = stations[i].get_temperature();
        radOfInflTemp = stations[i].get_influenceRadius();

        lengthUnits::fromBaseUnits(heightTemp, stations[i].heightUnits);
        //lengthUnits::fromBaseUnits(heightTemp, lengthUnits::feet);
        velocityUnits::fromBaseUnits(speedTemp, stations[i].inputSpeedUnits);
        //lengthUnits::fromBaseUnits(directionTemp, heightUnits);
        coverUnits::fromBaseUnits(ccTemp, stations[i].cloudCoverUnits);
        temperatureUnits::fromBaseUnits(temperatureTemp, stations[i].tempUnits);
        lengthUnits::fromBaseUnits(radOfInflTemp, stations[i].influenceRadiusUnits);

        fprintf( fout, "    <Placemark>\n" );
        fprintf( fout, "      <name>%s</name\n>",
                stations[i].get_stationName().c_str() );
        fprintf( fout, "    <Point>\n" );
        fprintf( fout, "        <altitudeMode>clampToGround</altitudeMode>\n" );
        fprintf( fout, "        <coordinates>\n" );
        fprintf( fout, "          %lf,%lf,0.0\n", stations[i].get_lon(),
                stations[i].get_lat() );
        fprintf( fout, "        </coordinates>\n" );
        fprintf( fout, "      </Point>\n" );
        fprintf( fout, "      <description>\n" );
        fprintf( fout, "        <![CDATA[\n" );
        fprintf( fout, "          Name: %s\n",
                stations[i].get_stationName().c_str() );
        fprintf( fout, "          Height: %.2lf %s\n", heightTemp, lengthUnits::getString(stations[i].heightUnits).c_str() );
        fprintf( fout, "          Speed: %.2lf %s\n", speedTemp, velocityUnits::getString(stations[i].inputSpeedUnits).c_str() );
        fprintf( fout, "          Direction: %.0lf %s\n",
                directionTemp, "degrees" );
        fprintf( fout, "          Cloud Cover: %.2lf %s\n",
                ccTemp, coverUnits::getString(stations[i].cloudCoverUnits).c_str() );
        fprintf( fout, "          Temperature: %.1lf %s\n",
                temperatureTemp, temperatureUnits::getString(stations[i].tempUnits).c_str() );
        if(stations[i].get_influenceRadius() > 0.0)
        {
            fprintf( fout, "          Radius of Influence: %.2lf %s\n",
                    radOfInflTemp, lengthUnits::getString(stations[i].influenceRadiusUnits).c_str() );
        }else
        {
            fprintf( fout, "          Radius of Influence: infinite\n");
        }
        fprintf( fout, "        ]]>\n" );
        fprintf( fout, "      </description>\n" );
        fprintf( fout, "    </Placemark>\n" );
    }
    fprintf( fout, "  </Document>\n" );
    fprintf( fout, "</kml>\n" );

    fclose( fout );
}








