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

#ifndef	WX_STATION_H
#define WX_STATION_H

#include <stdio.h>
#include <vector>
#include <string>

#include "gdal_util.h"
#include "gdal_priv.h"
#include "ogr_spatialref.h"
#include "ogrsf_frmts.h"
#include "cpl_conv.h"
#include "cpl_string.h"
#include "ninjaUnits.h"
#include "ninjaException.h"

#include "cpl_port.h"
#include "cpl_error.h"
#include "cpl_http.h"
#include "cpl_multiproc.h"

#ifndef Q_MOC_RUN
#include "boost/date_time/posix_time/posix_time.hpp"
#include "boost/date_time/posix_time/posix_time_io.hpp"
#include "boost/date_time/local_time_adjustor.hpp"
#include "boost/date_time/gregorian/gregorian.hpp"
#include "boost/date_time/local_time/local_time.hpp"
#endif

#include "iostream"
#include "numeric"
#include "math.h"
#include "cplIsNan.h"

static const char *apszValidHeader1[] = {
    "Station_Name", "Coord_Sys(PROJCS,GEOGCS)", "Datum(WGS84,NAD83,NAD27)",
    "Lat/YCoord", "Lon/XCoord", "Height", "Height_Units(meters,feet)", "Speed",
    "Speed_Units(mph,kph,mps,kts)", "Direction(degrees)", "Temperature",
    "Temperature_Units(F,C)", "Cloud_Cover(%)", "Radius_of_Influence",
    "Radius_of_Influence_Units(miles,feet,meters,km)", NULL};

static const char *apszValidHeader2[] = {
    "Station_Name", "Coord_Sys(PROJCS,GEOGCS)", "Datum(WGS84,NAD83,NAD27)",
    "Lat/YCoord", "Lon/XCoord", "Height", "Height_Units(meters,feet)", "Speed",
    "Speed_Units(mph,kph,mps,kts)", "Direction(degrees)", "Temperature",
    "Temperature_Units(F,C)", "Cloud_Cover(%)", "Radius_of_Influence",
    "Radius_of_Influence_Units(miles,feet,meters,km)", "date_time", NULL};

//static const char *apszValidHeader3[] = {
//    "Station_File_List", NULL};


/** Class representing a weather station
 */
class wxStation
{
 public:

    wxStation();	    //Default constructor
    void initialize();      //Initializes to default values
    ~wxStation();	    //Destructor

    wxStation( wxStation const& m );               // Copy constructor
    wxStation& operator= ( wxStation const& m );   // Assignment operator

    enum eCoordType { PROJCS, GEOGCS };
    enum eDatumType { WGS84, NAD83, NAD27 };
    enum eStationFormat{ oldFormat, newFormat, invalidFormat };

    static bool check_station(wxStation station);
    static bool fix_direction(wxStation station); //Fixes a failed direction check by zeroing out the direction
    //This is likely temporary until I figure out what is going on.

    void set_stationName( std::string Name );
    inline std::string get_stationName() { return stationName; }
    void set_location_projected( double Xord, double Yord, std::string demFile );
    inline double get_projXord() { return projXord; }
    inline double get_projYord() { return projYord; }
    inline double get_xord() { return xord; }
    inline double get_yord() { return yord; }
    void set_location_LatLong( double Lat, double Lon, std::string demFile,
                   const char *datum );
    inline double get_lon() { return lon; }
    inline double get_lat() { return lat; }
    void set_height( double Height, lengthUnits::eLengthUnits units );
    double get_height(lengthUnits::eLengthUnits units = lengthUnits::meters);
    void set_speed( double Speed, velocityUnits::eVelocityUnits units );
    double get_speed(velocityUnits::eVelocityUnits units = velocityUnits::metersPerSecond);
    void set_direction( double Direction );
    double get_direction();
    void set_w_speed( double W_Speed, velocityUnits::eVelocityUnits units );
    double get_w_speed(velocityUnits::eVelocityUnits units = velocityUnits::metersPerSecond);
    void set_temperature(double Temperature, temperatureUnits::eTempUnits units);
    double get_temperature(temperatureUnits::eTempUnits units = temperatureUnits::K);
    void set_cloudCover( double CloudCover, coverUnits::eCoverUnits units );
    double get_cloudCover(coverUnits::eCoverUnits units = coverUnits::fraction);
    void set_influenceRadius( double InfluenceRadius, lengthUnits::eLengthUnits units );
    double get_influenceRadius(lengthUnits::eLengthUnits units = lengthUnits::meters);

