/******************************************************************************
*
* $Id: ncepNamSurfInitialization.cpp 1757 2012-08-07 18:40:40Z kyle.shannon $
*
* Project:  WindNinja
* Purpose:  Initializing with NCEP NAM forecsasts in GRIB2 format
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

#include "ncepNamGrib2SurfInitialization.h"

/**
* Constructor for the class initializes the variable names
*
*/
ncepNamGrib2SurfInitialization::ncepNamGrib2SurfInitialization() : wxModelInitialization()
{

}

/**
* Copy constructor.
* @param A Copied value.
*/
ncepNamGrib2SurfInitialization::ncepNamGrib2SurfInitialization(ncepNamGrib2SurfInitialization const& A ) : wxModelInitialization(A)
{
    wxModelFileName = A.wxModelFileName;
}

/**
* Destructor.
*/
ncepNamGrib2SurfInitialization::~ncepNamGrib2SurfInitialization()
{
}

/**
* Equals operator.
* @param A Value to set equal to.
* @return a copy of an object
*/
ncepNamGrib2SurfInitialization& ncepNamGrib2SurfInitialization::operator= (ncepNamGrib2SurfInitialization const& A)
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
double ncepNamGrib2SurfInitialization::Get_Wind_Height()
{
    //can grab this directly from grib file later
    return 10;
}

/**
*@brief Returns horizontal grid resolution of the model
*@return return grid resolution (in km unless < 1, then degrees)
*/
double ncepNamGrib2SurfInitialization::getGridResolution()
{
    return 12.0;
}


/**
* Fetch the variable names
*
* @return a vector of variable names
*/
std::vector<std::string> ncepNamGrib2SurfInitialization::getVariableList()
{
    std::vector<std::string> varList;
    varList.push_back( "2t" );
    varList.push_back( "10v" );
    varList.push_back( "10u" );
    varList.push_back( "tcc" ); // total cloud cover
    return varList;
}

/**
* Fetch a unique identifier for the forecast.  This string can be used for
* identifying and storing.
*
* @return unique name
*/
std::string ncepNamGrib2SurfInitialization::getForecastIdentifier()
{
    return std::string( "NCEP-NAM-12km-SURFACE-GRIB2" );
}

/**
* Fetch the path to the forecast
*
* @return path to the forecast
*/
std::string ncepNamGrib2SurfInitialization::getPath()
{
    return std::string( "" );
}

int ncepNamGrib2SurfInitialization::getStartHour()
{
    return 0;
}

int ncepNamGrib2SurfInitialization::getEndHour()
{
    return 84;
}

/**
* Checks the downloaded data to see if it is all valid.
*/
void ncepNamGrib2SurfInitialization::checkForValidData()
{

}

/**
* Static identifier to determine if the file is a NAM forecast in GRIB2 format.
* If don't have access to grib api, identificaion is done based
* on filename.
* @param fileName grib2 filename
*
* @return true if the forecast is a NCEP Nam forecast in GRIB2 format
*/

