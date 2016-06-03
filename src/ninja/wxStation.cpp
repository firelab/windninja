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
#include "ogr_api.h"
#include "sstream"
#include "algorithm"
#include "iterator"
#include "string"
#include "iostream"
#include "fstream"

/**
 * Default constructor for wxStation class populated with default values
 * @see wxStation::initialize
 */

#define dtoken "33e3c8ee12dc499c86de1f2076a9e9d4"
#define dstation "kmso" //Missoula International Airport
#define altstation "TR266" //FIRELAB Fire Raws

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



// builds the time component of the url
string wxStation::sand(std::string year_0,std::string month_0, std::string day_0,std::string clock_0,std::string year_1,std::string month_1,std::string day_1,std::string clock_1)
{
    std::string start;
    std::string end;
    std::string y20;
    std::string m20;
    std::string d20;
    std::string c20;
    std::string y21;
    std::string m21;
    std::string d21;
    std::string c21;
    std::string twofull;
    std::string timemainfull;
    std::string estartfull;
    std::string eendfull;
    std::string startfull;
    std::string endfull;

    start="&start=";
    y20="2016";
    m20="05";
    d20="22";
    c20="1000";
    estartfull=start+y20+m20+d20+c20;
    end="&end=";
    y21="2016";
    m21="05";
    d21="23";
    c21="1000";
    eendfull=end+y21+m21+d21+c21;

    twofull=estartfull+eendfull;

    startfull=start+year_0+month_0+day_0+clock_0;
    endfull=end+year_1+month_1+day_1+clock_1;

    timemainfull=startfull+endfull;

    return timemainfull;
}

// builds the rest of the url+time
const char* wxStation::singlebuilder(std::string token, std::string station_id, std::string svar,std::string yearx,std::string monthx, std::string dayx,std::string clockx,std::string yeary,std::string monthy,std::string dayy,std::string clocky)
{
   std::string etoken;
   std::string eburl;
   std::string etokfull;
   std::string estid;
   std::string et01;
   std::string et11;
   std::string esvar;
   std::string eurl;
   std::string verify;

   // default url

   etoken="33e3c8ee12dc499c86de1f2076a9e9d4";
   eburl="http://api.mesowest.net/v2/stations/timeseries?";
   etokfull="&token="+token;
   estid="stid=kmso";
   et01="&start=201605221000";
   et11="&end=201605231000";
   esvar="&vars=wind_speed";
   eurl=eburl+estid+esvar+et01+et11+etokfull;

   const char* a=eurl.c_str();

   std::string url;
   std::string tokfull;
   std::string stidfull;
   std::string svarfull;
   std::string timestart1;
   std::string timestop1;
   std::string timefull;
   std::string timesand;
   std::string output;

   timesand=wxStation::sand(yearx,monthx,dayx,clockx,yeary,monthy,dayy,clocky);

   tokfull="&token="+token;
   stidfull="stid="+station_id;
   svarfull="&vars="+svar;
   output="&output=geojson";


   url=eburl+stidfull+svarfull+timesand+output+tokfull;

   const char* charurl=url.c_str();

   return charurl;

}
const char* wxStation::multibuilder(std::string token,std::string station_ids,std::string svar,std::string yearx,std::string monthx, std::string dayx,std::string clockx,std::string yeary,std::string monthy,std::string dayy,std::string clocky)
{
    std::string eburl;
    std::string url;
    std::string tokfull;
    std::string stidfull;
    std::string svarfull;
    std::string timesand;
    std::string output;

    timesand=wxStation::sand(yearx,monthx,dayx,clockx,yeary,monthy,dayy,clocky);

    eburl="http://api.mesowest.net/v2/stations/timeseries?";
    tokfull="&token="+token;
    stidfull="stid="+station_ids;
    svarfull="&vars="+svar;
    output="&output=geojson";


    url=eburl+stidfull+svarfull+timesand+output+tokfull;

    const char* charurl=url.c_str();

    return charurl;
}
const char* wxStation::radiusbuilder(std::string token, std::string station_id, std::string radius,std::string limit,std::string svar,std::string yearx,std::string monthx, std::string dayx,std::string clockx,std::string yeary,std::string monthy,std::string dayy,std::string clocky)
{
    std::string eburl;
    std::string url;
    std::string tokfull;
    std::string stidfull;
    std::string svarfull;
    std::string timesand;
    std::string output;
    std::string limiter;

    timesand=wxStation::sand(yearx,monthx,dayx,clockx,yeary,monthy,dayy,clocky);

    eburl="http://api.mesowest.net/v2/stations/timeseries?";
    limiter="&limit="+limit;
    tokfull="&token="+token;
    stidfull="stid="+station_id+","+radius;
    svarfull="&vars="+svar;
    output="&output=geojson";


    url=eburl+stidfull+svarfull+limiter+timesand+output+tokfull;

    const char* charurl=url.c_str();

    return charurl;
}
const char* wxStation::latlonrad(std::string token,std::string lat, std::string lon, std::string radius, std::string limit,std::string svar,std::string yearx,std::string monthx, std::string dayx,std::string clockx,std::string yeary,std::string monthy,std::string dayy,std::string clocky)
{
    std::string eburl;
    std::string url;
    std::string tokfull;
    std::string stidfull;
    std::string svarfull;
    std::string timesand;
    std::string output;
    std::string limiter;

    timesand=wxStation::sand(yearx,monthx,dayx,clockx,yeary,monthy,dayy,clocky);

    eburl="http://api.mesowest.net/v2/stations/timeseries?";
    limiter="&limit="+limit;
    tokfull="&token="+token;
    stidfull="&radius"+lat+","+lon+","+radius;
    svarfull="&vars="+svar;
    output="&output=geojson";


    url=eburl+stidfull+svarfull+limiter+timesand+output+tokfull;

    const char* charurl=url.c_str();

    return charurl;
}
const char* wxStation::bboxbuilder(std::string token,std::string lat1,std::string lon1, std::string lat2, std::string lon2,std::string svar,std::string yearx,std::string monthx, std::string dayx,std::string clockx,std::string yeary,std::string monthy,std::string dayy,std::string clocky)
{
    std::string eburl;
    std::string url;
    std::string tokfull;
    std::string bbox;
    std::string svarfull;
    std::string timesand;
    std::string output;


    timesand=wxStation::sand(yearx,monthx,dayx,clockx,yeary,monthy,dayy,clocky);

    eburl="http://api.mesowest.net/v2/stations/timeseries?";
    tokfull="&token="+token;
    bbox="&bbox="+lon1+","+lat1+","+lon2+","+lat2;
    svarfull="&vars="+svar;
    output="&output=geojson";


    url=eburl+bbox+svarfull+timesand+output+tokfull;

    const char* charurl=url.c_str();

    return charurl;
}