    void set_datetime(boost::posix_time::ptime timedata);
    boost::posix_time::ptime get_datetime(int idx);
    void set_localDateTime(boost::local_time::local_date_time timedata);
    boost::local_time::local_date_time get_localDateTime(int idx);
    boost::local_time::local_date_time get_currentTimeStep();
    void set_currentTimeStep(boost::local_time::local_date_time step);

    void update_speed( double Speed, velocityUnits::eVelocityUnits units, int index );
    void update_direction(double direction, int index);
    int get_listSize();

    static void wxPrinter(wxStation wxObject);
    static void wxVectorPrinter(std::vector<wxStation> wxObject, int count);

    inline eCoordType get_coordType() { return coordType; }
    inline void set_coordType( eCoordType c ){ coordType = c; }
    inline eDatumType get_datumType() { return datumType; }
    inline void set_datumType( eDatumType d ) { datumType = d; }

    inline lengthUnits::eLengthUnits get_heightUnits() { return heightUnits; }
    inline velocityUnits::eVelocityUnits get_speedUnits() { return inputSpeedUnits; }
    inline velocityUnits::eVelocityUnits get_w_speedUnits() { return w_speedUnits; }
    inline temperatureUnits::eTempUnits get_tempUnits(){ return tempUnits; }
    inline coverUnits::eCoverUnits get_cloudCoverUnits() { return cloudCoverUnits; }
    inline lengthUnits::eLengthUnits get_influenceRadiusUnits() { return influenceRadiusUnits; }

    static void stationViewer(wxStation station);

    static void writeKmlFile( std::vector<wxStation> stations,
                  std::string demFileName,std::string basePath, velocityUnits::eVelocityUnits velUnits);
    static void writeKMZFile(std::vector<wxStation> stations,std::string basePath, std::string demFileName);
    static vector<std::string> stationKmlNames;

    static void writeStationFile( std::vector<wxStation> StationVect,
                  const std::string outFileName );

    static void writeBlankStationFile( std::string outFileName );

    static int GetHeaderVersion(const char *pszFilename);
    static int GetFirstStationLine(const char *xFilename);
    static void SetStationFormat(eStationFormat format);
    static wxStation::eStationFormat GetStationFormat();

    static eStationFormat stationFormat;

 private:

    std::string stationName;
    double lat, lon;           //latitude and longitude
    double projXord;           //x-coordinate of station (in projected coordinate system)
    double projYord;	       //y-coordinate of station (in projected coordinate system)
    double xord;	       //x-cordinate of station (in ninja coordinate system)
    double yord;	       //y-cordinate of station (in ninja coordinate system)

    //data are stored as vectors, each entry is a time step
    vector<double> heightList;	       //in m (above vegetation)
    vector<double> speedList;	       //in m/s (value < 0 indicates no value available)
    vector<double> directionList;	       //in degrees from north
    vector<double> temperatureList;	       //in K (value < -1000 indicates no value available)
    vector<double> cloudCoverList;	       //in fractions of cloud cover (0-1) (value < 0 indicates no value available)
    // *Note: Value < 0 is a flag indicating influence radius is infinity (ie. influences everything)
    vector<double> influenceRadiusList;    //maximum radius that the station is used for during interpolation (m)

    double w_speed;	       //vertical velocity in m/s

    //Unit enums
    lengthUnits::eLengthUnits heightUnits;
    velocityUnits::eVelocityUnits inputSpeedUnits;
    velocityUnits::eVelocityUnits w_speedUnits;
    temperatureUnits::eTempUnits tempUnits;
    coverUnits::eCoverUnits cloudCoverUnits;
    lengthUnits::eLengthUnits influenceRadiusUnits;
    eDatumType datumType;
    eCoordType coordType;

    boost::local_time::local_date_time currentTime;

    //this is UTC and is used to match data points from MesoWest and Interpolation.
    vector<boost::posix_time::ptime> datetimeList; 
    vector<boost::local_time::local_date_time> localDateTimeList;

    friend class Stability;
    friend class pointInitialization;
};


#endif /* WX_STATION_H */
