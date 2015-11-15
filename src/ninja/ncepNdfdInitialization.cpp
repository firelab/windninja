/*****************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja
 * Purpose:  NCEP NDFD implementation of wxModelInitialization
 * Author:   Kyle Shannon <ksshannon@gmail.com>
 *
 *****************************************************************************
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
 ****************************************************************************/

#include "ncepNdfdInitialization.h"

/**
* Constructor for the class initializes the variable names
*
*/
ncepNdfdInitialization::ncepNdfdInitialization() : wxModelInitialization()
{
    heightVarName = "height_above_ground1";
    path = "/thredds/ncss/grib/NCEP/NDFD/NWS/CONUS/NOAAPORT/Best?north=USER_NORTH&west=USER_WEST&east=USER_EAST&south=USER_SOUTH&time_start=present&time_duration=PTUSER_TIMEH&accept=netcdf";
    LoadFromCsv();
}

/**
* Copy constructor.
* @param A Right hand side to be copied.
*/
ncepNdfdInitialization::ncepNdfdInitialization(ncepNdfdInitialization const& A ) : wxModelInitialization(A)
{
    wxModelFileName = A.wxModelFileName;
}

/**
* Destructor.
*/
ncepNdfdInitialization::~ncepNdfdInitialization()
{
}

/**
* Equals operator.
* @param A Value to set equal to.
* @return a copy of an object
*/
ncepNdfdInitialization& ncepNdfdInitialization::operator= (ncepNdfdInitialization const& A)
{
    if(&A != this) {
        wxModelInitialization::operator=(A);
    }
    return *this;
}

/**
*@brief wrapper for GetWindHeight
*@return return wind height
*/
double ncepNdfdInitialization::Get_Wind_Height()
{
    return GetWindHeight("height_above_ground1");
}

/**
*@brief Returns horizontal grid resolution of the model
*@return return grid resolution (in km unless < 1, then degrees)
*/
double ncepNdfdInitialization::getGridResolution()
{
    return 2.5;
}


std::vector<blt::local_date_time> ncepNdfdInitialization::getTimeList(blt::time_zone_ptr timeZonePtr)
{
    return wxModelInitialization::getTimeList("Total_cloud_cover_entire_atmosphere_single_layer_layer", timeZonePtr);
}

/**
* Fetch a list of times that the forecast holds for the temp
* variable.  This is needed due to the min/max storage of temperature
* in the ndfd model ouptut.  Uses the netcdf api.
*
* @param timeZonePtr Pointer to a boost time zone object.
* @throw runtime_error for bad file i/o
* @return a vector of boost::posix_time::ptime objects for the forecast
*/
std::vector<blt::local_date_time> ncepNdfdInitialization::getTempTimeList(eTempType type, blt::time_zone_ptr timeZonePtr)
{
    if(type == max)
        return wxModelInitialization::getTimeList("Maximum_temperature_height_above_ground_12_Hour_Maximum", timeZonePtr);
    else if(type == min)
        return wxModelInitialization::getTimeList("Minimum_temperature_height_above_ground_12_Hour_Minimum", timeZonePtr);
}

/**
* Fetch the variable names for this model
*
* @return a vector of variable names
*/
std::vector<std::string> ncepNdfdInitialization::getVariableList()
{
    std::vector<std::string> varList;
    varList.push_back( "Maximum_temperature_height_above_ground_12_Hour_Maximum" );
    varList.push_back( "Minimum_temperature_height_above_ground_12_Hour_Minimum" );
    varList.push_back( "Total_cloud_cover_entire_atmosphere_single_layer_layer" );
    varList.push_back( "Wind_direction_from_which_blowing_height_above_ground" );
    varList.push_back( "Wind_speed_height_above_ground" );
    return varList;
}

/**
* Fetch a unique identifier for the forecast.  this string can be used for
* identifying and storing.
*
* @return unique name
*/
std::string ncepNdfdInitialization::getForecastIdentifier()
{
    return std::string( "UCAR-NDFD-CONUS-2.5-KM" );
}

int ncepNdfdInitialization::getStartHour()
{
    return 6;
}

int ncepNdfdInitialization::getEndHour()
{
    return 168;
}

