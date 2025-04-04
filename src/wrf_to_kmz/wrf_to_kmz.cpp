/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja
 * Purpose:  Executable for converting wrf netcdf files to kmz without running WindNinja itself in a simulation
 * Author:   Loren Atwood <loren.atwood@usda.gov>
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

#include "ninja_init.h"
#include "ninja_conv.h"

#include "ascii_grid.h"

#include "netcdf.h"
#include "gdal_util.h"

#include "ninjaUnits.h"
#include "KmlVector.h"


/**
* function for converting the read in netcdf units to WindNinja units
* specifically for WindNinja velocityUnits
* throws an error if the input unit_string does not yet have a conversion written in the code
* if the input string is "", implies the units were not available for read in, the default units will be returned with a warning
* @param unit_string The read in netcdf unit string to be converted to a WindNinja velocityUnit
* @return spd_units The corresponding WindNinja velocityUnit enum to the input unit_string
*/
velocityUnits::eVelocityUnits processVelocityUnits( std::string unit_string )
{
    velocityUnits::eVelocityUnits spd_units;
    spd_units = velocityUnits::metersPerSecond;

    if ( unit_string == "" )
    {
        std::cout << "processVelocityUnits warning, input unit_string is empty implying the last read in netcdf band had no units to read in"
                  << "\n returning default velocityUnits \"" << velocityUnits::getString(spd_units).c_str() << "\"" << std::endl;
        return spd_units;
    }

    if ( unit_string == "m s-1" || unit_string == "mps" || unit_string == "m/s" )
    {
        spd_units = velocityUnits::metersPerSecond;
    } else if ( unit_string == "mi hr-1" || unit_string == "mph" || unit_string == "mi/hr" )
    {
        spd_units = velocityUnits::milesPerHour;
    } else if ( unit_string == "km hr-1" || unit_string == "kph" || unit_string == "km/hr" )
    {
        spd_units = velocityUnits::kilometersPerHour;
    } else if ( unit_string == "kts" || unit_string == "knots" )
    {
        spd_units = velocityUnits::knots;
    } else
    {
        ostringstream os;
        os << "unknown unit_string \"" << unit_string << "\" input to processVelocityUnits\n"
           << " need to update units processing code for input velocityUnit\n";
        throw std::runtime_error( os.str() );
    }

    return spd_units;
}

/**
* function for converting the read in netcdf units to WindNinja units
* specifically for WindNinja temperatureUnits
* throws an error if the input unit_string does not yet have a conversion written in the code
* if the input string is "", implies the units were not available for read in, the default units will be returned with a warning
* @param unit_string The read in netcdf unit string to be converted to a WindNinja temperatureUnits
* @return T_units The corresponding WindNinja temperatureUnits enum to the input unit_string
*/
temperatureUnits::eTempUnits processTemperatureUnits( std::string unit_string )
{
    temperatureUnits::eTempUnits T_units;
    T_units = temperatureUnits::K;

    if ( unit_string == "" )
    {
        std::cout << "processTemperatureUnits warning, input unit_string is empty implying the last read in netcdf band had no units to read in"
                  << "\n returning default temperatureUnits \"" << temperatureUnits::getString(T_units).c_str() << "\"" << std::endl;
        return T_units;
    }

    if ( unit_string == "K" || unit_string == "degK" || unit_string == "deg K" )
    {
        T_units = temperatureUnits::K;
    } else if ( unit_string == "C" || unit_string == "degC" || unit_string == "deg C" )
    {
        T_units = temperatureUnits::C;
    } else if ( unit_string == "R" || unit_string == "degR" || unit_string == "degR" )
    {
        T_units = temperatureUnits::R;
    } else if ( unit_string == "F" || unit_string == "degF" || unit_string == "deg F" )
    {
        T_units = temperatureUnits::F;
    } else
    {
        ostringstream os;
        os << "unknown unit_string \"" << unit_string << "\" input to processTemperatureUnits\n"
           << " need to update units processing code for input temperatureUnits\n";
        throw std::runtime_error( os.str() );
    }

    return T_units;
}


/**
* Static identifier to determine if the netcdf file is a WRF forecast.
* Uses netcdf c api.
* @param fileName netcdf filename.
* @return true if the forecast is a WRF forecast.
*/
bool identify( std::string fileName )
{
    bool identified = true;

    // Open the dataset
    int status, ncid, ndims, nvars, ngatts, unlimdimid;
    status = nc_open( fileName.c_str(), 0, &ncid );
    if ( status != NC_NOERR ) {
        identified = false;
    }

    // Check the global attributes for the following tag:
    //   :TITLE = "...WRF...""

    size_t len;
    nc_type type;
    char* model;
    status = nc_inq_att( ncid, NC_GLOBAL, "TITLE", &type, &len );
    if( status != NC_NOERR )
        identified = false;
    else {
        model = new char[len + 1];
        status = nc_get_att_text( ncid, NC_GLOBAL, "TITLE", model );
        model[len] = '\0';
        std::string s( model );
        if( s.find("WRF") == s.npos ) {
            identified = false;
        }

        delete[] model;
    }

    status = nc_close( ncid );

    return identified;
}


/**
* Fetch the variable names.
* @return a vector of variable names.
*/
std::vector<std::string> getVariableList()
{
    std::vector<std::string> varList;
    varList.push_back( "T2" );   // T at 2 m
    varList.push_back( "V10" );  // V at 10 m
    varList.push_back( "U10" );  // U at 10 m
    varList.push_back( "QCLOUD" );  // cloud water mixing ratio
    return varList;
}


