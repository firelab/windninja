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
    for( i = nStart; i < nStop; i += nStride )
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

    nomads_utc *tmp;
    NomadsUtcCreate( &tmp );
    NomadsUtcCopy( tmp, now );

    papszHours = CSLTokenizeString2( ppszKey[NOMADS_FCST_RUN_HOURS], ",", 0 );
    nTokenCount = CSLCount( papszHours );
    CPLAssert( nTokenCount > 0 );
    papszRunHours = CSLTokenizeString2( papszHours[nTokenCount - 1], ":", 0);

    (*panRunHours) = (int*)CPLMalloc( sizeof( int ) * atoi( papszRunHours[1] ) );
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

int NomadsFetch( const char *pszModelKey, int nHours, double *padfBbox,
                 const char *pszDstVsiPath )
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
    int nStart = 0;
    int nStop = 0;
    int nStride = 0;
    char **papszHours = NULL;
    int nFilesToGet = 0;
    const char *pszOutFilename = NULL;
    int bAlreadyWentBack = FALSE;
#ifdef NOMADS_USE_VSI_READ
    VSILFILE *fin = NULL;
#else
    CPLHTTPResult *psResult;
#endif
    VSILFILE *fout = NULL;
    vsi_l_offset nOffset = 0;
    char pabyBuffer[1024];
    nomads_utc *now, *end, *tmp, *fcst;

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
    for( i = 0; i < nFilesToGet; i++ )
    {
        pszGribFile = CPLStrdup( CPLSPrintf( ppszKey[NOMADS_FILE_NAME_FRMT],
                                             nFcstHour, panRunHours[i] ) );
        NomadsUtcStrfTime( fcst, ppszKey[NOMADS_DIR_DATE_FRMT] );
        /* Special case for gfs */
        if( EQUAL( pszModelKey, "gfs" ) )
            pszGribDir = CPLStrdup( CPLSPrintf( ppszKey[NOMADS_DIR_FRMT],
                                                fcst->s, nFcstHour ) );
        else
            pszGribDir = CPLStrdup( CPLSPrintf( ppszKey[NOMADS_DIR_FRMT], fcst->s ) );

        pszUrl =
            //CPLSPrintf( "/vsicurl/%s%s?%s&%s%s&file=%s&dir=/%s", NOMADS_URL_CGI,
            CPLSPrintf( "%s%s?%s&%s%s&file=%s&dir=/%s", NOMADS_URL_CGI,
                        ppszKey[NOMADS_FILTER_BIN],
                        NomadsBuildArgList( ppszKey[NOMADS_VARIABLES], "var" ),
                        NomadsBuildArgList( ppszKey[NOMADS_LEVELS], "lev" ),
                        NOMADS_SUBREGION, pszGribFile, pszGribDir );
        pszUrl = CPLSPrintf( pszUrl, padfBbox[0], padfBbox[1],
                             padfBbox[2], padfBbox[3] );

#ifdef NOMADS_USE_VSI_READ
        pszUrl = CPLSPrintf( "/vsicurl/%s", pszUrl );
        fin = VSIFOpenL( pszUrl, "rb" );
        if( !fin )
#else /* NOMADS_USE_VSI */
        psResult = CPLHTTPFetch( pszUrl, NULL );
        if( !psResult || psResult->nStatus != 0 )
#endif /* NOMADS_USE_VSI */
        {
            if( !bAlreadyWentBack )
            {
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
                CSLDestroy( papszHours );
                i--;
                CPLFree( (void*)pszGribFile );
                CPLFree( (void*)pszGribDir );
                CPLFree( (void*)panRunHours );
#ifndef NOMADS_USE_VSI_READ
                CPLHTTPDestroyResult( psResult );
#endif
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
#ifdef NOMADS_USE_VSI_READ
                VSIFCloseL( fin );
#else
                CPLHTTPDestroyResult( psResult );
#endif
                return NOMADS_ERR;
            }
        }
        /* Write to zip file or folder */
        pszOutFilename = CPLSPrintf( "%s/%s", pszDstVsiPath, pszGribFile );
        if( strstr( pszDstVsiPath, ".zip" ) )
            pszOutFilename = CPLSPrintf( "/vsizip/%s", pszOutFilename );

        fout = VSIFOpenL( pszOutFilename, "wb" );
        if( !fout )
        {
            CPLError( CE_Failure, CPLE_AppDefined,
                      "Could not open zip file for writing" );
            CPLFree( (void*)pszGribFile );
            CPLFree( (void*)pszGribDir );
            CPLFree( (void*)panRunHours );
            NomadsUtcFree( now );
            NomadsUtcFree( end );
            NomadsUtcFree( tmp );
            NomadsUtcFree( fcst );
#ifdef NOMADS_USE_VSI_READ
            VSIFCloseL( fin );
#else
            CPLHTTPDestroyResult( psResult );
#endif
            return NOMADS_ERR;
        }
#ifdef NOMADS_USE_VSI_READ
        do
        {
            nOffset = VSIFReadL( pabyBuffer, 1, 1024, fin );
            VSIFWriteL( pabyBuffer, 1, nOffset, fout );
        } while ( nOffset != 0 );
        VSIFCloseL( fin );
#else
        VSIFWriteL( psResult->pabyData, psResult->nDataLen, 1, fout );
        CPLHTTPDestroyResult( psResult );
#endif
        VSIFCloseL( fout );
        CPLFree( (void*)pszGribFile );
        CPLFree( (void*)pszGribDir );
    }
    NomadsUtcFree( now );
    NomadsUtcFree( end );
    NomadsUtcFree( tmp );
    NomadsUtcFree( fcst );
    CPLFree( (void*)panRunHours );

    return NOMADS_OK;
}
