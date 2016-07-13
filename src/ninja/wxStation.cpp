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
}

/**
 * Fetch the height of the wxStation.  Default height is meters
 * @param units to retrieve the height in
 * @return height of station
 */
double wxStation::get_height(int idx, lengthUnits::eLengthUnits units)
{
    double h = height[idx];
    lengthUnits::fromBaseUnits( h, units );
    return h;
}

/**
 * Fetch the station speed, default unit is mps
 * @param units to retrieve speed in
 * @return speed
 */
double wxStation::get_speed( int idx, velocityUnits::eVelocityUnits units)
{
    double s = speed[idx];
    velocityUnits::fromBaseUnits( s, units );
    return s;
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
double wxStation::get_temperature(int idx , temperatureUnits::eTempUnits units)
{
    double t = temperature[idx];
    temperatureUnits::fromBaseUnits( t, units );
    return t;
}
/**
 * Fetch the cloud cover for the station
 * @param units to retrieve the cover in, default is fraction
 * @return cloud cover
 */
double wxStation::get_cloudCover( int idx, coverUnits::eCoverUnits units)
{
    double c = cloudCover[idx];
    coverUnits::fromBaseUnits( c, units );
    return c;
}
/**
 * Fetch the radius of influence of the station
 * @param units to retrieve the radius in, default is meters
 * @return radius of infuence
 */
double wxStation::get_influenceRadius(int idx, lengthUnits::eLengthUnits units)
{
    double i = influenceRadius[idx];
    lengthUnits::fromBaseUnits( i, units );
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
boost::local_time::local_date_time wxStation::get_localDateTime(int idx)
{
    boost::local_time::local_date_time time=localDateTime[idx];
    return time;
}

/** Converts vector<vector<wxStationList> > to vector<wxStation> (wxStation for now)
 * wxStation List is what I made originally and does a good job or organizing the data,
 * but it isn't very readable or intuitive. All of the old wx code was renamed to a new class
 * "wxStationList", the new code, which is mainly the below function and some parasite functions
 * that are used to call certain variables outside this file.
 */
vector<wxStation> wxStation::makeWxStation(string csvFile,string demFile,vector<vector<wxStationList> > inputStation)
{

    cout<<"Converting wxList to wxStation"<<endl;

    vector<std::string> stationNames;
    vector<wxStation> stationData;


    OGRDataSourceH hDS;
    hDS = OGROpen( csvFile.c_str(), FALSE, NULL );

    OGRLayer *poLayer;
    OGRFeature *poFeature;
    OGRFeatureDefn *poFeatureDefn;
    poLayer = (OGRLayer*)OGR_DS_GetLayer( hDS, 0 );

    std::string oStationName;

    OGRLayerH hLayer;
    hLayer=OGR_DS_GetLayer(hDS,0);
    OGR_L_ResetReading(hLayer);

    poLayer->ResetReading();
    while( ( poFeature = poLayer->GetNextFeature() ) != NULL )
    {
    poFeatureDefn = poLayer->GetLayerDefn();

    // get Station name
    oStationName = poFeature->GetFieldAsString( 0 );
    stationNames.push_back(oStationName);
//    cout<<oStationName<<endl;
    }

    int statCount;
    statCount=stationNames.size();

    int specCount=statCount;

    vector<int> idxCount;
    int j=0;
    int q=0;

    for (int i=0;i<statCount;i++)
    {
//        cout<<"looks at: "<<q<<endl;
//        cout<<"starts at: "<<j<<endl;

        int idx1=0;
        for(j;j<specCount;j++)
        {
            if(stationNames[j]==stationNames[q])
            {
                idx1++;
            }
        }
        idxCount.push_back(idx1);
        j=std::accumulate(idxCount.begin(),idxCount.end(),0);
        q=j;
        if (j==statCount)
        {
//            cout<<"exiting loop"<<endl;
            break;
        }
    }

//     cout<<"idxCount size: "<<idxCount.size()<<endl;
//     for (int ii=0;ii<idxCount.size();ii++)
//     {
//         cout<<idxCount[ii]<<endl;
//     }
     vector<int> countLimiter;

     for (int ei=1;ei<=idxCount.size();ei++)
     {
     //    cout<<ei<<endl;
         int rounder=idxCount.size()-ei;
         int e=std::accumulate(idxCount.begin(),idxCount.end()-rounder,0);
         countLimiter.push_back(e);
     }

//     for (int i=0;i<idxCount.size();i++)
//     {
//         cout<<countLimiter[i]-1<<" "<<stationNames[countLimiter[i]-1]<<endl;
//     }



//    cout<<countLimiter[0]-1<<" "<<stationNames[countLimiter[0]-1]<<endl;
//    cout<<countLimiter[1]-1<<" "<<stationNames[countLimiter[1]-1]<<endl;
//    cout<<countLimiter[2]-1<<" "<<stationNames[countLimiter[2]-1]<<endl;
//     vector<wxStationList> Liszt=wxStationList::readStationFetchFile(csvFile,demFile);

     vector<vector<wxStationList> >stationDataList;
//     if (inputStation[0][0].get_stationName()=="")
//     {
//     vector<vector<wxStationList> >stationDataList=wxStationList::vectorRead(csvFile,demFile);
//     }
//     else

       stationDataList=inputStation;

     for (int i=0;i<countLimiter[0];i++)
     {
//         wxStationList::wxPrinter(Liszt[i]);
     }
for (int i=0;i<idxCount.size();i++)
{
    wxStation subDat;
    subDat.set_stationName(stationDataList[i][0].get_stationName());
//    cout<<subDat.get_stationName()<<endl;

    const char* stCoordDat;

    int iCT=stationDataList[i][0].get_coordType();

    if (iCT==0)
    {
     stCoordDat="PROJCS";
    }
    if (iCT==1)
    {
     stCoordDat="WGS84";
    }
    else
    {
     cout<<"defaulting to WGS84 "<<endl;
     stCoordDat="WGS84";
    }

    subDat.set_location_LatLong(stationDataList[i][0].get_lat(),stationDataList[i][0].get_lon(),
            demFile,stCoordDat);

    for (int k=0;k<stationDataList[i].size();k++)
    {
     subDat.set_speed(stationDataList[i][k].get_speed(),velocityUnits::metersPerSecond);
     subDat.set_direction(stationDataList[i][k].get_direction());
     subDat.set_temperature(stationDataList[i][k].get_temperature(),temperatureUnits::K);
     subDat.set_cloudCover(stationDataList[i][k].get_cloudCover(),coverUnits::fraction);
     subDat.set_influenceRadius(stationDataList[i][k].get_influenceRadius(),lengthUnits::meters);
     subDat.set_height(stationDataList[i][k].get_height(),lengthUnits::meters);
     subDat.set_datetime(stationDataList[i][k].get_datetime());

    }
//    cout<<subDat.speed.size()<<endl;
//    cout<<subDat.direction.size()<<endl;
    stationData.push_back(subDat);
}
//cout<<stationData.size()<<endl;

//     for (int kk=0;kk<stationDataList[1].size();kk++)
//     {
//         cout<<stationDataList[1][kk].get_cloudCover()<<" "<<stationData[1].get_cloudCover(coverUnits::fraction,kk)<<endl;
//     }




//     wxStation eval=readIndStat(csvFile,demFile,idxCount,idxCount[0]);

//      exit(1);

return stationData;
}


/**
 * Write CSV and KML files and stuff used to be here, I don't know what do to/if they are needed
 */


wxStationList::wxStationList()
{
    initialize();
}
/**
  * This is the old wxStation code, nothing has changed but every function other
  * than the constructors are private and shouldn't be used outside this file, as
  * they have been replaced by the above stuff.
  *
  *
  * /
/**
 * Copy Constructor for wxStationList
 * @param m right hand side wxStationList object
 */
wxStationList::wxStationList( wxStationList const& m )
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

}

/**
 * Equals operator for wxStationList
 * @param m right hand side wxStationList object
 * @return reference to a new wxStationList object
 */
wxStationList& wxStationList::operator= ( wxStationList const& m )
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
    }
    return *this;
}
/**
 * Destructor
 */