vector<string> wxStation::split(char* str,const char* delim)
{
    char* saveptr;
    char* token = strtok_r(str,delim,&saveptr);

    vector<string> result;

    while(token != NULL)
    {
        result.push_back(token);
        token = strtok_r(NULL,delim,&saveptr);
    }
    return result;
}

void wxStation::chameleon(const char* url)
{


}

void wxStation::nardis(char **redman, int counter, std::string name)
{
    cout<<name<<endl;
    for(int i=0;i<counter;i++)
    {
        printf("%s, ",redman[i]);
    }
    printf("\n");
    cout<<"count of char: "<<counter<<endl;
    printf("\n");


}


void wxStation::cataclysm(const double *data, int counter,std::string name)
{
        cout<<"("<<name<<")"<<endl;
        for(int i=0;i<counter;i++)
        {
            printf("%.3f, ",data[i]);
        }
        printf("\n");
        cout<<"count of doubles: "<<counter<<endl;
        printf("\n");

}


void wxStation::singlestation_fetch(std::string station_id, int nHours)
{

    CPLDebug("STATION_FETCH", "station_id = %s", station_id.c_str());
    CPLDebug("STATION_FETCH", "nHours = %d", nHours);

    const char* newtest = "testfile";
    printf("\n");
    const char* pszvtry=wxStation::singlebuilder(dtoken,altstation,"wind_speed,wind_direction,air_temp,solar_radiation","2016","06","01","1200","2016","06","02","1200");
    cout<<pszvtry<<endl;
    printf("\n");

    OGRDataSourceH hDS;
    OGRLayerH hLayer;
    OGRFeatureH hFeature;

    hDS=OGROpen(pszvtry,0,NULL);
//    hDS=GDALOpenEx(pszvtry,GDAL_OF_ALL,NULL,NULL,NULL);
    if (hDS==NULL)
        printf("miserable failure \n");


//    hLayer = OGR_DS_GetLayerByName(hDS,"OGRGeoJSON");
    hLayer=OGR_DS_GetLayer(hDS,0);
    OGR_L_ResetReading(hLayer);

//    while( (hFeature = OGR_L_GetNextFeature(hLayer)) != NULL )
//    {
//        OGRFeatureDefnH hFDefn;
//        int iField;
//        OGRGeometryH hGeometry;

//        hFDefn = OGR_L_GetLayerDefn(hLayer);

//        for( iField = 0; iField < OGR_FD_GetFieldCount(hFDefn); iField++ )
//        {
//            OGRFieldDefnH hFieldDefn = OGR_FD_GetFieldDefn( hFDefn, iField );

//            if( OGR_Fld_GetType(hFieldDefn) == OFTInteger )
//                printf( "%d,", OGR_F_GetFieldAsInteger( hFeature, iField ) );
//            else if( OGR_Fld_GetType(hFieldDefn) == OFTReal )
//                printf( "%.3f,", OGR_F_GetFieldAsDouble( hFeature, iField) );
//            else if( OGR_Fld_GetType(hFieldDefn) == OFTString )
//                printf( "%s,", OGR_F_GetFieldAsString( hFeature, iField) );
//            else
//                printf( "%s,", OGR_F_GetFieldAsString( hFeature, iField) );
//        }

//        hGeometry = OGR_F_GetGeometryRef(hFeature);
//        if( hGeometry != NULL
//            && wkbFlatten(OGR_G_GetGeometryType(hGeometry)) == wkbPoint )
//        {
//            printf( "%.3f,%3.f\n", OGR_G_GetX(hGeometry, 0), OGR_G_GetY(hGeometry, 0) );
//        }
//        else
//        {
//            printf( "no point geometry\n");

//        }
//    }



    int idx=0;
    int idx2=0;
    int idx3=0;
    int idx4=0;
    int idx5=0;
    int idx6=0;
    int idx7=0;
    int idx8=0;
    int count1=0;
    int count2=0;
    int count3=0;
    int count4=0;
    int countstat=0;
    int countlat=0;
    int countlon=0;
    int countdate=0;
    const double *windspd;
    const double *winddir;
    const double *airtemp;
    const double *solrad;
    double latitude;
    double longitude;
    std::string station;
    char **datetime;


//    const double *data;

    while((hFeature=OGR_L_GetNextFeature(hLayer))!=NULL)
    {
//       idx=OGR_F_GetFieldIndex(hFeature,"wind_speed");
//       data=OGR_F_GetFieldAsDoubleList(hFeature,idx,&count);

        idx=OGR_F_GetFieldIndex(hFeature,"wind_speed");
        windspd=OGR_F_GetFieldAsDoubleList(hFeature,idx,&count1);

        idx2=OGR_F_GetFieldIndex(hFeature,"wind_direction");
        winddir=OGR_F_GetFieldAsDoubleList(hFeature,idx2,&count2);

        idx3=OGR_F_GetFieldIndex(hFeature,"air_temp");
        airtemp=OGR_F_GetFieldAsDoubleList(hFeature,idx3,&count3);

        idx4=OGR_F_GetFieldIndex(hFeature,"solar_radiation");
        solrad=OGR_F_GetFieldAsDoubleList(hFeature,idx4,&count4);

        idx5=OGR_F_GetFieldIndex(hFeature, "LATITUDE");
        latitude=OGR_F_GetFieldAsDouble(hFeature,idx5);

        idx6=OGR_F_GetFieldIndex(hFeature,"LONGITUDE");
        longitude=OGR_F_GetFieldAsDouble(hFeature,idx6);

        idx7=OGR_F_GetFieldIndex(hFeature,"NAME");
        station=OGR_F_GetFieldAsString(hFeature,idx7);

        idx8=OGR_F_GetFieldIndex(hFeature,"date_times");
        datetime=OGR_F_GetFieldAsStringList(hFeature,idx8);
        //note: Mesowest data downloaded as GEOJson names date n time data as "date_times", Mewsowest data using
        // Json names time data as date_time!!!!!!


    }
    printf("\n\n");


    cataclysm(windspd,count1,"wind_speed");
    cataclysm(winddir,count2,"wind_direction");
    cataclysm(airtemp,count3,"air_temp");
    cataclysm(solrad,count4,"solar_radiation");
    nardis(datetime,count1,"date_time");

    cout<<"LATITUDE: "<<latitude<<endl;
    cout<<"LONGITUDE: "<<longitude<<endl;
    cout<<"STATION NAME: "<<station<<endl;


ofstream tsetse;
tsetse.open("singlestation_fetch.csv");
tsetse << "Station_Name,Coord_Sys,DATUM(WGS84),Lat/YCoord,Lon/XCoord,Height,Height_Units,Speed,Speed_Units,Direction(degrees),Tempertaure,Temperature_Units,Cloud_Cover(%),Radius_of_influence,Radius_of_influence_Units,date_time"<<endl;

tsetse<<station<<",GEOCS,"<<"WGS84,"<<latitude<<","<<longitude<<",10,"<<"m,"<<windspd[0]<<",m/s,"<<winddir[0]<<","<<airtemp[0]<<",C,"<<solrad[0]<<",,,"<<datetime[0]<<endl;

for(int q=1;q<count1;q++)
{
    tsetse<<",,,,,,,"<<windspd[q]<<",,"<<winddir[q]<<","<<airtemp[q]<<",,"<<solrad[q]<<",,,"<<datetime[q]<<endl;
}


}

