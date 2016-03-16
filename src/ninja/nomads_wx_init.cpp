/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja
 * Purpose:  Nomads weather model initialization
 * Author:   Kyle Shannon <kyle at pobox dot com>
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

#include "nomads_wx_init.h"

int NomadsWxModel::CheckFileName( const char *pszFile, const char *pszFormat )
{
    const char *a, *b;
    a = pszFile;
    b = pszFormat;
    while( *a++ != '\0' && *b++ != '\0' )
    {
        if( *b == '%' )
        {
            while( isdigit( *a++ ) ); a--;
            while( *b++ != 'd' );
        }
        if( *a != *b )
            return FALSE;
    }
    return TRUE;
}

NomadsWxModel::NomadsWxModel()
{
    pszKey = NULL;
    ppszModelData = NULL;
    pfnProgress = NULL;
    NomadsUtcCreate( &u );
}

NomadsWxModel::NomadsWxModel( std::string filename )
{
    wxModelFileName = filename;
    pfnProgress = NULL;
    ppszModelData = FindModelKey( filename.c_str() );
    if( ppszModelData )
        pszKey = CPLStrdup( ppszModelData[NOMADS_NAME] );
    else
        pszKey = NULL;
    NomadsUtcCreate( &u );
}

NomadsWxModel::NomadsWxModel( const char *pszModelKey )
{
    ppszModelData = NULL;
    pfnProgress = NULL;
    NomadsUtcCreate( &u );
    pszKey = NULL;
    ppszModelData = NomadsFindModel( pszModelKey );
    if( ppszModelData == NULL )
    {
        CPLError( CE_Failure, CPLE_AppDefined,
                  "Could not find model key in nomads data" );
        pszKey = NULL;
    }
    else
    {
        pszKey = CPLStrdup( pszModelKey );
    }
}

NomadsWxModel::~NomadsWxModel()
{
    NomadsUtcFree( u );
    CPLFree( (void*)pszKey );
}

const char ** NomadsWxModel::FindModelKey( const char *pszFilename )
{
    const char *pszVsiDir;
    VSIStatBufL sStat;
    VSIStatL( pszFilename, &sStat );
    if( VSI_ISDIR( sStat.st_mode ) )
        pszVsiDir = CPLStrdup( pszFilename );
    else if( strstr( pszFilename, ".zip" ) )
        pszVsiDir = CPLStrdup( CPLSPrintf( "/vsizip/%s", pszFilename ) );
    else
        pszVsiDir = CPLStrdup( CPLGetPath( pszFilename ) );
    char **papszFileList = NULL;
    int nCount;
    papszFileList = VSIReadDir( pszVsiDir );
    CPLFree( (void*)pszVsiDir );
    if( !papszFileList )
    {
        return FALSE;
    }
    nCount = CSLCount( papszFileList );
    /* Must match one file name format */
    int i = 0;
    int j = 0;
    const char **ppszKey = NULL;
    while( apszNomadsKeys[i][0] != NULL )
    {
        for( j = 0; j < nCount; j++ )
        {
            SKIP_DOT_AND_DOTDOT( papszFileList[j] );
            if( CheckFileName( papszFileList[j],
                               apszNomadsKeys[i][NOMADS_FILE_NAME_FRMT] ) )
            {
                ppszKey = apszNomadsKeys[i];
                goto found;
            }
        }
        i++;
    }
found:
    CSLDestroy( papszFileList );
    return ppszKey;
}

bool NomadsWxModel::identify( std::string fileName )
{
    return NomadsWxModel::FindModelKey( fileName.c_str() ) ? TRUE : FALSE;
}

int NomadsWxModel::getEndHour()
{
    if( !ppszModelData )
    {
        return 0;
    }
    char **papszTokens = NULL;
    int nHour = 0;
    int nCount = 0;
    papszTokens = CSLTokenizeString2( ppszModelData[NOMADS_FCST_RUN_HOURS],
                                      ":,", 0 );
    nCount = CSLCount( papszTokens );
    nHour = atoi( papszTokens[nCount - 2] );
    CSLDestroy( papszTokens );
    return nHour;
}

int NomadsWxModel::getStartHour()
{
    if( !ppszModelData )
    {
        return 0;
    }
    char **papszTokens = NULL;
    int nHour = 0;
    int nCount = 0;
    papszTokens = CSLTokenizeString2( ppszModelData[NOMADS_FCST_RUN_HOURS],
                                      ":,", 0 );
    nCount = CSLCount( papszTokens );
    if( nCount == 0 )
        return 0;
    nHour = atoi( papszTokens[0] );
    CSLDestroy( papszTokens );
    return nHour;
}