wxStationList::~wxStationList()
{

}
/**
 *@brief Fetches station data from a known station id
 *@param station_id Known station id
 *@param nHours Duration of data to fetch in hours
 *@return void
 */




/**
 * Initialize a wxStationList object with default values
 */
void wxStationList::initialize()
{
    stationName = "";
    lat = lon = 0.0;
    datumType = wxStationList::WGS84;
    coordType = wxStationList::GEOGCS;
    height = 10.0;
    heightUnits = lengthUnits::meters;
    speed = 0.0;
    inputSpeedUnits = velocityUnits::metersPerSecond;
    direction = 0.0;
    w_speed = 0.0;
    w_speedUnits = velocityUnits::metersPerSecond;
    temperature = 0.0;
    tempUnits = temperatureUnits::K;
    cloudCover = 0.0;
    cloudCoverUnits = coverUnits::fraction;
    influenceRadius = -1;
    influenceRadiusUnits = lengthUnits::meters;
    datumType = WGS84;
    coordType = GEOGCS;
    datetime;
}

bool wxStation::check_station(wxStation station)
{
    if(station.stationName.empty() || station.stationName == "")
        return false;

    if(station.coordType != GEOGCS && station.coordType != PROJCS)
        return false;

    if(station.datumType != WGS84 && station.datumType != NAD83 && station.datumType !=NAD27)
        return false;

    if(station.lat < -90.0 || station.lat > 90.0)
        return false;

    if(station.lon < -180.0 || station.lon > 360.0)
        return false;
    for (int i=0;i<station.height.size();i++)
    {
    if(station.height[i] < 0.0)
        return false;

    if(station.speed[i] < 0.0)
        return false;

    if(station.direction[i] < 0.0 || station.direction[i] > 360.0)
        return false;

    }

    if(station.w_speed < 0.0)
        return false;

    return true;
}