/**
* Checks the downloaded data to see if it is all valid.
* @param wxModelFileName wrf netcdf weather model filename.
* Comment From WindNinja code: May not be functional yet for this class...
*/
void checkForValidData( std::string wxModelFileName )
{
    // open ds variable by variable
    GDALDataset *srcDS;
    std::string temp;
    std::string srcWkt;
    int nBands = 0;
    bool noDataValueExists;
    bool noDataIsNan;

    velocityUnits::eVelocityUnits spd_units = velocityUnits::metersPerSecond;  // initialize to default units
    temperatureUnits::eTempUnits T_units = temperatureUnits::K;

    std::vector<std::string> varList = getVariableList();

    for( unsigned int i = 0;i < varList.size();i++ ) {

        temp = "NETCDF:\"" + wxModelFileName + "\":" + varList[i];

        CPLPushErrorHandler(&CPLQuietErrorHandler);
        srcDS = (GDALDataset*)GDALOpen( temp.c_str(), GA_ReadOnly );
        CPLPopErrorHandler();
        if( srcDS == NULL )
            throw badForecastFile("Cannot open forecast file.");

        // Get total bands (time steps)
        nBands = srcDS->GetRasterCount();
        nBands = srcDS->GetRasterCount();
        int nXSize, nYSize;
        GDALRasterBand *poBand;
        int pbSuccess;
        double dfNoData;
        double *padfScanline;

        nXSize = srcDS->GetRasterXSize();
        nYSize = srcDS->GetRasterYSize();

        // loop over all bands for this variable (bands are time steps)
        for(int j = 1; j <= nBands; j++)
        {
            poBand = srcDS->GetRasterBand( j );

            pbSuccess = 0;
            dfNoData = poBand->GetNoDataValue( &pbSuccess );
            if( pbSuccess == false )
                noDataValueExists = false;
            else
            {
                noDataValueExists = true;
                noDataIsNan = CPLIsNan(dfNoData);
            }

            const char * poBand_units = poBand->GetUnitType();
            if( varList[i] == "T2" )
            {
                T_units = processTemperatureUnits( poBand_units );
            }
            if( varList[i] == "V10" || varList[i] == "U10" )
            {
                spd_units = processVelocityUnits( poBand_units );
            }

            // set the data
            padfScanline = new double[nXSize*nYSize];
            poBand->RasterIO(GF_Read, 0, 0, nXSize, nYSize, padfScanline, nXSize, nYSize,
                    GDT_Float64, 0, 0);
            for(int k = 0;k < nXSize*nYSize; k++)
            {
                double current_val = padfScanline[k];
                // Check if value is no data (if no data value was defined in file)
                if(noDataValueExists)
                {
                    if(noDataIsNan)
                    {
                        if(CPLIsNan(current_val))
                            throw badForecastFile("Forecast file contains no_data values.");
                    }else
                    {
                        if(current_val == dfNoData)
                            throw badForecastFile("Forecast file contains no_data values.");
                    }
                }
                if( varList[i] == "T2" )   // usual units are Kelvin
                {
                    temperatureUnits::toBaseUnits( current_val, T_units );  // convert to Kelvin if not in Kelvin
                    if(current_val < 180.0 || current_val > 340.0)  // these are near the most extreme temperatures ever recored on earth
                        throw badForecastFile("Temperature is out of range in forecast file.");
                }
                else if( varList[i] == "V10" )  // usual units are m/s
                {
                    velocityUnits::toBaseUnits( current_val, spd_units );  // convert to m/s if not in m/s
                    if(std::abs(current_val) > 220.0)
                        throw badForecastFile("V-velocity is out of range in forecast file.");
                }
                else if( varList[i] == "U10" )  // usual units are m/s
                {
                    velocityUnits::toBaseUnits( current_val, spd_units );  // convert to m/s if not in m/s
                    if(std::abs(current_val) > 220.0)
                        throw badForecastFile("U-velocity is out of range in forecast file.");
                }
                else if( varList[i] == "QCLOUD" )  // usual units are kg/kg
                {
                    if(current_val < -0.0001 || current_val > 100.0)
                        throw badForecastFile("Total cloud cover is out of range in forecast file.");
                }
            }

            delete [] padfScanline;
        }

        GDALClose((GDALDatasetH) srcDS );
    }
}