/**
*@brief Returns horizontal grid resolution of the model
*@return return grid resolution (in km unless < 1, then degrees)
*/
double NomadsWxModel::getGridResolution()
{
    double resolution = -1.0;
    
    if(getForecastReadable('-').find("0.25-DEG") != getForecastReadable('-').npos)
        resolution = 0.25;
    else if(getForecastReadable('-').find("2.5-KM") != getForecastReadable('-').npos)
        resolution = 2.5;
    else if(getForecastReadable('-').find("3-KM") != getForecastReadable('-').npos)
        resolution = 3.0;
    else if(getForecastReadable('-').find("5-KM") != getForecastReadable('-').npos)
        resolution = 5.0;
    else if(getForecastReadable('-').find("11.25-KM") != getForecastReadable('-').npos)
        resolution = 11.25;
    else if(getForecastReadable('-').find("12-KM") != getForecastReadable('-').npos)
        resolution = 12.0;
    else if(getForecastReadable('-').find("13-KM") != getForecastReadable('-').npos)
        resolution = 13.0;
    else if(getForecastReadable('-').find("32-KM") != getForecastReadable('-').npos)
        resolution = 32.0;
    
    return resolution;
}

std::string NomadsWxModel::fetchForecast( std::string demFile, int nHours )
{
    if( !ppszModelData )
    {
        throw badForecastFile( "Model not found" );
    }
    GDALDatasetH hDS = GDALOpen( demFile.c_str(), GA_ReadOnly );
    
    double adfNESW[4], adfWENS[4];
    ComputeWxModelBuffer( (GDALDataset*)hDS, adfNESW );
    /* Different order for nomads */
    adfWENS[0] = adfNESW[3];
    adfWENS[1] = adfNESW[1];
    adfWENS[2] = adfNESW[0];
    adfWENS[3] = adfNESW[2];

    GDALClose( hDS );

    std::string path( CPLGetDirname( demFile.c_str() ) );
    std::string fileName( CPLGetFilename( demFile.c_str() ) );
    std::string newPath( path + "/" + getForecastReadable('-')
             + "-" + fileName + "/" );
    int rc;
    VSIStatBufL sStat;
    memset( &sStat, 0, sizeof( VSIStatBufL ) );
    VSIStatL( newPath.c_str(), &sStat );
    if( !VSI_ISDIR( sStat.st_mode ) )
        rc = VSIMkdir( newPath.c_str(), 0777 );
    const char *pszTmpFile = CPLGenerateTempFilename( "NINJA_FCST" );
    pszTmpFile = CPLSPrintf( "%s", CPLFormFilename( NULL, pszTmpFile, ".zip" ) );
    /* Copy the temp file, many CPL calls in NomadsFetch */
    pszTmpFile = CPLStrdup( pszTmpFile );

    rc = NomadsFetch( pszKey, NULL, nHours, 1, adfWENS, pszTmpFile, NULL,
                      pfnProgress );
    if( rc )
    {
        CPLFree( (void*)pszTmpFile );
        throw badForecastFile( "Could not download from nomads server!" );
    }
    wxModelFileName = pszTmpFile;
    std::vector<blt::local_date_time> oTimes;
    oTimes = wxModelInitialization::getTimeList();
    if( oTimes.size() < 1 )
    {
        throw badForecastFile( "Could not open forecast from nomads server" );
    }
    std::string filename;
    filename = bpt::to_iso_string(oTimes[0].utc_time());
    //remove trailing seconds
    filename = filename.erase( filename.size() - 2 );
    newPath += filename;
    VSIMkdir( newPath.c_str(), 0777 );
    newPath += "/" + filename + ".zip";
    CPLMoveFile( newPath.c_str(), pszTmpFile );
    wxModelFileName = newPath;
    CPLFree( (void*)pszTmpFile );
    return newPath;
}

std::vector<std::string> NomadsWxModel::getVariableList()
{
    if( !ppszModelData )
    {
        throw badForecastFile( "Invalid model" );
    }
    char **papszTokens = NULL;
    papszTokens = CSLTokenizeString2( ppszModelData[NOMADS_VARIABLES], ",", 0 );
    int nCount = CSLCount( papszTokens );
    std::vector<std::string>v;
    int i;
    for( i = 0; i < nCount; i++ )
    {
        v.push_back( std::string( papszTokens[i] ) );
    }
    CSLDestroy( papszTokens );
    return v;
}

