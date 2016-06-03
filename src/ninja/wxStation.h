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

    enum eCoordType { PROJCS,
		      GEOGCS };
    enum eDatumType { WGS84,
		      NAD83,
		      NAD27 };

    string sand(std::string year_0,std::string month_0, std::string day_0,std::string clock_0,std::string year_1,std::string month_1,std::string day_1,std::string clock_1);
    const char* singlebuilder(std::string token, std::string station_id, std::string svar,std::string yearx,std::string monthx, std::string dayx,std::string clockx,std::string yeary,std::string monthy,std::string dayy,std::string clocky);
    const char* multibuilder(std::string token,std::string station_ids,std::string svar,std::string yearx,std::string monthx, std::string dayx,std::string clockx,std::string yeary,std::string monthy,std::string dayy,std::string clocky);
    const char* radiusbuilder(std::string token, std::string staion_id, std::string radius,std::string limit,std::string svar,std::string yearx,std::string monthx, std::string dayx,std::string clockx,std::string yeary,std::string monthy,std::string dayy,std::string clocky);
    const char* latlonrad(std::string token,std::string lat, std::string lon, std::string radius, std::string limit,std::string svar,std::string yearx,std::string monthx, std::string dayx,std::string clockx,std::string yeary,std::string monthy,std::string dayy,std::string clocky);
    const char* bboxbuilder(std::string token,std::string lat1,std::string lon1, std::string lat2, std::string lon2,std::string svar,std::string yearx,std::string monthx, std::string dayx,std::string clockx,std::string yeary,std::string monthy,std::string dayy,std::string clocky);
    vector<string> split(char* str,const char* delim);
    void chameleon(const char* url);
    void nardis(char **redman, int counter, std::string name);
    void cataclysm(const double *data, int counter,std::string name);

    void singlestation_fetch(std::string station_id, int nHours);
    void multistation_fetch(std::string station_id, int nHours);
    void radiusstation_fetch(std::string station_id, int nHours);
    void radiuslatlon_fetch(std::string station_id, int nHours);
    void bbox_fetch(std::string station_id, int nHours);

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
    double get_height( lengthUnits::eLengthUnits units = lengthUnits::meters );
    void set_speed( double Speed, velocityUnits::eVelocityUnits units );
    double get_speed( velocityUnits::eVelocityUnits units = velocityUnits::metersPerSecond);
    void set_direction( double Direction );
    inline double get_direction() { return direction; }
    void set_w_speed( double W_Speed, velocityUnits::eVelocityUnits units );
    double get_w_speed(velocityUnits::eVelocityUnits units = velocityUnits::metersPerSecond );
    void set_temperature(double Temperature, temperatureUnits::eTempUnits units );
    double get_temperature(temperatureUnits::eTempUnits units = temperatureUnits::K );
    void set_cloudCover( double CloudCover, coverUnits::eCoverUnits units );
    double get_cloudCover( coverUnits::eCoverUnits units = coverUnits::fraction );
    void set_influenceRadius( double InfluenceRadius, lengthUnits::eLengthUnits units );
    double get_influenceRadius( lengthUnits::eLengthUnits units = lengthUnits::meters );

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

    bool check_station();

    static char** getValidHeader();
    static std::vector<wxStation> readStationFile( std::string csvFile, 
					      std::string demFile );
    static void writeKmlFile( std::vector<wxStation> stations, 
			      std::string outFileName );

    static void writeStationFile( std::vector<wxStation> StationVect, 
				  const std::string outFileName );
    static void writeBlankStationFile( std::string outFileName );

 private:

    std::string stationName;
    double lat, lon;           //latitude and longitude
    double projXord;           //x-coordinate of station (in projected coordinate system)
    double projYord;	       //y-coordinate of station (in projected coordinate system)      
    double xord;	       //x-cordinate of station (in ninja coordinate system)
    double yord;	       //y-cordinate of station (in ninja coordinate system)
    double height;	       //in m (above vegetation)
    double speed;	       //in m/s (value < 0 indicates no value available)
    double direction;	       //in degrees from north
    double w_speed;	       //vertical velocity in m/s
    double temperature;	       //in K (value < -1000 indicates no value available)
    double cloudCover;	       //in fractions of cloud cover (0-1) (value < 0 indicates no value available)
    double influenceRadius;    //maximum radius that the station is used for during interpolation (m)
    // *Note: Value < 0 is a flag indicating influence radius is infinity (ie. influences everything)
    //Unit enums
    lengthUnits::eLengthUnits heightUnits;
    velocityUnits::eVelocityUnits inputSpeedUnits;
    velocityUnits::eVelocityUnits w_speedUnits;    
    temperatureUnits::eTempUnits tempUnits;
    coverUnits::eCoverUnits cloudCoverUnits;    
    lengthUnits::eLengthUnits influenceRadiusUnits;
    eDatumType datumType;
    eCoordType coordType;
    
    friend class Stability;

};

#endif /* WX_STATION_H */
