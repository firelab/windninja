/******************************************************************************
*
* $Id:$
*
* Project:  WindNinja
* Purpose:  Initialing with RegCM forecasts
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

#include "regcmSurfInitialization.h"

/**
* Constructor for the class initializes the variable names
*
*/
regcmSurfInitialization::regcmSurfInitialization() : wxModelInitialization()
{

}

/**
* Copy constructor.
* @param A Copied value.
*/
regcmSurfInitialization::regcmSurfInitialization(regcmSurfInitialization const& A ) : wxModelInitialization(A)
{
    wxModelFileName = A.wxModelFileName;
}

/**
* Destructor.
*/
regcmSurfInitialization::~regcmSurfInitialization()
{
}

/**
* Equals operator.
* @param A Value to set equal to.
* @return a copy of an object
*/
regcmSurfInitialization& regcmSurfInitialization::operator= (regcmSurfInitialization const& A)
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
double regcmSurfInitialization::getGridResolution()
{
    return -1.0;
}

/**
* Fetch the variable names
*
* @return a vector of variable names
*/
std::vector<std::string> regcmSurfInitialization::getVariableList()
{
    std::vector<std::string> varList;
    varList.push_back( "tas" );   // T at 2 m
    varList.push_back( "vas" );  // V at 10 m
    varList.push_back( "uas" );  // U at 10 m
    //varList.push_back( "QCLOUD" );  // cloud water mixing ratio
    return varList;
}

/**
* Fetch a unique identifier for the forecast.  This string can be used for
* identifying and storing.
*
* @return unique name
*/
std::string regcmSurfInitialization::getForecastIdentifier()
{
    return std::string( "REGCM-SURFACE" );
}

/**
* Fetch the path to the forecast
*
* @return path to the forecast
*/
std::string regcmSurfInitialization::getPath()
{
    return std::string( "");
}

int regcmSurfInitialization::getStartHour()
{
    return 0;
}

int regcmSurfInitialization::getEndHour()
{
    return 3;
}

/**
* Checks the downloaded data to see if it is all valid.
* May not be functional yet for this class...
*/
void regcmSurfInitialization::checkForValidData()
{
    //no check for valid data yet
}

/**
* Static identifier to determine if the netcdf file is a RegCM forecast.
* Uses netcdf c api
*
* @param fileName netcdf filename
*
* @return true if the forecast is a RegCM forecast
*/
bool regcmSurfInitialization::identify( std::string fileName )
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
     * :title = "ICTP Regional Climatic model V4""
     */
    size_t len;
    nc_type type;
    char* model;
    status = nc_inq_att( ncid, NC_GLOBAL, "title", &type, &len );
    if( status != NC_NOERR )
        identified = false;
    else {
        model = new char[len + 1];
        status = nc_get_att_text( ncid, NC_GLOBAL, "title", model );
        model[len] = '\0';
        std::string s( model );
        if( s.find("ICTP Regional Climatic model V4") == s.npos ) {
            identified = false;
        }

        delete[] model;
    }

    status = nc_close( ncid );

    return identified;
}


/**
* Sets the surface grids based on a RegCM (surface only!) forecast.
* @param input The WindNinjaInputs for misc. info.
* @param airGrid The air temperature grid to be filled.
* @param cloudGrid The cloud cover grid to be filled.
* @param uGrid The u velocity grid to be filled.
* @param vGrid The v velocity grid to be filled.
* @param wGrid The w velocity grid to be filled (filled with zeros here?).
*/
void regcmSurfInitialization::setSurfaceGrids( WindNinjaInputs &input,
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

    GDALDataset* poDS;
    //attempt to grab the projection from the dem?
    //check for member prjString first
    std::string dstWkt;
    dstWkt = input.dem.prjString;
    if ( dstWkt.empty() ) {
        //try to open original
        poDS = (GDALDataset*)GDALOpen( input.dem.fileName.c_str(), GA_ReadOnly );
        if( poDS == NULL ) {
            CPLDebug( "wrfInitialization::setSurfaceGrids()",
                    "Bad projection reference" );
            throw("Cannot open dem file in wrfInitialization::setSurfaceGrids()");
        }
        dstWkt = poDS->GetProjectionRef();
        if( dstWkt.empty() ) {
            CPLDebug( "wrfInitialization::setSurfaceGrids()",
                    "Bad projection reference" );
            throw("Cannot open dem file in wrfInitialization::setSurfaceGrids()");
        }
        GDALClose((GDALDatasetH) poDS );
    }

    // open ds one by one, set projection, warp, then write to grid
    GDALDataset *srcDS, *wrpDS;
    std::string temp;
    std::string srcWkt;
    std::vector<std::string> varList = getVariableList();

    /*
     * Set the initial values in the warped dataset to no data
     */
    GDALWarpOptions* psWarpOptions;
    double adfGeoTransform[6];

    for( unsigned int i = 0;i < varList.size();i++ ) {

        temp = "NETCDF:" + input.forecastFilename + ":" + varList[i];
        
        srcDS = (GDALDataset*)GDALOpenShared( temp.c_str(), GA_ReadOnly );
        if( srcDS == NULL ) {
            cout<<"Bad forecast file in regcmSurfaceIinitialization::setSurfaceGrids."<<endl;
        }

        srcWkt = srcDS->GetProjectionRef();

        if( srcWkt.empty() ) {
            CPLDebug( "regcmSurfInitialization::setSurfaceGrids()",
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

        if( pbSuccess == FALSE )
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

        if( varList[i] == "tas" ) {
            GDAL2AsciiGrid( wrpDS, bandNum, airGrid );
            if( CPLIsNan( dfNoData ) ) {
                airGrid.set_noDataValue(-9999.0);
                airGrid.replaceNan( -9999.0 );
            }
        }
        else if( varList[i] == "vas" ) {
            GDAL2AsciiGrid( wrpDS, bandNum, vGrid );
            if( CPLIsNan( dfNoData ) ) {
                vGrid.set_noDataValue(-9999.0);
                vGrid.replaceNan( -9999.0 );
            }
        }
        else if( varList[i] == "uas" ) {
            GDAL2AsciiGrid( wrpDS, bandNum, uGrid );
            if( CPLIsNan( dfNoData ) ) {
                uGrid.set_noDataValue(-9999.0);
                uGrid.replaceNan( -9999.0 );
            }
        }
//        else if( varList[i] == "QCLOUD" ) {
//            //GDAL2AsciiGrid( wrpDS, bandNum, cloudGrid );
//            cloudGrid = 0.0;
//            if( CPLIsNan( dfNoData ) ) {
//                cloudGrid.set_noDataValue(-9999.0);
//                cloudGrid.replaceNan( -9999.0 );
//            }
//        }
        GDALDestroyWarpOptions( psWarpOptions );
        GDALClose((GDALDatasetH) srcDS );
        GDALClose((GDALDatasetH) wrpDS );
    }
    cloudGrid /= 100.0;
    wGrid.set_headerData( uGrid );
    wGrid = 0.0;
}

double regcmSurfInitialization::Get_Wind_Height()
{
    return 10.0;
}


