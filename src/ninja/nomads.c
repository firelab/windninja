/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja
 * Purpose:  Nomads C client
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

#include "nomads.h"

const char ** NomadsFindModel( const char *pszKey )
{
    int i = 0;
    const char **ppszKey = NULL;
    while( apszNomadsKeys[i][0] != NULL )
    {
        if( EQUAL( pszKey, apszNomadsKeys[i][0] ) )
        {
            ppszKey = apszNomadsKeys[i];
            CPLDebug( "NOMADS", "Found model key: %s",
                      ppszKey[NOMADS_NAME] );
            return ppszKey;
        }
        i++;
    }
    return ppszKey;
}

static const char * NomadsBuildArgList( const char *pszVars,
                                        const char *pszPrefix )
{
    int i, n;
    const char *pszList;
    char **papszArgs = CSLTokenizeString2( pszVars, ",", 0 );
    n = CSLCount( papszArgs );
    if( !n )
        return NULL;
    pszList = CPLSPrintf( "%s_%s=on", pszPrefix, papszArgs[0] );
    for( i = 1; i < n; i++ )
    {
        pszList = CPLSPrintf( "%s&%s_%s=on", pszList, pszPrefix, papszArgs[i] );
    }
    CSLDestroy( papszArgs );
    return pszList;
}
/*
** Find the forecast time for a model that is n runs back.
*/
static int NomadsFindForecastHour( const char **ppszKey, nomads_utc *now, int n )
{

    char **papszHours;
    int nStart, nStop, nStride;
    int nFcstHour;
    int i;
    nomads_utc *u;
    NomadsUtcCreate( &u );
    NomadsUtcCopy( u, now );

    n = -abs(n);

    papszHours = CSLTokenizeString2( ppszKey[NOMADS_FCST_HOURS], ":", 0 );
    nStart = atoi( papszHours[0] );
    nStop = atoi( papszHours[1] );
    nStride = atoi( papszHours[2] );
    NomadsUtcAddHours( u, nStride * n );
    for( i = nStart; i <= nStop; i += nStride )
    {
        if( i > u->ts->tm_hour )
            break;
    }
    nFcstHour = i - nStride;
    CSLDestroy( papszHours );
    papszHours = NULL;
    NomadsUtcFree( u );
    return nFcstHour;
}

static int NomadsBuildForecastRunHours( const char **ppszKey,
                                        const nomads_utc *now,
                                        const nomads_utc *end,
                                        int nHours, int **panRunHours )
{
    char **papszHours, **papszRunHours;
    int nStart, nStop, nStride;
    int nTokenCount;
    int i, j, k;
    int nSize;

    nomads_utc *tmp;
    NomadsUtcCreate( &tmp );
    NomadsUtcCopy( tmp, now );

    papszHours = CSLTokenizeString2( ppszKey[NOMADS_FCST_RUN_HOURS], ",", 0 );
    nTokenCount = CSLCount( papszHours );
    CPLAssert( nTokenCount > 0 );
    papszRunHours = CSLTokenizeString2( papszHours[nTokenCount - 1], ":", 0);

    /* We usually have a 0 hour, add one to the max. */
    nSize = atoi( papszRunHours[1] ) + 1;
    (*panRunHours) = (int*)CPLMalloc( sizeof( int ) * nSize );
    CSLDestroy( papszRunHours );
    papszRunHours = NULL;
    CSLDestroy( papszHours );
    papszHours = NULL;
    papszHours = CSLTokenizeString2( ppszKey[NOMADS_FCST_RUN_HOURS], ",", 0 );
    k = 0;
    for( i = 0; i < nTokenCount; i++ )
    {
        papszRunHours = CSLTokenizeString2( papszHours[i], ":", 0 );
        nStart = atoi( papszRunHours[0] );
        nStop = atoi( papszRunHours[1] );
        nStride = atoi( papszRunHours[2] );
        for( j = nStart; j <= nStop; j += nStride )
        {
            (*panRunHours)[k++] = j;
            NomadsUtcAddHours( tmp, nStride );
            if( NomadsUtcCompare( tmp, end ) > 0 )
            {
                CSLDestroy( papszRunHours );
                NomadsUtcFree( tmp );
                CSLDestroy( papszHours );
                return k;
            }
        }
        CSLDestroy( papszRunHours );
        papszRunHours = NULL;
    }
    CSLDestroy( papszHours );
    NomadsUtcFree( tmp );
    return k;
}