void wxStation::multistation_fetch(std::string station_id, int nHours)
{

}
void wxStation::radiusstation_fetch(std::string station_id, int nHours)
{

}
void wxStation::radiuslatlon_fetch(std::string station_id, int nHours)
{

}
void wxStation::bbox_fetch(std::string station_id, int nHours)
{

}














/**
 * Initialize a wxStation object with default values
 */
void wxStation::initialize()
{
    stationName = "";
    lat = lon = 0.0;
    datumType = wxStation::WGS84;
    coordType = wxStation::GEOGCS;
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
}

/** Checks wxStation for valid parameters.
 * @return True if wxStation parameters are OK.
 */
bool wxStation::check_station()
{
    if(stationName.empty() || stationName == "")
        return false;

    if(coordType != GEOGCS && coordType != PROJCS)
        return false;

    if(datumType != WGS84 && datumType != NAD83 && datumType !=NAD27)
        return false;

    if(lat < -90.0 || lat > 90.0)
        return false;

    if(lon < -180.0 || lon > 360.0)
        return false;

    if(height < 0.0)
        return false;

    if(speed < 0.0)
        return false;

    if(direction < 0.0 || direction > 360.0)
        return false;

    if(w_speed < 0.0)
        return false;

    return true;
}


/**
 * Fetch a valid header associated with values contained in wxStation.
 * This is for displaying data and contains extra tags that represent
 * choices in units and other quantities.
 * @return array of strings representing the header
 */

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
    return papszList;
}