/**
* Uses netcfd c api commands to get wrf netcdf file global attributes, to later use
* to set the gdal dataset projection and geotransform information, which are required
* to allow the wrf netcdf file data to be accessible by standard gdal commands.
* @param wxModelFileName wrf netcdf weather model filename, from which the global attributes data are read in and processed.
* @param dx The cell size x value of the dataset to be filled.
* @param dy The cell size y value of the dataset to be filled.
* @param cenLat The center latitude value of the dataset to be filled.
* @param cenLon The center longitude value of the dataset to be filled.
* @param projString The projection string of the dataset to be filled.
* Note: there are additional wrf netcdf file global attributes that are accessed and used to get and process the projection string, 
* but are not used outside this function, so they are not returned. These include mapProj, moadCenLat, standLon, trueLat1, trueLat2.
*/
void getNcGlobalAttributes( const std::string &wxModelFileName, float &dx, float &dy, float &cenLat, float &cenLon, std::string &projString )
{

    //==========get global attributes to set projection===========================
    
    // Open the dataset
    int status, ncid;
    status = nc_open( wxModelFileName.c_str(), 0, &ncid );
    if ( status != NC_NOERR ) {
        ostringstream os;
        os << "The netcdf file: " << wxModelFileName
           << " cannot be opened\n";
        throw std::runtime_error( os.str() );
    }

    // Get global attribute MAP_PROJ
    // 1 = Lambert Conformal Conic
    // 2 = Polar Stereographic
    // 3 = Mercator
    // 6 = Lat/Long

    int mapProj;
    nc_type type;
    size_t len;
    status = nc_inq_att( ncid, NC_GLOBAL, "MAP_PROJ", &type, &len );
    if( status != NC_NOERR ){
        ostringstream os;
        os << "Global attribute 'MAP_PROJ' in the netcdf file: " << wxModelFileName
        << " cannot be opened\n";
        throw std::runtime_error( os.str() );
    }
    else {
        status = nc_get_att_int( ncid, NC_GLOBAL, "MAP_PROJ", &mapProj );
    }

    // Get global attributes DX, DY
    status = nc_inq_att( ncid, NC_GLOBAL, "DX", &type, &len );
    if( status != NC_NOERR ){
        ostringstream os;
        os << "Global attribute DX in the netcdf file: " << wxModelFileName
        << " cannot be opened\n";
        throw std::runtime_error( os.str() );
    }
    else {
        status = nc_get_att_float( ncid, NC_GLOBAL, "DX", &dx );
    }
    status = nc_inq_att( ncid, NC_GLOBAL, "DY", &type, &len );
    if( status != NC_NOERR ){
        ostringstream os;
        os << "Global attribute DY in the netcdf file: " << wxModelFileName
        << " cannot be opened\n";
        throw std::runtime_error( os.str() );
    }
    else {
        status = nc_get_att_float( ncid, NC_GLOBAL, "DY", &dy );
    }

    // Get global attributes CEN_LAT, CEN_LON
    status = nc_inq_att( ncid, NC_GLOBAL, "CEN_LAT", &type, &len );
    if( status != NC_NOERR ){
        ostringstream os;
        os << "Global attribute CEN_LAT in the netcdf file: " << wxModelFileName
        << " cannot be opened\n";
        throw std::runtime_error( os.str() );
    }
    else {
        status = nc_get_att_float( ncid, NC_GLOBAL, "CEN_LAT", &cenLat );
    }
    status = nc_inq_att( ncid, NC_GLOBAL, "CEN_LON", &type, &len );
    if( status != NC_NOERR ){
        ostringstream os;
        os << "Global attribute CEN_LON in the netcdf file: " << wxModelFileName
        << " cannot be opened\n";
        throw std::runtime_error( os.str() );
    }
    else {
        status = nc_get_att_float( ncid, NC_GLOBAL, "CEN_LON", &cenLon );
    }

    // Get global attributes MOAD_CEN_LAT, STAND_LON
    float moadCenLat, standLon;
    status = nc_inq_att( ncid, NC_GLOBAL, "MOAD_CEN_LAT", &type, &len );
    if( status != NC_NOERR ){
        ostringstream os;
        os << "Global attribute MOAD_CEN_LAT in the netcdf file: " << wxModelFileName
        << " cannot be opened\n";
        throw std::runtime_error( os.str() );
    }
    else {
        status = nc_get_att_float( ncid, NC_GLOBAL, "MOAD_CEN_LAT", &moadCenLat );
    }
    status = nc_inq_att( ncid, NC_GLOBAL, "STAND_LON", &type, &len );
    if( status != NC_NOERR ){
        ostringstream os;
        os << "Global attribute STAND_LON in the netcdf file: " << wxModelFileName
        << " cannot be opened\n";
        throw std::runtime_error( os.str() );
    }
    else {
        status = nc_get_att_float( ncid, NC_GLOBAL, "STAND_LON", &standLon );
    }

    // Get global attributes TRUELAT1, TRUELAT2
    float trueLat1, trueLat2;
    status = nc_inq_att( ncid, NC_GLOBAL, "TRUELAT1", &type, &len );
    if( status != NC_NOERR ){
        ostringstream os;
        os << "Global attribute TRUELAT1 in the netcdf file: " << wxModelFileName
        << " cannot be opened\n";
        throw std::runtime_error( os.str() );
    }
    else {
        status = nc_get_att_float( ncid, NC_GLOBAL, "TRUELAT1", &trueLat1 );
    }
    status = nc_inq_att( ncid, NC_GLOBAL, "TRUELAT2", &type, &len );
    if( status != NC_NOERR ){
        ostringstream os;
        os << "Global attribute TRUELAT2 in the netcdf file: " << wxModelFileName
        << " cannot be opened\n";
        throw std::runtime_error( os.str() );
    }
    else {
        status = nc_get_att_float( ncid, NC_GLOBAL, "TRUELAT2", &trueLat2 );
    }

    // Close the dataset
    status = nc_close( ncid );
    if( status != NC_NOERR ) {
        ostringstream os;
        os << "The netcdf file: " << wxModelFileName
           << " cannot be closed\n";
        throw std::runtime_error( os.str() );
    }

    //======end get global attributes========================================


    //std::string projString;  // this is a function input, to be filled
    if(mapProj == 1){  //lambert conformal conic
        projString = "PROJCS[\"WGC 84 / WRF Lambert\",GEOGCS[\"WGS 84\",DATUM[\"World Geodetic System 1984\",\
                      SPHEROID[\"WGS 84\",6378137.0,298.257223563,AUTHORITY[\"EPSG\",\"7030\"]],AUTHORITY[\"EPSG\",\"6326\"]],\
                      PRIMEM[\"Greenwich\",0.0,AUTHORITY[\"EPSG\",\"8901\"]],UNIT[\"degree\",0.017453292519943295],\
                      AXIS[\"Geodetic longitude\",EAST],AXIS[\"Geodetic latitude\",NORTH],AUTHORITY[\"EPSG\",\"4326\"]],\
                      PROJECTION[\"Lambert_Conformal_Conic_2SP\"],\
                      PARAMETER[\"central_meridian\","+boost::lexical_cast<std::string>(standLon)+"],\
                      PARAMETER[\"latitude_of_origin\","+boost::lexical_cast<std::string>(moadCenLat)+"],\
                      PARAMETER[\"standard_parallel_1\","+boost::lexical_cast<std::string>(trueLat1)+"],\
                      PARAMETER[\"false_easting\",0.0],PARAMETER[\"false_northing\",0.0],\
                      PARAMETER[\"standard_parallel_2\","+boost::lexical_cast<std::string>(trueLat2)+"],\
                      UNIT[\"m\",1.0],AXIS[\"Easting\",EAST],AXIS[\"Northing\",NORTH]]";
    }
    else if(mapProj == 2){  //polar stereographic
        projString = "PROJCS[\"WGS 84 / Antarctic Polar Stereographic\",GEOGCS[\"WGS 84\",\
                     DATUM[\"WGS_1984\",SPHEROID[\"WGS 84\",6378137,298.257223563,AUTHORITY[\"EPSG\",\"7030\"]],\
                     AUTHORITY[\"EPSG\",\"6326\"]],PRIMEM[\"Greenwich\",0,AUTHORITY[\"EPSG\",\"8901\"]],\
                     UNIT[\"degree\",0.01745329251994328,AUTHORITY[\"EPSG\",\"9122\"]],\
                     AUTHORITY[\"EPSG\",\"4326\"]],UNIT[\"metre\",1,AUTHORITY[\"EPSG\",\"9001\"]],\
                     PROJECTION[\"Polar_Stereographic\"],\
                     PARAMETER[\"latitude_of_origin\","+boost::lexical_cast<std::string>(moadCenLat)+"],\
                     PARAMETER[\"central_meridian\","+boost::lexical_cast<std::string>(standLon)+"],\
                     PARAMETER[\"scale_factor\",1],\
                     PARAMETER[\"false_easting\",0],\
                     PARAMETER[\"false_northing\",0],AUTHORITY[\"EPSG\",\"3031\"],\
                     AXIS[\"Easting\",UNKNOWN],AXIS[\"Northing\",UNKNOWN]]";
    }
    else if(mapProj == 3){  //mercator
        projString = "PROJCS[\"World_Mercator\",GEOGCS[\"GCS_WGS_1984\",DATUM[\"WGS_1984\",\
                      SPHEROID[\"WGS_1984\",6378137,298.257223563]],PRIMEM[\"Greenwich\",0],\
                      UNIT[\"Degree\",0.017453292519943295]],PROJECTION[\"Mercator_1SP\"],\
                      PARAMETER[\"False_Easting\",0],\
                      PARAMETER[\"False_Northing\",0],\
                      PARAMETER[\"Central_Meridian\","+boost::lexical_cast<std::string>(standLon)+"],\
                      PARAMETER[\"latitude_of_origin\","+boost::lexical_cast<std::string>(moadCenLat)+"],\
                      UNIT[\"Meter\",1]]";
    }
    else if(mapProj == 6){  //lat/long
        throw badForecastFile("Cannot initialize with a forecast file in lat/long spacing. \
                               Forecast file must be in a projected coordinate system.");
    }
    else throw badForecastFile("Cannot determine projection from the forecast file information.");
}

