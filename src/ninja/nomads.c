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
** Find that last possible forecast time for a model.
*/
static int NomadsFindLatestForecastHour( const char **ppszKey, nomads_utc *now )
{

    char **papszHours;
    int nStart, nStop, nStride;
    int nFcstHour;
    int i;

    papszHours = CSLTokenizeString2( ppszKey[NOMADS_FCST_HOURS], ":", 0 );
    nStart = atoi( papszHours[0] );
    nStop = atoi( papszHours[1] );
    nStride = atoi( papszHours[2] );
    for( i = nStart; i <= nStop; i += nStride )
    {
        if( i > now->ts->tm_hour )
            break;
    }
    nFcstHour = i - nStride;
    CSLDestroy( papszHours );
    papszHours = NULL;
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

static int NomadsFetchVsi( const char *pszUrl, const char *pszFilename )
{
    int rc;
    const char *pszVsiUrl;
    VSILFILE *fin, *fout;
    vsi_l_offset nOffset, nBytesWritten, nVsiBlockSize;
    char *pabyBuffer;
    nVsiBlockSize = atoi( CPLGetConfigOption( "NOMADS_VSI_BLOCK_SIZE", "512" ) );

    if( !EQUALN( pszUrl, "/vsicurl/", 9 ) )
        pszVsiUrl = CPLSPrintf( "/vsicurl/%s", pszUrl );
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
        if( psResult )
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
    const char *pszUrl = NULL;
    const char *pszGribFile = NULL;
    const char *pszGribDir = NULL;
    const char *pszLevels = NULL;
    const char *pszVars = NULL;
    int nFcstHour = 0;
    int *panRunHours = NULL;
    int i = 0;
    int j = 0;
    int nStart = 0;
    int nStop = 0;
    int nStride = 0;
    char **papszHours = NULL;
    int nFilesToGet = 0;
    const char *pszOutFilename = NULL;
    int bAlreadyWentBack = FALSE;
    int bFirstFile = TRUE;
    char szMessage[512];
    int rc;
    void **pThreads;
    int nThreads;
    NomadsThreadData *pasData;
    nomads_utc *now, *end, *tmp, *fcst;
    CPLSetConfigOption( "GDAL_HTTP_TIMEOUT", "20" );
    nThreads = atoi( CPLGetConfigOption( "NOMADS_THREAD_COUNT", "4" ) );

    while( apszNomadsKeys[i][0] != NULL )
    {
        if( EQUAL( pszModelKey, apszNomadsKeys[i][0] ) )
        {
            ppszKey = apszNomadsKeys[i];
            CPLDebug( "NOMADS", "Found model key: %s",
                      ppszKey[NOMADS_NAME] );
            break;
        }
        i++;
    }
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

    /* Forecast hours, only one token */
    nFcstHour = NomadsFindLatestForecastHour( ppszKey, now );
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
        nFilesToGet = NomadsBuildForecastRunHours( ppszKey, now, end, nHours,
                                                   &panRunHours );
    }
    /* Open our output file */
    if( pfnProgress )
    {
        pfnProgress( 0.0, "Starting download...", NULL );
    }
    szMessage[0] = '\0';
#define NOMADS_ENABLE_ASYNC
#ifdef NOMADS_ENABLE_ASYNC
    pThreads = CPLMalloc( sizeof( void * ) * nFilesToGet );
    pasData = CPLMalloc( sizeof( NomadsThreadData ) * nFilesToGet );
#else
    pThreads = NULL;
    pasData = NULL;
#endif
    for( i = 0; i < nFilesToGet; i++ )
    {
        pszGribFile = CPLStrdup( CPLSPrintf( ppszKey[NOMADS_FILE_NAME_FRMT],
                                             nFcstHour, panRunHours[i] ) );
        CPLDebug( "WINDNINJA", "NOMADS generated grib file name: %s",
                  pszGribFile );
        if( pfnProgress )
        {
            sprintf( szMessage, "Downloading %s...", pszGribFile );
            if( pfnProgress( (double)i / nFilesToGet, szMessage, NULL ) )
            {
                CPLError( CE_Failure, CPLE_UserInterrupt,
                          "Cancelled by user." );
                CPLFree( (void*)pszGribFile );
                NomadsUtcFree( now );
                NomadsUtcFree( end );
                NomadsUtcFree( tmp );
                NomadsUtcFree( fcst );
                CPLFree( (void*)pThreads );
                CPLFree( (void*)pasData );
                return NOMADS_OK;
            }
        }

        NomadsUtcStrfTime( fcst, ppszKey[NOMADS_DIR_DATE_FRMT] );
        /* Special case for gfs */
        if( EQUAL( pszModelKey, "gfs" ) )
            pszGribDir = CPLStrdup( CPLSPrintf( ppszKey[NOMADS_DIR_FRMT],
                                                fcst->s, nFcstHour ) );
        else
            pszGribDir = CPLStrdup( CPLSPrintf( ppszKey[NOMADS_DIR_FRMT], fcst->s ) );

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
        CPLDebug( "WINDNINJA", "NOMADS generated url: %s",
                  pszUrl );
        pszOutFilename = CPLSPrintf( "%s/%s", pszDstVsiPath, pszGribFile );
        if( strstr( pszDstVsiPath, ".zip" ) )
            pszOutFilename = CPLSPrintf( "/vsizip/%s", pszOutFilename );
#ifdef NOMADS_ENABLE_ASYNC
        pasData[i].pszUrl = CPLStrdup( pszUrl );
        pasData[i].pszFilename = CPLStrdup( pszOutFilename );
        pThreads[i] = CPLCreateJoinableThread( NomadsFetchAsync, &pasData[i] );
        CPLFree( (void*)pszGribFile );
        CPLFree( (void*)pszGribDir );
        continue;
#endif

#ifdef NOMADS_USE_VSI_READ
        rc = NomadsFetchVsi( pszUrl, pszOutFilename );
#else
        rc = NomadsFetchHttp( pszUrl, pszOutFilename );
#endif /* NOMADS_USE_VSI_READ */
        if( rc )
        {
            if( !bAlreadyWentBack && bFirstFile )
            {
                CPLError( CE_Warning, CPLE_AppDefined,
                          "Failed to download forecast, " \
                          "stepping back one forecast run time step." );
                papszHours = CSLTokenizeString2( ppszKey[NOMADS_FCST_HOURS],
                                                 ":", 0 );
                nStart = atoi( papszHours[0] );
                nStop = atoi( papszHours[1] );
                nStride = atoi( papszHours[2] );
                if( nFcstHour - nStride < nStart )
                    nFcstHour = nStop;
                else
                    nFcstHour -= nStride;
                bAlreadyWentBack = TRUE;
                bFirstFile = FALSE;
                CSLDestroy( papszHours );
                i--;
                CPLFree( (void*)pszGribFile );
                CPLFree( (void*)pszGribDir );
                CPLFree( (void*)panRunHours );
                goto try_again;
            }
            else
            {
                CPLError( CE_Failure, CPLE_AppDefined,
                          "Could not open url for reading" );
                CPLFree( (void*)pszGribFile );
                CPLFree( (void*)pszGribDir );
                CPLFree( (void*)panRunHours );
                NomadsUtcFree( now );
                NomadsUtcFree( end );
                NomadsUtcFree( tmp );
                NomadsUtcFree( fcst );
                return NOMADS_ERR;
            }
        }
        bFirstFile = FALSE;
        CPLFree( (void*)pszGribFile );
        CPLFree( (void*)pszGribDir );
    }
#ifdef NOMADS_ENABLE_ASYNC
    for( i = 0; i < nFilesToGet; i++ )
    {
        CPLJoinThread( pThreads[i] );
        CPLFree( (void*)pasData[i].pszUrl );
        CPLFree( (void*)pasData[i].pszFilename );
    }
    CPLFree( (void*)pasData );
    CPLFree( pThreads );
#endif /* NOMADS_ENABLE_ASYNC */
    NomadsUtcFree( now );
    NomadsUtcFree( end );
    NomadsUtcFree( tmp );
    NomadsUtcFree( fcst );
    CPLFree( (void*)panRunHours );
    if( pfnProgress )
    {
        pfnProgress( 1.0, NULL, NULL );
    }
    return NOMADS_OK;
}

