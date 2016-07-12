#ifndef	WX_TWO_H
#define WX_TWO_H

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

#include "boost/date_time/posix_time/posix_time.hpp"
#include "boost/date_time/posix_time/posix_time_io.hpp"
#include "boost/date_time/local_time_adjustor.hpp"
#include "boost/date_time/gregorian/gregorian.hpp"
#include "boost/date_time/local_time/local_time.hpp"
#include "iostream"
#include "numeric"
#include "wxStation.h"



/** Class representing a weather station
 */
class wxTwo
{
 public:

    wxTwo();	    //Default constructor
    void initialize();      //Initializes to default values
    ~wxTwo();	    //Destructor

    wxTwo( wxTwo const& m );               // Copy constructor
    wxTwo& operator= ( wxTwo const& m );   // Assignment operator

    enum eCoordType { PROJCS,
              GEOGCS };
    enum eDatumType { WGS84,
              NAD83,
              NAD27 };


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
    double get_height( lengthUnits::eLengthUnits units, int idx );
    void set_speed( double Speed, velocityUnits::eVelocityUnits units );
    double get_speed( velocityUnits::eVelocityUnits units, int idx);
    void set_direction( double Direction );
    inline double get_direction(int idx) { return direction[idx]; }
    void set_w_speed( double W_Speed, velocityUnits::eVelocityUnits units );
    double get_w_speed(velocityUnits::eVelocityUnits units = velocityUnits::metersPerSecond);
    void set_temperature(double Temperature, temperatureUnits::eTempUnits units );
    double get_temperature(temperatureUnits::eTempUnits units,int idx );
    void set_cloudCover( double CloudCover, coverUnits::eCoverUnits units );
    double get_cloudCover( coverUnits::eCoverUnits units,int idx );
    void set_influenceRadius( double InfluenceRadius, lengthUnits::eLengthUnits units );
    double get_influenceRadius( lengthUnits::eLengthUnits units ,int idx );

    void set_datetime(boost::posix_time::ptime timedata);
    boost::posix_time::ptime get_datetime(int idx);

    static void wxPrinter(wxTwo wxObject);
    static void wxVectorPrinter(std::vector<wxTwo> wxObject, int count);

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

    static vector<wxTwo> readStationFile2(std::string csvFile,std::string demFile,vector<vector<wxStation> > inputStation);

//    static wxTwo readStationFetchFile(std::string csvFile,std::string demFile,int piCount);

//    static void writeKmlFile( std::vector<wxTwo> stations,
//                  std::string outFileName );

//    static void writeStationFile( std::vector<wxTwo> StationVect,
//                  const std::string outFileName );
//    static void writeBlankStationFile( std::string outFileName );


 private:

    vector<double> zero;

    std::string stationName;
    double lat, lon;           //latitude and longitude
    double projXord;           //x-coordinate of station (in projected coordinate system)
    double projYord;	       //y-coordinate of station (in projected coordinate system)
    double xord;	       //x-cordinate of station (in ninja coordinate system)
    double yord;	       //y-cordinate of station (in ninja coordinate system)
    vector<double> height;	       //in m (above vegetation)
    vector<double> speed;	       //in m/s (value < 0 indicates no value available)
    vector<double> direction;	       //in degrees from north
    double w_speed;	       //vertical velocity in m/s
    vector<double> temperature;	       //in K (value < -1000 indicates no value available)
    vector<double> cloudCover;	       //in fractions of cloud cover (0-1) (value < 0 indicates no value available)
    vector<double> influenceRadius;    //maximum radius that the station is used for during interpolation (m)
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

    vector<boost::posix_time::ptime> datetime;




    friend class Stability;
    friend class pointInitialization;
};


class wxStationList
{

public:
    wxStationList();	    //Default constructor
    void initialize();      //Initializes to default values
    ~wxStationList();	    //Destructor

    wxStationList( wxStationList const& m );               // Copy constructor
    wxStationList& operator= ( wxStationList const& m );   // Assignment operator

    enum eCoordType { PROJCS,
              GEOGCS };
    enum eDatumType { WGS84,
              NAD83,
              NAD27 };

 private:
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

    void set_datetime(boost::posix_time::ptime timedata);
    boost::posix_time::ptime get_datetime();

    static void wxPrinter(wxStationList wxObject);
    static void wxVectorPrinter(std::vector<wxStationList> wxObject, int count);

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

    static char** oldGetValidHeader();
    static char** getValidHeader();

    static std::vector<std::vector<wxStationList> > vectorRead(std::string csvFile,std::string demFile);

    static std::vector<wxStationList> readStationFile( std::string csvFile,
                          std::string demFile);
    static std::vector<wxStationList> readStationFetchFile(std::string csvFile,std::string demFile);
//    static wxStationList readStationFetchFile(std::string csvFile,std::string demFile,int piCount);

    static void writeKmlFile( std::vector<wxStationList> stations,
                  std::string outFileName );

    static void writeStationFile( std::vector<wxStationList> StationVect,
                  const std::string outFileName );
    static void writeBlankStationFile( std::string outFileName );


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

    boost::posix_time::ptime datetime;

    friend class wxTwo;

};

#endif /* WX_TWO_H */