/**
* Checks the downloaded data to see if it is all valid.
*/
void ncepNdfdInitialization::checkForValidData()
{
    //just make up a "dummy" timezone for use here
    boost::local_time::time_zone_ptr zone(new boost::local_time::posix_time_zone("MST-07"));

    //get time list
    std::vector<boost::local_time::local_date_time> timeList( getTimeList(zone) );
    std::vector<boost::local_time::local_date_time> timeListMax( getTempTimeList(max, zone) );
    std::vector<boost::local_time::local_date_time> timeListMin( getTempTimeList(min, zone) );


    boost::posix_time::ptime pt_low(boost::gregorian::date(1900,boost::gregorian::Jan,1), boost::posix_time::hours(12));
    boost::posix_time::ptime pt_high(boost::gregorian::date(2100,boost::gregorian::Jan,1), boost::posix_time::hours(12));
    boost::local_time::local_date_time low_time(pt_low, zone);
    boost::local_time::local_date_time high_time(pt_high, zone);

    //check times
    for(unsigned int i = 0; i < timeList.size(); i++)
    {
        if(timeList[i].is_special())    //if time is any special value (not_a_date_time, infinity, etc.)
            throw badForecastFile("Bad time in forecast file.");
        if(timeList[i] < low_time || timeList[i] > high_time)
            throw badForecastFile("Bad time in forecast file.");
    }
    for(unsigned int i = 0; i < timeListMax.size(); i++)
    {
        if(timeListMax[i].is_special())    //if time is any special value (not_a_date_time, infinity, etc.)
            throw badForecastFile("Bad time in forecast file.");
        if(timeListMax[i] < low_time || timeListMax[i] > high_time)
            throw badForecastFile("Bad time in forecast file.");
    }
    for(unsigned int i = 0; i < timeListMin.size(); i++)
    {
        if(timeListMin[i].is_special())    //if time is any special value (not_a_date_time, infinity, etc.)
            throw badForecastFile("Bad time in forecast file.");
        if(timeListMin[i] < low_time || timeListMin[i] > high_time)
            throw badForecastFile("Bad time in forecast file.");
    }


    // open ds variable by variable
    GDALDataset *srcDS;
    std::string temp;
    std::string srcWkt;
    int nBands = 0;
    bool noDataValueExists;
    bool noDataIsNan;

    std::vector<std::string> varList = getVariableList();

    //Acquire a lock to protect the non-thread safe netCDF libraryi
#ifdef _OPENMP
    omp_guard netCDF_guard(netCDF_lock);
#endif

    for( unsigned int i = 0;i < varList.size();i++ ) {

        temp = "NETCDF:" + wxModelFileName + ":" + varList[i];

        srcDS = (GDALDataset*)GDALOpen( temp.c_str(), GA_ReadOnly );
        if( srcDS == NULL )
            throw badForecastFile("Cannot open forecast file.");

        srcWkt = srcDS->GetProjectionRef();

        if( srcWkt.empty() )
            throw badForecastFile("Forecast file doesn't have projection information.");

        //Get total bands (time steps)
        nBands = srcDS->GetRasterCount();
        int nXSize, nYSize;
        GDALRasterBand *poBand;
        int pbSuccess;
        double dfNoData;
        double *padfScanline;

        nXSize = srcDS->GetRasterXSize();
        nYSize = srcDS->GetRasterYSize();

        //loop over all bands for this variable (bands are time steps)
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

            //set the data
            padfScanline = new double[nXSize*nYSize];
            poBand->RasterIO(GF_Read, 0, 0, nXSize, nYSize, padfScanline, nXSize, nYSize,
                    GDT_Float64, 0, 0);
            for(int k = 0;k < nXSize*nYSize; k++)
            {
                //Check if value is no data (if no data value was defined in file)
                if(noDataValueExists)
                {
                    if(noDataIsNan)
                    {
                        if(CPLIsNan(padfScanline[k]))
                            throw badForecastFile("Forecast file contains no_data values.");
                    }else
                    {
                        if(padfScanline[k] == dfNoData)
                            throw badForecastFile("Forecast file contains no_data values.");
                    }
                }

                if( varList[i] == "Maximum_temperature_height_above_ground_12_Hour_Maximum" )   //units are Kelvin
                {
                    if(padfScanline[k] < 180.0 || padfScanline[k] > 340.0)  //these are near the most extreme temperatures ever recored on earth
                        throw badForecastFile("Temperature is out of range in forecast file.");
                }
                else if( varList[i] == "Minimum_temperature_height_above_ground_12_Hour_Minimum" )   //units are Kelvin
                {
                    if(padfScanline[k] < 180.0 || padfScanline[k] > 340.0)  //these are near the most extreme temperatures ever recored on earth
                        throw badForecastFile("Temperature is out of range in forecast file.");
                }
                else if( varList[i] == "Wind_direction_from_which_blowing_height_above_ground" )  //units are m/s
                {
                    if(padfScanline[k] < 0.0 || padfScanline[k] > 360.0)
                        throw badForecastFile("Wind direction is out of range in forecast file.");
                }
                else if( varList[i] == "Wind_speed_height_above_ground" )  //units are m/s
                {
                    if(padfScanline[k] < 0.0 || padfScanline[k] > 220.0)
                        throw badForecastFile("Wind speed is out of range in forecast file.");
                }
                else if( varList[i] == "Total_cloud_cover_entire_atmosphere_single_layer_layer" )  //units are percent
                {
                    if(padfScanline[k] < 0.0 || padfScanline[k] > 100.0)
                        throw badForecastFile("Total cloud cover is out of range in forecast file.");
                }
            }

            delete [] padfScanline;
        }

        GDALClose((GDALDatasetH) srcDS );
    }
}