std::string NomadsWxModel::getForecastIdentifier()
{
    return getForecastReadable( '-' );
}

std::string NomadsWxModel::getForecastReadable( const char bySwapWithSpace )
{
    if( !ppszModelData )
    {
        throw badForecastFile( "Invalid Model" );
    }

    char *p = NULL;
    char *s = NULL;
    int i;
    s = NomadsFormName( pszKey, bySwapWithSpace );
    CPLAssert( s );
    std::string os = s;
    NomadsFree( s );
    return os;
}

static int NomadsCompareStrings( const void *a, const void *b )
{
    int n = strcmp( *(const char **)a, *(const char **)b );
    return n;
}

std::vector<blt::local_date_time>
NomadsWxModel::getTimeList( const char *pszVariable, 
                            blt::time_zone_ptr timeZonePtr )
{
    if( aoCachedTimes.size() > 0 )
        return aoCachedTimes;
    (void)pszVariable;
    if( wxModelFileName == "" || ppszModelData == NULL )
    {
        throw badForecastFile( "Invalid forecast file name" );
    }

    int i;
    char **papszFileList = NULL;
    const char *pszPath;
    VSIStatBufL sStat;
    VSIStatL( wxModelFileName.c_str(), &sStat );
    if( VSI_ISDIR( sStat.st_mode ) )
        pszPath = CPLStrdup( wxModelFileName.c_str() );
    else if( strstr( wxModelFileName.c_str(), ".zip" ) )
        pszPath = CPLStrdup( CPLSPrintf( "/vsizip/%s", wxModelFileName.c_str() ) );
    else
        pszPath = CPLStrdup( CPLGetPath( wxModelFileName.c_str() ) );
    const char *pszFullPath = NULL;
    papszFileList = VSIReadDir( pszPath );
    int nCount = CSLCount( papszFileList );
    if( !nCount )
    {
        throw badForecastFile( "Could not open forecast path" );
    }
    qsort( (void*)papszFileList, nCount, sizeof( char * ), NomadsCompareStrings );
           //(int (*)(const void*, const void*))strcmp );
    time_t nValidTime;
    const char *pszValidTime;
    GDALDatasetH hDS;
    GDALDatasetH hBand;
    std::vector<blt::local_date_time>aoTimeList;
    for( i = 0; i < nCount; i++ )
    {
        SKIP_DOT_AND_DOTDOT( papszFileList[i] );
        pszFullPath = CPLSPrintf( "%s/%s", pszPath, papszFileList[i] );
        if( !CheckFileName( papszFileList[i],
                            ppszModelData[NOMADS_FILE_NAME_FRMT] ) )
        {
            continue;
        }
        hDS = GDALOpenShared( pszFullPath, GA_ReadOnly );
        if( !hDS )
        {
            CSLDestroy( papszFileList );
            CPLFree( (void*)pszPath );
            throw badForecastFile( "Could not open forecast file with GDAL" );
        }
        hBand = GDALGetRasterBand( hDS, 1 );
        pszValidTime = GDALGetMetadataItem( hBand, "GRIB_VALID_TIME", NULL );
        if( !pszValidTime )
        {
            CSLDestroy( papszFileList );
            CPLFree( (void*)pszPath );
            GDALClose( hDS );
            throw badForecastFile( "Could not fetch ref time or forecast time " \
                                   "from GRIB file" );
        }
        CPLDebug( "WINDNINJA", "Found valid time in grib: %s", pszValidTime );
        nValidTime = (time_t)atoi( pszValidTime );
        GDALClose( hDS );

        bpt::ptime time_t_epoch( boost::gregorian::date( 1970,1,1 ) );

        bpt::time_duration duration( 0, 0, nValidTime );

        bpt::ptime first_pt( time_t_epoch + duration );
        blt::local_date_time first_pt_local( first_pt, timeZonePtr );
        aoTimeList.push_back( first_pt_local );
    }
    CSLDestroy( papszFileList );
    CPLFree( (void*)pszPath );
    aoCachedTimes = aoTimeList;
    return aoCachedTimes;
}