/**
 * Fetch a valid header associated with values contained in wxStationList.
 * This is for displaying data and contains extra tags that represent
 * choices in units and other quantities.
 * @return array of strings representing the header
 */

char **wxStationList::oldGetValidHeader()
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
char **wxStationList::getValidHeader()
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

/**
 * Fetch the height of the wxStationList.  Default height is meters
 * @param units to retrieve the height in
 * @return height of station
 */
double wxStationList::get_height( lengthUnits::eLengthUnits units )
{
    double h = height;
    lengthUnits::fromBaseUnits( h, units );
    return h;
}

/**
 * Fetch the station speed, default unit is mps
 * @param units to retrieve speed in
 * @return speed
 */
double wxStationList::get_speed( velocityUnits::eVelocityUnits units )
{
    double s = speed;
    velocityUnits::fromBaseUnits( s, units );
    return s;
}

/**
 * Fetch the vertical speed for the station
 * @param units to retrieved the vertical speed in, default is mps
 * @return vertical speed
 */
double wxStationList::get_w_speed( velocityUnits::eVelocityUnits units )
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
double wxStationList::get_temperature( temperatureUnits::eTempUnits units )
{
    double t = temperature;
    temperatureUnits::fromBaseUnits( t, units );
    return t;
}
/**
 * Fetch the cloud cover for the station
 * @param units to retrieve the cover in, default is fraction
 * @return cloud cover
 */
double wxStationList::get_cloudCover( coverUnits::eCoverUnits units )
{
    double c = cloudCover;
    coverUnits::fromBaseUnits( c, units );
    return c;
}
/**
 * Fetch the radius of influence of the station
 * @param units to retrieve the radius in, default is meters
 * @return radius of infuence
 */
double wxStationList::get_influenceRadius( lengthUnits::eLengthUnits units )
{
    double i = influenceRadius;
    lengthUnits::fromBaseUnits( i, units );
    return i;
}
/**
 * Set the weather station name
 * @param Name representation of the station name.
 */
