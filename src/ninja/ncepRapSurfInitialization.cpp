/******************************************************************************
*
* $Id$
*
* Project:  WindNinja
* Purpose:  general disclaimer for windninja
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

#include "ncepRapSurfInitialization.h"

/**
* Constructor for the class initializes the variable names
*
*/
ncepRapSurfInitialization::ncepRapSurfInitialization() : wxModelInitialization()
{
    heightVarName = "height_above_ground1";
    path = "/thredds/ncss/grib/NCEP/RAP/CONUS_13km/best?north=USER_NORTH&west=USER_WEST&east=USER_EAST&south=USER_SOUTH&time_start=present&time_duration=PTUSER_TIMEH&accept=netcdf";
    LoadFromCsv();
}

/**
* Copy constructor.
* @param A Copied value.
*/
ncepRapSurfInitialization::ncepRapSurfInitialization(ncepRapSurfInitialization const& A ) : wxModelInitialization(A)
{
    wxModelFileName = A.wxModelFileName;
}

/**
* Destructor.
*/
ncepRapSurfInitialization::~ncepRapSurfInitialization()
{
}

/**
* Equals operator.
* @param A Value to set equal to.
* @return a copy of an object
*/
ncepRapSurfInitialization& ncepRapSurfInitialization::operator= (ncepRapSurfInitialization const& A)
{
    if(&A != this)
    {
        wxModelInitialization::operator=(A);
    }
    return *this;
}

/**
*@brief wrapper for GetWindHeight
*@return return wind height
*/
double ncepRapSurfInitialization::Get_Wind_Height()
{
    return GetWindHeight("height_above_ground1");
}

/**
*@brief Returns horizontal grid resolution of the model
*@return return grid resolution (in km unless < 1, then degrees)
*/
double ncepRapSurfInitialization::getGridResolution()
{
    return 13.0;
}

/**
* Fetch the variable names
*
* @return a vector of variable names
*/
std::vector<std::string> ncepRapSurfInitialization::getVariableList()
{
    std::vector<std::string> varList;
    varList.push_back( "Temperature_height_above_ground" );
    varList.push_back( "v-component_of_wind_height_above_ground" );
    varList.push_back( "u-component_of_wind_height_above_ground" );
    varList.push_back( "Geopotential_height_cloud_tops" );
    return varList;
}

/**
* Fetch a unique identifier for the forecast.  This string can be used for
* identifying and storing.
*
* @return unique name
*/
std::string ncepRapSurfInitialization::getForecastIdentifier()
{
    return std::string( "UCAR-RAP-CONUS-13-KM" );
}

int ncepRapSurfInitialization::getStartHour()
{
    return 1;
}

int ncepRapSurfInitialization::getEndHour()
{
    return 18;
}

/**
* Checks the downloaded data to see if it is all valid.
*/
void ncepRapSurfInitialization::checkForValidData()
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


    {//Acquire a lock to protect the non-thread safe netCDF library
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
                    /* Since geopotential cloud tops is treated a little
                     * different, don't check for no data.  No data  just
                     * means no clouds.  The stats for the US report no value
                     * under 0, so we are assuming nan=no cloud now.
                     */
                    if(noDataValueExists && varList[i] != "Geopotential_height_cloud_tops")
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
                    else if( varList[i] == "v-component_of_wind_height_above_ground" )  //units are m/s
                    {
                        if(std::abs(padfScanline[k]) > 220.0)
                            throw badForecastFile("V-velocity is out of range in forecast file.");
                    }
                    else if( varList[i] == "u-component_of_wind_height_above_ground" )  //units are m/s
                    {
                        if(std::abs(padfScanline[k]) > 220.0)
                            throw badForecastFile("U-velocity is out of range in forecast file.");
                    }
                    else if( varList[i] == "Geopotential_height_cloud_tops" )  //units are meters
                    {
                        if(padfScanline[k] != -99999.0) //means there are clouds
                        {
                            if(padfScanline[k] < -500.0 || padfScanline[k] > 30000.0)
                                throw badForecastFile("Geopotential_height_cloud_tops is out of range in forecast file.");
                        }
                    }
                }

                delete [] padfScanline;
            }

            GDALClose((GDALDatasetH) srcDS );
        }
    }   //netCDF_guard
}