/**
* Gets a time zone name string corresponding to a latitude and longitude point
* usually the input lat/lon point is the center latitude and longitude of the dataset, 
* or the center latitude and longitude of a bounding box subset of the dataset.
* @param lat The latitude used to locate the timezone.
* @param lon The longitude used to locate the timezone.
* @return timeZoneString The time zone name for the lat/lon point, found from the file "date_time_zonespec.csv".
*/
std::string getTimeZoneString( const double &lat, const double &lon )
{
    std::string timeZoneString = FetchTimeZone(lon, lat, NULL);
    if( timeZoneString == "" )
    {
        fprintf(stderr, "Could not get timezone for lat,lon %f,%f location!!!\n", lat, lon);
        std::exit(1);
    }

    return timeZoneString;
}

/**
 * Fetch the list of times that the wrf file holds. It is assumed that
 * the time variable is "Times" and the units string is "units". If this
 * is not the case, this function needs to be rewritten.
 * Uses netcdf api commands.
 *
 * @param timeZoneString Time zone name from the file "date_time_zonespec.csv".
 * @param wxModelFileName the input wrf file to open and read times from.
 * @throw runtime_error for bad file i/o.
 * @return a vector of boost::local_time::local_date_time objects for the forecast.
 */
std::vector<blt::local_date_time>
getTimeList( const std::string &timeZoneString, const std::string &wxModelFileName )
{
    const char *pszVariable = NULL;

    boost::local_time::time_zone_ptr timeZonePtr;
    timeZonePtr = globalTimeZoneDB.time_zone_from_region(timeZoneString.c_str());
    if( timeZonePtr == NULL ) {
        ostringstream os;
        os << "The time zone string: " << timeZoneString.c_str()
       << " does not match any in "
       << "the time zone database file: date_time_zonespec.csv.";
        throw std::runtime_error( os.str() );
    }

    std::vector<blt::local_date_time>timeList;

    int status, ncid, ndims, nvars, ngatts, unlimdimid;
    nc_type vartype;
    int varndims, varnatts;
    double *varvals;
    int vardimids[NC_MAX_VAR_DIMS];   // dimension IDs

    // Open the dataset
    status = nc_open( wxModelFileName.c_str(), 0, &ncid );
    if ( status != NC_NOERR ) {
        ostringstream os;
        os << "The netcdf file: " << wxModelFileName
           << " cannot be opened\n";
        throw std::runtime_error( os.str() );
    }

    // note that "Times" is the time variable name found in wrf files, this variable is usually set to "time" for many of the other weather model file types
    std::string timename = "Times";

    // If we can't get simple data from the file, return false
    status = nc_inq(ncid, &ndims, &nvars, &ngatts, &unlimdimid);
    if ( status != NC_NOERR ) {
        ostringstream os;
        os << "The variables in the netcdf file: " << wxModelFileName
           << " cannot be read\n";
        throw std::runtime_error( os.str() );
    }

    // Check the variable names, return false if any aren't found
    int varid;
    status = nc_inq_varid( ncid, timename.c_str(), &varid );
    if( status != NC_NOERR ) {
        ostringstream os;
        //os << "The variable \"time\" in the netcdf file: "
        os << "The variable \"Times\" in the netcdf file: "
           << wxModelFileName << " cannot be read\n";
        throw std::runtime_error( os.str() );
    }

    //==============The forecast is WRF, parse Times============================

    // Check to see if we can read Times variable
    status = nc_inq_var( ncid, varid, 0, &vartype, &varndims, vardimids,
                        &varnatts );
    if( status != NC_NOERR ) {
        ostringstream os;
        os << "The values for variable \"Times\" "
            << " in the netcdf file: " << wxModelFileName
            << " cannot be read\n";
        throw std::runtime_error( os.str() );
    }

    // Get varid for U10 --> use this to get length of time dimension
    status = nc_inq_varid( ncid, "U10", &varid );
    if( status != NC_NOERR ) {
        ostringstream os;
        os << "The variable \"U10\" in the netcdf file: "
        << wxModelFileName << " cannot be read\n";
        throw std::runtime_error( os.str() );
    }

    // Get dimid for Time in U10
    int dimid;
    status = nc_inq_dimid(ncid, "Time", &dimid );
    if( status != NC_NOERR ) {
        ostringstream os;
        os << "The dimension \"Time\" "
            << " in the netcdf file: " << wxModelFileName
            << " cannot be read\n";
        throw std::runtime_error( os.str() );
    }

    // Get length of the time dimension in U10
    size_t time_len;
    status = nc_inq_dimlen( ncid, dimid, &time_len );
    if( status != NC_NOERR ) {
        ostringstream os;
        os << "The time dimension for variable \"Times\" in the netcdf file: "
        << wxModelFileName << " cannot be read\n";
        throw std::runtime_error( os.str() );
    }

    // Reset varid to 'Times'
    status = nc_inq_varid( ncid, timename.c_str(), &varid );
    if( status != NC_NOERR ) {
        ostringstream os;
        os << "The variable \"U10\" in the netcdf file: "
        << wxModelFileName << " cannot be read\n";
        throw std::runtime_error( os.str() );
    }

    // Get dimid for DateStrLen
    status = nc_inq_dimid(ncid, "DateStrLen", &dimid );
    if( status != NC_NOERR ) {
        ostringstream os;
        os << "The dimension \"DateStrLen\" "
            << " in the netcdf file: " << wxModelFileName
            << " cannot be read\n";
        throw std::runtime_error( os.str() );
    }

    // Get the length of the time string
    size_t t_len;
    status = nc_inq_dimlen( ncid, dimid, &t_len );
    if( status != NC_NOERR ) {
        ostringstream os;
        os << "The dimension for variable \"Times\" in the netcdf file: "
        << wxModelFileName << " cannot be read\n";
        throw std::runtime_error( os.str() );
    }

    for( unsigned int t = 0;t < time_len;t++ ) {

        // Get value for one Times variable
        char* tp = new char[t_len + 1];
        for (int i=0; i<t_len; i++){
            const size_t varindex[] = {t,static_cast<size_t>(i)};  // where to get value from
            status = nc_get_var1_text( ncid, varid, varindex, tp+i );
        }
        if( status != NC_NOERR ) {
            ostringstream os;
            os << "The values for variable \"time\" "
                << " in the netcdf file: " << wxModelFileName
                << " cannot be read\n";
            throw std::runtime_error( os.str() );
        }

        tp[t_len] = '\0';
        std::string refString( tp );
        delete[] tp;

        // Clean the string for the boost constructor.  Remove the prefix, suffix, and delimiters
        int pos = -1;
        if( refString.find( "_" ) != refString.npos ) {
            pos = refString.find( "_" );
            refString.replace(pos, 1, 1, 'T');
        }
        while( refString.find( "-" ) != refString.npos ) {
            pos = refString.find( "-" );
            refString.erase( pos, 1 );
        }
        while( refString.find( ":" ) != refString.npos ) {
            pos = refString.find( ":" );
            refString.erase( pos, 1 );
        }
        pos = refString.find("Hour since ");
        if(pos != refString.npos)
        {
            refString.erase(pos, strlen("Hour since "));
        }

        // Make a posix time in UTC/GMT, to call the boost::local_time::local_date_time input UTC time constructor
        bpt::ptime reference_pt;
        reference_pt = bpt::from_iso_string( refString );
        bpt::ptime first_pt( reference_pt );
        blt::local_date_time first_pt_local(first_pt, timeZonePtr);
        timeList.push_back( first_pt_local );

    } // end for loop

    return timeList;
}