void wxStationList::set_stationName( std::string Name )
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
void wxStationList::set_location_projected( double Xord, double Yord,
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
void wxStationList::set_location_LatLong( double Lat, double Lon,
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
void wxStationList::set_height( double Height, lengthUnits::eLengthUnits units )
{
    lengthUnits::toBaseUnits( Height, units );
    height = Height;
    heightUnits = units;
}
/** Set the speed for the station
 *
 *
 * @param Speed
 * @param units
 */
void wxStationList::set_speed( double Speed, velocityUnits::eVelocityUnits units )
{
    velocityUnits::toBaseUnits( Speed, units );
    speed = Speed;
    inputSpeedUnits = units;
}

void wxStationList::set_direction( double Direction )
{
    direction = Direction;
}

void wxStationList::set_w_speed( double W_Speed, velocityUnits::eVelocityUnits units )
{
    velocityUnits::toBaseUnits( W_Speed, units );
    w_speed = W_Speed;
    w_speedUnits = units;
}

void wxStationList::set_temperature( double Temperature,
                 temperatureUnits::eTempUnits units )
{
    temperatureUnits::toBaseUnits( Temperature, units );
    temperature = Temperature;
    tempUnits = units;
}

void wxStationList::set_cloudCover( double CloudCover,
                coverUnits::eCoverUnits units )
{
    coverUnits::toBaseUnits( CloudCover, units );
    cloudCover = CloudCover;
    cloudCoverUnits = units;
}

void wxStationList::set_influenceRadius( double InfluenceRadius,
                     lengthUnits::eLengthUnits units )
{
    lengthUnits::toBaseUnits( InfluenceRadius, units );
    influenceRadius = InfluenceRadius;
    influenceRadiusUnits = units;
}

void wxStationList::set_datetime(boost::posix_time::ptime timedata)
{
    datetime=timedata;
}
boost::posix_time::ptime wxStationList::get_datetime()
{
    boost::posix_time::ptime time=datetime;
    return time;
}

void wxStationList::wxPrinter(wxStationList wxObject)
{
    cout<<"Station Name: "<<wxObject.stationName<<endl;
    cout<<"lat,lon: "<<wxObject.lat<<" , "<<wxObject.lon<<endl;
    cout<<"speed,dir: "<<wxObject.speed<<" , "<<wxObject.direction<<endl;
    cout<<"temperature: "<<wxObject.temperature<<endl;
    cout<<"cloud cover: "<<wxObject.cloudCover<<endl;
    cout<<"datetime: "<<wxObject.datetime<<endl;
    cout<<"coord sys: "<<wxObject.coordType<<endl;
    cout<<"datum: "<<wxObject.datumType<<endl;
    cout<<"RI:"<<wxObject.influenceRadius<<endl;
    cout<<"height: "<<wxObject.height<<endl;

}

void wxStationList::wxVectorPrinter(std::vector<wxStationList> wxObject, int count)
{
    for(int i=0;i<count;i++)
    {
        cout<<"station step #"<<i<<endl;
        cout<<"Station Name: "<<wxObject[i].stationName<<endl;
        cout<<"lat,lon: "<<wxObject[i].lat<<" , "<<wxObject[i].lon<<endl;
        cout<<"speed,dir: "<<wxObject[i].speed<<" , "<<wxObject[i].direction<<endl;
        cout<<"temperature: "<<wxObject[i].temperature<<endl;
        cout<<"cloud cover: "<<wxObject[i].cloudCover<<endl;
        cout<<"datetime: "<<wxObject[i].datetime<<endl;
    }

}


/** vector of vectors*/

std::vector<std::vector<wxStationList> >wxStationList::vectorRead(std::string csvFile,std::string demFile)
{
    std::vector<std::vector<wxStationList> > a;

    vector<std::string> stationNames;

    OGRDataSourceH hDS;
    hDS = OGROpen( csvFile.c_str(), FALSE, NULL );

    OGRLayer *poLayer;
    OGRFeature *poFeature;
    OGRFeatureDefn *poFeatureDefn;
    poLayer = (OGRLayer*)OGR_DS_GetLayer( hDS, 0 );

    std::string oStationName;

    OGRLayerH hLayer;
    hLayer=OGR_DS_GetLayer(hDS,0);
    OGR_L_ResetReading(hLayer);

    poLayer->ResetReading();
    while( ( poFeature = poLayer->GetNextFeature() ) != NULL )
    {
    poFeatureDefn = poLayer->GetLayerDefn();

    // get Station name
    oStationName = poFeature->GetFieldAsString( 0 );
    stationNames.push_back(oStationName);
    }

    int statCount;
    statCount=stationNames.size();

    int specCount=statCount;

    vector<int> idxCount;
    int j=0;
    int q=0;

    for (int i=0;i<statCount;i++)
    {
//        cout<<"looks at: "<<q<<endl;
//        cout<<"starts at: "<<j<<endl;

        int idx1=0;
        for(j;j<specCount;j++)
        {
            if(stationNames[j]==stationNames[q])
            {
                idx1++;
            }
        }
        idxCount.push_back(idx1);
        j=std::accumulate(idxCount.begin(),idxCount.end(),0);
        q=j;
        if (j==statCount)
        {
//            cout<<"exiting loop"<<endl;
            break;
        }
    }

// cout<<"idxCount size: "<<idxCount.size()<<endl;
// for (int ii=0;ii<idxCount.size();ii++)
// {
//     cout<<idxCount[ii]<<endl;
// }

std::vector<std::vector<wxStationList> > wxVector;
std::vector<wxStationList> cata;
cata=readStationFetchFile(csvFile,demFile);
int t=0;
vector<int> countLimiter;

for (int ei=1;ei<=idxCount.size();ei++)
{
//    cout<<ei<<endl;
    int rounder=idxCount.size()-ei;
    int e=std::accumulate(idxCount.begin(),idxCount.end()-rounder,0);
    countLimiter.push_back(e);
}
//cout<<"limiter 0: "<<countLimiter[0]<<endl;
//cout<<"limiter 1: "<<countLimiter[1]<<endl;
//cout<<"limiter 2: "<<countLimiter[2]<<endl;

//cout<<"idx 0: "<<idxCount[0]<<endl;
//cout<<"idx 1: "<<idxCount[1]<<endl;
//cout<<"idx 2: "<<idxCount[2]<<endl;

for (int ei=0;ei<idxCount.size();ei++)
{

std::vector<wxStationList> sub(&cata[t],&cata[countLimiter[ei]]);

//cout<<"subsize: "<<sub.size()<<endl;

wxVector.push_back(sub);

t=countLimiter[ei];

}

//for (int i=0;i<idxCount.size();i++)
//{
////    wxPrinter(wxVector[i][0]);
////    wxVectorPrinter(wxVector[i],wxVector[i].size());
//    cout<<wxVector[i].size()<<endl;
//}
//cout<<wxVector.size()<<endl;


//    exit(1);

    return wxVector;
}
///**Write a csv file with no data, just a header
// * @param outFileName file to write
// */
//void wxStation::writeBlankStationFile( std::string outFileName )
//{
//    if( outFileName.empty() || outFileName == "" )
//    return;
//    FILE *fout;
//    fout = fopen( outFileName.c_str(), "w" );
//    if( fout == NULL )
//    return;
//    char** papszHeader = NULL;
//    papszHeader = getValidHeader();
//    for( int i = 0;i < CSLCount( papszHeader ) - 1;i++ )
//    fprintf( fout, "\"%s\",", papszHeader[i] );
//    fprintf( fout, "\"%s\"\n", papszHeader[CSLCount( papszHeader ) - 1] );
//    fclose( fout );
//    CSLDestroy( papszHeader );
//}

///** Write a csv file representing wxStation objects
// * @param StationVect std::vector of wxStation objects
// * @param outFileName output file name as std::string
// */
//void wxStation::writeStationFile( std::vector<wxStation>StationVect,
//                                  std::string outFileName )
//{
//    if( outFileName.empty() || outFileName == "" )
//    return;
//    if( StationVect.empty() ) {
//    writeBlankStationFile( outFileName );
//    return;
//    }

//    FILE *fout;
//    fout = fopen( outFileName.c_str(), "w" );
//    if( fout == NULL )
//    return;
//    char** papszHeader = NULL;
//    papszHeader = getValidHeader();
//    for( int i = 0;i < CSLCount( papszHeader ) - 1;i++ )
//    fprintf( fout, "\"%s\",", papszHeader[i] );
//    fprintf( fout, "\"%s\"\n", papszHeader[CSLCount( papszHeader ) - 1] );

//    for( unsigned int i = 0;i < StationVect.size();i++ ) {
//    fprintf( fout, "\"%s\",", StationVect[i].get_stationName().c_str() );
//    fprintf( fout, "\"GEOGCS\"," );
//    fprintf( fout, "\"WGS84\"," );
//    fprintf( fout, "%.10lf,", StationVect[i].get_lat() );
//    fprintf( fout, "%.10lf,", StationVect[i].get_lon() );
//    fprintf( fout, "%.2lf,", StationVect[i].get_height() );
//    fprintf( fout, "\"meters\"," );
//    fprintf( fout, "%.2lf,", StationVect[i].get_speed() );
//    fprintf( fout, "\"mps\"," );
//    fprintf( fout, "%.1lf,", StationVect[i].get_direction() );
//    fprintf( fout, "%.1lf,", StationVect[i].get_temperature() );
//    fprintf( fout, "\"K\"," );
//    fprintf( fout, "%.1lf,", StationVect[i].get_cloudCover() * 100 );
//    if( StationVect[i].get_influenceRadius() < 0.0 )
//        fprintf( fout, "%.2lf,", -1.0 );
//    else
//        fprintf( fout, "%.2lf,", StationVect[i].get_influenceRadius() );
//    fprintf( fout, "\"meters\"" );

//    fprintf( fout, "\n" );
//    }

//    fclose( fout );
//    CSLDestroy( papszHeader );
//}
///** Write a point kml file for all the weather stations in stations vector.
// * @param stations A vector of wxStation objects
// * @param outFileName A string for output file
// * @return void
// */
//void wxStation::writeKmlFile( std::vector<wxStation> stations,
//        const std::string outFileName )
//{
//    if( stations.size() == 0 )
//        return;
//    FILE *fout = fopen( outFileName.c_str(), "w" );
//    if( fout == NULL ) {
//        printf( "Cannot open kml file: %s for writing.", outFileName.c_str() );
//        return;
//    }

//    double heightTemp, speedTemp, directionTemp, ccTemp, temperatureTemp, radOfInflTemp;


//    fprintf( fout, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n" );
//    fprintf( fout, "<kml>\n" );
//    fprintf( fout, "  <Document>\n" );
//    fprintf( fout, "    <description>Weather Stations:%s</description>\n",
//            outFileName.c_str() );
//    for( unsigned int i = 0;i < stations.size();i++ ) {

//        heightTemp = stations[i].get_height();
//        speedTemp = stations[i].get_speed();
//        directionTemp = stations[i].get_direction();
//        ccTemp = stations[i].get_cloudCover();
//        temperatureTemp = stations[i].get_temperature();
//        radOfInflTemp = stations[i].get_influenceRadius();

//        lengthUnits::fromBaseUnits(heightTemp, stations[i].heightUnits);
//        //lengthUnits::fromBaseUnits(heightTemp, lengthUnits::feet);
//        velocityUnits::fromBaseUnits(speedTemp, stations[i].inputSpeedUnits);
//        //lengthUnits::fromBaseUnits(directionTemp, heightUnits);
//        coverUnits::fromBaseUnits(ccTemp, stations[i].cloudCoverUnits);
//        temperatureUnits::fromBaseUnits(temperatureTemp, stations[i].tempUnits);
//        lengthUnits::fromBaseUnits(radOfInflTemp, stations[i].influenceRadiusUnits);

//        fprintf( fout, "    <Placemark>\n" );
//        fprintf( fout, "      <name>%s</name\n>",
//                stations[i].get_stationName().c_str() );
//        fprintf( fout, "    <Point>\n" );
//        fprintf( fout, "        <altitudeMode>clampToGround</altitudeMode>\n" );
//        fprintf( fout, "        <coordinates>\n" );
//        fprintf( fout, "          %lf,%lf,0.0\n", stations[i].get_lon(),
//                stations[i].get_lat() );
//        fprintf( fout, "        </coordinates>\n" );
//        fprintf( fout, "      </Point>\n" );
//        fprintf( fout, "      <description>\n" );
//        fprintf( fout, "        <![CDATA[\n" );
//        fprintf( fout, "          Name: %s\n",
//                stations[i].get_stationName().c_str() );
//        fprintf( fout, "          Height: %.2lf %s\n", heightTemp, lengthUnits::getString(stations[i].heightUnits).c_str() );
//        fprintf( fout, "          Speed: %.2lf %s\n", speedTemp, velocityUnits::getString(stations[i].inputSpeedUnits).c_str() );
//        fprintf( fout, "          Direction: %.0lf %s\n",
//                directionTemp, "degrees" );
//        fprintf( fout, "          Cloud Cover: %.2lf %s\n",
//                ccTemp, coverUnits::getString(stations[i].cloudCoverUnits).c_str() );
//        fprintf( fout, "          Temperature: %.1lf %s\n",
//                temperatureTemp, temperatureUnits::getString(stations[i].tempUnits).c_str() );
//        if(stations[i].get_influenceRadius() > 0.0)
//        {
//            fprintf( fout, "          Radius of Influence: %.2lf %s\n",
//                    radOfInflTemp, lengthUnits::getString(stations[i].influenceRadiusUnits).c_str() );
//        }else
//        {
//            fprintf( fout, "          Radius of Influence: infinite\n");
//        }
//        fprintf( fout, "        ]]>\n" );
//        fprintf( fout, "      </description>\n" );
//        fprintf( fout, "    </Placemark>\n" );
//    }
//    fprintf( fout, "  </Document>\n" );
//    fprintf( fout, "</kml>\n" );

//    fclose( fout );
//}


/** Read a csv file to populate a wxStationList vector
 * This funtion uses OGR Spatial Features lib to open the csv file
 * @param csvFile input csv file for reading
 * @param demFile georeferenced dem file for converting lat/lon to projcs
 * @return oStations std::vector of wxStationList objects
 */
std::vector<wxStationList> wxStationList::readStationFetchFile(std::string csvFile,std::string demFile)
//wxStationList wxStationList::readStationFetchFile(std::string csvFile,std::string demFile,int piCount)
{
wxStationList oStation;
std::vector<wxStationList> oStations;
std::string oErrorString = "";


OGRDataSourceH hDS;
hDS = OGROpen( csvFile.c_str(), FALSE, NULL );
if( hDS == NULL )
{
oErrorString = "Cannot open csv file: ";
oErrorString += csvFile;
throw( std::runtime_error( oErrorString ) );
}
OGRFeatureH hFeature;

double dfTempValue = 0.0;
OGRLayer *poLayer;
OGRFeature *poFeature;
OGRFeatureDefn *poFeatureDefn;
OGRFieldDefn *poFieldDefn;
poLayer = (OGRLayer*)OGR_DS_GetLayer( hDS, 0 );

OGRLayerH hLayer;
hLayer=OGR_DS_GetLayer(hDS,0);
OGR_L_ResetReading(hLayer);
int fCount=OGR_L_GetFeatureCount(hLayer,1);

cout<<"Reading csvName: "<<csvFile<<endl;
//cout<<"fCount: "<<fCount<<endl;

const char* station;
int idx=0;

poLayer->ResetReading();

char **papszHeader = NULL;
//papszHeader = getValidHeader();
papszHeader=getValidHeader();
char **papszOldHeader=oldGetValidHeader();

bool fetchType;

poFeatureDefn = poLayer->GetLayerDefn();
//check for correct number of fields, and proper header
int nFields = poFeatureDefn->GetFieldCount();
    if( nFields != CSLCount( papszHeader ) )
    {
        papszHeader=papszOldHeader;
        fetchType=false;
        cout<<"old way"<<endl;

    }
    else if (nFields !=CSLCount (papszHeader))
    {
        OGR_DS_Destroy( hDS );
        oErrorString = "Incorrect number of definitions in csv file. ";
        oErrorString += "There are ";
        oErrorString += nFields;
        oErrorString += " in the file, it needs ";
        oErrorString += CSLCount( papszHeader );
        CSLDestroy( papszHeader );
        throw( std::domain_error( oErrorString ) );
    }
    else
    {
    fetchType=true;
    cout<<"new way"<<endl;
    }

//cout<<fetchType<<endl;

const char *pszKey;
std::string oStationName;
std::string datetime;


poLayer->ResetReading();

while( ( poFeature = poLayer->GetNextFeature() ) != NULL )
{

poFeatureDefn = poLayer->GetLayerDefn();

// get Station name
oStationName = poFeature->GetFieldAsString( 0 );
oStation.set_stationName( oStationName );
//cout<<"stid: "<<oStationName<<endl;

pszKey = poFeature->GetFieldAsString( 1 );

//LAT LON COORDINATES
if( EQUAL( pszKey, "geogcs" ) )
{
//    cout<<"geogcs"<<endl;
    //check for valid latitude in degrees
    dfTempValue = poFeature->GetFieldAsDouble( 3 );
    if( dfTempValue > 90.0 || dfTempValue < -90.0 ) {
    OGRFeature::DestroyFeature( poFeature );
    OGR_DS_Destroy( hDS );
    CSLDestroy( papszHeader );

    oErrorString = "Bad latitude in weather station csv file";
    oErrorString += " at station: ";
    oErrorString += oStationName;

    throw( std::domain_error( oErrorString ) );
    }
    //check for valid longitude in degrees
    dfTempValue = poFeature->GetFieldAsDouble( 4 );

    if( dfTempValue < -180.0 || dfTempValue > 360.0 )
    {
    OGRFeature::DestroyFeature( poFeature );
    OGR_DS_Destroy( hDS );
    CSLDestroy( papszHeader );

    oErrorString = "Bad longitude in weather station csv file";
    oErrorString += " at station: ";
    oErrorString += oStationName;

    throw( std::domain_error( oErrorString ) );
    }
    const char *pszDatum = poFeature->GetFieldAsString( 2 );
    oStation.set_location_LatLong( poFeature->GetFieldAsDouble( 3 ),
                   poFeature->GetFieldAsDouble( 4 ),
                   demFile,
                   pszDatum );
//    cout<<"lat,lon: "<<oStation.lat<<" , "<<oStation.lon<<endl;
}
else if( EQUAL( pszKey, "projcs" ) )
{
    oStation.set_location_projected( poFeature->GetFieldAsDouble( 3 ),
                     poFeature->GetFieldAsDouble( 4 ),
                     demFile );
}
else
{
    oErrorString = "Invalid coordinate system: ";
    oErrorString += poFeature->GetFieldAsString( 1 );
    oErrorString += " at station: ";
    oErrorString += oStationName;

    throw( std::domain_error( oErrorString ) );
}

//MIDDLE STUFF
pszKey = poFeature->GetFieldAsString( 6 );

dfTempValue = poFeature->GetFieldAsDouble( 5 );

if( dfTempValue <= 0.0 )
{
    oErrorString = "Invalid height: ";
    oErrorString += poFeature->GetFieldAsString( 5 );
    oErrorString += " at station: ";
    oErrorString += oStationName;

    throw( std::domain_error( oErrorString ) );
}
if( EQUAL( pszKey, "meters" ) )
{
    oStation.set_height( dfTempValue, lengthUnits::meters );
//    cout<<"height: "<<oStation.height<<", "<<oStation.heightUnits<<endl;
}
else if( EQUAL( pszKey, "feet" ) )
    oStation.set_height( dfTempValue, lengthUnits::feet );
else
{
    oErrorString = "Invalid units for height: ";
    oErrorString += poFeature->GetFieldAsString( 6 );
    oErrorString += " at station: ";
    oErrorString += oStationName;

    throw( std::domain_error( oErrorString ) );
}



//WIND SPEED
pszKey = poFeature->GetFieldAsString( 8 );
dfTempValue = poFeature->GetFieldAsDouble( 7 );
if( dfTempValue < 0.0 )
{
    dfTempValue=0.0;
    oErrorString = "Invalid value for speed: ";
    oErrorString += poFeature->GetFieldAsString( 7 );
    oErrorString += " at station: ";
    oErrorString += oStationName;
    throw( std::domain_error( oErrorString ) );
}

if ( EQUAL( pszKey, "mps" ) )
{
    oStation.set_speed( dfTempValue, velocityUnits::metersPerSecond );
//    cout<<"windspd: "<<oStation.speed<<" , "<<oStation.inputSpeedUnits<<endl;
}
else if( EQUAL( pszKey, "mph" ) )
    oStation.set_speed( dfTempValue, velocityUnits::milesPerHour );
else if( EQUAL( pszKey, "kph" ) )
    oStation.set_speed( dfTempValue,
            velocityUnits::kilometersPerHour );
else
{
    oErrorString = "Invalid units for speed: ";
    oErrorString += poFeature->GetFieldAsString( 8 );
    oErrorString += " at station: ";
    oErrorString += oStationName;
    throw( std::domain_error( oErrorString ) );
}
//WIND DIRECTION
dfTempValue = poFeature->GetFieldAsDouble( 9 );
if( dfTempValue > 360.1 || dfTempValue < 0.0 )
{
    oErrorString = "Invalid value for direction: ";
    oErrorString += poFeature->GetFieldAsString( 9 );
    oErrorString += " at station: ";
    oErrorString += oStationName;
    throw( std::domain_error( oErrorString ) );
}
oStation.set_direction( dfTempValue );
//cout<<"winddir: "<<oStation.direction<<endl;

//TEMPERATURE
pszKey = poFeature->GetFieldAsString( 11 );

if( EQUAL(pszKey, "f" ) )
    oStation.set_temperature( poFeature->GetFieldAsDouble( 10 ),
                  temperatureUnits::F );
else if( EQUAL( pszKey, "c" ) )
{
    oStation.set_temperature( poFeature->GetFieldAsDouble( 10 ),
                  temperatureUnits::C );
//    cout<<"temp (K): "<<oStation.temperature<<endl;
}
else if( EQUAL( pszKey, "k" ) )
    oStation.set_temperature( poFeature->GetFieldAsDouble( 10 ),
                  temperatureUnits::K );
else
{
    oErrorString = "Invalid units for temperature: ";
    oErrorString += poFeature->GetFieldAsString( 11 );
    oErrorString += " at station: ";
    oErrorString += oStationName;
    throw( std::domain_error( oErrorString ) );
}
//CLOUD COVER
dfTempValue = poFeature->GetFieldAsDouble( 12 );
if( dfTempValue > 100.0 || dfTempValue < 0.0 )
{
    oErrorString = "Invalid value for cloud cover: ";
    oErrorString += poFeature->GetFieldAsString( 12 );
    oErrorString += " at station: ";
    oErrorString += oStationName;
//    throw( std::domain_error( oErrorString ) );
//    cout<<oErrorString<<endl;
    dfTempValue=0.0; //TEMPORARY UNTIL SOLRAD IS FIXED
}
oStation.set_cloudCover( dfTempValue, coverUnits::percent );
//cout<<"cloud cover: "<<oStation.cloudCover<<endl;

//RADIUS OF INFLUENCE
pszKey = poFeature->GetFieldAsString( 14 );

dfTempValue = poFeature->GetFieldAsDouble( 13 );

if( EQUAL( pszKey, "miles" ) )
    oStation.set_influenceRadius( dfTempValue, lengthUnits::miles );
else if( EQUAL( pszKey, "feet" ) )
    oStation.set_influenceRadius( dfTempValue, lengthUnits::feet );
else if( EQUAL( pszKey, "km" ) )
    oStation.set_influenceRadius( dfTempValue,
                  lengthUnits::kilometers );
else if( EQUAL( pszKey, "meters" ) )
    oStation.set_influenceRadius( dfTempValue, lengthUnits::meters );
else
{
    oErrorString = "Invalid units for influence radius: ";
    oErrorString += poFeature->GetFieldAsString( 14 );
    oErrorString += " at station: ";
    oErrorString += oStationName;
    throw( std::domain_error( oErrorString ) );
}
//cout<<"influence: "<<oStation.influenceRadius<<endl;

//pszKey=poFeature->GetFieldAsString(15);
//cout<<pszKey<<endl;
datetime=poFeature->GetFieldAsString(15);
std::string trunk=datetime.substr(0,datetime.size()-1);
//cout<<trunk<<endl;

boost::posix_time::ptime abs_time;

boost::posix_time::time_input_facet *fig=new boost::posix_time::time_input_facet;
fig->set_iso_extended_format();
std::istringstream iss(trunk);
iss.imbue(std::locale(std::locale::classic(),fig));
iss>>abs_time;
//cout<<abs_time<<endl;
oStation.set_datetime(abs_time);
//cout<<"datetime: "<<oStation.datetime<<endl;


oStations.push_back( oStation );
oStation.initialize();
}


if (fCount==oStations.size())

{
    cout<<"matched features"<<endl;
//    for (int i=0; i<fCount;i++)
//    {
//        cout<<oStations[i].datetime<<endl;
//    }


}


//cout<<oStations.size()<<endl;
//cout<<fCount<<endl;

OGRFeature::DestroyFeature( poFeature );
OGR_DS_Destroy( hDS );
CSLDestroy( papszHeader );

return oStations;
}


/**Write a csv file with no data, just a header
 * @param outFileName file to write
 */
void wxStationList::writeBlankStationFile( std::string outFileName )
{
    if( outFileName.empty() || outFileName == "" )
    return;
    FILE *fout;
    fout = fopen( outFileName.c_str(), "w" );
    if( fout == NULL )
    return;
    char** papszHeader = NULL;
    papszHeader = getValidHeader();
    for( int i = 0;i < CSLCount( papszHeader ) - 1;i++ )
    fprintf( fout, "\"%s\",", papszHeader[i] );
    fprintf( fout, "\"%s\"\n", papszHeader[CSLCount( papszHeader ) - 1] );
    fclose( fout );
    CSLDestroy( papszHeader );
}

/** Write a csv file representing wxStationList objects
 * @param StationVect std::vector of wxStationList objects
 * @param outFileName output file name as std::string
 */
void wxStationList::writeStationFile( std::vector<wxStationList>StationVect,
                                  std::string outFileName )
{
    if( outFileName.empty() || outFileName == "" )
    return;
    if( StationVect.empty() ) {
    writeBlankStationFile( outFileName );
    return;
    }

    FILE *fout;
    fout = fopen( outFileName.c_str(), "w" );
    if( fout == NULL )
    return;
    char** papszHeader = NULL;
    papszHeader = getValidHeader();
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
    fprintf( fout, "\"meters\"" );

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
void wxStationList::writeKmlFile( std::vector<wxStationList> stations,
        const std::string outFileName )
{
    if( stations.size() == 0 )
        return;
    FILE *fout = fopen( outFileName.c_str(), "w" );
    if( fout == NULL ) {
        printf( "Cannot open kml file: %s for writing.", outFileName.c_str() );
        return;
    }

    double heightTemp, speedTemp, directionTemp, ccTemp, temperatureTemp, radOfInflTemp;


    fprintf( fout, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n" );
    fprintf( fout, "<kml>\n" );
    fprintf( fout, "  <Document>\n" );
    fprintf( fout, "    <description>Weather Stations:%s</description>\n",
            outFileName.c_str() );
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