/**
* Static identifier to determine if the netcdf file is a RUC forecast.
* Uses netcdf c api
*
* @param fileName netcdf filename
*
* @return true if the forecast is a NCEP Ruc forecast
*/
bool ncepRapSurfInitialization::identify( std::string fileName )
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
     * Check the global attributes for the following tag:
     * :Generating_Model = "RUC Model from FSL (isentropic; scale: 20km at 40N)"
     */
    size_t len;
    nc_type type;
    char* model;
    status = nc_inq_att( ncid, NC_GLOBAL, "History", &type, &len );
    if( status != NC_NOERR )
        identified = false;
    else {
        model = new char[len + 1];
        status = nc_get_att_text( ncid, NC_GLOBAL, "History", model );
        model[len] = '\0';
        std::string s( model );
        if( s.find( "RAP-CONUS" ) == s.npos ) {
            identified = false;
        }
        delete[] model;
    }

    status = nc_close( ncid );


    return identified;
}

/**
* Sets the surface grids based on a ncepRap (surface only!) forecast.
* @param input The WindNinjaInputs for misc. info.
* @param airGrid The air temperature grid to be filled.
* @param cloudGrid The cloud cover grid to be filled.
* @param uGrid The u velocity grid to be filled.
* @param vGrid The v velocity grid to be filled.
* @param wGrid The w velocity grid to be filled (filled with zeros here?).
*/
void ncepRapSurfInitialization::setSurfaceGrids(  WindNinjaInputs &input,
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

    {//Acquire a lock to protect the non-thread safe netCDF library
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
                CPLDebug( "ncepRAPSurfInitialization::setSurfaceGrids()",
                        "Bad projection reference" );
                //throw();
            }
            dstWkt = poDS->GetProjectionRef();
            if( dstWkt.empty() ) {
                CPLDebug( "ncepRAPSurfInitialization::setSurfaceGrids()",
                        "Bad projection reference" );
                //throw()
            }
            GDALClose((GDALDatasetH) poDS );
        }

        poDS = (GDALDataset*)GDALOpenShared( wxModelFileName.c_str(), GA_ReadOnly );

        if( poDS == NULL ) {
            CPLDebug( "ncepRAPSurfInitialization::setSurfaceGrids()",
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

            temp = "NETCDF:" + wxModelFileName + ":" + varList[i];

            srcDS = (GDALDataset*)GDALOpenShared( temp.c_str(), GA_ReadOnly );
            if( srcDS == NULL ) {
                CPLDebug( "ncepRAPSurfInitialization::setSurfaceGrids()",
                        "Bad forecast file" );
            }

            srcWkt = srcDS->GetProjectionRef();

            if( srcWkt.empty() ) {
                CPLDebug( "ncepRAPSurfInitialization::setSurfaceGrids()",
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
        if(wrpDS == NULL)
        {
            throw badForecastFile("Could not warp the forecast file, "
                                  "possibly non-uniform grid.");
        }

        if( varList[i] == "Temperature_height_above_ground" ) {
        GDAL2AsciiGrid( wrpDS, bandNum, airGrid );
        if( CPLIsNan( dfNoData ) ) {
            airGrid.set_noDataValue(-9999.0);
            airGrid.replaceNan( -9999.0 );
        }
        }
            else if( varList[i] == "v-component_of_wind_height_above_ground" ) {
        GDAL2AsciiGrid( wrpDS, bandNum, vGrid );
        if( CPLIsNan( dfNoData ) ) {
            vGrid.set_noDataValue(-9999.0);
            vGrid.replaceNan( -9999.0 );
        }
        }
            else if( varList[i] == "u-component_of_wind_height_above_ground" ) {
                GDAL2AsciiGrid( wrpDS, bandNum, uGrid );
        if( CPLIsNan( dfNoData ) ) {
            uGrid.set_noDataValue(-9999.0);
            uGrid.replaceNan( -9999.0 );
        }
        }
            else if( varList[i] == "Geopotential_height_cloud_tops" ) {
                GDAL2AsciiGrid( wrpDS, bandNum, cloudGrid );
        if( CPLIsNan( dfNoData ) ) {
            cloudGrid.set_noDataValue(-9999.0);
            cloudGrid.replaceNan( -99999.0 );
        }
        }

        GDALDestroyWarpOptions( psWarpOptions );
        GDALClose((GDALDatasetH) srcDS );
        GDALClose((GDALDatasetH) wrpDS );
        }
    }   //netCDF_guard

    //Set cloud fraction, either 0 or 1 nothing in between for RUC...
    for(int i=0; i<cloudGrid.get_nRows(); i++)
    {
        for(int j=0; j<cloudGrid.get_nCols(); j++)
        {
            if(cloudGrid(i, j) == -99999.0)    //Check for no clouds
                cloudGrid(i, j) = 0.0;
            else if(cloudGrid(i, j) == cloudGrid.get_noDataValue())
                continue;
            else
                cloudGrid(i, j) = 1.0;
        }
    }

    wGrid.set_headerData( uGrid );
    wGrid = 0.0;
}