const char * NomadsWxModel::NomadsFindForecast( const char *pszFilePath,
                                                time_t nTime )
{
    int i;
    char **papszFileList = NULL;
    VSIStatBufL sStat;
    VSIStatL( pszFilePath, &sStat );
    const char *pszPath;
    if( VSI_ISDIR( sStat.st_mode ) )
        pszPath = CPLStrdup( pszFilePath );
    else if( strstr( wxModelFileName.c_str(), ".zip" ) )
        pszPath = CPLStrdup( CPLSPrintf( "/vsizip/%s", wxModelFileName.c_str() ) );
    else
        pszPath = CPLStrdup( CPLGetPath( pszFilePath ) );
    const char *pszFullPath = NULL;
    papszFileList = VSIReadDir( pszPath );
    int nCount = CSLCount( papszFileList );
    if( !nCount )
    {
        CPLFree( (void*)pszPath );
        return NULL;
    }
    time_t nValidTime;
    const char *pszValidTime;
    GDALDatasetH hDS;
    GDALDatasetH hBand;
    CPLPushErrorHandler( CPLQuietErrorHandler );
    for( i = 0; i < nCount; i++ )
    {
        SKIP_DOT_AND_DOTDOT( papszFileList[i] );
        if( !CheckFileName( papszFileList[i],
                            ppszModelData[NOMADS_FILE_NAME_FRMT] ) )
        {
            continue;
        }
        pszFullPath = CPLSPrintf( "%s/%s", pszPath, papszFileList[i] );
        hDS = GDALOpenShared( pszFullPath, GA_ReadOnly );
        if( !hDS )
        {
            continue;
        }
        hBand = GDALGetRasterBand( hDS, 1 );
        pszValidTime = GDALGetMetadataItem( hBand, "GRIB_VALID_TIME", NULL );
        if( !pszValidTime )
        {
            GDALClose( hDS );
            continue;
        }
        nValidTime = (time_t)atoi( pszValidTime );
        if( nValidTime == nTime )
        {
            CPLFree( (void*)pszPath );
            CSLDestroy( papszFileList );
            CPLPopErrorHandler();
            GDALClose( hDS );
            return CPLStrdup( pszFullPath );
        }
        GDALClose( hDS );
    }
    CPLFree( (void*)pszPath );
    CSLDestroy( papszFileList );
    CPLPopErrorHandler();
    return NULL;
}

