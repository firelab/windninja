/******************************************************************************
 *
 * Project:  WindNinja
 * Purpose:  Nomads C client
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

/*
** Results must be free'd.  We can't use CPLSPrintf on large lists of arguments
** because we overrun the ring buffer.  A straight-forward strcat or other
** method wasn't obvious, but brute force method is.  The previous
** implementation had an overlapping buffer, was wrong, and valgrind complained
** correctly.
*/
static char * NomadsBuildArgList( const char *pszVars, const char *pszPrefix )
{
    int i, j, k, n, m;
    char *pszList;
    char **papszArgs = CSLTokenizeString2( pszVars, ",", 0 );
    n = CSLCount(papszArgs);
    if (!n) {
      return NULL;
    }
    /* var_##=on& */
    /* variable names -n-1 for commas */
    m = strlen(pszVars) - (n - 1);
    m += (strlen(pszPrefix) + strlen("_")) * n;
    m += strlen("&") * (n - 1);
    m += (strlen("=on") * n);
    m += 1; /* terminate */
    pszList = CPLMalloc(sizeof(char) * m);
    memset(pszList, 0, m);
    k = 0;
    for (i = 0; k < m && i < n; i++) {
      // copy the prefix, add a '_'
      for (j = 0; j < strlen(pszPrefix); j++) {
        pszList[k++] = pszPrefix[j];
      }
      pszList[k++] = '_';
      for (j = 0; j < strlen(papszArgs[i]); j++) {
        pszList[k++] = papszArgs[i][j];
      }
      pszList[k++] = '=';
      pszList[k++] = 'o';
      pszList[k++] = 'n';
      if (i < n - 1) {
        pszList[k++] = '&';
      }
    }
    assert(k == m - 1);
    // terminate, k should be waiting at the proper position
    pszList[k] = '\0';
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
                                        const nomads_utc *ref,
                                        const nomads_utc *end,
                                        int nHours, int nFcstStride, 
                                        int **panRunHours )
{
    char **papszHours, **papszRunHours;
    int nStart, nStop, nStride;
    int nTokenCount;
    int i, j, k, l;
    int nSize;

    nomads_utc *tmp;
    NomadsUtcCreate( &tmp );
    NomadsUtcCopy( tmp, ref );

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
        l = 0;
        for( j = nStart; j <= nStop; j += nStride )
        {
            if( l++ % nFcstStride == 0 )
            {
                (*panRunHours)[k++] = j;
            }
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
    char *pszLevels;
    char *pszVars;
    const char *pszUrl;
    char **papszFileList = NULL;

    int i;

    ppszKey = NomadsFindModel( pszKey );
    if( !ppszKey )
    {
        return NULL;
    }
    pszVars = NomadsBuildArgList( ppszKey[NOMADS_VARIABLES], "var" );
    pszLevels = NomadsBuildArgList( ppszKey[NOMADS_LEVELS], "lev" );
    for( i = 0; i < nHours; i++ )
    {
        pszGribFile = CPLSPrintf( ppszKey[NOMADS_FILE_NAME_FRMT], nFcstHour,
                                  panRunHours[i] );
        CPLDebug( "WINDNINJA", "NOMADS generated grib file name: %s",
                  pszGribFile );
        NomadsUtcStrfTime( fcst, ppszKey[NOMADS_DIR_DATE_FRMT] );
        /* Special case for gfs */
        if( EQUALN( pszKey, "gfs_global", 10 ) )
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
#else /* NOMADS_USE_IP */
                        NOMADS_URL_CGI_HOST,
#endif /* NOMADS_USE_IP */
                        ppszKey[NOMADS_FILTER_BIN], pszVars, pszLevels,
                        NOMADS_SUBREGION, pszGribFile, pszGribDir );
        pszUrl = CPLSPrintf( pszUrl, padfBbox[0], padfBbox[1],
                             padfBbox[2], padfBbox[3] );
        CPLDebug( "WINDNINJA", "NOMADS generated url: %s", pszUrl );
        papszFileList = CSLAddString( papszFileList, pszUrl );
    }
    CPLFree( (void*)pszVars );
    CPLFree( (void*)pszLevels );
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
    else
    {
        pszVsiUrl = pszUrl;
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
    if( !psResult || psResult->nStatus != 0 || psResult->nDataLen < 1 ||
        strstr( (char*)psResult->pabyData, "HTTP error code : 404" ) ||
        strstr( (char*)psResult->pabyData, "data file is not present" ) )
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
#else /* NOMADS_USE_VSI_READ */
    rc = NomadsFetchHttp( psData->pszUrl, psData->pszFilename );
#endif /* NOMADS_USE_VSI_READ */
    psData->nErr = rc;
}

static double NomadsGetMinSize( const char **ppszModel )
{
    double x = 0;
    double dfRes = 0;
    char **papszTokens = CSLTokenizeString2( ppszModel[NOMADS_GRID_RES], " ", 0 );
    if( papszTokens == NULL || CSLCount( papszTokens ) < 2 )
    {
        CSLDestroy( papszTokens );
        assert( 0 );
        return 0;
    }
    x = atof( papszTokens[0] );
    if( EQUAL( papszTokens[1], "deg" ) )
    {
        dfRes = x;
    }
    else if( EQUAL( papszTokens[1], "km" ) )
    {
        /* https://en.wikipedia.org/wiki/Decimal_degrees */
        dfRes = x / 111.32;
    }
    else
    {
        assert( 0 );
    }
    CSLDestroy( papszTokens );
    return dfRes;
}

/*
** Fetch a nomads forecast from the NWS servers.  The forecasts consist of grib
** files, one forecast for each forecast hour.  The time starts and the nearest
** forecast hour *before* the reference time passed.  If no reference time is
** passed, or it is not valid, now() is used.
**
** The forecasts are downloaded into a temporary directory.  If the entire
** download succeeds, then the files are copied into the path specified.  If
** the path specified is a zip file (ends in *.zip), then the forecast files
** are zipped.
**
** Available compile time configuration options:
**        NOMADS_USE_IP: Use the ip address instead of the hostname.  It
**                       possibly goes around dns lookup, but it's doubtfull.
**        NOMADS_ENABLE_ASYNC: Allow for asynchronous connections to the
**                             server.
**        NOMADS_RTMA : Enable the RTMA forecasts.
**        NOMADS_EXPER_FORECASTS: Compile in forecasts that may not work with
**                                the current configuration, essentially
**                                unsupported (ie NARRE, RTMA).
**        NOMADS_USE_VSI_READ: Use the VSI*L api for downloading.  This will
**                             allow definition of chunk sizes for download,
**                             although it is probably unnecessary.  See
**                             NOMADS_VSI_BLOCK_SIZE below.  If not enabled, a
**                             single fetch is made for each file, which is
**                             faster.
** Available runtime configuration options:
**        NOMADS_THREAD_COUNT: Number of threads to use for downloads if
**                             NOMADS_ENABLE_ASYNC is set to ON during
**                             compilation.  Default is 4.
**        NOMADS_VSI_BLOCK_SIZE: Number of bytes to request at a time when
**                               downloading files if NOMADS_USE_VSI_READ is
**                               set to ON during compilation. Default is 512.
**        NOMADS_MAX_FCST_REWIND: Number of forecast run time steps to go back
**                                to attempt to get a full time frame.
**        GDAL_HTTP_TIMEOUT: Timeout for HTTP requests in seconds.  We should
**                           be able to set this reasonably low.
**
** Information on available models can be found at:
**
**     http://nomads.ncep.noaa.gov
**
** Model availability is typically consistent, however there are outages
** occasionally.  A list serve with outage alerts can be subscribed to here:
**
**     https://lstsrv.ncep.noaa.gov/mailman/listinfo/ncep.list.nomads-ftpprd
**
** \param pszModelKey The name key of the model to use, ie "nam_conus".  For a
**                    listing of models, see nomads.ncep.gov or /see nomads.h
**
** \param pszRefTime The reference time to begin searching for forecasts from.
**                   The search starts at refrence time, then goes back until
**                   it hits a valid forecast time.  If the download cannot be
**                   complete, it will step back NOMADS_MAX_FCST_REWIND
**                   foreacst times to attempt to get a full forecast.
**
** \param nHours The extent of hours to download forecasts from the reference
**               time.  If we step back, we will still grab all forecasts up
**               until nHours from the reference time, not the forecast time.
**
** \param nStride The number of forecasts to skip in time steps.  For example,
**                if 12 hours of gfs is requested (normally 5 files/steps (0,
**                3, 6, 9, 12) with a stride of 2, you'd get 0, 6, 12 forecast
**                hours.
**
** \param padfBbox The bounding box of the request in WGS84 decimal degrees.
**                 Order is xmin, xmax, ymax, ymin.
**
** \param pszDstVsiPath The location to write the files.  If the location
**                      has an extension of ".zip", then the output files are
**                      written as a zip archive, otherwise to a path.
**
** \param papszOptions List of key=value options, unused.
**
** \param pfnProgress Optional progress function.
**
** \return NOMADS_OK(0) on success, NOMADS_ERR(1) otherwise.
*/
int NomadsFetch( const char *pszModelKey, const char *pszRefTime,
                 int nHours, int nStride, double *padfBbox,
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
    const char *pszConfigOpt;
    int nFcstTries;
    int nMaxFcstRewind;
    int nrc;
    int bZip;
    void **pThreads;
    int nThreads;
    const char *pszThreadCount;
    double dfMinRes;
    double adfBufBbox[4];
    double dfXMax, dfXMin, dfYMax, dfYMin;
    int nBufTries;

    NomadsThreadData *pasData;
    nomads_utc *ref, *end, *fcst;
    nrc = NOMADS_OK;

    ppszKey = NomadsFindModel( pszModelKey );
    if( ppszKey == NULL )
    {
        CPLError( CE_Failure, CPLE_AppDefined,
                  "Could not find model key in nomads data" );
        return NOMADS_ERR;
    }

    assert( padfBbox[NOMADS_XMIN] < padfBbox[NOMADS_XMAX] &&
            padfBbox[NOMADS_YMIN] < padfBbox[NOMADS_YMAX] );

    /*
    ** Check our bounding box size.  If we request a forecast smaller than one
    ** cell size, it returns everything.  Note that the meta fetcher buffers
    ** our DEM, so this should never be a problem.
    **
    ** Order is xmin, xmax, ymax, ymin.
    */
    memcpy( adfBufBbox, padfBbox, sizeof( double ) * 4 );
    dfMinRes = NomadsGetMinSize( ppszKey );
    nBufTries = 0;
    while( fabs( adfBufBbox[NOMADS_XMAX] - adfBufBbox[NOMADS_XMIN] ) < dfMinRes &&
           nBufTries <= 10 )
    {
        adfBufBbox[NOMADS_XMAX] += dfMinRes / 2;
        adfBufBbox[NOMADS_XMIN] -= dfMinRes / 2;
        nBufTries++;
    }

    nBufTries = 0;
    while( fabs( adfBufBbox[NOMADS_YMAX] - adfBufBbox[NOMADS_YMIN] ) < dfMinRes &&
           nBufTries <= 10 )
    {
        adfBufBbox[NOMADS_YMAX] += dfMinRes / 2;
        adfBufBbox[NOMADS_YMIN] -= dfMinRes / 2;
        nBufTries++;
    }

    CPLDebug( "NOMADS", "Fetching data for bounding box: %lf, %lf, %lf, %lf",
              adfBufBbox[0], adfBufBbox[1],adfBufBbox[2],adfBufBbox[3] );

    NomadsUtcCreate( &ref );
    NomadsUtcCreate( &end );
    rc = NOMADS_OK;
    if( pszRefTime )
    {
        rc = NomadsUtcFromIsoFrmt( ref, pszRefTime );
    }
    if( rc != NOMADS_OK || pszRefTime == NULL )
    {
        NomadsUtcNow( ref );
    }
    NomadsUtcCopy( end, ref );
    NomadsUtcAddHours( end, nHours );

    /* Disable unneeded reading of entire directories, good speedup */
    CPLSetConfigOption( "GDAL_DISABLE_READDIR_ON_OPEN", "TRUE" );

    pszConfigOpt = CPLGetConfigOption( "NOMADS_HTTP_TIMEOUT", "20" );
    if( pszConfigOpt != NULL )
    {
        CPLSetConfigOption( "GDAL_HTTP_TIMEOUT", pszConfigOpt );
    }

    nMaxFcstRewind = atoi( CPLGetConfigOption( "NOMADS_MAX_FCST_REWIND", "4" ) );
    if( nMaxFcstRewind < 1 || nMaxFcstRewind > 24 )
    {
        nMaxFcstRewind = 2;
    }
    /*
    ** Go back at least 3 for rap, hrrr, and rtma, as it may not get updated all
    ** the time.
    */
    if( EQUALN( pszModelKey, "rap", 3 ) || EQUALN( pszModelKey, "hrrr", 4 ) ||
        EQUALN( pszModelKey, "rtma", 4 ) )
    {
        nMaxFcstRewind = nMaxFcstRewind > 5 ? nMaxFcstRewind : 5;
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
#else /* NOMADS_ENABLE_ASYNC */
    /* Unused variables, set to null to so free is no-op */
    nThreads = 1;
    pThreads = NULL;
    pasData = NULL;
#endif /* NOMADS_ENABLE_ASYNC */

    fcst = NULL;
    nFcstTries = 0;
    while( nFcstTries < nMaxFcstRewind )
    {
        nrc = NOMADS_OK;
        fcst = NomadsSetForecastTime( ppszKey, ref, nFcstTries );
        nFcstHour = fcst->ts->tm_hour;
        CPLDebug( "WINDNINJA", "Generated forecast time in utc: %s",
                  NomadsUtcStrfTime( fcst, "%Y%m%dT%HZ" ) );

        if( EQUALN( pszModelKey, "rtma", 4 ) )
        {
            panRunHours = (int*)CPLMalloc( sizeof( int ) );
            nFilesToGet = 1;
        }
        else
        {
            nFilesToGet =
                NomadsBuildForecastRunHours( ppszKey, fcst, end, nHours,
                                             nStride, &panRunHours );
        }

        papszDownloadUrls =
            NomadsBuildForecastFileList( pszModelKey, nFcstHour, panRunHours,
                                         nFilesToGet, fcst, adfBufBbox );
        if( papszDownloadUrls == NULL )
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
        if( papszOutputFiles == NULL )
        {
            CPLError( CE_Failure, CPLE_AppDefined,
                      "Could not generate list of URLs to download, invalid data" );
            nFcstTries++;
            CSLDestroy( papszDownloadUrls );
            NomadsUtcFree( fcst );
            fcst = NULL;
            nrc = NOMADS_ERR;
            continue;
        }

        CPLAssert( CSLCount( papszDownloadUrls ) == nFilesToGet );
        CPLAssert( CSLCount( papszOutputFiles ) == nFilesToGet );

        CPLDebug( "NOMADS", "Starting download..." );
        if( pfnProgress )
        {
            pfnProgress( 0.0, "Starting download...", NULL );
        }

        /* Download one file and start over if it's not there. */
#ifdef NOMADS_USE_VSI_READ
        nrc = NomadsFetchVsi( papszDownloadUrls[0], papszOutputFiles[0] );
#else /* NOMADS_USE_VSI_READ */
        nrc = NomadsFetchHttp( papszDownloadUrls[0], papszOutputFiles[0] );
#endif /* NOMADS_USE_VSI_READ */
        if( nrc != NOMADS_OK )
        {
            CPLError( CE_Warning, CPLE_AppDefined,
                      "Failed to download forecast, " \
                      "stepping back one forecast run time step." );
            nFcstTries++;
            CPLSleep( 1 );
            /*
            ** Don't explicitly break here.  We'll skip the while loop because
            ** nrc != NOMADS_OK, and we can clean up memory and shift times in
            ** one spot to avoid duplicate code.
            */
        }
        /* Get the rest */
        i = 1;
        while( i < nFilesToGet && nrc == NOMADS_OK )
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
                    nrc = NOMADS_ERR;
                    nFcstTries = nMaxFcstRewind;
                    break;
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
            for( t = 0; t < k; t++ )
            {
                if( pasData[t].nErr )
                {
                    CPLError( CE_Warning, CPLE_AppDefined,
                              "Threaded download failed, attempting " \
                              "serial download for %s",
                              CPLGetFilename( pasData[t].pszFilename ) );
                    /* Try again, serially though */
                    if( CPLCheckForFile( (char *)pasData[t].pszFilename, NULL ) )
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
                    if( rc != NOMADS_OK )
                    {
                        nrc = rc;
                    }
                }
            }
#else /* NOMADS_ENABLE_ASYNC */
#ifdef NOMADS_USE_VSI_READ
            nrc = NomadsFetchVsi( papszDownloadUrls[i], papszOutputFiles[i] );
#else /* NOMADS_USE_VSI_READ */
            nrc = NomadsFetchHttp( papszDownloadUrls[i], papszOutputFiles[i] );
#endif /* NOMADS_USE_VSI_READ */
            i++;
#endif /* NOMADS_ENABLE_ASYNC */
            if( nrc != NOMADS_OK )
            {
                CPLError( CE_Warning, CPLE_AppDefined,
                          "Failed to download forecast, " \
                          "stepping back one forecast run time step." );
                nFcstTries++;
                CPLSleep( 1 );
                break;
            }
        }
        /*
        ** XXX Cleanup XXX
        ** After each loop we can get rid of the urls, but we can only get rid
        ** of the others if they are to be reallocated in the next loop.  Any
        ** cleanup in the else nrc == NOMADS_OK clause should be cleaned up in 
        ** the nrc == NOMADS_OK outside the loop.  Those are held so we can
        ** process the output files and zip them into an archive.
        */
        CSLDestroy( papszDownloadUrls );
        NomadsUtcFree( fcst );
        if( nrc == NOMADS_OK )
        {
            break;
        }
        else
        {
            CPLFree( (void*)panRunHours );
            CSLDestroy( papszOutputFiles );
            CPLUnlinkTree( pszTmpDir );
            CPLFree( (void*)pszTmpDir );
        }
    }
    if( nrc == NOMADS_OK )
    {
        bZip = EQUAL( CPLGetExtension( pszDstVsiPath ), "zip" ) ? TRUE : FALSE;
        papszFinalFiles =
            NomadsBuildOutputFileList( pszModelKey, nFcstHour, panRunHours,
                                       nFilesToGet, pszDstVsiPath,
                                       bZip );
        CPLFree( (void*)panRunHours );
        if( CPLCheckForFile( (char*)pszDstVsiPath, NULL ) )
        {
            CPLUnlinkTree( pszDstVsiPath );
        }
        if( !bZip )
        {
            VSIMkdir( pszDstVsiPath, 0777 );
        }
        nrc = NomadsZipFiles( papszOutputFiles, papszFinalFiles );
        CSLDestroy( papszOutputFiles );
        CSLDestroy( papszFinalFiles );
        if( nrc != NOMADS_OK )
        {
            CPLError( CE_Failure, CPLE_AppDefined,
                      "Could not copy files into path, unknown i/o failure" );
            CPLUnlinkTree( pszDstVsiPath );
        }
        CPLUnlinkTree( pszTmpDir );
        CPLFree( (void*)pszTmpDir );
    }
    CPLFree( (void*)pasData );
    CPLFree( (void**)pThreads );

    NomadsUtcFree( ref );
    NomadsUtcFree( end );
    CPLSetConfigOption( "GDAL_HTTP_TIMEOUT", NULL );
    CPLSetConfigOption( "GDAL_DISABLE_READDIR_ON_OPEN", NULL );
    if( nrc == NOMADS_OK && pfnProgress )
    {
        pfnProgress( 1.0, NULL, NULL );
    }
    return nrc;
}

/*
** Almalgamate the data for a model into a readable represetnation including
** source, model name, sub model if any, domain, resolution.  It should be
** free'd by the caller using NomadsFree();
*/
char * NomadsFormName( const char *pszKey, char pszSpacer )
{
    const char **ppszKey = NomadsFindModel( pszKey );
    const char *s;
    char *t, *p;
    int n;
    if( ppszKey == NULL )
    {
        return NULL;
    }
    /* Count the levels to check on 3D */
    n = 0;
    s = ppszKey[NOMADS_LEVELS];
    while( *s != '\0' )
    {
        if( *s++ == ',' )
        {
            n++;
        }
    }
    if( n > 3 )
    {
        s = CPLSPrintf( "NOMADS %s 3D %s", ppszKey[NOMADS_HUMAN_READABLE],
                        ppszKey[NOMADS_GRID_RES] );
    }
    else
    {
        s = CPLSPrintf( "NOMADS %s %s", ppszKey[NOMADS_HUMAN_READABLE],
                        ppszKey[NOMADS_GRID_RES] );
    }
    t = CPLStrdup( s );
    if( pszSpacer != ' ' )
    {
        p = strchr( t, ' ');
        while( p )
        {
            *p = pszSpacer;
            p = strchr( t, ' ' );
        }
    }
    p = t;
    while( *p != '\0' )
    {
        *p = toupper( *p );
        p++;
    }
    return t;
}

void NomadsFree( void *p )
{
    CPLFree( p );
}

/*
** NomadsAutoCreateWarpedVRT is a copy of GDALAutoCreateWarpedVRT() that allows
** for band subsetting.
*/
GDALDatasetH NomadsAutoCreateWarpedVRT(GDALDatasetH hSrcDS,
                          const char *pszSrcWKT,
                          const char *pszDstWKT,
                          GDALResampleAlg eResampleAlg,
                          double dfMaxError,
                          const GDALWarpOptions *psOptionsIn) {

    int i = 0;
    GDALWarpOptions *psWO = NULL;
    double adfDstGeoTransform[6];
    int nDstPixels = 0;
    int nDstLines = 0;

    adfDstGeoTransform[0] = 0.0;
    adfDstGeoTransform[1] = 0.0;
    adfDstGeoTransform[2] = 0.0;
    adfDstGeoTransform[3] = 0.0;
    adfDstGeoTransform[4] = 0.0;
    adfDstGeoTransform[5] = 0.0;

    CPLDebug("NOMADS", "Using internal AutoCreateWarpedVRT");
    VALIDATE_POINTER1( hSrcDS, "GDALAutoCreateWarpedVRT", NULL );

    if(psOptionsIn == NULL) {
        return GDALAutoCreateWarpedVRT(hSrcDS, pszSrcWKT, pszDstWKT, eResampleAlg,
                    dfMaxError, psOptionsIn);
    }

    if( psOptionsIn != NULL ) {
        psWO = GDALCloneWarpOptions( psOptionsIn );
    }
    else {
        psWO = GDALCreateWarpOptions();
    }

    psWO->eResampleAlg = eResampleAlg;

    psWO->hSrcDS = hSrcDS;

    /*
    ** This is where we diffentiate from GDALAutoCreateWarpedVRT().  We allow
    ** for band mapping, while the original doesn't.  We also use older
    ** semantics here, as some functions were introduced around 2.3.x.
    */
    if(psWO->nBandCount == 0 || psWO->panSrcBands == NULL || psWO->panDstBands == NULL) {
        psWO->nBandCount = GDALGetRasterCount(hSrcDS);
        psWO->panSrcBands = CPLMalloc(psWO->nBandCount * sizeof(int));
        if (psWO->panSrcBands == NULL) {
            return NULL;
        }
        psWO->panDstBands = CPLMalloc(psWO->nBandCount * sizeof(int));
        if (psWO->panDstBands == NULL) {
            return NULL;
        }
        for( i = 0; i < GDALGetRasterCount( hSrcDS ); i++ ){
            psWO->panSrcBands[i] = i+1;
            psWO->panDstBands[i] = i+1;
        }
    }
    for( i = 0; i < psWO->nBandCount; i++ )
    {
        GDALRasterBandH band = GDALGetRasterBand(psWO->hSrcDS, psWO->panSrcBands[i]);
        int hasNoDataValue;
        double noDataValue = GDALGetRasterNoDataValue(band, &hasNoDataValue);

        if( hasNoDataValue )
        {
            // Check if the nodata value is out of range
            int bClamped = FALSE;
            int bRounded = FALSE;
            GDALAdjustValueToDataType(GDALGetRasterDataType(band),
                                      noDataValue, &bClamped, &bRounded );
            /* 
            if( !bClamped )
            {
                GDALWarpInitNoDataReal(psWO, -1e10);

                psWO->padfSrcNoDataReal[i] = noDataValue;
                psWO->padfDstNoDataReal[i] = noDataValue;
            }
            */
        }
    }

    if( psWO->padfDstNoDataReal != NULL )
    {
        if (CSLFetchNameValue( psWO->papszWarpOptions, "INIT_DEST" ) == NULL)
        {
            psWO->papszWarpOptions =
                CSLSetNameValue(psWO->papszWarpOptions, "INIT_DEST", "NO_DATA");
        }
    }

/* -------------------------------------------------------------------- */
/*      Create the transformer.                                         */
/* -------------------------------------------------------------------- */
    psWO->pfnTransformer = GDALGenImgProjTransform;
    psWO->pTransformerArg =
        GDALCreateGenImgProjTransformer( psWO->hSrcDS, pszSrcWKT,
                                         NULL, pszDstWKT,
                                         TRUE, 1.0, 0 );

    if( psWO->pTransformerArg == NULL )
    {
        GDALDestroyWarpOptions( psWO );
        return NULL;
    }

/* -------------------------------------------------------------------- */
/*      Figure out the desired output bounds and resolution.            */
/* -------------------------------------------------------------------- */
    CPLErr eErr =
        GDALSuggestedWarpOutput( hSrcDS, psWO->pfnTransformer,
                                 psWO->pTransformerArg,
                                 adfDstGeoTransform, &nDstPixels, &nDstLines );
    if( eErr != CE_None )
    {
        GDALDestroyTransformer( psWO->pTransformerArg );
        GDALDestroyWarpOptions( psWO );
        return NULL;
    }

/* -------------------------------------------------------------------- */
/*      Update the transformer to include an output geotransform        */
/*      back to pixel/line coordinates.                                 */
/*                                                                      */
/* -------------------------------------------------------------------- */
    GDALSetGenImgProjTransformerDstGeoTransform(
        psWO->pTransformerArg, adfDstGeoTransform );

/* -------------------------------------------------------------------- */
/*      Do we want to apply an approximating transformation?            */
/* -------------------------------------------------------------------- */
    if( dfMaxError > 0.0 )
    {
        psWO->pTransformerArg =
            GDALCreateApproxTransformer( psWO->pfnTransformer,
                                         psWO->pTransformerArg,
                                         dfMaxError );
        psWO->pfnTransformer = GDALApproxTransform;
        GDALApproxTransformerOwnsSubtransformer(psWO->pTransformerArg, TRUE);
    }

/* -------------------------------------------------------------------- */
/*      Create the VRT file.                                            */
/* -------------------------------------------------------------------- */
    GDALDatasetH hDstDS
        = GDALCreateWarpedVRT( hSrcDS, nDstPixels, nDstLines,
                               adfDstGeoTransform, psWO );

    GDALDestroyWarpOptions( psWO );

    if( pszDstWKT != NULL )
        GDALSetProjection( hDstDS, pszDstWKT );
    else if( pszSrcWKT != NULL )
        GDALSetProjection( hDstDS, pszSrcWKT );
    else if( GDALGetGCPCount( hSrcDS ) > 0 )
        GDALSetProjection( hDstDS, GDALGetGCPProjection( hSrcDS ) );
    else
        GDALSetProjection( hDstDS, GDALGetProjectionRef( hSrcDS ) );

    return hDstDS;
}