static char ** NomadsBuildForecastFileList( const char *pszKey, int nFcstHour,
                                            const int *panRunHours, int nHours,
                                            nomads_utc *fcst,
                                            double *padfBbox )
{
    const char **ppszKey;
    const char *pszGribFile;
    const char *pszGribDir;
    const char *pszUrl;
    char **papszFileList = NULL;

    int i;

    ppszKey = NomadsFindModel( pszKey );
    if( !ppszKey )
    {
        return NULL;
    }

    for( i = 0; i < nHours; i++ )
    {
        pszGribFile = CPLSPrintf( ppszKey[NOMADS_FILE_NAME_FRMT], nFcstHour,
                                  panRunHours[i] );
        CPLDebug( "WINDNINJA", "NOMADS generated grib file name: %s",
                  pszGribFile );
        NomadsUtcStrfTime( fcst, ppszKey[NOMADS_DIR_DATE_FRMT] );
        /* Special case for gfs */
        if( EQUAL( pszKey, "gfs" ) )
        {
            pszGribDir = CPLSPrintf( ppszKey[NOMADS_DIR_FRMT], fcst->s, nFcstHour );
        }
        else
        {
            pszGribDir = CPLSPrintf( ppszKey[NOMADS_DIR_FRMT], fcst->s );
        }
        CPLDebug( "WINDNINJA", "NOMADS generated grib directory: %s",
                  pszGribDir );
        pszUrl =
            CPLSPrintf( "%s%s?%s&%s%s&file=%s&dir=/%s", NOMADS_URL_CGI,
                        ppszKey[NOMADS_FILTER_BIN],
                        NomadsBuildArgList( ppszKey[NOMADS_VARIABLES], "var" ),
                        NomadsBuildArgList( ppszKey[NOMADS_LEVELS], "lev" ),
                        NOMADS_SUBREGION, pszGribFile, pszGribDir );
        pszUrl = CPLSPrintf( pszUrl, padfBbox[0], padfBbox[1],
                             padfBbox[2], padfBbox[3] );
        CPLDebug( "WINDNINJA", "NOMADS generated url: %s", pszUrl );
        papszFileList = CSLAddString( papszFileList, pszUrl );
    }
    return papszFileList;
}

static char ** NomadsBuildOutputFileList( const char *pszKey, int nFcstHour,
                                          int *panRunHours, int nHours, 
                                          const char *pszPath, int bVsiZip )
{
    const char **ppszKey;
    const char *pszGribFile;
    char **papszFileList = NULL;

    int i;

    ppszKey = NomadsFindModel( pszKey );
    if( !ppszKey )
    {
        return NULL;
    }

    for( i = 0; i < nHours; i++ )
    {
        pszGribFile = CPLSPrintf( ppszKey[NOMADS_FILE_NAME_FRMT], nFcstHour,
                                  panRunHours[i] );
        if( pszPath )
        {
            pszGribFile = CPLSPrintf( "%s/%s", pszPath, pszGribFile );
        }
        if( bVsiZip )
        {
            pszGribFile = CPLSPrintf( "/vsizip/%s", pszGribFile );
        }
        CPLDebug( "WINDNINJA", "NOMADS generated grib file name: %s",
                  pszGribFile );
        papszFileList = CSLAddString( papszFileList, pszGribFile );
    }
    return papszFileList;
}

static int NomadsZipFiles( char **papszIn, char **papszOut )
{
    int i, n, c, rc;
    VSILFILE *fin, *fout;
    char *pabyBuffer;
    n = CSLCount( papszIn );
    if( n != CSLCount( papszOut ) )
        return NOMADS_ERR;
    for( i = 0; i < n; i++ )
    {
        //VSIRename( papszIn[i], papszOut[i] );
        fin = VSIFOpenL( papszIn[i], "rb" );
        fout = VSIFOpenL( papszOut[i], "wb" );
        if( !fin || !fout )
            return NOMADS_ERR;
        rc = VSIFSeekL( fin, 0, SEEK_END );
        c = VSIFTellL( fin );
        rc = VSIFSeekL( fin, 0, SEEK_SET );
        pabyBuffer = CPLMalloc( sizeof( char ) * c );
        rc = VSIFReadL( pabyBuffer, c, 1, fin );
        rc = VSIFWriteL( pabyBuffer, c, 1, fout );
        CPLFree( (void*)pabyBuffer );
        rc = VSIFCloseL( fin );
        rc = VSIFCloseL( fout );
    }
    return NOMADS_OK;
}

