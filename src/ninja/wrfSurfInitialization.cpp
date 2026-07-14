/******************************************************************************
*
* $Id:$
*
* Project:  WindNinja
* Purpose:  Initialing with WRF forecasts
* Author:   Natalie Wagenbrenner <nwagenbrenner@gmail.com>
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

#include "wrfSurfInitialization.h"

/**
* Constructor for the class initializes the variable names
*
*/
wrfSurfInitialization::wrfSurfInitialization() : wxModelInitialization()
{

}

/**
* Copy constructor.
* @param A Copied value.
*/
wrfSurfInitialization::wrfSurfInitialization(wrfSurfInitialization const& A ) : wxModelInitialization(A)
{
    wxModelFileName = A.wxModelFileName;
}

/**
* Destructor.
*/
wrfSurfInitialization::~wrfSurfInitialization()
{
}

/**
* Equals operator.
* @param A Value to set equal to.
* @return a copy of an object
*/
wrfSurfInitialization& wrfSurfInitialization::operator= (wrfSurfInitialization const& A)
{
    if(&A != this) {
        wxModelInitialization::operator=(A);
    }
    return *this;
}

/**
*@brief Returns horizontal grid resolution of the model
*@return return grid resolution (in km unless < 1, then degrees)
*/
double wrfSurfInitialization::getGridResolution()
{
    return -1.0;
}

/**
* Fetch the variable names
*
* @return a vector of variable names
*/
std::vector<std::string> wrfSurfInitialization::getVariableList()
{
    std::vector<std::string> varList;
    varList.push_back( "T2" );   // T at 2 m
    varList.push_back( "V10" );  // V at 10 m
    varList.push_back( "U10" );  // U at 10 m
    varList.push_back( "QCLOUD" );  // cloud water mixing ratio
    return varList;
}

/**
* Fetch a unique identifier for the forecast.  This string can be used for
* identifying and storing.
*
* @return unique name
*/
std::string wrfSurfInitialization::getForecastIdentifier()
{
    return std::string( "WRF-SURFACE" );
}

/**
* Fetch the path to the forecast
*
* @return path to the forecast
*/
std::string wrfSurfInitialization::getPath()
{
    return std::string( "");
}

int wrfSurfInitialization::getStartHour()
{
    return 0;
}

int wrfSurfInitialization::getEndHour()
{
    return 84;
}