/**
* Sets the surface grids based on a WRF (surface only!) forecast.
* @param wxModelFileName The wrf input filename from which data are read from.
* @param timeBandIdx The band index from which data are read from, corresponding to a specific expected time from getTimeList().
* @param dx The cell size x value of the dataset, used to set the gdal dataset geotransform.
* @param dy The cell size y value of the dataset, used to set the gdal dataset geotransform.
* @param cenLat The center latitude value of the dataset, used to set the gdal dataset geotransform.
* @param cenLon The center longitude value of the dataset, used to set the gdal dataset geotransform.
* @param airGrid The air temperature grid to be filled.
* @param cloudGrid The cloud cover grid to be filled.
* @param uGrid The u velocity grid to be filled.
* @param vGrid The v velocity grid to be filled.
* @param wGrid The w velocity grid to be filled (filled with zeros).
*/
void setSurfaceGrids( const std::string &wxModelFileName, const int &timeBandIdx, const float &dx, const float &dy, const float &cenLat, const float &cenLon, const std::string &projString, AsciiGrid<double> &airGrid, AsciiGrid<double> &cloudGrid, AsciiGrid<double> &uGrid, AsciiGrid<double> &vGrid, AsciiGrid<double> &wGrid )
{

    int bandNum = timeBandIdx+1;  // timeIdx is 0 to N-1, bandNum is 1 to N.

    GDALDataset* poDS;
    CPLPushErrorHandler(&CPLQuietErrorHandler);
    poDS = (GDALDataset*)GDALOpenShared( wxModelFileName.c_str(), GA_ReadOnly );
    CPLPopErrorHandler();
    if( poDS == NULL ) {
        throw badForecastFile("Cannot open forecast file in setSurfaceGrids()");
    }
    else {
        GDALClose((GDALDatasetH) poDS ); // close original wxModel file
    }

    // open ds one by one, set geotranform, set projection, then write to grid
    GDALDataset *srcDS;
    std::string temp;
    std::vector<std::string> varList = getVariableList();

    velocityUnits::eVelocityUnits spd_units = velocityUnits::metersPerSecond;  // initialize to default units
    temperatureUnits::eTempUnits T_units = temperatureUnits::K;

    for( unsigned int i = 0;i < varList.size();i++ ) {

        temp = "NETCDF:\"" + wxModelFileName + "\":" + varList[i];
        
        CPLPushErrorHandler(&CPLQuietErrorHandler);
        srcDS = (GDALDataset*)GDALOpenShared( temp.c_str(), GA_ReadOnly );
        CPLPopErrorHandler();
        if( srcDS == NULL ) {
            throw badForecastFile("Cannot open forecast file in setSurfaceGrids()");
        }

        // Set up spatial reference stuff for setting projections
        // and geotransformations
        OGRSpatialReference oSRS, *poLatLong;
        char *srcWKT = NULL;
        char* prj2 = (char*)projString.c_str();
        oSRS.importFromWkt(&prj2);
        oSRS.exportToWkt(&srcWKT);

        poLatLong = oSRS.CloneGeogCS();
        char *dstWkt = NULL;
        poLatLong->exportToWkt(&dstWkt);

        // Transform domain center from lat/long to WRF space
        double zCenter;
        zCenter = 0;
        double xCenter, yCenter;
        xCenter = (double)cenLon;
        yCenter = (double)cenLat;

#ifdef GDAL_COMPUTE_VERSION
#if GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(3,0,0)
    oSRS.SetAxisMappingStrategy(OAMS_TRADITIONAL_GIS_ORDER);
#endif /* GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(3,0,0) */
#endif /* GDAL_COMPUTE_VERSION */

        OGRCoordinateTransformation *poCT;
        poCT = OGRCreateCoordinateTransformation(poLatLong, &oSRS);
        delete poLatLong;

        if(poCT==NULL || !poCT->Transform(1, &xCenter, &yCenter))
            throw std::runtime_error("Transformation of lat/lon center to dataset coordinate system failed!");

        // Set the geostransform for the WRF file
        // upper corner is calculated from transformed x, y
        // (in WRF space)
        double ncols, nrows;
        int nXSize = srcDS->GetRasterXSize();
        int nYSize = srcDS->GetRasterYSize();
        ncols = (double)(nXSize)/2;
        nrows = (double)(nYSize)/2;

        double adfGeoTransform[6] = {(xCenter-(ncols*dx)), dx, 0,
                                    (yCenter+(nrows*dy)),
                                    0, -(dy)};

        srcDS->SetGeoTransform(adfGeoTransform);

        // get the noDataValue from the current band
        GDALRasterBand *poBand = srcDS->GetRasterBand( bandNum );
        int pbSuccess;
        double dfNoData = poBand->GetNoDataValue( &pbSuccess );

        int nBandCount = srcDS->GetRasterCount();

        if( pbSuccess == false )
            dfNoData = -9999.0;

        const char * poBand_units = poBand->GetUnitType();
        if( varList[i] == "T2" )
        {
            T_units = processTemperatureUnits( poBand_units );
        }
        if( varList[i] == "V10" || varList[i] == "U10" )
        {
            spd_units = processVelocityUnits( poBand_units );
        }

        // set the dataset projection
        int rc = srcDS->SetProjection( projString.c_str() );

        // final setting of the datasets to ascii grids, in the past usually done using a wrp dataset
        // TODO: data must be in SI units, need to check units here and convert if necessary
        if( varList[i] == "T2" ) {
            GDAL2AsciiGrid( srcDS, bandNum, airGrid );
            temperatureUnits::toBaseUnits( airGrid, T_units );
            if( CPLIsNan( dfNoData ) ) {
                airGrid.set_noDataValue(-9999.0);
                airGrid.replaceNan( -9999.0 );
            }
        }
        else if( varList[i] == "V10" ) {
            GDAL2AsciiGrid( srcDS, bandNum, vGrid );
            velocityUnits::toBaseUnits( vGrid, spd_units );
            if( CPLIsNan( dfNoData ) ) {
                vGrid.set_noDataValue(-9999.0);
                vGrid.replaceNan( -9999.0 );
            }
        }
        else if( varList[i] == "U10" ) {
            GDAL2AsciiGrid( srcDS, bandNum, uGrid );
            velocityUnits::toBaseUnits( uGrid, spd_units );
            if( CPLIsNan( dfNoData ) ) {
                uGrid.set_noDataValue(-9999.0);
                uGrid.replaceNan( -9999.0 );
            }
        }
        else if( varList[i] == "QCLOUD" ) {
            GDAL2AsciiGrid( srcDS, bandNum, cloudGrid );
            if( CPLIsNan( dfNoData ) ) {
                cloudGrid.set_noDataValue(-9999.0);
                cloudGrid.replaceNan( -9999.0 );
            }
        }
        CPLFree(srcWKT);
        CPLFree(dstWkt);
        delete poCT;
        GDALClose((GDALDatasetH) srcDS );
    }  // end for loop

    // don't allow small negative values in cloud cover
    for(int i=0; i<cloudGrid.get_nRows(); i++){
        for(int j=0; j<cloudGrid.get_nCols(); j++){
            if(cloudGrid(i,j) < 0.0){
                cloudGrid(i,j) = 0.0;
            }
        }
    }
    cloudGrid /= 100.0;

    wGrid.set_headerData( uGrid );
    wGrid = 0.0;
}