static int NomadsFetchVsi( const char *pszUrl, const char *pszFilename )
{
    int rc;
    const char *pszVsiUrl;
    VSILFILE *fin, *fout;
    vsi_l_offset nOffset, nBytesWritten, nVsiBlockSize;
    char *pabyBuffer;
    nVsiBlockSize = atoi( CPLGetConfigOption( "NOMADS_VSI_BLOCK_SIZE", "512" ) );

    if( !EQUALN( pszUrl, "/vsicurl/", 9 ) )
    {
        pszVsiUrl = CPLSPrintf( "/vsicurl/%s", pszUrl );
    }
    fin = VSIFOpenL( pszVsiUrl, "rb" );
    if( !fin )
    {
        CPLError( CE_Failure, CPLE_AppDefined,
                  "Failed to download file." );
        return NOMADS_ERR;
    }
    fout = VSIFOpenL( pszFilename, "wb" );
    if( !fout )
    {
        CPLError( CE_Failure, CPLE_AppDefined,
                  "Failed to open file for writing." );
        VSIFCloseL( fin );
        return NOMADS_ERR;
    }
    nBytesWritten = 0;
    pabyBuffer = CPLMalloc( sizeof( char ) * nVsiBlockSize );
    rc = 0;
    do
    {
        nOffset = VSIFReadL( pabyBuffer, 1, nVsiBlockSize, fin );
        nBytesWritten += nOffset;
        VSIFWriteL( pabyBuffer, 1, nOffset, fout );
    } while ( nOffset != 0 );
    VSIFCloseL( fin );
    VSIFCloseL( fout );
    if( nBytesWritten == 0 )
    {
        VSIUnlink( pszFilename );
        rc = NOMADS_ERR;
    }
    CPLFree( (void*)pabyBuffer );
    return rc;
}

static int NomadsFetchHttp( const char *pszUrl, const char *pszFilename )
{
    CPLHTTPResult *psResult;
    VSILFILE *fout;
    psResult = NULL;
    psResult = CPLHTTPFetch( pszUrl, NULL );
    if( !psResult || psResult->nStatus != 0 ||
        strstr( psResult->pabyData, "HTTP error code : 404" ) ||
        strstr( psResult->pabyData, "data file is not present" ) )
    {
        CPLHTTPDestroyResult( psResult );
        CPLError( CE_Failure, CPLE_AppDefined,
                  "Failed to download file." );
        return NOMADS_ERR;
    }
    fout = VSIFOpenL( pszFilename, "wb" );
    if( !fout )
    {
        CPLError( CE_Failure, CPLE_AppDefined,
                  "Failed to open file for writing." );
        CPLHTTPDestroyResult( psResult );
        return NOMADS_ERR;
    }
    VSIFWriteL( psResult->pabyData, psResult->nDataLen, 1, fout );
    VSIFCloseL( fout );
    CPLHTTPDestroyResult( psResult );
    return NOMADS_OK;
}

static void NomadsFetchAsync( void *pData )
{
    int rc;
    NomadsThreadData *psData;
    psData = (NomadsThreadData*)pData;
#ifdef NOMADS_USE_VSI_READ
    rc = NomadsFetchVsi( psData->pszUrl, psData->pszFilename );
#else
    rc = NomadsFetchHttp( psData->pszUrl, psData->pszFilename );
#endif
    psData->nErr = rc;
}