/**
* function for converting the read in netcdf units to WindNinja units
* specifically for WindNinja velocityUnits
* throws an error if the input unit_string does not yet have a conversion written in the code
* if the input string is "", implies the units were not available for read in, the default units will be returned with a warning
* @param unit_string The read in netcdf unit string to be converted to a WindNinja velocityUnit
* @return spd_units The corresponding WindNinja velocityUnit enum to the input unit_string
*/
velocityUnits::eVelocityUnits wrfSurfInitialization::processVelocityUnits(std::string unit_string)
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
temperatureUnits::eTempUnits wrfSurfInitialization::processTemperatureUnits(std::string unit_string)
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
* Checks the downloaded data to see if it is all valid.
* May not be functional yet for this class...
*/
void wrfSurfInitialization::checkForValidData(std::string timeZoneString)
{
    (void)timeZoneString;

    GDALDataset *srcDS;
    srcDS = (GDALDataset*)GDALOpen(wxModelFileName.c_str(), GA_ReadOnly);
    if(srcDS == NULL)
    {
        throw std::runtime_error("Could not open forecast file, bad forecast file.");
    }
    GDALClose((GDALDatasetH)srcDS);

    // open ds variable by variable
    std::string temp;
    std::string srcWkt;
    int nBands = 0;
    bool noDataValueExists;
    bool noDataIsNan;

    velocityUnits::eVelocityUnits spd_units = velocityUnits::metersPerSecond;  // initialize to default units
    temperatureUnits::eTempUnits T_units = temperatureUnits::K;

    std::vector<std::string> varList = getVariableList();

    //Acquire a lock to protect the non-thread safe netCDF library
#ifdef _OPENMP
    omp_guard netCDF_guard(netCDF_lock);
#endif

    for( unsigned int i = 0;i < varList.size();i++ ) {

        temp = "NETCDF:\"" + wxModelFileName + "\":" + varList[i];
        CPLDebug("WRF", "temp = %s", temp.c_str());

        CPLPushErrorHandler(&CPLQuietErrorHandler);
        srcDS = (GDALDataset*)GDALOpen( temp.c_str(), GA_ReadOnly );
        CPLPopErrorHandler();
        if(srcDS == NULL)
        {
            throw badForecastFile("Could not get NETCDF variable '"+varList[i]+"' from forecast file, bad forecast file.");
        }

        //Get total bands (time steps)
        nBands = srcDS->GetRasterCount();
        CPLDebug("WRF", "bands = %d", nBands);
        int nXSize, nYSize;
        GDALRasterBand *poBand;
        int pbSuccess;
        double dfNoData;
        double *padfScanline;

        nXSize = srcDS->GetRasterXSize();
        nYSize = srcDS->GetRasterYSize();

        CPLDebug("WRF", "nXSize = %d", nXSize);
        CPLDebug("WRF", "nYSize = %d", nYSize);

        //loop over all bands for this variable (bands are time steps)
        for(int j = 1; j <= nBands; j++)
        {
            poBand = srcDS->GetRasterBand( j );

            pbSuccess = 0;
            dfNoData = poBand->GetNoDataValue(&pbSuccess);
            if(pbSuccess == false)
            {
                noDataValueExists = false;
            }
            else
            {
                noDataValueExists = true;
                noDataIsNan = std::isnan(dfNoData);
            }

            const char * poBand_units = poBand->GetUnitType();
            if(varList[i] == "T2")
            {
                T_units = processTemperatureUnits(poBand_units);
            }
            if(varList[i] == "V10" || varList[i] == "U10")
            {
                spd_units = processVelocityUnits(poBand_units);
            }

            // set the data
            padfScanline = new double[nXSize*nYSize];
            poBand->RasterIO(GF_Read, 0, 0, nXSize, nYSize, padfScanline, nXSize, nYSize, GDT_Float64, 0, 0);
            for(int k = 0;k < nXSize*nYSize; k++)
            {
                double current_val = padfScanline[k];
                // Check if value is no data (if no data value was defined in file)
                if(noDataValueExists)
                {
                    if(noDataIsNan)
                    {
                        if(std::isnan(current_val))
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
* Static identifier to determine if the netcdf file is a WRF forecast.
* Uses netcdf c api
*
* @param fileName netcdf filename
* @return true if the forecast is a WRF forecast
*/
bool wrfSurfInitialization::identify( std::string fileName )
{
    bool identified = true;

    //Acquire a lock to protect the non-thread safe netCDF library
#ifdef _OPENMP
    omp_guard netCDF_guard(netCDF_lock);
#endif

    /*
     * Open the dataset
     */

    int status, ncid, ndims, nvars, ngatts, unlimdimid;
    status = nc_open( fileName.c_str(), 0, &ncid );
    if ( status != NC_NOERR ) {
        identified = false;
    }

    /*
     * Check the global attributes for the following tag:
     * :TITLE = "...WRF...""
     */
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
* Uses netcfd c api commands to get wrf netcdf file global attributes, to use later
* to set the gdal dataset projection and geotransform information, which are required
* to allow the wrf netcdf file data to be accessible by standard gdal commands.
* @param dx The cell size x value of the dataset to be filled.
* @param dy The cell size y value of the dataset to be filled.
* @param cenLat The center latitude value of the dataset to be filled.
* @param cenLon The center longitude value of the dataset to be filled.
* @param projString The projection string of the dataset to be filled.
* Note: there are additional wrf netcdf file global attributes that are accessed and used to get and process the projection string,
* but are not used outside this function, so they are not returned. These include mapProj, moadCenLat, standLon, trueLat1, trueLat2.
*/
void wrfSurfInitialization::getNcGlobalAttributes(float &dx, float &dy, float &cenLat, float &cenLon, std::string &projString)
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

    CPLDebug("WRF", "MAP_PROJ = %d", mapProj);

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

    // Get global attribute BOTTOM-TOP_GRID_DIMENSION
    status = nc_inq_att( ncid, NC_GLOBAL, "BOTTOM-TOP_GRID_DIMENSION", &type, &len );
    if( status != NC_NOERR ){
        ostringstream os;
        os << "Global attribute BOTTOM-TOP_GRID_DIMENSION  in the netcdf file: "
        << wxModelFileName
        << " cannot be opened\n";
        throw std::runtime_error( os.str() );
    }
    else {
        status = nc_get_att_int( ncid, NC_GLOBAL, "BOTTOM-TOP_GRID_DIMENSION", &wxModel_nLayers );
    }

    // Get global attribute WEST-EAST_GRID_DIMENSION
    // Not currently used. WX model x/y dims set based on
    // reprojected image (in DEM space).
    /*status = nc_inq_att( ncid, NC_GLOBAL, "WEST-EAST_GRID_DIMENSION", &type, &len );
    if( status != NC_NOERR ){
        ostringstream os;
        os << "Global attribute WEST-EAST_GRID_DIMENSION  in the netcdf file: "
        << wxModelFileName
        << " cannot be opened\n";
        throw std::runtime_error( os.str() );
    }
    else {
        status = nc_get_att_int( ncid, NC_GLOBAL, "WEST-EAST_GRID_DIMENSION", &wxModel_nCols );
    }*/

    // Get global attribute SOUTH-NORTH_GRID_DIMENSION
    // Not currently used. WX model x/y dims set based on
    // reprojected image (in DEM space).
    /*status = nc_inq_att( ncid, NC_GLOBAL, "SOUTH-NORTH_GRID_DIMENSION", &type, &len );
    if( status != NC_NOERR ){
        ostringstream os;
        os << "Global attribute SOUTH-NORTH_GRID_DIMENSION  in the netcdf file: "
        << wxModelFileName
        << " cannot be opened\n";
        throw std::runtime_error( os.str() );
    }
    else {
        status = nc_get_att_int( ncid, NC_GLOBAL, "SOUTH-NORTH_GRID_DIMENSION", &wxModel_nRows );
    }*/

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
    else if(mapProj == 6)  //lat/long
    {
        throw badForecastFile("Cannot initialize with a forecast file in lat/long spacing. Forecast file must be in a projected coordinate system.");
    }
    else
    {
        throw badForecastFile("Cannot determine projection from the forecast file information.");
    }
}

/**
* Sets the surface grids based on a WRF (surface only!) forecast.
* @param input The WindNinjaInputs for misc. info.
* @param airGrid The air temperature grid to be filled.
* @param cloudGrid The cloud cover grid to be filled.
* @param uGrid The u velocity grid to be filled.
* @param vGrid The v velocity grid to be filled.
* @param wGrid The w velocity grid to be filled (filled with zeros here?).
*/
void wrfSurfInitialization::setSurfaceGrids( WindNinjaInputs &input,
        AsciiGrid<double> &airGrid,
        AsciiGrid<double> &cloudGrid,
        AsciiGrid<double> &uGrid,
        AsciiGrid<double> &vGrid,
        AsciiGrid<double> &wGrid )
{
    GDALWarpOptions* psWarpOptions = nullptr;

    int bandNum = -1;

    //get time list
    std::vector<boost::local_time::local_date_time> timeList( getTimeList(input.ninjaTimeZone) );
    //Search time list for our time to identify our band number for cloud/speed/dir
    for(unsigned int i = 0; i < timeList.size(); i++)
    {
        if(input.ninjaTime == timeList[i])
        {
            bandNum = i + 1;
            CPLDebug("WRF", "Grabbing band number: %d", bandNum);
            break;
        }
    }
    if(bandNum < 0)
        throw std::runtime_error("Could not match ninjaTime with a band number in the forecast file.");

    GDALDataset* poDS;
    //attempt to grab the projection from the dem?
    //check for member prjString first
    std::string dstWkt;
    dstWkt = input.dem.prjString;
    if ( dstWkt.empty() ) {
        //try to open original
        poDS = (GDALDataset*)GDALOpen( input.dem.fileName.c_str(), GA_ReadOnly );
        if( poDS == NULL ) {
            throw std::runtime_error("Could not open input dem file.");
        }
        dstWkt = poDS->GetProjectionRef();
        if( dstWkt.empty() ) {
            throw std::runtime_error("Could not get projection reference from input dem file.");
        }
        GDALClose((GDALDatasetH) poDS );
    }

//==========get global attributes to set projection===========================
    //Acquire a lock to protect the non-thread safe netCDF library
#ifdef _OPENMP
    omp_guard netCDF_guard(netCDF_lock);
#endif
    float dx;
    float dy;
    float cenLat;
    float cenLon;
    std::string projString;
    getNcGlobalAttributes(dx, dy, cenLat, cenLon, projString);
//======end get global attributes========================================

    CPLPushErrorHandler(&CPLQuietErrorHandler);
    poDS = (GDALDataset*)GDALOpenShared( input.forecastFilename.c_str(), GA_ReadOnly );
    CPLPopErrorHandler();
    if( poDS == NULL ) {
        throw std::runtime_error("Could not open forecast file, bad forecast file.");
    }
    GDALClose((GDALDatasetH) poDS); // close original wxModel file

    // open ds one by one, set geotranform, set projection, warp, then write to grid
    GDALDataset *srcDS, *wrpDS;
    std::string temp;
    std::vector<std::string> varList = getVariableList();

    velocityUnits::eVelocityUnits spd_units = velocityUnits::metersPerSecond;  // initialize to default units
    temperatureUnits::eTempUnits T_units = temperatureUnits::K;

    double coordinateTransformationAngle = 0.0;

    /*
     * Set the initial values in the warped dataset to no data
     */
    for( unsigned int i = 0;i < varList.size();i++ ) {

        temp = "NETCDF:\"" + input.forecastFilename + "\":" + varList[i];
        
        CPLPushErrorHandler(&CPLQuietErrorHandler);
        srcDS = (GDALDataset*)GDALOpenShared( temp.c_str(), GA_ReadOnly );
        CPLPopErrorHandler();
        if( srcDS == NULL ) {
            throw std::runtime_error("Could not get NETCDF variable '"+varList[i]+"' from forecast file, bad forecast file.");
        }

        CPLDebug("WRF", "varList[i] = %s", varList[i].c_str());

        /*
         * Set up spatial reference stuff for setting projections
         * and geotransformations
         */
        OGRSpatialReference oSRS, *poLatLong;
        char *srcWKT = NULL;
        char* prj2 = (char*)projString.c_str();
        oSRS.importFromWkt(&prj2);
        oSRS.exportToWkt(&srcWKT);

        CPLDebug("WRF", "srcWKT= %s", srcWKT);

        poLatLong = oSRS.CloneGeogCS();

        /*
         * Transform domain center from lat/long to WRF space
         */
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
        {
            throw std::runtime_error("Transformation failed.");
        }

        CPLDebug("WRF", "xCenter, yCenter= %f, %f", xCenter, yCenter);

        /*
         * Set the geostransform for the WRF file
         * upper corner is calculated from transformed x, y
         * (in WRF space)
         */

        double ncols, nrows;
        int nXSize = srcDS->GetRasterXSize();
        int nYSize = srcDS->GetRasterYSize();
        ncols = (double)(nXSize)/2;
        nrows = (double)(nYSize)/2;

        double adfGeoTransform[6] = {(xCenter-(ncols*dx)), dx, 0,
                                    (yCenter+(nrows*dy)),
                                    0, -(dy)};

        CPLDebug("WRF", "ulcornerX, ulcornerY= %f, %f", (xCenter-(ncols*dx)), (yCenter+(nrows*dy)));
        CPLDebug("WRF", "nXSize, nYsize= %d, %d", nXSize, nYSize);
        CPLDebug("WRF", "dx= %f", dx);

        srcDS->SetGeoTransform(adfGeoTransform);


        /*if( varList[i] == "U10" ) {  // just a check
            AsciiGrid<double> tempSrcGrid;   
            GDAL2AsciiGrid( srcDS, 12, tempSrcGrid );
            tempSrcGrid.write_Grid("before_warp", 2);
        }*/

        int nBandCount = srcDS->GetRasterCount();
        CPLDebug("WRF", "band count = %d", nBandCount);

        // get the noDataValue from the current band
        GDALRasterBand *poBand = srcDS->GetRasterBand( bandNum );
        int pbSuccess;
        double dfNoData = poBand->GetNoDataValue(&pbSuccess);
        if(pbSuccess == false)
        {
            dfNoData = -9999.0;
        }

        const char * poBand_units = poBand->GetUnitType();
        if(varList[i] == "T2")
        {
            T_units = processTemperatureUnits(poBand_units);
        }
        if(varList[i] == "V10" || varList[i] == "U10")
        {
            spd_units = processVelocityUnits(poBand_units);
        }

        // Setup warp options
        psWarpOptions = GDALCreateWarpOptions();
        psWarpOptions->padfDstNoDataReal = (double*)CPLMalloc(sizeof(double) * nBandCount);
        psWarpOptions->padfDstNoDataImag = (double*)CPLMalloc(sizeof(double) * nBandCount);
        for(int b = 0; b < nBandCount; b++)
        {
            psWarpOptions->padfDstNoDataReal[b] = dfNoData;
            psWarpOptions->padfDstNoDataImag[b] = dfNoData;
        }

        psWarpOptions->papszWarpOptions = CSLSetNameValue(psWarpOptions->papszWarpOptions, "INIT_DEST", "NO_DATA");
        if(pbSuccess == false)  // if GDALGetRasterNoDataValue() fails to return that a NO_DATA value is in the source dataset
        {
            psWarpOptions->papszWarpOptions = CSLSetNameValue(psWarpOptions->papszWarpOptions, "INIT_DEST", boost::lexical_cast<std::string>(dfNoData).c_str());
        }

        // set the dataset projection
        int rc = srcDS->SetProjection(projString.c_str());

        // compute the coordinateTransformationAngle, the angle between the y coordinate grid lines of the pre-warped and warped datasets,
        // going FROM the y coordinate grid line of the pre-warped dataset TO the y coordinate grid line of the warped dataset
        // in this case, going FROM weather model projection coordinates TO dem projection coordinates
        if( varList[i] == "U10" )
        {
            if( CSLTestBoolean(CPLGetConfigOption("DISABLE_COORDINATE_TRANSFORMATION_ANGLE_CALCULATIONS", "FALSE")) == false )
            {
                // direct calculation of FROM wx TO dem, already has the appropriate sign
                if(!GDALCalculateCoordinateTransformationAngle( srcDS, coordinateTransformationAngle, dstWkt.c_str() ))  // this is FROM wx TO dem
                {
                    input.Com->ninjaCom(ninjaComClass::ninjaWarning, "Unable to calculate coordinate transform angle for the wxModel.");
                }
            }
        }

        wrpDS = (GDALDataset*) GDALAutoCreateWarpedVRT( srcDS, srcWKT,
                                                        dstWkt.c_str(),
                                                        GRA_NearestNeighbour,
                                                        1.0, psWarpOptions );
        if(wrpDS == NULL)
        {
            throw std::runtime_error("Could not warp the forecast file, possibly non-uniform grid.");
        }

        //=======for testing==================================//
        /*AsciiGrid<double> tempGrid;
        AsciiGrid<double> temp2Grid;
        
        if( varList[i] == "U10" ) {
            GDAL2AsciiGrid( wrpDS, 12, tempGrid );
            if( std::isnan( dfNoData ) ) {
                tempGrid.set_noDataValue(-9999.0);
                tempGrid.replaceNan( -9999.0 );
            }
            tempGrid.write_Grid("after_warp", 2);


            //Make final grids with same header as dem
            temp2Grid.set_headerData(input.dem);
            temp2Grid.interpolateFromGrid(tempGrid, AsciiGrid<double>::order1);
            temp2Grid.set_noDataValue(-9999.0);
            temp2Grid.write_Grid("after_interpolation", 2);
        }*/
        //=======end testing=================================//

        CPLDebug("WRF", "band number to write = %d", bandNum);

        if( varList[i] == "T2" ) {
            GDAL2AsciiGrid( wrpDS, bandNum, airGrid );
            temperatureUnits::toBaseUnits( airGrid, T_units );
            if( std::isnan( dfNoData ) ) {
                airGrid.set_noDataValue(-9999.0);
                airGrid.replaceNan( -9999.0 );
            }
        }
        else if( varList[i] == "V10" ) {
            GDAL2AsciiGrid( wrpDS, bandNum, vGrid );
            velocityUnits::toBaseUnits( vGrid, spd_units );
            if( std::isnan( dfNoData ) ) {
                vGrid.set_noDataValue(-9999.0);
                vGrid.replaceNan( -9999.0 );
            }
        }
        else if( varList[i] == "U10" ) {
            GDAL2AsciiGrid( wrpDS, bandNum, uGrid );
            velocityUnits::toBaseUnits( uGrid, spd_units );
            if( std::isnan( dfNoData ) ) {
                uGrid.set_noDataValue(-9999.0);
                uGrid.replaceNan( -9999.0 );
            }
        }
        else if( varList[i] == "QCLOUD" ) {
            GDAL2AsciiGrid( wrpDS, bandNum, cloudGrid );
            if( std::isnan( dfNoData ) ) {
                cloudGrid.set_noDataValue(-9999.0);
                cloudGrid.replaceNan( -9999.0 );
            }
        }

        CPLFree(srcWKT);
        delete poCT;
        GDALClose((GDALDatasetH) srcDS );
        GDALClose((GDALDatasetH) wrpDS );

        GDALDestroyWarpOptions(psWarpOptions);
    }

    //don't allow small negative values in cloud cover
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

    //use the coordinateTransformationAngle to correct the angles of the output dataset
    //to convert from the original dataset projection angles to the warped dataset projection angles
    if( CSLTestBoolean(CPLGetConfigOption("DISABLE_COORDINATE_TRANSFORMATION_ANGLE_CALCULATIONS", "FALSE")) == false )
    {
        // need an intermediate spd and dir set of ascii grids
        AsciiGrid<double> speedGrid;
        AsciiGrid<double> dirGrid;
        speedGrid.set_headerData(uGrid);
        dirGrid.set_headerData(uGrid);
        for(int i=0; i<uGrid.get_nRows(); i++)
        {
            for(int j=0; j<uGrid.get_nCols(); j++)
            {
                if( uGrid(i,j) == uGrid.get_NoDataValue() || vGrid(i,j) == vGrid.get_NoDataValue() )
                {
                    speedGrid(i,j) = speedGrid.get_NoDataValue();
                    dirGrid(i,j) = dirGrid.get_NoDataValue();
                } else
                {
                    wind_uv_to_sd(uGrid(i,j), vGrid(i,j), &(speedGrid)(i,j), &(dirGrid)(i,j));
                }
            }
        }

        // use the coordinateTransformationAngle to correct each spd,dir, u,v dataset for the warp
        for(int i=0; i<dirGrid.get_nRows(); i++)
        {
            for(int j=0; j<dirGrid.get_nCols(); j++)
            {
                if( speedGrid(i,j) != speedGrid.get_NoDataValue() && dirGrid(i,j) != dirGrid.get_NoDataValue() )
                {
                    dirGrid(i,j) = wrap0to360( dirGrid(i,j) - coordinateTransformationAngle ); //convert FROM wxModel projection coordinates TO dem projected coordinates
                    // always recalculate the u and v grids from the corrected dir grid, the changes need to go together
                    wind_sd_to_uv(speedGrid(i,j), dirGrid(i,j), &(uGrid)(i,j), &(vGrid)(i,j));
                }
            }
        }

        // cleanup the intermediate grids
        speedGrid.deallocate();
        dirGrid.deallocate();
    }
}

double wrfSurfInitialization::Get_Wind_Height()
{
    return 10.0;
}