/**
* write the ascii grids to kmz for a wrf surface forecast
* @param outputPath The output path to save the output files to.
* @param forecastTime The boost::local_time::local_date_time corresponding to the data to be plotted, corresponding to a specific time from getTimeList().
* @param outputSpeedUnits The speed units to write the output kmz speed data to.
* @param uGrid The u velocity grid to be plotted.
* @param vGrid The v velocity grid to be plotted.
*/
void writeWxModelGrids( const std::string &outputPath, const boost::local_time::local_date_time &forecastTime, const velocityUnits::eVelocityUnits &outputSpeedUnits, const AsciiGrid<double> &uGrid_wxModel, const AsciiGrid<double> &vGrid_wxModel )
{

    // make speed and direction grids from u,v components
    AsciiGrid<double> speedInitializationGrid_wxModel( uGrid_wxModel );
    AsciiGrid<double> dirInitializationGrid_wxModel( uGrid_wxModel );

    for(int i=0; i<uGrid_wxModel.get_nRows(); i++)
    {
        for(int j=0; j<uGrid_wxModel.get_nCols(); j++)
        {
            if( uGrid_wxModel(i,j) == uGrid_wxModel.get_NoDataValue() ||
                vGrid_wxModel(i,j) == vGrid_wxModel.get_NoDataValue() ) {
                speedInitializationGrid_wxModel(i,j) = speedInitializationGrid_wxModel.get_NoDataValue();
                dirInitializationGrid_wxModel(i,j) = dirInitializationGrid_wxModel.get_NoDataValue();
            } else
            {
                wind_uv_to_sd(uGrid_wxModel(i,j), vGrid_wxModel(i,j), &(speedInitializationGrid_wxModel)(i,j), &(dirInitializationGrid_wxModel)(i,j));
            }
        }
    }

    // setup filename parts from the forecastIdentifier and using the forecast time
    ostringstream wxModelTimestream;
    blt::local_time_facet* wxModelOutputFacet;
    wxModelOutputFacet = new blt::local_time_facet();
    wxModelTimestream.imbue(std::locale(std::locale::classic(), wxModelOutputFacet));
    wxModelOutputFacet->format("%m-%d-%Y_%H%M");
    wxModelTimestream << forecastTime;
    std::string forecastIdentifier = "WRF-SURFACE";
    std::string rootname = forecastIdentifier + "-" + wxModelTimestream.str();

    // don't forget the to output units unit conversion
    velocityUnits::fromBaseUnits(speedInitializationGrid_wxModel, outputSpeedUnits);

    // now do the kmz preparation and writing stuff

    KmlVector ninjaKmlFiles;

    ninjaKmlFiles.setKmlFile( CPLFormFilename(outputPath.c_str(), rootname.c_str(), "kml") );
    ninjaKmlFiles.setKmzFile( CPLFormFilename(outputPath.c_str(), rootname.c_str(), "kmz") );

    //compute angle between N-S grid lines in the dataset and true north
    double angleFromNorth = 0.0;
    if( CSLTestBoolean(CPLGetConfigOption("DISABLE_ANGLE_FROM_NORTH_CALCULATION", "FALSE")) == false )
    {
        GDALDatasetH hDS = dirInitializationGrid_wxModel.ascii2GDAL();
        if(!GDALCalculateAngleFromNorth( hDS, angleFromNorth ))
        {
            printf("Warning: Unable to calculate angle departure from north for the wxModel.");
        }
        GDALClose(hDS);
    }

    // add the angleFromNorth to each spd,dir, u,v dataset for kmz output, kmlVector requires the dirGrid to be in projected form, not lat/lon form
    for(int i=0; i<dirInitializationGrid_wxModel.get_nRows(); i++)
    {
        for(int j=0; j<dirInitializationGrid_wxModel.get_nCols(); j++)
        {
            dirInitializationGrid_wxModel(i,j) = wrap0to360( dirInitializationGrid_wxModel(i,j) + angleFromNorth ); //account for projection rotation from north
            // always recalculate the u and v grids from the corrected dir grid, the changes need to go together
            // however, these u and v grids are not actually being used past this point
            //wind_sd_to_uv(speedInitializationGrid_wxModel(i,j), dirInitializationGrid_wxModel(i,j),
            //        &(uGrid_wxModel)(i,j), &(vGrid_wxModel)(i,j));
        }
    }

    ninjaKmlFiles.setLegendFile( CPLFormFilename(outputPath.c_str(), rootname.c_str(), "bmp") );
	ninjaKmlFiles.setSpeedGrid(speedInitializationGrid_wxModel, outputSpeedUnits);
	ninjaKmlFiles.setAngleFromNorth(angleFromNorth);
	ninjaKmlFiles.setDirGrid(dirInitializationGrid_wxModel);

    ninjaKmlFiles.setLineWidth(3.0);  // input.wxModelGoogLineWidth value
    std::string dateTimewxModelLegFileTemp = CPLFormFilename(outputPath.c_str(), (rootname+"_date_time").c_str(), "bmp");
    ninjaKmlFiles.setTime(forecastTime);
    ninjaKmlFiles.setDateTimeLegendFile(dateTimewxModelLegFileTemp, forecastTime);
    ninjaKmlFiles.setWxModel(forecastIdentifier, forecastTime);

    // default values for input.wxModelGoogSpeedScaling/input.googSpeedScaling,input.googColor,input.googVectorScale are KmlVector::equal_interval,"default",false respectively
    if(ninjaKmlFiles.writeKml(KmlVector::equal_interval,"default",false))
	{
		if(ninjaKmlFiles.makeKmz())
			ninjaKmlFiles.removeKmlFile();
	}
}

