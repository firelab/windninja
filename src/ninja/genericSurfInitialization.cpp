/******************************************************************************
*
* $Id$
*
* Project:  WindNinja
* Purpose:  Generic NAM implementation.  If the file has the variables, it
*           passes identify.
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

#include "genericSurfInitialization.h"

/**
* Constructor for the class initializes the variable names
*
*/
genericSurfInitialization::genericSurfInitialization() : wxModelInitialization()
{

}

/**
* Copy constructor.
* @param A Copied value.
*/
genericSurfInitialization::genericSurfInitialization(genericSurfInitialization const& A ) : wxModelInitialization(A)
{
    wxModelFileName = A.wxModelFileName;
}

/**
* Destructor.
*/
genericSurfInitialization::~genericSurfInitialization()
{
}

/**
* Equals operator.
* @param A Value to set equal to.
* @return a copy of an object
*/
genericSurfInitialization& genericSurfInitialization::operator= (genericSurfInitialization const& A)
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
double genericSurfInitialization::Get_Wind_Height()
{
    return GetWindHeight("height_above_ground1");
}

/**
*@brief Returns horizontal grid resolution of the model
*@return return grid resolution (in km unless < 1, then degrees)
*/
double genericSurfInitialization::getGridResolution()
{
    return -1.0;
}

/**
* Fetch the variable names
*
* @return a vector of variable names
*/
std::vector<std::string> genericSurfInitialization::getVariableList()
{
    std::vector<std::string> varList;
    varList.push_back( "Temperature_height_above_ground" );
    varList.push_back( "V-component_of_wind_height_above_ground" );
    varList.push_back( "U-component_of_wind_height_above_ground" );
    varList.push_back( "Total_cloud_cover" );
    return varList;
}

/**
* Fetch a unique identifier for the forecast.  This string can be used for
* identifying and storing.
*
* @return unique name
*/
std::string genericSurfInitialization::getForecastIdentifier()
{
    return std::string( "GENERIC" );
}

/**
* Fetch the path to the forecast
*
* @return path to the forecast
*/
std::string genericSurfInitialization::getPath()
{
    return std::string( "" );
}

int genericSurfInitialization::getStartHour()
{
    return 0;
}

int genericSurfInitialization::getEndHour()
{
    return 0;
}

