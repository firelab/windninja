/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja
 * Purpose:  Client to download landscape files
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

#ifdef WITH_LCP_CLIENT
#include "landfireclient.h"

#define SIZE (1024)

LandfireClient::LandfireClient() : SurfaceFetch()
{
    xRes = 30.0;
    yRes = 30.0;
    northeast_x = -40;
    northeast_y = 72;
    southeast_x = -40;
    southeast_y = 12;
    southwest_x = -180;
    southwest_y = 12;
    northwest_x = -180;
    northwest_y = 72;
}

/*
** Replace the default value srs value in the url with a utm zone.
**
** Steps:
**       Check for default SRS key/value
**       Erase it from the string
**       Add a new token with the EPSG code provided.
**
** The result should be freed using CPLFree();
*/
const char * LandfireClient::ReplaceSRS( int nEpsgCode, const char *pszUrl )
{
    int i, n;
    int nUrlSize = CPLStrnlen( pszUrl, 8192 );
    const char *pszEpsg = CPLSPrintf( "&prj=%d", nEpsgCode );
    int nEpsgSize = CPLStrnlen( pszEpsg, 8192 );
    char **papszBadTokens = CSLTokenizeString2( LF_DEFAULT_SRS_TOKENS, ",", 0 );
    int nBadTokenSize = 0;
    char *p, *q;
    char *pszNewUrl = CPLStrdup( pszUrl );

    i = 0;
    do
    {
        p = strstr( pszNewUrl, papszBadTokens[i] );
        nBadTokenSize = strlen( papszBadTokens[i] );
        i++;
    } while( i < CSLCount( papszBadTokens ) && !p );
    i--;
    if( p )
    {
        if( nBadTokenSize < nEpsgSize )
        {
            n = nUrlSize + nEpsgSize - nBadTokenSize + 1;
            CPLAssert( n > strlen( pszNewUrl ) );
            pszNewUrl = (char*)CPLRealloc( pszNewUrl, sizeof( char ) * n );
            /* reset p  after realloc */
            p = strstr( pszNewUrl, papszBadTokens[i] );
        }
        q = p + nBadTokenSize;
        while( *q != '\0' )
        {
            *(p++) = *(q++);
        }
        *p = '\0';
    }
    else
    {
        n = nUrlSize + nEpsgSize + 1;
        pszNewUrl = (char*)CPLRealloc( pszNewUrl, sizeof( char ) * n );
    }
    strcat( pszNewUrl, pszEpsg );
    CSLDestroy( papszBadTokens );
    return pszNewUrl;
}

LandfireClient::~LandfireClient()
{
}