/**
* Static identifier to determine if the netcdf file is an ndfd forecast.
* Uses netcdf c api
*
* @param fileName netcdf filename
*
* @return true if the forecast is a NCEP NDFD forecast
*/
bool ncepNdfdInitialization::identify( std::string fileName )
{
    /*
     * Open the dataset
     */

    bool identified = true;

    //Acquire a lock to protect the non-thread safe netCDF library
#ifdef _OPENMP
    omp_guard netCDF_guard(netCDF_lock);
#endif

    int status, ncid, ndims, nvars, ngatts, unlimdimid;

    status = nc_open( fileName.c_str(), 0, &ncid );

    if ( status != NC_NOERR )
        identified = false;

    /*
     * If we can't get simple data from the file, return false
     */
    status = nc_inq(ncid, &ndims, &nvars, &ngatts, &unlimdimid);
    if ( status != NC_NOERR )
        identified = false;

    /*
     * Check the variable names, return false if any aren't found
     */
    int varid;
    std::vector<std::string> varList = getVariableList();
    for( unsigned int i = 0;i < varList.size();i++ ) {
        status = nc_inq_varid( ncid, varList[i].c_str(), &varid );
        if( status != NC_NOERR )
            identified = false;
    }

    /*
     * Check the global attributes for the following tag:
     * :Generating_Model = "NDFD product generated by NCEP/HPC"
     */
    size_t len;
    nc_type type;
    char* model;
    status = nc_inq_att( ncid, NC_GLOBAL, "Analysis_or_forecast_generating_process_identifier_defined_by_originating_centre", &type, &len );
    if( status != NC_NOERR )
        identified = false;
    else {
        model = new char[len + 1];
        status = nc_get_att_text( ncid, NC_GLOBAL, "Analysis_or_forecast_generating_process_identifier_defined_by_originating_centre", model );
        model[len] = '\0';
        std::string s( model );
        if( s.find( "NDFD product generated by NCEP/HPC" ) == s.npos ){
            identified = false;
        }
        delete [] model;
    }

    /*
     * Close the dataset
     */
    status = nc_close( ncid );

    return identified;
}