/**
* Checks the downloaded data to see if it is all valid.
*/
void genericSurfInitialization::checkForValidData()
{
    //just make up a "dummy" timezone for use here
    boost::local_time::time_zone_ptr zone(new boost::local_time::posix_time_zone("MST-07"));

    //get time list
    std::vector<boost::local_time::local_date_time> timeList( getTimeList(zone) );

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

    // open ds variable by variable
    GDALDataset *srcDS;
    std::string temp;
    std::string srcWkt;
    int nBands = 0;
    bool noDataValueExists;
    bool noDataIsNan;

    std::vector<std::string> varList = getVariableList();

    //Acquire a lock to protect the non-thread safe netCDF library
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

                if( varList[i] == "Temperature_height_above_ground" )   //units are Kelvin
                {
                    if(padfScanline[k] < 180.0 || padfScanline[k] > 340.0)  //these are near the most extreme temperatures ever recored on earth
                        throw badForecastFile("Temperature is out of range in forecast file.");
                }
                else if( varList[i] == "V-component_of_wind_height_above_ground" )  //units are m/s
                {
                    if(std::abs(padfScanline[k]) > 220.0)
                        throw badForecastFile("V-velocity is out of range in forecast file.");
                }
                else if( varList[i] == "U-component_of_wind_height_above_ground" )  //units are m/s
                {
                    if(std::abs(padfScanline[k]) > 220.0)
                        throw badForecastFile("U-velocity is out of range in forecast file.");
                }
                else if( varList[i] == "Total_cloud_cover" )  //units are percent
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
* Static identifier to determine if the netcdf file is a NAM forecast.
* Uses netcdf c api
*
* @param fileName netcdf filename
*
* @return true if the forecast is a NCEP Nam forecast
*/
bool genericSurfInitialization::identify( std::string fileName )
{

    bool identified = true;

    //Acquire a lock to protect the non-thread safe netCDF library
#ifdef _OPENMP
    omp_guard netCDF_guard(netCDF_lock);
#endif

    int status, ncid, ndims, nvars, ngatts, unlimdimid;

    /*
     * Open the dataset
     */
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
     * We do not check any of the global variables here, if the file has the
     * proper variables, time, and projection info, it is good enough.  This
     * is for 3rd party weather files, not files downloaded by windninja
     */

    status = nc_close( ncid );

    return identified;
}

/**
* Sets the surface grids based on a ncepNam (surface only!) forecast.
* @param input The WindNinjaInputs for misc. info.
* @param airGrid The air temperature grid to be filled.
* @param cloudGrid The cloud cover grid to be filled.
* @param uGrid The u velocity grid to be filled.
* @param vGrid The v velocity grid to be filled.
* @param wGrid The w velocity grid to be filled (filled with zeros here?).
*/
void genericSurfInitialization::setSurfaceGrids( WindNinjaInputs &input,
        AsciiGrid<double> &airGrid,
        AsciiGrid<double> &cloudGrid,
        AsciiGrid<double> &uGrid,
        AsciiGrid<double> &vGrid,
        AsciiGrid<double> &wGrid )
{
    int bandNum = -1;

    //get time list
    std::vector<boost::local_time::local_date_time> timeList( getTimeList(input.ninjaTimeZone) );
    //Search time list for our time to identify our band number for cloud/speed/dir
    for(unsigned int i = 0; i < timeList.size(); i++)
    {
        if(input.ninjaTime == timeList[i])
        {
            bandNum = i + 1;
            break;
        }
    }
    if(bandNum < 0)
        throw std::runtime_error("Could not match ninjaTime with a band number in the forecast file.");

    //get some info from the nam file in input

    //Acquire a lock to protect the non-thread safe netCDF library
#ifdef _OPENMP
    omp_guard netCDF_guard(netCDF_lock);
#endif

    GDALDataset* poDS;

    //attempt to grab the projection from the dem?
    //check for member prjString first
    std::string dstWkt;
    dstWkt = input.dem.prjString;
    if ( dstWkt.empty() ) {
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
        GDALClose((GDALDatasetH) poDS );
    }

    poDS = (GDALDataset*)GDALOpen( input.forecastFilename.c_str(), GA_ReadOnly );

    if( poDS == NULL ) {
        CPLDebug( "ncepNdfdInitialization::setSurfaceGrids()",
                "Bad forecast file" );
    }
    else
        GDALClose((GDALDatasetH) poDS );

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

        /*
         * Grab the first band to get the nodata value for the variable,
         * assume all bands have the same ndv
         */
        GDALRasterBand *poBand = srcDS->GetRasterBand( 1 );
        int pbSuccess;
        double dfNoData = poBand->GetNoDataValue( &pbSuccess );

        psWarpOptions = GDALCreateWarpOptions();

        int nBandCount = srcDS->GetRasterCount();

        psWarpOptions->nBandCount = nBandCount;

        psWarpOptions->padfDstNoDataReal =
            (double*) CPLMalloc( sizeof( double ) * nBandCount );
        psWarpOptions->padfDstNoDataImag =
            (double*) CPLMalloc( sizeof( double ) * nBandCount );

        for( int b = 0;b < srcDS->GetRasterCount();b++ ) {
            psWarpOptions->padfDstNoDataReal[b] = dfNoData;
            psWarpOptions->padfDstNoDataImag[b] = dfNoData;
        }

        if( pbSuccess == false )
            dfNoData = -9999.0;

        psWarpOptions->papszWarpOptions =
            CSLSetNameValue( psWarpOptions->papszWarpOptions,
                            "INIT_DEST", "NO_DATA" );

        wrpDS = (GDALDataset*) GDALAutoCreateWarpedVRT( srcDS, srcWkt.c_str(),
                                                        dstWkt.c_str(),
                                                        GRA_NearestNeighbour,
                                                        1.0, psWarpOptions );

        if( varList[i] == "Temperature_height_above_ground" ) {
            GDAL2AsciiGrid( wrpDS, bandNum, airGrid );
        if( CPLIsNan( dfNoData ) ) {
        airGrid.set_noDataValue(-9999.0);
        airGrid.replaceNan( -9999.0 );
        }
    }
        else if( varList[i] == "V-component_of_wind_height_above_ground" ) {
            GDAL2AsciiGrid( wrpDS, bandNum, vGrid );
        if( CPLIsNan( dfNoData ) ) {
        vGrid.set_noDataValue(-9999.0);
        vGrid.replaceNan( -9999.0 );
        }
    }
        else if( varList[i] == "U-component_of_wind_height_above_ground" ) {
            GDAL2AsciiGrid( wrpDS, bandNum, uGrid );
        if( CPLIsNan( dfNoData ) ) {
        uGrid.set_noDataValue(-9999.0);
        uGrid.replaceNan( -9999.0 );
        }
    }
        else if( varList[i] == "Total_cloud_cover" ) {
            GDAL2AsciiGrid( wrpDS, bandNum, cloudGrid );
        if( CPLIsNan( dfNoData ) ) {
        cloudGrid.set_noDataValue(-9999.0);
        cloudGrid.replaceNan( -9999.0 );
        }
    }

        GDALDestroyWarpOptions( psWarpOptions );
        GDALClose((GDALDatasetH) srcDS );
        GDALClose((GDALDatasetH) wrpDS );
    }
    cloudGrid /= 100.0;

    wGrid.set_headerData( uGrid );
    wGrid = 0.0;
}