SURF_FETCH_E LandfireClient::FetchBoundingBox( double *bbox, double resolution,
                                               const char *filename,
                                               char **options )
{
    (void)resolution;
    if( NULL == filename )
    {
        return SURF_FETCH_E_BAD_INPUT;
    }

    /*
    ** We have an arbitrary limit on request size, 0.001 degrees
    */
    if( fabs( bbox[0] - bbox[2] ) < 0.001 || fabs( bbox[1] - bbox[3] ) < 0.001 )
    {
        CPLError( CE_Failure, CPLE_AppDefined,
                  "Bounding box too small, must be greater than " \
                  "0.001 x 0.001 degrees." );
        return SURF_FETCH_E_BAD_INPUT;
    }

    /*-----------------------------------------------------------------------------
     *  Local Variable Declarations
     *-----------------------------------------------------------------------------*/
    int i = 0;
    char *p;
    int nMaxTries = atoi( CPLGetConfigOption( "LCP_MAX_DOWNLOAD_TRIES", "40" ) );
    double dfWait = atof( CPLGetConfigOption( "LCP_DOWNLOAD_WAIT", "3" ) );
    const char *pszProduct = CPLStrdup( CSLFetchNameValue( options, "PRODUCT" ) );
    /*
    ** Stupidly simple heuristics to try to get the 'correct' product.
    */
    if( pszProduct == NULL || EQUAL( pszProduct, "" ) )
    {
        if( EQUAL( pszProduct, "" ) )
            CPLFree( (void*)pszProduct );
        std::string osDataPath = FindDataPath( "landfire.zip" );
        osDataPath = "/vsizip/" + osDataPath;
        const char *pszGeom;
        pszGeom = CPLSPrintf( "POLYGON((%lf %lf,%lf %lf,%lf %lf,%lf %lf,%lf %lf))",
                               bbox[1], bbox[0], bbox[3], bbox[0],
                               bbox[3], bbox[2], bbox[1], bbox[2],
                               bbox[1], bbox[0] );
        CPLDebug( "LCP_CLIENT", "Testing if %s contains %s", osDataPath.c_str(),
                  pszGeom );
        if( NinjaOGRContain( pszGeom, osDataPath.c_str(), "conus" ) )
        {
            //pszProduct = CPLStrdup( "F4W21HZ" ); //2008 data
            //pszProduct = CPLStrdup( "FHY60HZ" ); //2016 data
            pszProduct = CPLStrdup( "200FVC_20" ); //2016 data
        }
        else if( NinjaOGRContain( pszGeom, osDataPath.c_str(), "ak" ) )
        {
            //pszProduct = CPLStrdup( "F7C29HZ" ); //2008 data
            pszProduct = CPLStrdup( "F0740HZ" ); //2014 data
        }
        else if( NinjaOGRContain( pszGeom, osDataPath.c_str(), "hi" ) )
        {
            //pszProduct = CPLStrdup( "F4825HZ" ); //2008 data
            pszProduct = CPLStrdup( "FCM42HZ" ); //2014 data
        }
        /* Contiguous US */
        //if( bbox[0] < 52 && bbox[1] < -60 && bbox[2] > 22 && bbox[3] > -136 )
        //    pszProduct = CPLStrdup( "F4W21HZ" );
        /* Alaska */
        //else if( bbox[0] < 75 && bbox[1] < -125 && bbox[2] > 50 && bbox[3] > -179 )
        //    pszProduct = CPLStrdup( "F7C29HZ" );
        /* Hawaii */
        //else if( bbox[0] < 25 && bbox[1] < -150 && bbox[2] > 15 && bbox[3] > -170 )
        //    pszProduct = CPLStrdup( "F4825HZ" );
        else
        {
            CPLError( CE_Failure, CPLE_AppDefined,
                      "Failed to locate product." );
            return SURF_FETCH_E_BAD_INPUT;
        }
    }
    CPLDebug( "LCP_CLIENT", "Using product: %s", pszProduct );
    const char *pszUrl;

    const char *pszTemp = CSLFetchNameValue( options, "OVERRIDE_BEST_UTM" );
    int nEpsgCode = -1;
    if( pszTemp == NULL )
    {
        nEpsgCode = BoundingBoxUtm( bbox );
    }
    else
    {
        nEpsgCode = atoi( pszTemp );
    }
    /*
    ** Better check?
    */
    if( nEpsgCode < 1 )
    {
        CPLError( CE_Failure, CPLE_AppDefined, "Invalid EPSG code." );
        CPLFree( (void*)pszProduct );
        return SURF_FETCH_E_BAD_INPUT;
    }

    /*-----------------------------------------------------------------------------
     *  Request a Model via the lfps.usgs.gov REST client
     *-----------------------------------------------------------------------------*/
    // Fix the SRS
    //const char *pszNewUrl = ReplaceSRS( nEpsgCode, pszResponse );
    //CPLDebug( "LCP_CLIENT", "Sanitized SRS Download URL: %s", pszNewUrl );
    pszUrl = CPLSPrintf( LF_REQUEST_TEMPLATE, bbox[3], bbox[2], bbox[1],
                                              bbox[0], pszProduct );
    CPLFree( (void*)pszProduct );
    m_poResult = CPLHTTPFetch( pszUrl, NULL );
    CHECK_HTTP_RESULT( "Failed to get download URL" );
    CPLDebug( "LCP_CLIENT", "Request URL: %s", pszUrl );
     /*-----------------------------------------------------------------------------
     *  Parse the JSON result of the request
     *-----------------------------------------------------------------------------*/
    CPLDebug( "LCP_CLIENT", "JSON Response: %s", m_poResult->pabyData );

    char **papszTokens = NULL;
    papszTokens = CSLTokenizeString2( (const char*)m_poResult->pabyData, ",:",
                                      CSLT_HONOURSTRINGS | CSLT_PRESERVEESCAPES |
                                      CSLT_STRIPENDSPACES | CSLT_STRIPLEADSPACES );
    int nTokens = CSLCount( papszTokens );
    CPLDebug( "LCP_CLIENT", "papszTokens[0]: %s", papszTokens[0]);
    CPLDebug( "LCP_CLIENT", "papszTokens[1]: %s", papszTokens[1]);
    CPLDebug( "LCP_CLIENT", "papszTokens[2]: %s", papszTokens[2]);
    CPLDebug( "LCP_CLIENT", "papszTokens[3]: %s", papszTokens[3]);

    m_JobId = std::string(papszTokens[1]);
    CPLDebug( "LCP_CLIENT", "m_JobId: %s", m_JobId.c_str());

    if( nTokens < 4)
    {
        CPLError( CE_Failure, CPLE_AppDefined,
                  "Failed to generate valid URL for LCP download." );
        CPLHTTPDestroyResult( m_poResult );
        CSLDestroy( papszTokens );
        return SURF_FETCH_E_IO_ERR;
    }

    /*-----------------------------------------------------------------------------
     *  Download the landfire model when it's ready
     *-----------------------------------------------------------------------------*/
    int dl_status = 0;
    int nSize = 0;
    bool downloadReady = false;
    bool downloadFailed = false;
    CPLDebug( "LCP_CLIENT", "m_JobId: %s", m_JobId.c_str());
    pszUrl = CPLStrdup(CPLSPrintf( LF_GET_STATUS_TEMPLATE, m_JobId.c_str() ));
    CPLDebug( "LCP_CLIENT", "Status url: %s", pszUrl );
    do
    {
        m_poResult = CPLHTTPFetch( pszUrl, NULL );
        CHECK_HTTP_RESULT( "Failed to get job status" );
        papszTokens = CSLTokenizeString2( (const char*)m_poResult->pabyData, ",:",
                                          CSLT_HONOURSTRINGS | CSLT_PRESERVEESCAPES |
                                          CSLT_STRIPENDSPACES | CSLT_STRIPLEADSPACES );
        int nTokens = CSLCount( papszTokens );

        CPLDebug( "LCP_CLIENT", "papszTokens[0]: %s", papszTokens[0]);
        CPLDebug( "LCP_CLIENT", "papszTokens[1]: %s", papszTokens[1]);
        CPLDebug( "LCP_CLIENT", "papszTokens[2]: %s", papszTokens[2]);
        CPLDebug( "LCP_CLIENT", "papszTokens[3]: %s", papszTokens[3]);
        CPLDebug( "LCP_CLIENT", "papszTokens[4]: %s", papszTokens[4]);
        CPLDebug( "LCP_CLIENT", "papszTokens[5]: %s", papszTokens[5]);

        for( int i = 1; i < nTokens; i++ )
        {
            if(EQUAL( papszTokens[i], "jobStatus" )) 
            {
                if(EQUAL( papszTokens[i + 1], "esriJobSucceeded" ) )
                    downloadReady = true;
                if(EQUAL( papszTokens[i + 1], "esriJobFailed" ) )
                    downloadFailed = true;
                break;
            }
        }
        CPLDebug( "LCP_CLIENT", "Attempting to fetch LCP, try %d of %d, jobStatus: %s", i,
            nMaxTries, papszTokens[3] );

        CSLDestroy( papszTokens );
        CPLHTTPDestroyResult( m_poResult );
        i++;
        CPLSleep( dfWait );

    } while( downloadFailed == false && downloadReady == false && i < nMaxTries );

    CPLFree( (void*) pszUrl );

    if(downloadFailed)
    {
        CPLError( CE_Warning, CPLE_AppDefined, "Failed to download lcp," \
                                               "There was an extraction " \
                                               "error on the server." );
        return SURF_FETCH_E_IO_ERR;
    }
    else if( !downloadReady )
    {
        CPLError( CE_Warning, CPLE_AppDefined, "Failed to download lcp, timed " \
                                               "out.  Try increasing " \
                                               "LCP_MAX_DOWNLOAD_TRIES or "
                                               "LCP_DOWNLOAD_WAIT" );
        return SURF_FETCH_E_TIMEOUT;
    }
    CHECK_HTTP_RESULT( "Failed to get job status" ); 

    nSize = m_poResult->nDataLen;
    VSILFILE *fout;
    const char *pszTmpZip = CPLFormFilename( NULL, 
                                             CPLGenerateTempFilename( "NINJA_LCP_CLIENT" ),
                                             ".zip" );
    fout = VSIFOpenL( pszTmpZip, "w+" );
    if( NULL == fout )
    {
        CPLError( CE_Warning, CPLE_AppDefined, "Failed to create output file" );
        CPLHTTPDestroyResult( m_poResult );
        return SURF_FETCH_E_IO_ERR;
    }
    VSIFWriteL( m_poResult->pabyData, nSize, 1, fout );
    VSIFCloseL( fout );

    CPLHTTPDestroyResult( m_poResult );

    /*
    ** Extract the lcp and the prj file, and 'save as'
    */
    char **papszFileList = NULL;
    std::string osPathInZip;
    const char *pszVSIZip = CPLSPrintf( "/vsizip/%s", pszTmpZip );
    CPLDebug( "LCP_CLIENT", "Extracting lcp from %s", pszVSIZip );
    papszFileList = VSIReadDirRecursive( pszVSIZip );
    int bFound = FALSE;
    std::string osFullPath;
    for( int i = 0; i < CSLCount( papszFileList ); i++ )
    {
        osFullPath = papszFileList[i];
        if( osFullPath.find( "Landscape_1.lcp" ) != std::string::npos )
        {
            osPathInZip = CPLGetPath( papszFileList[i] );
            CPLDebug( "LCP_CLIENT", "Found lcp in: %s", osPathInZip.c_str() );
            bFound = TRUE;
            break;
        }
    }
    CSLDestroy( papszFileList );
    if( !bFound )
    {
        CPLError( CE_Failure, CPLE_AppDefined,
                  "Failed to find lcp in archive" );
        //VSIUnlink( pszTmpZip );
        return SURF_FETCH_E_IO_ERR;
    }
    int nError = 0;
    const char *pszFileToFind = CPLSPrintf( "%s/Landscape_1.lcp",
                                            osPathInZip.c_str() );
    nError = ExtractFileFromZip( pszTmpZip, pszFileToFind, filename );
    if( nError )
    {
        CPLError( CE_Failure, CPLE_AppDefined,
                  "Failed to extract LCP from zip." );
        VSIUnlink( pszTmpZip );
        return SURF_FETCH_E_IO_ERR;
    }
    pszFileToFind = CPLSPrintf( "%s/Landscape_1.prj", osPathInZip.c_str() );
    nError = ExtractFileFromZip( pszTmpZip, pszFileToFind,
                                 CPLFormFilename( CPLGetPath( filename ),
                                                  CPLGetBasename( filename ),
                                                  ".prj" ) );
    if( nError )
    {
        CPLError( CE_Failure, CPLE_AppDefined,
                  "Failed to extract PRJ from zip." );
        return SURF_FETCH_E_IO_ERR;
    }

    if( !CSLTestBoolean( CPLGetConfigOption( "LCP_KEEP_ARCHIVE", "FALSE" ) ) )
    {
        VSIUnlink( pszTmpZip );
    }

    return SURF_FETCH_E_NONE;
}

#endif /* WITH_LCP_CLIENT */