void Usage()
{
    printf("wrf_to_kmz [--osu/output_speed_units mph/mps/kph/kts]\n"
           "           [--op/output_path path]\n"
           "           input_wrf_filename\n"
           "\n"
           "Defaults:\n"
           "    --output_speed_units mps\n"
           "    --output_path \".\"\n");
    exit(1);
}

int main( int argc, char* argv[] )
{
    std::string input_wrf_filename = "";
    std::string outputSpeedUnits_str = "mps";
    std::string output_path = ".";
    
    // parse input arguments
    int i = 1;
    while( i < argc )
    {
        if( EQUAL(argv[i], "--output_speed_units") || EQUAL(argv[i], "--osu") )
        {
            outputSpeedUnits_str = std::string( argv[++i] );
        } else if( EQUAL(argv[i], "--output_path") || EQUAL(argv[i], "--op") )
        {
            output_path = std::string( argv[++i] );
        } else if( EQUAL(argv[i], "--help") || EQUAL(argv[i], "--h") || EQUAL(argv[i], "-help") || EQUAL(argv[i], "-h") )
        {
            Usage();
        } else if( input_wrf_filename == "" )
        {
            input_wrf_filename = argv[i];
        } else
        {
            printf("Invalid argument: \"%s\"\n", argv[i]);
            Usage();
        }
        i++;
    }

    if( input_wrf_filename == "" )
    {
        std::cout << "please enter a valid input_wrf_filename" << std::endl;
        Usage();
    }
    int isValidFile = CPLCheckForFile(input_wrf_filename.c_str(),NULL);
    if( isValidFile != 1 )
    {
        printf("input_wrf_filename \"%s\" file does not exist!!\n", input_wrf_filename.c_str());
        exit(1);
    }
    VSIDIR *pathDir;
    pathDir = VSIOpenDir( output_path.c_str(), 0, NULL);
    if( pathDir == NULL )
    {
        printf("output_path \"%s\" is not a valid path!!\n", output_path.c_str());
        exit(1);
    }
    VSICloseDir(pathDir);

    std::cout << "input_wrf_filename = \"" << input_wrf_filename.c_str() << "\"" << std::endl;
    std::cout << "output_speed_units = \"" << outputSpeedUnits_str.c_str() << "\"" << std::endl;
    std::cout << "output_path = \"" << output_path.c_str() << "\"" << std::endl;

    NinjaInitialize();  // needed for GDALAllRegister()

    // test and set units
    velocityUnits::eVelocityUnits outputSpeedUnits = velocityUnits::getUnit(outputSpeedUnits_str);

    // check dataset to verify it is a wrf dataset, and to verify it is readable
    if ( identify( input_wrf_filename ) == false )
    {
        throw badForecastFile("input input_wrf_filename is not a valid WRF file!!!");
    }
    checkForValidData( input_wrf_filename );

    // now start processing the data
    float dx;
    float dy;
    float cenLat;
    float cenLon;
    std::string projString;
    getNcGlobalAttributes( input_wrf_filename, dx, dy, cenLat, cenLon, projString );

    std::string timeZoneString = getTimeZoneString( cenLat, cenLon );

    std::vector<boost::local_time::local_date_time> timeList = getTimeList( timeZoneString, input_wrf_filename );

    for(unsigned int timeIdx = 0; timeIdx < timeList.size(); timeIdx++)
    {
        boost::local_time::local_date_time forecastTime = timeList[timeIdx];

        AsciiGrid<double> airGrid;
        AsciiGrid<double> cloudGrid;
        AsciiGrid<double> uGrid;
        AsciiGrid<double> vGrid;
        AsciiGrid<double> wGrid;

        setSurfaceGrids( input_wrf_filename, timeIdx, dx, dy, cenLat, cenLon, projString, airGrid, cloudGrid, uGrid, vGrid, wGrid );

        writeWxModelGrids( output_path, forecastTime, outputSpeedUnits, uGrid, vGrid );
    }

    return 0;
}