/**
 * Fetch the height of the wxStation.  Default height is meters
 * @param units to retrieve the height in
 * @return height of station
 */
double wxStation::get_height( lengthUnits::eLengthUnits units )
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
double wxStation::get_speed( velocityUnits::eVelocityUnits units )
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
double wxStation::get_w_speed( velocityUnits::eVelocityUnits units )
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
double wxStation::get_temperature( temperatureUnits::eTempUnits units )
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
double wxStation::get_cloudCover( coverUnits::eCoverUnits units )
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
double wxStation::get_influenceRadius( lengthUnits::eLengthUnits units )
{
    double i = influenceRadius;
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
    height = Height;
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
    speed = Speed;
    inputSpeedUnits = units;
}

void wxStation::set_direction( double Direction )
{
    direction = Direction;
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
    temperature = Temperature;
    tempUnits = units;
}

void wxStation::set_cloudCover( double CloudCover,
                coverUnits::eCoverUnits units )
{
    coverUnits::toBaseUnits( CloudCover, units );
    cloudCover = CloudCover;
    cloudCoverUnits = units;
}

void wxStation::set_influenceRadius( double InfluenceRadius,
                     lengthUnits::eLengthUnits units )
{
    lengthUnits::toBaseUnits( InfluenceRadius, units );
    influenceRadius = InfluenceRadius;
    influenceRadiusUnits = units;
}

/** Read a csv file to populate a wxStation vector
 * This funtion uses OGR Spatial Features lib to open the csv file
 * @param csvFile input csv file for reading
 * @param demFile georeferenced dem file for converting lat/lon to projcs
 * @return oStations std::vector of wxStation objects
 */