/**
* Sets the surface grids based on a ncepNdfd surface forecast.
* Use times to identify which band numbers to extract.
* Note that cloud cover, speed, and direction are coincident in time, but
* temperature is just given as a max and min for the day.  So we'll need to
* interpolate in time to get a temperature grid for each cloud/speed/dir time.
*
* @param input The WindNinjaInputs for misc. info.
* @param airGrid The air temperature grid to be filled.  Should be filled
*                as Kelvin.
* @param cloudGrid The cloud cover grid to be filled.  Should be filled as
*                  fraction of cloud cover (0-1).
* @param uGrid The u velocity grid to be filled.  Should be filled with m/s.
* @param vGrid The v velocity grid to be filled.  Should be filled with m/s.
* @param wGrid The w velocity grid to be filled (filled with zeros here?).  Should be filled with m/s.
*/
void ncepNdfdInitialization::setSurfaceGrids(  WindNinjaInputs &input,
        AsciiGrid<double> &airGrid,
        AsciiGrid<double> &cloudGrid,
        AsciiGrid<double> &uGrid,
        AsciiGrid<double> &vGrid,
        AsciiGrid<double> &wGrid )
{

    AsciiGrid<double> airHighGrid;
    AsciiGrid<double> airLowGrid;
    AsciiGrid<double> speedGrid;
    AsciiGrid<double> directionGrid;

    int bandNum = -1;

    //get cloud/speed/dir time list
    std::vector<blt::local_date_time> timeList( getTimeList(input.ninjaTimeZone) );
    //Search time list for our time to identify our band number for cloud/speed/dir
    for(unsigned int i = 0; i < timeList.size(); i++)
    {
        if(input.ninjaTime == timeList[i])
        {
            bandNum = i + 1;
            break;
        }

    }
    if(bandNum <= 0)
        throw std::runtime_error("Could not match ninjaTime with a band number in the forecast file.");



    //Find temperature band(s)
    int bandNumTempLow = -1;    //This is the band that is less than our time
    int bandNumTempHigh = -1;   //This is the band that is greater than our time
    int bandNumTempLuck = -1;   //This is the band to use if we're lucky enough to have a temp time

    std::vector<blt::local_date_time> timeListMaxTemp( getTempTimeList(max, input.ninjaTimeZone) );
    std::vector<blt::local_date_time> timeListMinTemp( getTempTimeList(min, input.ninjaTimeZone) );
    if( timeListMaxTemp.size() == 0 && timeListMinTemp.size() == 0 )
    {
        throw badForecastFile( "No temperature data in forecast file" );
    }
    std::vector<blt::local_date_time> timeListTemp;

    bool areEqual;
    bool minFirst;

    if(timeListMaxTemp.size() == timeListMinTemp.size())
        areEqual = true;
    else
        areEqual = false;

    if(timeListMaxTemp[0] > timeListMinTemp[0])
        minFirst = true;
    else
        minFirst = false;

    if(minFirst) //if min list starts first
    {
        for(unsigned int i=0; i<timeListMaxTemp.size(); i++)
        {
            timeListTemp.push_back(timeListMinTemp[i]);
            timeListTemp.push_back(timeListMaxTemp[i]);
        }
        if(!areEqual)
            timeListTemp.push_back(timeListMinTemp[timeListMinTemp.size()-1]);

    }else   //max list starts first
    {
        for(unsigned int i=0; i<timeListMinTemp.size(); i++)
        {
            timeListTemp.push_back(timeListMaxTemp[i]);
            timeListTemp.push_back(timeListMinTemp[i]);
        }
        if(!areEqual)
            timeListTemp.push_back(timeListMaxTemp[timeListMaxTemp.size()-1]);
    }

    //Search time list for an exactly matching time first
    for(unsigned int i = 0; i < timeListTemp.size(); i++)
    {
        if(input.ninjaTime == timeListTemp[i])  //Check for exact match
        {
            bandNumTempLuck = i + 1;
            break;
        }
    }

    if(bandNumTempLuck < 0) //If we weren't lucky
    {
        if(input.ninjaTime < timeListTemp[0])   //If the ninjaTime is less than all times, use lowest time band
        {
            bandNumTempLuck = 1;
        }else if(input.ninjaTime > timeListTemp[timeListTemp.size()-1]) //If the ninjaTime is greater than all times, use highest time band
        {
            bandNumTempLuck = timeListTemp.size();
        }else   //Else, find the 2 time bands that span the ninjaTime
        {
            //Search time list for "low" time
            for(unsigned int i = 0; i < timeListTemp.size(); i++)
            {
                bandNumTempLow = i;
                if(timeListTemp[i] > input.ninjaTime)  //Check if we're over
                    break;
            }
            //Search time list for "high" time
            for(int i = timeListTemp.size()-1; i >= 0; i--)
            {
                bandNumTempHigh = i;
                if(timeListTemp[i] < input.ninjaTime)  //Check for exact match
                {
                    bandNumTempHigh = i + 2;
                    break;
                }
            }
        }
    }

    //    for(unsigned int i = 0; i < timeListTemp.size(); i++)
    //    {
    //        std::cout << "timeListTemp = " << timeListTemp[i] << "\n";
    //    }
    //    for(unsigned int i = 0; i < timeListMinTemp.size(); i++)
    //    {
    //        std::cout << "timeListMinTemp = " << timeListMinTemp[i] << "\n";
    //    }
    //    for(unsigned int i = 0; i < timeListMaxTemp.size(); i++)
    //    {
    //        std::cout << "timeListMaxTemp = " << timeListMaxTemp[i] << "\n";
    //    }
    //
    //    std::cout << "bandNumTempLuck = " << bandNumTempLuck << "\n";
    //    std::cout << "bandNumTempLow = " << bandNumTempLow << "\n";
    //    std::cout << "bandNumTempHigh = " << bandNumTempHigh << "\n";

    if(bandNumTempLuck < 0 && bandNumTempHigh < 0 && bandNumTempLow < 0)
        throw std::runtime_error("Could not match ninjaTime with a band number for temperature in the forecast file.");

    //get some info from the ndfd file in input

    //Acquire a lock to protect the non-thread safe netCDF library
#ifdef _OPENMP
    omp_guard netCDF_guard(netCDF_lock);
#endif

    GDALDataset* poDS;

    //attempt to grab the projection from the dem?
    //check for member prjString first
    std::string dstWkt;
    dstWkt = input.dem.prjString;
    if ( dstWkt.empty() || dstWkt == "!set" ) {
        //try to open original
        poDS = (GDALDataset*)GDALOpen( input.dem.fileName.c_str(), GA_ReadOnly );
        if( poDS == NULL ) {
            CPLDebug( "ncepNdfdInitialization::setSurfaceGrids()",
                    "Bad projection reference" );
            //throw();
        }
        dstWkt = poDS->GetProjectionRef();
        if( dstWkt.empty() ) {
            CPLDebug( "ncepNdfdInitialization::setSurfaceGrids()",
                    "Bad projection reference" );
            //throw()
        }
        GDALClose( (GDALDatasetH)poDS );
    }

    poDS = (GDALDataset*)GDALOpen( input.forecastFilename.c_str(), GA_ReadOnly );

    if( poDS == NULL ) {
        CPLDebug( "ncepNdfdInitialization::setSurfaceGrids()",
                "Bad forecast file" );
    }
    else
        GDALClose( (GDALDatasetH)poDS );

    // open ds one by one and warp, then write to grid
    GDALDataset *srcDS, *wrpDS;
    std::string temp;
    std::string srcWkt;

    std::vector<std::string> varList = getVariableList();

    /*
     * Set the initial values in the warped dataset to no data
     */
    GDALWarpOptions* psWarpOptions;

    for( unsigned int i = 0;i < varList.size();i++ ) {

        temp = "NETCDF:" + input.forecastFilename + ":" + varList[i];

        srcDS = (GDALDataset*)GDALOpenShared( temp.c_str(), GA_ReadOnly );
        if( srcDS == NULL ) {
            CPLDebug( "ncepNdfdInitialization::setSurfaceGrids()",
                    "Bad forecast file" );
        }

        srcWkt = srcDS->GetProjectionRef();

        if( srcWkt.empty() ) {
            CPLDebug( "ncepNdfdInitialization::setSurfaceGrids()",
                    "Bad forecast file" );
            //throw
        }

    GDALRasterBand *poBand = srcDS->GetRasterBand( 1 );
    int pbSuccess;
    double dfNoData = poBand->GetNoDataValue( &pbSuccess );
    psWarpOptions = GDALCreateWarpOptions();

    if( pbSuccess == false )
        dfNoData = -9999.0;

    int nBandCount = srcDS->GetRasterCount();
    psWarpOptions->nBandCount = nBandCount;
    psWarpOptions->panSrcBands = 
        (int*) CPLMalloc( sizeof( int ) * nBandCount );
    psWarpOptions->panDstBands = 
        (int*) CPLMalloc( sizeof( int ) * nBandCount );
    psWarpOptions->padfDstNoDataReal =
        (double*) CPLMalloc( sizeof( double ) * nBandCount );
    psWarpOptions->padfDstNoDataImag =
        (double*) CPLMalloc( sizeof( double ) * nBandCount );

    for( int b = 0;b < srcDS->GetRasterCount();b++ ) {
        psWarpOptions->padfDstNoDataReal[b] = dfNoData;
        psWarpOptions->padfDstNoDataImag[b] = dfNoData;
    }

    psWarpOptions->papszWarpOptions =
        CSLSetNameValue( psWarpOptions->papszWarpOptions,
                 "INIT_DEST", "NO_DATA" );

        wrpDS = (GDALDataset*) GDALAutoCreateWarpedVRT( srcDS, srcWkt.c_str(), dstWkt.c_str(),
                GRA_NearestNeighbour, 1.0, psWarpOptions );
        if(wrpDS == NULL)
        {
            throw badForecastFile("Could not warp the forecast file, "
                                  "possibly non-uniform grid.");
        }

        if(varList[i] == "Maximum_temperature_height_above_ground_12_Hour_Maximum")
        {
            if(bandNumTempLuck < 0)
            {
                if(minFirst)    //if mins are evens
                {
                    if((bandNumTempLow-1)%2 != 0)  //if odd number in array (meaning it's a maxTemp value)
                        GDAL2AsciiGrid( wrpDS, bandNumTempLow/2, airLowGrid );
                    if((bandNumTempHigh-1)%2 != 0)  //if odd number in array (meaning it's a maxTemp value)
                        GDAL2AsciiGrid( wrpDS, bandNumTempHigh/2, airHighGrid );
                }else   //if maxs are evens
                {
                    if((bandNumTempLow-1)%2 == 0)  //if even number in array (meaning it's a maxTemp value)
                        GDAL2AsciiGrid( wrpDS, bandNumTempLow/2, airLowGrid );
                    if((bandNumTempHigh-1)%2 == 0)  //if even number in array (meaning it's a maxTemp value)
                        GDAL2AsciiGrid( wrpDS, bandNumTempHigh/2, airHighGrid );
                }

            }else{
                if(minFirst)
                {
                    if((bandNumTempLuck-1)%2 != 0)  //if odd number in array (meaning it's a maxTemp value)
                        GDAL2AsciiGrid( wrpDS, bandNumTempLuck/2, airGrid );

                }else
                {
                    if((bandNumTempLuck-1)%2 == 0)  //if even number in array (meaning it's a maxTemp value)
                        GDAL2AsciiGrid( wrpDS, bandNumTempLuck/2, airGrid );
                }
            }
        if( CPLIsNan( dfNoData ) ) {
        airHighGrid.set_noDataValue(-9999.0);
        airHighGrid.replaceNan( -9999.0 );
        airLowGrid.set_noDataValue(-9999.0);
        airLowGrid.replaceNan( -9999.0 );
        airGrid.set_noDataValue(-9999.0);
        airGrid.replaceNan( -9999.0 );
        }
        }else if(i == 1)    //Minimum_temperature
        {
            if(bandNumTempLuck < 0)
            {
                if(!minFirst)    //if maxs are evens
                {
                    if((bandNumTempLow-1)%2 != 0)  //if odd number in array (meaning it's a maxTemp value)
                        GDAL2AsciiGrid( wrpDS, bandNumTempLow/2, airLowGrid );
                    if((bandNumTempHigh-1)%2 != 0)  //if odd number in array (meaning it's a maxTemp value)
                        GDAL2AsciiGrid( wrpDS, bandNumTempHigh/2, airHighGrid );
                }else   //if mins are evens
                {
                    if((bandNumTempLow-1)%2 == 0)  //if even number in array (meaning it's a maxTemp value)
                        GDAL2AsciiGrid( wrpDS, bandNumTempLow/2, airLowGrid );
                    if((bandNumTempHigh-1)%2 == 0)  //if even number in array (meaning it's a maxTemp value)
                        GDAL2AsciiGrid( wrpDS, bandNumTempHigh/2, airHighGrid );
                }
            }else{
                if(!minFirst)   //if maxs are even
                {
                    if((bandNumTempLuck-1)%2 != 0)  //if odd number in array (meaning it's a maxTemp value)
                        GDAL2AsciiGrid( wrpDS, bandNumTempLuck/2, airGrid );

                }else   //if mins are even
                {
                    if((bandNumTempLuck-1)%2 == 0)  //if even number in array (meaning it's a maxTemp value)
                        GDAL2AsciiGrid( wrpDS, bandNumTempLuck/2, airGrid );
                }
            }

        /* fix no data in the air high, low, and regular grid */
        if( CPLIsNan( dfNoData ) ) {
        airHighGrid.set_noDataValue(-9999.0);
        airHighGrid.replaceNan( -9999.0 );
        airLowGrid.set_noDataValue(-9999.0);
        airLowGrid.replaceNan( -9999.0 );
        airGrid.set_noDataValue(-9999.0);
        airGrid.replaceNan( -9999.0 );
        }
        }else if(varList[i] == "Total_cloud_cover_entire_atmosphere_single_layer_layer")
        {
            GDAL2AsciiGrid( wrpDS, bandNum, cloudGrid );
        if( CPLIsNan( dfNoData ) ) {
        cloudGrid.set_noDataValue(-9999.0);
        cloudGrid.replaceNan( -9999.0 );
        }
            cloudGrid /= 100.0; //Since it comes as percent, divide by 100 to make fraction

        }else if(varList[i] == "Wind_direction_from_which_blowing_height_above_ground")
        {
            GDAL2AsciiGrid( wrpDS, bandNum, directionGrid );
        if( CPLIsNan( dfNoData ) ) {
        directionGrid.set_noDataValue(-9999.0);
        directionGrid.replaceNan( -9999.0 );
        }

        }else if(varList[i] == "Wind_speed_height_above_ground")
        {
            GDAL2AsciiGrid( wrpDS, bandNum, speedGrid );
        speedGrid.set_noDataValue(-9999.0);
        speedGrid.replaceNan( -9999.0 );
        }else
            throw std::runtime_error("Problem identifying the variables in the forecast file.");

        GDALDestroyWarpOptions( psWarpOptions );
        GDALClose( (GDALDatasetH)srcDS );
        GDALClose( (GDALDatasetH)wrpDS );
    }
    if(bandNumTempLuck < 0)
    {
        bpt::time_duration a, b;
        airGrid.set_headerData(airLowGrid);

        //interpolate linearly for temperature
        for(int i=0; i<airLowGrid.get_nRows(); i++)
        {
            for(int j=0; j<airLowGrid.get_nCols(); j++)
            {
                a = input.ninjaTime - timeListTemp[bandNumTempLow-1];
                b = timeListTemp[bandNumTempHigh-1] - timeListTemp[bandNumTempLow-1];
        if( airLowGrid(i,j) == airLowGrid.get_NoDataValue() ||
            airHighGrid(i,j) == airHighGrid.get_NoDataValue() ) {
            airGrid(i,j) = airGrid.get_NoDataValue();
        }
        else
            airGrid(i,j) = airLowGrid(i,j) + (a.total_seconds())*((airHighGrid(i,j) - airLowGrid(i,j))/(b.total_seconds()));
            }
        }
    }

    uGrid.set_headerData(speedGrid);
    vGrid.set_headerData(speedGrid);
    wGrid.set_headerData(speedGrid);

    for(int i=0; i<speedGrid.get_nRows(); i++)
    {
        for(int j=0; j<speedGrid.get_nCols(); j++)
        {
        if( speedGrid(i,j) == speedGrid.get_NoDataValue() || directionGrid(i,j) == directionGrid.get_NoDataValue() ) {
        uGrid(i,j) = uGrid.get_NoDataValue();
        vGrid(i,j) = vGrid.get_NoDataValue();
        }
        else
        wind_sd_to_uv(speedGrid(i,j), directionGrid(i,j), &(uGrid)(i,j), &(vGrid)(i,j));
        }
    }

    wGrid = 0.0;
}