void NomadsWxModel::setSurfaceGrids( WindNinjaInputs &input,
                                     AsciiGrid<double> &airGrid,
                                     AsciiGrid<double> &cloudGrid,
                                     AsciiGrid<double> &uGrid,
                                     AsciiGrid<double> &vGrid,
                                     AsciiGrid<double> &wGrid )
{
    std::vector<blt::local_date_time> timeList( getTimeList( NULL, input.ninjaTimeZone ) );
    int i;
    GDALDatasetH hSrcDS, hVrtDS;
    GDALDatasetH hBand;
    const char *pszSrcWkt, *pszDstWkt;
    GDALWarpOptions *psWarpOptions;
    int bSuccess;
    double dfNoData;
    int nBandCount;
    int bNeedNextCloud = FALSE;
    /*
    ** We need to find the correct file in the directory.  It may not be the
    ** filename.
    */
    const char *pszForecastFile = NULL;
    for( i = 0; i < (int)timeList.size(); i++ )
    {
        if( timeList[i] == input.ninjaTime )
        {
            bpt::ptime epoch( boost::gregorian::date( 1970, 1, 1 ) );
            bpt::time_duration::sec_type t;
            t = (input.ninjaTime.utc_time() - epoch).total_seconds();
            pszForecastFile =
                NomadsFindForecast( input.forecastFilename.c_str(), (time_t)t );
            if( i == 0 && timeList.size() > 1 )
            {
                bNeedNextCloud = TRUE;
            }
            break;
        }

    }
    if( !pszForecastFile )
    {
        throw badForecastFile( "Could not find forecast associated with " \
                               "requested time step" );
    }

    hSrcDS = GDALOpenShared( pszForecastFile, GA_ReadOnly );
    nBandCount = GDALGetRasterCount( hSrcDS );
    hBand = GDALGetRasterBand( hSrcDS, 1 );
    dfNoData = GDALGetRasterNoDataValue( hBand, &bSuccess );
    if( bSuccess == FALSE )
    {
        dfNoData = -9999.0;
    }

    psWarpOptions = GDALCreateWarpOptions();
    psWarpOptions->padfDstNoDataReal =
        (double*) CPLMalloc( sizeof( double ) * nBandCount );
    psWarpOptions->padfDstNoDataImag =
        (double*) CPLMalloc( sizeof( double ) * nBandCount );
    for( i = 0; i < nBandCount; i++ )
    {
        psWarpOptions->padfDstNoDataReal[i] = dfNoData;
        psWarpOptions->padfDstNoDataImag[i] = dfNoData;
    }

    pszSrcWkt = GDALGetProjectionRef( hSrcDS );
    pszDstWkt = input.dem.prjString.c_str();
    hVrtDS = GDALAutoCreateWarpedVRT( hSrcDS, pszSrcWkt, pszDstWkt,
                                      GRA_NearestNeighbour, 1.0,
                                      psWarpOptions );

    const char *pszElement;
    const char *pszShortName;
    int bHaveTemp, bHaveCloud;
    bHaveTemp = FALSE;
    bHaveCloud = FALSE;
    for( i = 0; i < nBandCount; i++ )
    {
        hBand = GDALGetRasterBand( hVrtDS, i + 1 );
        /*
        ** Check the shortname and make sure it's valid.  HRRR TCDC uses
        ** entire_atmosphere instead of
        ** entire_atmosphere_(considered_as_a_single_layer), so we account for
        ** this with the 0-RESERVED(10).
        */
        pszShortName = GDALGetMetadataItem( hBand, "GRIB_SHORT_NAME", NULL );
        if( !EQUAL( "10-HTGL", pszShortName ) &&
            !EQUAL( "2-HTGL", pszShortName ) &&
            !EQUAL( "0-EATM", pszShortName ) &&
            !EQUAL( "0-CCY", pszShortName ) &&
            !EQUAL( "0-RESERVED(10)", pszShortName ) )
        {
            continue;
        }

        pszElement = GDALGetMetadataItem( hBand, "GRIB_ELEMENT", NULL );
        if( !pszElement )
        {
            throw badForecastFile( "Could not fetch proper band" );
        }
        if( EQUAL( pszElement, "TMP" ) )
        {
            GDAL2AsciiGrid( (GDALDataset*)hVrtDS, i + 1, airGrid );
            if( CPLIsNan( dfNoData ) )
            {
                airGrid.set_noDataValue( -9999.0 );
                airGrid.replaceNan( -9999.0 );
            }
            bHaveTemp = TRUE;
        }
        else if( EQUAL( pszElement, "UGRD" ) )
        {
            GDAL2AsciiGrid( (GDALDataset*)hVrtDS, i + 1, uGrid );
            if( CPLIsNan( dfNoData ) )
            {
                uGrid.set_noDataValue( -9999.0 );
                uGrid.replaceNan( -9999.0 );
            }
        }
        else if( EQUAL( pszElement, "VGRD" ) )
        {
            GDAL2AsciiGrid( (GDALDataset*)hVrtDS, i + 1, vGrid );
            if( CPLIsNan( dfNoData ) )
            {
                vGrid.set_noDataValue( -9999.0 );
                vGrid.replaceNan( -9999.0 );
            }
        }
        else if( EQUAL( pszElement, "TCDC" ) )
        {
            GDAL2AsciiGrid( (GDALDataset*)hVrtDS, i + 1, cloudGrid );
            if( CPLIsNan( dfNoData ) )
            {
                cloudGrid.set_noDataValue( -9999.0 );
                cloudGrid.replaceNan( -9999.0 );
            }
            bHaveCloud = TRUE;
        }
    }
    GDALClose( hSrcDS );
    GDALClose( hVrtDS );
    if( !bHaveCloud )
    {
        /*
        ** If we don't have cloud cover, and we are on the first time step, we
        ** may be in a strange discretization situation.  GFS 0th time step
        ** doesn't have time averaged cloud cover.  Let's go forward in time
        ** and copy the cloud cover from the 1st time.  Issue a warning.
        **
        ** Note that GDAL handles thread safety *in* the GRIB driver by
        ** acquiring a mutex in the driver.  We should be fine here, as long as
        ** we use GDALOpenShared().
        */
        if( bNeedNextCloud == TRUE )
        {
            GDALDatasetH hDSNextCloud;
            const char *pszNextFcst;
            bpt::ptime epoch( boost::gregorian::date( 1970, 1, 1 ) );
            bpt::time_duration::sec_type t;
            t = (timeList[1].utc_time() - epoch).total_seconds();
            pszNextFcst =
                NomadsFindForecast( input.forecastFilename.c_str(), (time_t)t );
            hSrcDS = GDALOpenShared( pszNextFcst, GA_ReadOnly );
            if( hSrcDS == NULL )
            {
                throw badForecastFile( "Could not load cloud data." );
            }
            pszSrcWkt = GDALGetProjectionRef( hSrcDS );
            hVrtDS = GDALAutoCreateWarpedVRT( hSrcDS, pszSrcWkt, pszDstWkt,
                                              GRA_NearestNeighbour, 1.0,
                                              psWarpOptions );
            if( hVrtDS == NULL )
            {
                throw badForecastFile( "Could not load cloud data." );
            }
            int j = 0;
            for( j = 0; j < GDALGetRasterCount( hVrtDS ); j++ )
            {
                hBand = GDALGetRasterBand( hVrtDS, j + 1 );
                pszElement = GDALGetMetadataItem( hBand, "GRIB_ELEMENT", NULL );
                if( EQUAL( pszElement, "TCDC" ) )
                {
                    break;
                }
                hBand = NULL;
            }
            if( hBand == NULL )
            {
                throw badForecastFile( "Could not find cloud data band." );
            }
            GDAL2AsciiGrid( (GDALDataset*)hVrtDS, j + 1, cloudGrid );
            dfNoData = GDALGetRasterNoDataValue( hBand, &bSuccess );
            if( bSuccess == FALSE )
            {
                dfNoData = -9999.0;
            }
            if( CPLIsNan( dfNoData ) )
            {
                cloudGrid.set_noDataValue( -9999.0 );
                cloudGrid.replaceNan( -9999.0 );
            }
            CPLFree( (void*)pszNextFcst );
            GDALClose( hSrcDS );
            GDALClose( hVrtDS );
            CPLError( CE_Warning, CPLE_AppDefined, "Could not load cloud data "
                      "from 0th time step, using time step 1." );
        }
        else
        {
            CPLError( CE_Warning, CPLE_AppDefined, "Could not load cloud data " \
                      "from the forecast file, setting to 0." );

            cloudGrid.set_headerData( uGrid );
            cloudGrid = 0.0;
        }
    }
    cloudGrid /= 100.0;
    airGrid += 273.15;

    wGrid.set_headerData( uGrid );
    wGrid = 0.0;

    GDALDestroyWarpOptions( psWarpOptions );
}