bool ncepNamGrib2SurfInitialization::identify( std::string fileName )
{
    bool identified = true;

    if( fileName.find("nam") == fileName.npos ) {
        identified = false;
    }
    GDALDatasetH hDS;
    if( strstr( fileName.c_str(), ".tar" ) ){
        hDS = GDALOpenShared( CPLSPrintf( "/vsitar/%s", fileName.c_str() ) , GA_ReadOnly );
    }
    else{
        hDS = GDALOpenShared( fileName.c_str(), GA_ReadOnly );
    }
    if( !hDS || GDALGetRasterCount( hDS ) < 8 )
        identified = false;
    GDALClose( hDS );
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
void ncepNamGrib2SurfInitialization::setSurfaceGrids( WindNinjaInputs &input,
        AsciiGrid<double> &airGrid,
        AsciiGrid<double> &cloudGrid,
        AsciiGrid<double> &uGrid,
        AsciiGrid<double> &vGrid,
        AsciiGrid<double> &wGrid )
{

    int bandNum = -1;

    //get time list
    std::vector<boost::local_time::local_date_time> timeList( getTimeList( input.ninjaTimeZone ) );

    //Search time list for our time to identify our band number for cloud/speed/dir
    //Right now, just one time step per file
    std::vector<int> bandList;
    for(unsigned int i = 0; i < timeList.size(); i++)
    {
        if(input.ninjaTime == timeList[i])
        {
            bandList.push_back( 6 ); // 2t
            bandList.push_back( 10 ); // 10v
            bandList.push_back( 9 );  // 10u
            bandList.push_back( 358 ); // total cloud cover
            break;
        }
    }

    if(bandList.size() < 4)
        throw std::runtime_error("Could not match ninjaTime with a band number in the forecast file.");

    GDALDataset* poDS;

    std::string dstWkt;
    dstWkt = input.dem.prjString;

    GDALDataset *srcDS, *wrpDS;
    std::string temp;
    std::string srcWkt;

    GDALWarpOptions* psWarpOptions;

    if( strstr( input.forecastFilename.c_str(), ".tar" ) ){
        srcDS = (GDALDataset*)GDALOpenShared(
        CPLSPrintf( "/vsitar/%s", input.forecastFilename.c_str() ), GA_ReadOnly );
    }
    else{
        srcDS = (GDALDataset*)GDALOpenShared(
        input.forecastFilename.c_str(), GA_ReadOnly );
    }

    srcWkt = srcDS->GetProjectionRef();

    GDALRasterBand *poBand = srcDS->GetRasterBand( 9 );
    int pbSuccess;
    double dfNoData = poBand->GetNoDataValue( &pbSuccess );

    psWarpOptions = GDALCreateWarpOptions();

    int nBandCount = bandList.size();

    psWarpOptions->nBandCount = nBandCount;
    psWarpOptions->panSrcBands =
        (int*) CPLMalloc( sizeof( int ) * nBandCount );
    psWarpOptions->panDstBands =
        (int*) CPLMalloc( sizeof( int ) * nBandCount );
    psWarpOptions->padfDstNoDataReal =
        (double*) CPLMalloc( sizeof( double ) * nBandCount );
    psWarpOptions->padfDstNoDataImag =
        (double*) CPLMalloc( sizeof( double ) * nBandCount );

    if( pbSuccess == false )
        dfNoData = -9999.0;

    psWarpOptions->panSrcBands =
        (int *) CPLMalloc(sizeof(int) * psWarpOptions->nBandCount );
    psWarpOptions->panSrcBands[0] = bandList[0];
    psWarpOptions->panSrcBands[1] = bandList[1];
    psWarpOptions->panSrcBands[2] = bandList[2];
    psWarpOptions->panSrcBands[3] = bandList[3];

    psWarpOptions->panDstBands =
        (int *) CPLMalloc(sizeof(int) * psWarpOptions->nBandCount );
    psWarpOptions->panDstBands[0] = 1;
    psWarpOptions->panDstBands[1] = 2;
    psWarpOptions->panDstBands[2] = 3;
    psWarpOptions->panDstBands[3] = 4;


    wrpDS = (GDALDataset*) GDALAutoCreateWarpedVRT( srcDS, srcWkt.c_str(),
                                                    dstWkt.c_str(),
                                                    GRA_NearestNeighbour,
                                                    1.0, psWarpOptions );

    std::vector<std::string> varList = getVariableList();

    for( unsigned int i = 0; i < varList.size(); i++ ) {
        if( varList[i] == "2t" ) {
            GDAL2AsciiGrid( wrpDS, i+1, airGrid );
            if( CPLIsNan( dfNoData ) ) {
                airGrid.set_noDataValue( -9999.0 );
                airGrid.replaceNan( -9999.0 );
            }
        }
        else if( varList[i] == "10v" ) {
            GDAL2AsciiGrid( wrpDS, i+1, vGrid );
            if( CPLIsNan( dfNoData ) ) {
                vGrid.set_noDataValue( -9999.0 );
                vGrid.replaceNan( -9999.0 );
            }
        }
        else if( varList[i] == "10u" ) {
            GDAL2AsciiGrid( wrpDS, i+1, uGrid );
            if( CPLIsNan( dfNoData ) ) {
                uGrid.set_noDataValue( -9999.0 );
                uGrid.replaceNan( -9999.0 );
            }
        }
        else if( varList[i] == "tcc" ) {
            GDAL2AsciiGrid( wrpDS, i+1, cloudGrid );
            if( CPLIsNan( dfNoData ) ) {
                cloudGrid.set_noDataValue( -9999.0 );
                cloudGrid.replaceNan( -9999.0 );
            }
        }
    }
    cloudGrid /= 100.0;
    airGrid += 273.15;

    wGrid.set_headerData( uGrid );
    wGrid = 0.0;

    GDALDestroyWarpOptions( psWarpOptions );

    GDALClose((GDALDatasetH) srcDS );
    GDALClose((GDALDatasetH) wrpDS );

}