std::vector<wxStation> wxStation::readStationFile( std::string csvFile,
                           std::string demFile )
{
    wxStation oStation;
    std::vector<wxStation> oStations;
    std::string oErrorString = "";

    OGRDataSourceH hDS;
    hDS = OGROpen( csvFile.c_str(), FALSE, NULL );
    if( hDS == NULL ) {
    oErrorString = "Cannot open csv file: ";
    oErrorString += csvFile;
    throw( std::runtime_error( oErrorString ) );
    }

    double dfTempValue = 0.0;
    OGRLayer *poLayer;
    OGRFeature *poFeature;
    OGRFeatureDefn *poFeatureDefn;
    OGRFieldDefn *poFieldDefn;
    poLayer = (OGRLayer*)OGR_DS_GetLayer( hDS, 0 );
    //make sure layer exists
    if( poLayer == NULL ) {
    oErrorString = "Cannot open layer in csv file: ";
    oErrorString += csvFile;
    throw( std::domain_error( oErrorString ) );
    }

    poLayer->ResetReading();

    char **papszHeader = NULL;
    papszHeader = getValidHeader();

    poFeatureDefn = poLayer->GetLayerDefn();
    //check for correct number of fields, and proper header
    int nFields = poFeatureDefn->GetFieldCount();
    if( nFields != CSLCount( papszHeader ) ) {
        OGR_DS_Destroy( hDS );
        oErrorString = "Incorrect number of definitions in csv file. ";
        oErrorString += "There are ";
        oErrorString += nFields;
        oErrorString += " in the file, it needs ";
        oErrorString += CSLCount( papszHeader );
        CSLDestroy( papszHeader );
        throw( std::domain_error( oErrorString ) );
    }

    while( ( poFeature = poLayer->GetNextFeature() ) != NULL ) {
        poFeatureDefn = poLayer->GetLayerDefn();
        int iField;
        for( iField = 0; iField < poFeatureDefn->GetFieldCount(); iField++) {
            poFieldDefn = poFeatureDefn->GetFieldDefn( iField );
            if( !EQUAL( poFieldDefn->GetNameRef(),
                papszHeader[iField] ) ) {
                OGRFeature::DestroyFeature( poFeature );
                OGR_DS_Destroy( hDS );
                CSLDestroy( papszHeader );
                //this one isn't working????
                throw( std::domain_error( "Incorrect header definition in " \
                                          "csv file" ) );
            }
        }
    }

    const char *pszKey;
    std::string oStationName;

    poLayer->ResetReading();
    while( ( poFeature = poLayer->GetNextFeature() ) != NULL ) {
    poFeatureDefn = poLayer->GetLayerDefn();

    // get Station name
    oStationName = poFeature->GetFieldAsString( 0 );
    oStation.set_stationName( oStationName );

    // get location
    pszKey = poFeature->GetFieldAsString( 1 );

    if( EQUAL( pszKey, "geogcs" ) ){
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
        if( dfTempValue < -180.0 || dfTempValue > 360.0 ) {
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
    }

    else if( EQUAL( pszKey, "projcs" ) ) {
        oStation.set_location_projected( poFeature->GetFieldAsDouble( 3 ),
                         poFeature->GetFieldAsDouble( 4 ),
                         demFile );
    }
    else {
        oErrorString = "Invalid coordinate system: ";
        oErrorString += poFeature->GetFieldAsString( 1 );
        oErrorString += " at station: ";
        oErrorString += oStationName;

        throw( std::domain_error( oErrorString ) );
    }

    //get height and units

    pszKey = poFeature->GetFieldAsString( 6 );

    dfTempValue = poFeature->GetFieldAsDouble( 5 );

    if( dfTempValue <= 0.0 ) {
        oErrorString = "Invalid height: ";
        oErrorString += poFeature->GetFieldAsString( 5 );
        oErrorString += " at station: ";
        oErrorString += oStationName;

        throw( std::domain_error( oErrorString ) );
    }
    if( EQUAL( pszKey, "meters" ) )
        oStation.set_height( dfTempValue, lengthUnits::meters );
    else if( EQUAL( pszKey, "feet" ) )
        oStation.set_height( dfTempValue, lengthUnits::feet );
    else {
        oErrorString = "Invalid units for height: ";
        oErrorString += poFeature->GetFieldAsString( 6 );
        oErrorString += " at station: ";
        oErrorString += oStationName;

        throw( std::domain_error( oErrorString ) );
    }

    // get speed and speed units
    pszKey = poFeature->GetFieldAsString( 8 );

    dfTempValue = poFeature->GetFieldAsDouble( 7 );
    if( dfTempValue < 0.0 ) {
        oErrorString = "Invalid value for speed: ";
        oErrorString += poFeature->GetFieldAsString( 7 );
        oErrorString += " at station: ";
        oErrorString += oStationName;
        throw( std::domain_error( oErrorString ) );
    }

    if( EQUAL( pszKey, "mph" ) )
        oStation.set_speed( dfTempValue, velocityUnits::milesPerHour );
    else if( EQUAL( pszKey, "kph" ) )
        oStation.set_speed( dfTempValue,
                velocityUnits::kilometersPerHour );
    else if ( EQUAL( pszKey, "mps" ) )
        oStation.set_speed( dfTempValue, velocityUnits::metersPerSecond );
    else {
        oErrorString = "Invalid units for speed: ";
        oErrorString += poFeature->GetFieldAsString( 8 );
        oErrorString += " at station: ";
        oErrorString += oStationName;
        throw( std::domain_error( oErrorString ) );
    }

    //set direction
    dfTempValue = poFeature->GetFieldAsDouble( 9 );
    if( dfTempValue > 360.1 || dfTempValue < 0.0 ) {
        oErrorString = "Invalid value for direction: ";
        oErrorString += poFeature->GetFieldAsString( 9 );
        oErrorString += " at station: ";
        oErrorString += oStationName;
        throw( std::domain_error( oErrorString ) );
    }
    oStation.set_direction( dfTempValue );

    //set temperature
    pszKey = poFeature->GetFieldAsString( 11 );

    if( EQUAL(pszKey, "f" ) )
        oStation.set_temperature( poFeature->GetFieldAsDouble( 10 ),
                      temperatureUnits::F );
    else if( EQUAL( pszKey, "c" ) )
        oStation.set_temperature( poFeature->GetFieldAsDouble( 10 ),
                      temperatureUnits::C );
    else if( EQUAL( pszKey, "k" ) )
        oStation.set_temperature( poFeature->GetFieldAsDouble( 10 ),
                      temperatureUnits::K );
    else {
        oErrorString = "Invalid units for temperature: ";
        oErrorString += poFeature->GetFieldAsString( 11 );
        oErrorString += " at station: ";
        oErrorString += oStationName;
        throw( std::domain_error( oErrorString ) );
    }

    //set cloud cover
    dfTempValue = poFeature->GetFieldAsDouble( 12 );
    if( dfTempValue > 100.0 || dfTempValue < 0.0 ) {
        oErrorString = "Invalid value for cloud cover: ";
        oErrorString += poFeature->GetFieldAsString( 12 );
        oErrorString += " at station: ";
        oErrorString += oStationName;
        throw( std::domain_error( oErrorString ) );
    }
    oStation.set_cloudCover( dfTempValue, coverUnits::percent );

    //set radius of influence
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
    else {
        oErrorString = "Invalid units for influence radius: ";
        oErrorString += poFeature->GetFieldAsString( 14 );
        oErrorString += " at station: ";
        oErrorString += oStationName;
        throw( std::domain_error( oErrorString ) );
    }

    oStations.push_back( oStation );
    oStation.initialize();
    }

    OGRFeature::DestroyFeature( poFeature );
    OGR_DS_Destroy( hDS );
    CSLDestroy( papszHeader );

    return oStations;
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
    papszHeader = getValidHeader();
    for( int i = 0;i < CSLCount( papszHeader ) - 1;i++ )
    fprintf( fout, "\"%s\",", papszHeader[i] );
    fprintf( fout, "\"%s\"\n", papszHeader[CSLCount( papszHeader ) - 1] );
    fclose( fout );
    CSLDestroy( papszHeader );
}

/** Write a csv file representing wxStation objects
 * @param StationVect std::vector of wxStation objects
 * @param outFileName output file name as std::string
 */
void wxStation::writeStationFile( std::vector<wxStation>StationVect,
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
 * @param stations A vector of wxStation objects
 * @param outFileName A string for output file
 * @return void
 */
void wxStation::writeKmlFile( std::vector<wxStation> stations,
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