int NomadsFetch( const char *pszModelKey, int nHours, double *padfBbox,
                 const char *pszDstVsiPath, char ** papszOptions,
                 GDALProgressFunc pfnProgress )
{
    const char **ppszKey = NULL;
    int nFcstHour = 0;
    int *panRunHours = NULL;
    int i = 0;
    int j = 0;
    int k = 0;
    int t = 0;
    int rc = 0;
    char **papszDownloadUrls = NULL;
    char **papszOutputFiles = NULL;
    char **papszFinalFiles = NULL;
    int nFilesToGet = 0;
    int bAlreadyWentBack = FALSE;
    int bFirstFile = TRUE;
    void **pThreads;
    int nThreads;
    const char *pszThreadCount;
    const char *pszTmpDir;
    VSIStatBuf sStat;

    int nrc;

    NomadsThreadData *pasData;
    nomads_utc *now, *end, *tmp, *fcst;
    CPLSetConfigOption( "GDAL_HTTP_TIMEOUT", "5" );
    nrc = NOMADS_OK;

    ppszKey = NomadsFindModel( pszModelKey );
    if( ppszKey == NULL )
    {
        CPLError( CE_Failure, CPLE_AppDefined,
                  "Could not find model key in nomads data" );
        return NOMADS_ERR;
    }

    NomadsUtcCreate( &now );
    NomadsUtcCreate( &end );
    NomadsUtcCreate( &tmp );
    NomadsUtcCreate( &fcst );
    NomadsUtcNow( now );
    NomadsUtcCopy( end, now );
    NomadsUtcAddHours( end, nHours );
    NomadsUtcCopy( tmp, now );

#ifdef NOMADS_ENABLE_ASYNC
    pszThreadCount = CPLGetConfigOption( "NOMADS_THREAD_COUNT", "4" );
    nThreads = atoi( pszThreadCount );
    nThreads = nThreads < 1 ? 4 : nThreads;
    pThreads = CPLMalloc( sizeof( void * ) * nThreads );
    pasData = CPLMalloc( sizeof( NomadsThreadData ) * nThreads );
#else
    pThreads = NULL;
    pasData = NULL;
#endif

    nFcstHour = NomadsFindForecastHour( ppszKey, now, 0 );
try_again:
    NomadsUtcCopy( fcst, now );
    NomadsUtcAddHours( fcst, nFcstHour - fcst->ts->tm_hour );
    CPLDebug( "WINDNINJA", "Generated forecast time in utc: %s",
              NomadsUtcStrfTime( fcst, "%Y%m%dT%HZ" ) );

    if( EQUAL( pszModelKey, "rtma_conus" ) )
    {
        panRunHours = (int*)CPLMalloc( sizeof( int ) );
        nFilesToGet = 1;
    }
    else
    {
        nFilesToGet =
            NomadsBuildForecastRunHours( ppszKey, now, end, nHours,
                                         &panRunHours );
    }

    papszDownloadUrls =
        NomadsBuildForecastFileList( pszModelKey, nFcstHour, panRunHours,
                                     nFilesToGet, fcst, padfBbox );
    pszTmpDir = CPLStrdup( CPLGenerateTempFilename( NULL ) );
    CPLDebug( "WINDNINJA", "Creating Temp directory: %s", pszTmpDir );
    VSIMkdir( pszTmpDir, 0777 );
    papszOutputFiles =
        NomadsBuildOutputFileList( pszModelKey, nFcstHour, panRunHours,
                                   nFilesToGet, pszTmpDir, FALSE );
                                                //pszDstVsiPath,
                                   //strstr( pszDstVsiPath, ".zip" ) ? 1 : 0 );
    if( !papszDownloadUrls || !papszOutputFiles )
    {
        CPLError( CE_Failure, CPLE_AppDefined,
                  "Could not generate list of URLs to download, invalid data" );
        nrc = NOMADS_ERR;
        goto cleanup;
    }

    CPLAssert( CSLCount( papszDownloadUrls ) == nFilesToGet );
    CPLAssert( CSLCount( papszOutputFiles ) == nFilesToGet );

    if( pfnProgress )
    {
        pfnProgress( 0.0, "Starting download...", NULL );
    }

    /* Download one file and start over if it's not there. */
#ifdef NOMADS_USE_VSI_READ
    rc = NomadsFetchVsi( papszDownloadUrls[0], papszOutputFiles[0] );
#else /* NOMADS_USE_VSI_READ */
    rc = NomadsFetchHttp( papszDownloadUrls[0], papszOutputFiles[0] );
#endif /* NOMADS_USE_VSI_READ */
    if( rc )
    {
        CPLError( CE_Warning, CPLE_AppDefined,
                  "Failed to download forecast, " \
                  "stepping back one forecast run time step." );
        nFcstHour = NomadsFindForecastHour( ppszKey, now, 1 );
        bAlreadyWentBack = TRUE;
        bFirstFile = FALSE;
        CPLUnlinkTree( pszTmpDir );
        CPLFree( (void*)panRunHours );
        CPLFree( (void*)pszTmpDir );
        CSLDestroy( papszDownloadUrls );
        CSLDestroy( papszOutputFiles );
        goto try_again;
    }
    /* Get the rest */
    for( i = 1; i < nFilesToGet; i++ )
    {
        if( pfnProgress )
        {
            if( pfnProgress( (double)i / nFilesToGet, 
                             CPLSPrintf( "Downloading %s...",
                                         CPLGetFilename( papszOutputFiles[i] ) ),
                             NULL ) )
            {
                CPLError( CE_Failure, CPLE_UserInterrupt,
                          "Cancelled by user." );
                nrc = NOMADS_OK;
                break;
            }
        }

#ifdef NOMADS_ENABLE_ASYNC
        k = i - 1;
        j = k < nThreads ? k : k % nThreads;
        if( j == 0 && i != 1 )
        {
            for( t = 0; t < nThreads; t++ )
            {
                CPLJoinThread( pThreads[t] );
            }
            for( t = 0; t < nThreads; t++ )
            {
                if( pasData[t].nErr )
                {
                    rc = NOMADS_ERR;
                }
            }
        }
        if( rc == 0 )
        {
            pasData[j].pszUrl = papszDownloadUrls[i];
            pasData[j].pszFilename = papszOutputFiles[i];
            pThreads[j] =
                CPLCreateJoinableThread( NomadsFetchAsync, &pasData[j] );
            if( i == nFilesToGet - 1 && j != 0 )
            {
                rc = NOMADS_OK;
                for( t = 0; t < j; t++ )
                {
                    CPLJoinThread( pThreads[t] );
                }
                for( t = 0; t < j; t++ )
                {
                    if( pasData[t].nErr )
                    {
                        rc = NOMADS_ERR;
                    }
                }
            }
            if( rc == 0 )
                continue;
        }
#else /* NOMADS_ENABLE_ASYNC */
#ifdef NOMADS_USE_VSI_READ
        rc = NomadsFetchVsi( papszDownloadUrls[i], papszOutputFiles[i] );
#else /* NOMADS_USE_VSI_READ */
        rc = NomadsFetchHttp( papszDownloadUrls,[i] papszOutputFiles[i] );
#endif /* NOMADS_USE_VSI_READ */
#endif /* NOMADS_ENABLE_ASYNC */
        if( rc )
        {
            if( !bAlreadyWentBack )
            {
                CPLError( CE_Warning, CPLE_AppDefined,
                          "Failed to download forecast, " \
                          "stepping back one forecast run time step." );
                nFcstHour = NomadsFindForecastHour( ppszKey, now, 1 );
                bAlreadyWentBack = TRUE;
                bFirstFile = FALSE;
                i = 0;
                CPLUnlinkTree( pszTmpDir );
                CPLFree( (void*)panRunHours );
                CPLFree( (void*)pszTmpDir );
                CSLDestroy( papszDownloadUrls );
                CSLDestroy( papszOutputFiles );
                goto try_again;
            }
            else
            {
                CPLError( CE_Failure, CPLE_AppDefined,
                          "Could not open url for reading" );
                nrc = NOMADS_ERR;
                break;
            }
        }
        bFirstFile = FALSE;
    }
    if( !nrc )
    {
        int bZip = strstr( pszDstVsiPath, ".zip" ) ? TRUE : FALSE;
        papszFinalFiles =
            NomadsBuildOutputFileList( pszModelKey, nFcstHour, panRunHours,
                                       nFilesToGet, pszDstVsiPath,
                                       bZip );

        VSIStat( pszDstVsiPath, &sStat );
        if( VSI_ISDIR( sStat.st_mode ) || VSI_ISREG( sStat.st_mode ) )
            CPLUnlinkTree( pszDstVsiPath );
        if( !bZip )
            VSIMkdir( pszDstVsiPath, 0777 );
        rc = NomadsZipFiles( papszOutputFiles, papszFinalFiles );
    }
cleanup:
#ifdef NOMADS_ENABLE_ASYNC
    CPLFree( (void*)pasData );
    CPLFree( (void**)pThreads );
#endif /* NOMADS_ENABLE_ASYNC */
    NomadsUtcFree( now );
    NomadsUtcFree( end );
    NomadsUtcFree( tmp );
    NomadsUtcFree( fcst );
    CPLUnlinkTree( pszTmpDir );
    CPLFree( (void*)panRunHours );
    CPLFree( (void*)pszTmpDir );
    CSLDestroy( papszDownloadUrls );
    CSLDestroy( papszOutputFiles );
    if( !nrc && pfnProgress )
    {
        pfnProgress( 1.0, NULL, NULL );
    }
    return nrc;
}