void NomadsWxModel::checkForValidData()
{
    return;
}

int NomadsWxModel::ClipNoData( GDALRasterBandH hBand, double dfNoData,
                               int *pnRowsToCull, int *pnColsToCull )
{
    int i, j, k;
    double *padfData;
    int nXSize, nYSize;
    nXSize = GDALGetRasterBandXSize( hBand );
    nYSize = GDALGetRasterBandYSize( hBand );
    padfData = (double*)CPLMalloc( sizeof( double ) * nXSize );
    GDALRasterIO( hBand, GF_Read, 0, 0, nXSize, 1,
                  padfData, nXSize, 1, GDT_Float64, 0, 0 );
    i = k = j = 0;
    while( padfData[i] == dfNoData && i < nXSize )
        i++;
    j = i;
    i = nXSize - 1;
    while( padfData[i] == dfNoData && i >= 0 )
        i--;
    k = nXSize - i;
    if( pnColsToCull )
        *pnColsToCull = j < k ? j : k;
    padfData = (double*)CPLRealloc( padfData, sizeof( double ) * nYSize );
    GDALRasterIO( hBand, GF_Read, 0, 0, 1, nYSize,
                  padfData, 1, nYSize, GDT_Float64, 0, 0 );
    i = k = j = 0;
    while( padfData[i] == dfNoData && i < nYSize )
        i++;
    j = i;
    i = nYSize - 1;
    while( padfData[i] == dfNoData && i >= 0 )
        i--;
    k = nYSize - i;
    if( pnRowsToCull )
        *pnRowsToCull = j < k ? j : k;
    CPLFree( (void*)padfData );
    return 0;
}

GDALRasterBandH NomadsWxModel::FindBand( GDALDatasetH hDS, const char *pszVar,
                                         const char *pszHeight )
{
    int i;
    int n = GDALGetRasterCount( hDS );
    const char *pszElement, *pszShortName, *pszNeedle;
    int nMillibars, nPascals;
    GDALRasterBandH hBand;
    nMillibars = atoi( pszHeight );
    nPascals = nMillibars * 100;
    pszNeedle = CPLSPrintf( "%d-ISBL", nPascals );
    for( i = 0; i < n; i++ )
    {
        hBand = GDALGetRasterBand( hDS, i + 1 );
        pszElement = GDALGetMetadataItem( hBand, "GRIB_ELEMENT", NULL );
        if( !EQUAL( pszVar, pszElement ) )
            continue;
        pszShortName = GDALGetMetadataItem( hBand, "GRIB_SHORT_NAME", NULL );
        if( EQUAL( pszNeedle, pszShortName ) )
            return hBand;
    }
    return NULL;
}

