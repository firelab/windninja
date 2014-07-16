/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja
 * Purpose:  Nomads weather model initialization
 * Author:   Kyle Shannon <kyle@pobox.com>
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

static int NomadsCheckFileName( const char *pszFile, const char *pszFormat )
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
            if( NomadsCheckFileName( papszFileList[j],
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
    pszTmpFile = CPLStrdup (CPLFormFilename( NULL, pszTmpFile, ".zip" ) );

    const char *p = strchr( pszTmpFile, '/' );
    if( !p )
        p = strchr( pszTmpFile, '\\' );
    if( !p )
        p = pszTmpFile;
    pszTmpFile = CPLStrdup( p );
    VSIMkdir( p, 0777 );

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
    if( !ppszModelData )
    {
        throw badForecastFile( "Invalid Model" );
    }
    return std::string( ppszModelData[NOMADS_NAME] );
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
    if( wxModelFileName == "" )
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
        if( !NomadsCheckFileName( papszFileList[i],
                                  ppszModelData[NOMADS_FILE_NAME_FRMT] ) )
        {
            continue;
        }
        hDS = GDALOpen( pszFullPath, GA_ReadOnly );
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
        if( !NomadsCheckFileName( papszFileList[i],
                                  ppszModelData[NOMADS_FILE_NAME_FRMT] ) )
        {
            continue;
        }
        pszFullPath = CPLSPrintf( "%s/%s", pszPath, papszFileList[i] );
        hDS = GDALOpen( pszFullPath, GA_ReadOnly );
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
    int bHaveTemp, bHaveCloud;
    bHaveTemp = FALSE;
    bHaveCloud = FALSE;
    for( i = 0; i < (nBandCount > 4 ? 4 : nBandCount); i++ )
    {
        hBand = GDALGetRasterBand( hVrtDS, i + 1 );
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
    if( !bHaveCloud )
        cloudGrid = 0.0;
    cloudGrid /= 100.0;
    airGrid += 273.15;

    wGrid.set_headerData( uGrid );
    wGrid = 0.0;

    GDALDestroyWarpOptions( psWarpOptions );
    GDALClose( hSrcDS );
    GDALClose( hVrtDS );
}

void NomadsWxModel::checkForValidData()
{
    return;
}


