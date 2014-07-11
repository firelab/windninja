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
nomads_utc * NomadsSetForecastTime( const char **ppszKey, nomads_utc *ref,
                                    int n )
{
    char **papszHours;
    int nStart, nStop, nStride;
    int i;
    nomads_utc *fcst;
    int bFound;

    NomadsUtcCreate( &fcst );
    NomadsUtcCopy( fcst, ref );

    papszHours = CSLTokenizeString2( ppszKey[NOMADS_FCST_HOURS], ":", 0 );
    nStart = atoi( papszHours[0] );
    nStop = atoi( papszHours[1] );
    nStride = atoi( papszHours[2] );
    CSLDestroy( papszHours );
    papszHours = NULL;
    if( nStart == nStop )
    {
        nStride = 24;
    }
    bFound = FALSE;
    while( !bFound )
    {
        for( i = nStart; i <= nStop; i += nStride )
        {
            if( fcst->ts->tm_hour == i )
            {
                bFound = TRUE;
                break;
            }
        }
        if( !bFound )
            NomadsUtcAddHours( fcst, -1 );
    }
    NomadsUtcAddHours( fcst, -abs( n ) * nStride );
    return fcst;
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
            CPLSPrintf( "%s%s?%s&%s%s&file=%s&dir=/%s",
#ifdef NOMADS_USE_IP
                        NOMADS_URL_CGI_IP,
#else
                        NOMADS_URL_CGI_HOST,
#endif
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
    const char *pszTmpDir;
    int nFcstTries;
    int nMaxFcstRewind;
    int nrc;
#ifdef NOMADS_ENABLE_ASYNC
    void **pThreads;
    int nThreads;
    const char *pszThreadCount;
    int trc;
#endif

    NomadsThreadData *pasData;
    nomads_utc *now, *end, *tmp, *fcst;
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
    NomadsUtcNow( now );
    NomadsUtcCopy( end, now );
    NomadsUtcAddHours( end, nHours );
    NomadsUtcCopy( tmp, now );

    nMaxFcstRewind = atoi( CPLGetConfigOption( "NOMADS_MAX_FCST_REWIND", "2" ) );
    if( nMaxFcstRewind < 1 || nMaxFcstRewind > 24 )
    {
        nMaxFcstRewind = 2;
    }
    /* Go back at least 3 for rap, as it may not get updated all the time. */
    if( EQUAL( pszModelKey, "rap" ) )
    {
        nMaxFcstRewind = nMaxFcstRewind > 3 ? nMaxFcstRewind : 3;
    }

#ifdef NOMADS_ENABLE_ASYNC
    pszThreadCount = CPLGetConfigOption( "NOMADS_THREAD_COUNT", "4" );
    nThreads = atoi( pszThreadCount );
    if( nThreads < 1 || nThreads > 96 )
    {
        nThreads = 4;
    }
    pThreads = CPLMalloc( sizeof( void * ) * nThreads );
    pasData = CPLMalloc( sizeof( NomadsThreadData ) * nThreads );
#endif

    fcst = NULL;
    nFcstTries = 0;
    while( nFcstTries < nMaxFcstRewind )
    {
        fcst = NomadsSetForecastTime( ppszKey, now, nFcstTries );
        nFcstHour = fcst->ts->tm_hour;
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
        if( !papszDownloadUrls )
        {
            CPLError( CE_Failure, CPLE_AppDefined,
                      "Could not generate list of URLs to download, invalid data" );
            nFcstTries++;
            NomadsUtcFree( fcst );
            fcst = NULL;
            nrc = NOMADS_ERR;
            continue;
        }
        pszTmpDir = CPLStrdup( CPLGenerateTempFilename( NULL ) );
        CPLDebug( "WINDNINJA", "Creating Temp directory: %s", pszTmpDir );
        VSIMkdir( pszTmpDir, 0777 );
        papszOutputFiles =
            NomadsBuildOutputFileList( pszModelKey, nFcstHour, panRunHours,
                                       nFilesToGet, pszTmpDir, FALSE );
        if( !papszOutputFiles )
        {
            CPLError( CE_Failure, CPLE_AppDefined,
                      "Could not generate list of URLs to download, invalid data" );
            nFcstTries++;
            NomadsUtcFree( fcst );
            fcst = NULL;
            nrc = NOMADS_ERR;
            continue;
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
            CPLUnlinkTree( pszTmpDir );
            CPLFree( (void*)panRunHours );
            panRunHours = NULL;
            CPLFree( (void*)pszTmpDir );
            pszTmpDir = NULL;
            CSLDestroy( papszDownloadUrls );
            papszDownloadUrls = NULL;
            CSLDestroy( papszOutputFiles );
            papszOutputFiles = NULL;
            NomadsUtcFree( fcst );
            fcst = NULL;

            nFcstTries++;
            nrc = rc;
            continue;
        }
        /* Get the rest */
        i = 1;
        while( i < nFilesToGet )
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
                    goto cleanup;
                }
            }
#ifdef NOMADS_ENABLE_ASYNC
            k = i > nFilesToGet - nThreads ? (nFilesToGet - 1) % nThreads : nThreads;
            for( t = 0; t < k; t++ )
            {
                pasData[t].pszUrl = papszDownloadUrls[i];
                pasData[t].pszFilename = papszOutputFiles[i];
                pThreads[t] =
                    CPLCreateJoinableThread( NomadsFetchAsync, &pasData[t] );
                i++;
            }
            for( t = 0; t < k; t++ )
            {
                CPLJoinThread( pThreads[t] );
            }
            trc = NOMADS_OK;
            for( t = 0; t < k; t++ )
            {
                if( pasData[t].nErr )
                {
                        CPLError( CE_Warning, CPLE_AppDefined,
                                  "Threaded download failed, attempting " \
                                  "serial download for %s",
                                  CPLGetFilename( pasData[t].pszFilename ) );
                        /* Try again, serially though */
                        if( CPLCheckForFile( (char *)pasData[t].pszFilename, NULL ) );
                        {
                            VSIUnlink( pasData[t].pszFilename );
                        }
#ifdef NOMADS_USE_VSI_READ
                        rc = NomadsFetchVsi( pasData[t].pszUrl,
                                             pasData[t].pszFilename );
#else /* NOMADS_USE_VSI_READ */
                        rc = NomadsFetchHttp( pasData[t].pszUrl,
                                              pasData[t].pszFilename );
#endif /* NOMADS_USE_VSI_READ */
                        if( rc )
                            trc = rc;
                }
            }
            nrc = rc = trc;
#else /* NOMADS_ENABLE_ASYNC */
#ifdef NOMADS_USE_VSI_READ
            rc = NomadsFetchVsi( papszDownloadUrls[i], papszOutputFiles[i] );
#else /* NOMADS_USE_VSI_READ */
            rc = NomadsFetchHttp( papszDownloadUrls[i], papszOutputFiles[i] );
#endif /* NOMADS_USE_VSI_READ */
            i++;
#endif /* NOMADS_ENABLE_ASYNC */
            nrc = rc;
            if( rc )
            {
                CPLError( CE_Warning, CPLE_AppDefined,
                          "Failed to download forecast, " \
                          "stepping back one forecast run time step." );
                nFcstTries++;
                i = 0;
                CPLUnlinkTree( pszTmpDir );
                CPLFree( (void*)panRunHours );
                panRunHours = NULL;
                CPLFree( (void*)pszTmpDir );
                pszTmpDir = NULL;
                CSLDestroy( papszDownloadUrls );
                papszDownloadUrls = NULL;
                CSLDestroy( papszOutputFiles );
                papszOutputFiles = NULL;
                NomadsUtcFree( fcst );
                fcst = NULL;
                nrc = rc;
                break;
            }
        }
        if( !nrc )
            break;
    }
    if( !nrc )
    {
        int bZip = strstr( pszDstVsiPath, ".zip" ) ? TRUE : FALSE;
        papszFinalFiles =
            NomadsBuildOutputFileList( pszModelKey, nFcstHour, panRunHours,
                                       nFilesToGet, pszDstVsiPath,
                                       bZip );

        if( CPLCheckForFile( (char*)pszDstVsiPath, NULL ) )
            CPLUnlinkTree( pszDstVsiPath );
        if( !bZip )
            VSIMkdir( pszDstVsiPath, 0777 );
        nrc = NomadsZipFiles( papszOutputFiles, papszFinalFiles );
        CSLDestroy( papszFinalFiles );
        if( nrc )
        {
            CPLError( CE_Failure, CPLE_AppDefined,
                      "Could not copy files into path, unknown i/o failure" );
            CPLUnlinkTree( pszDstVsiPath );
        }
    }
cleanup:
#ifdef NOMADS_ENABLE_ASYNC
    CPLFree( (void*)pasData );
    CPLFree( (void**)pThreads );
#endif /* NOMADS_ENABLE_ASYNC */

    if( pszTmpDir )
        CPLUnlinkTree( pszTmpDir );
    CPLFree( (void*)panRunHours );
    CPLFree( (void*)pszTmpDir );
    CSLDestroy( papszDownloadUrls );
    CSLDestroy( papszOutputFiles );
    NomadsUtcFree( fcst );

    NomadsUtcFree( now );
    NomadsUtcFree( end );
    NomadsUtcFree( tmp );
    if( !nrc && pfnProgress )
    {
        pfnProgress( 1.0, NULL, NULL );
    }
    return nrc;
}