#define NOMADS_NON_PRES 4

void NomadsWxModel::set3dGrids( WindNinjaInputs &input, Mesh const& mesh )
{
#ifdef NOMADS_ENABLE_3D
    if( ppszModelData == NULL )
        return;
    int g, h, i, j, k, n;
    GDALDatasetH hDS, hVrtDS;
    GDALRasterBandH hBand;
    const char *pszSrcWkt, *pszDstWkt;
    GDALWarpOptions *psWarpOptions;
    int nLayerCount;

    char **papszLevels =
        CSLTokenizeString2( ppszModelData[NOMADS_LEVELS], ",", 0 );
    nLayerCount = CSLCount( papszLevels );
    if( nLayerCount < 4 )
    {
        CSLDestroy( papszLevels );
        return;
    }
    std::vector<blt::local_date_time> timeList( getTimeList( NULL, input.ninjaTimeZone ) );
    /*
    ** We need to find the correct file in the directory.  It may not be the
    ** filename.
    */
    const char *pszForecastFile = NULL;
    for( i = 0; i < (int)timeList.size(); i++ )
    {
        if( timeList[i] == input.ninjaTime )
        {
            bpt::ptime epoch( boost::gregorian::date( 1970, 1, 1 ) );
            bpt::time_duration::sec_type t;
            t = (input.ninjaTime.utc_time() - epoch).total_seconds();
            pszForecastFile =
                NomadsFindForecast( input.forecastFilename.c_str(), (time_t)t );
            break;
        }
    }
    if( !pszForecastFile )
    {
        throw badForecastFile( "Could not find forecast associated with " \
                               "requested time step" );
    }

    hDS = GDALOpenShared( pszForecastFile, GA_ReadOnly );
    int nBandCount = GDALGetRasterCount( hDS );
    hBand = GDALGetRasterBand( hDS, 1 );
    int bSuccess;
    double dfNoData = GDALGetRasterNoDataValue( hBand, &bSuccess );
    if( !bSuccess )
        dfNoData = -9999.0;
    psWarpOptions = GDALCreateWarpOptions();
    psWarpOptions->padfDstNoDataReal =
        (double*) CPLMalloc( sizeof( double ) * nBandCount );
    psWarpOptions->padfDstNoDataImag =
        (double*) CPLMalloc( sizeof( double ) * nBandCount );
    for( i = 0; i < nBandCount; i++ )
    {
        psWarpOptions->padfDstNoDataReal[i] = dfNoData;
        psWarpOptions->padfDstNoDataImag[i] = dfNoData;
    }
    psWarpOptions->papszWarpOptions =
            CSLSetNameValue( psWarpOptions->papszWarpOptions,
                             "INIT_DEST", "-9999.0" );

    pszSrcWkt = GDALGetProjectionRef( hDS );
    pszDstWkt = input.dem.prjString.c_str();
    hVrtDS = GDALAutoCreateWarpedVRT( hDS, pszSrcWkt, pszDstWkt,
                                      GRA_NearestNeighbour, 1.0,
                                      psWarpOptions );

    int nSkipRows, nSkipCols;
    hBand = GDALGetRasterBand( hVrtDS, 1 );
    ClipNoData( hBand, dfNoData, &nSkipRows, &nSkipCols );
    nSkipRows++;
    nSkipCols++;

    int nXSize, nYSize;
    double dfXOrigin, dfYOrigin;
    double dfDeltaX, dfDeltaY;
    double adfGeoTransform[6];
    GDALGetGeoTransform( hVrtDS, adfGeoTransform );
    dfXOrigin = adfGeoTransform[0];
    dfYOrigin = adfGeoTransform[3];
    dfDeltaX = adfGeoTransform[1];
    dfDeltaY = adfGeoTransform[5];
    nXSize = GDALGetRasterXSize( hVrtDS );
    nYSize = GDALGetRasterYSize( hVrtDS );
    int nXSubSize ,nYSubSize;
    nXSubSize = nXSize - nSkipCols * 2;
    nYSubSize = nYSize - nSkipRows * 2;
    double *padfData = (double*)CPLMalloc( sizeof( double ) * nXSubSize );
    /* Subtract the surface layers */
    nLayerCount -= NOMADS_NON_PRES;
    oArray.allocate( nYSubSize, nXSubSize, nLayerCount );
    /* We assume our levels are in order from the groud up in the level list */
    int rc;
    i = 0;
    h = 0;
    while( h < nLayerCount && i < nLayerCount + NOMADS_NON_PRES )
    {
        if( strstr( papszLevels[i], "_m_above_ground" ) ||
            strstr( papszLevels[i], "surface" ) ||
            strstr( papszLevels[i], "entire_atmosphere" ) )
        {
            i++;
            continue;
        }

        hBand = FindBand( hVrtDS, "HGT", papszLevels[i] );
        if( hBand == NULL )
        {
            i++;
            continue;
        }
        n = 0;
        for( j = nYSize - nSkipRows - 1; j >= nSkipRows; j-- )
        {
            rc = GDALRasterIO( hBand, GF_Read, nSkipCols, j, nXSubSize, 1, 
                               padfData, nXSubSize, 1, GDT_Float64, 0, 0 );
            for( k = 0; k < nXSubSize; k++ )
            {
                oArray( n, k, h ) = padfData[k];
            }
            n++;
        }
        h++;
        i++;
    }
    double xOffset, yOffset;
    double xllNinja, yllNinja, xllWxModel, yllWxModel;
    input.dem.get_cellPosition( 0, 0, &xllNinja, &yllNinja );
    xllWxModel = dfXOrigin + nSkipCols * dfDeltaX;
    yllWxModel = dfYOrigin + ((nYSize - nSkipRows) * dfDeltaY);

    xllWxModel += dfDeltaX / 2;
    yllWxModel += dfDeltaY / 2;

    xOffset = xllWxModel - xllNinja;
    yOffset = yllWxModel - yllNinja;

    wxMesh.buildFrom3dWeatherModel( input, oArray, dfDeltaX,
                                    nYSubSize, nXSubSize, nLayerCount,
                                    xOffset, yOffset );
    volVTK vtk;
    vtk.writeMeshVolVTK(wxMesh.XORD, wxMesh.YORD, wxMesh.ZORD,
                        wxMesh.ncols, wxMesh.nrows, wxMesh.nlayers,
                        "wxMesh.vtk");
    Mesh m2 = mesh;
    vtk.writeMeshVolVTK(m2.XORD, m2.YORD, m2.ZORD, 
                        m2.ncols, m2.nrows, m2.nlayers,
                        "mackay.vtk");

    /* u,v,w,t */
    wxFields[0] = &wxU3d;
    wxFields[1] = &wxV3d;
    wxFields[2] = &wxW3d;
    wxFields[3] = &wxAir3d;
    wxFields[4] = &wxCloud3d;
    fields[0] = &u3d;
    fields[1] = &v3d;
    fields[2] = &w3d;
    fields[3] = &air3d;
    h = 0;

    static const char *apszVarList[] = { "UGRD", "VGRD", "DZDT", "TMP", NULL };
    while( apszVarList[h] != NULL )
    {
        wxFields[h]->allocate( &wxMesh );
        i = 0;
        g = 0;
        while( g < nLayerCount  && i < nLayerCount + NOMADS_NON_PRES )
        {
            if( strstr( papszLevels[i], "_m_above_ground" ) ||
                strstr( papszLevels[i], "surface" ) ||
                strstr( papszLevels[i], "entire_atmosphere" ) )
            {
                i++;
                continue;
            }
            hBand = FindBand( hVrtDS, apszVarList[h], papszLevels[i] );
            if( hBand == NULL )
            {
                i++;
                continue;
            }
            n = 0;
            for( j = nYSize - nSkipRows - 1; j >= nSkipRows; j-- )
            {
                rc = GDALRasterIO( hBand, GF_Read, nSkipCols, j, nXSubSize, 1, 
                                   padfData, nXSubSize, 1, GDT_Float64, 0, 0 );
                for( k = 0; k < nXSubSize; k++ )
                {
                    (*(wxFields[h]))( n, k, g ) = padfData[k];
                }
                n++;
            }
            g++;
            i++;
        }
        h++;
    }
    wxCloud3d.allocate( &wxMesh );

    CPLFree( (void*)padfData );
    CSLDestroy( papszLevels );
    GDALClose( hDS );
    GDALClose( hVrtDS );

    for( i = 0; i < 4; i++ )
    {
        fields[i]->allocate( &mesh );
        wxFields[i]->interpolateScalarData((*(fields[i])), mesh, input);
    }
#endif /* NOMADS_ENABLE_3D */
    return;
}

