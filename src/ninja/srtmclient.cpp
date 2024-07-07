/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja
 * Purpose:  Client to download SRTM files
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

#include "srtmclient.h"

SRTMClient::SRTMClient() : SurfaceFetch()
{
    if(CPLGetConfigOption("CUSTOM_SRTM_API_KEY", NULL) != NULL)
    {
        APIKey = CPLGetConfigOption("CUSTOM_SRTM_API_KEY", NULL);
        CPLDebug("SRTM_CLIENT", "Setting a custom SRTM API key to %s", APIKey);
    }
    else if(CPLGetConfigOption("HAVE_CLI_SRTM_API_KEY", NULL) != NULL)
    {
        APIKey = CPLGetConfigOption("HAVE_CLI_SRTM_API_KEY", NULL);
        CPLDebug("SRTM_CLIENT", "Setting CLI SRTM API key to %s", APIKey);
    }
    else
    { 
        //if a custom key wasn't set, use the one set in cmake_cli.cpp for CLI runs or the defalut key for GUI runs
        APIKey = CPLGetConfigOption("NINJA_GUI_SRTM_API_KEY", NULL);
        CPLDebug("SRTM_CLIENT", "Setting GUI SRTM API key to %s", APIKey);
    }
    
    xRes = 30.0;
    yRes = 30.0;
    northeast_x = -180;
    northeast_y = 60;
    southeast_x = 180;
    southeast_y = -56;
    southwest_x = -180;
    southwest_y = -56;
    northwest_x = -180;
    northwest_y = 60;
}

SRTMClient::~SRTMClient()
{
}

SURF_FETCH_E SRTMClient::FetchBoundingBox( double *bbox, double resolution,
                                               const char *filename,
                                               char **options )
{
    (void)resolution;
    if( NULL == filename )
    {
        return SURF_FETCH_E_BAD_INPUT;
    }

    if(APIKey == NULL)
    {
        CPLError( CE_Failure, CPLE_AppDefined,
                  "SRTM download failed. No API key specified.");
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

    //check that we are within the SRTM bounds
    std::string osDataPath = FindDataPath( "srtm_region.geojson" );
    osDataPath = osDataPath;
    const char *pszGeom;
    pszGeom = CPLSPrintf( "POLYGON((%lf %lf,%lf %lf,%lf %lf,%lf %lf,%lf %lf))",
                           bbox[1], bbox[0], bbox[3], bbox[0],
                           bbox[3], bbox[2], bbox[1], bbox[2],
                           bbox[1], bbox[0] );
    CPLDebug( "SRTM_CLIENT", "Testing if %s contains %s", osDataPath.c_str(),
              pszGeom );

    if( !NinjaOGRContain( pszGeom, osDataPath.c_str(), "srtm_region" ) )
    {
        CPLError( CE_Failure, CPLE_AppDefined,
                  "Requested DEM is outside of the SRTM bounds." );
        return SURF_FETCH_E_BAD_INPUT;
    }

    const char *pszUrl;

    int nEpsgCode = -1;
    //get UTM zone as EPSG
    nEpsgCode = BoundingBoxUtm( bbox );

    /*
    ** Better check?
    */
    if( nEpsgCode < 1 )
    {
        CPLError( CE_Failure, CPLE_AppDefined, "Invalid EPSG code." );
        return SURF_FETCH_E_BAD_INPUT;
    }

    /*-----------------------------------------------------------------------------
     *  Request a SRTMGL1 DEM via the OpenTopography API
     *-----------------------------------------------------------------------------*/
    pszUrl = CPLSPrintf( SRTM_REQUEST_TEMPLATE, bbox[2], bbox[0], bbox[3], bbox[1], APIKey );
    psResult = NULL;
    psResult = CPLHTTPFetch( pszUrl, NULL );
    CPLDebug( "SRTM_CLIENT", "Request URL: %s", pszUrl );

     /*-----------------------------------------------------------------------------
     *  Check the result of the request
     *-----------------------------------------------------------------------------*/
    CPLDebug( "SRTM_CLIENT", "Response: %s", psResult->pabyData );
    if( !psResult || psResult->nStatus != 0 || psResult->nDataLen < 1 || psResult->pszErrBuf != NULL) 
    {
        if( strstr( (char*)psResult->pszErrBuf, "HTTP error code : 401" ) )
        {
            CPLError( CE_Failure, CPLE_AppDefined,
                  "Failed to download file, bad API key." );
        }
        else
        {
            CPLError( CE_Failure, CPLE_AppDefined,
                      "Failed to download file." );
        }
        CPLHTTPDestroyResult( psResult );
        return SURF_FETCH_E_BAD_INPUT;
    }

    //make a temporary file to hold the result of the HTTP request (in lat/lon coordinates)
    CPLDebug( "SRTM_CLIENT", "Writing temporary file to: %s", CPLFormFilename(CPLGetPath(filename), "NINJA_SRTM", ".tif"));
    VSILFILE *fout;
    fout = VSIFOpenL( CPLFormFilename(CPLGetPath(filename), "NINJA_SRTM", ".tif"), "wb" );
    if( !fout )
    {
        CPLError( CE_Failure, CPLE_AppDefined,
                  "Failed to open srtm file for writing." );
        CPLHTTPDestroyResult( psResult );
        return SURF_FETCH_E_IO_ERR;
    }
    VSIFWriteL( psResult->pabyData, psResult->nDataLen, 1, fout );
    VSIFCloseL( fout );
    CPLHTTPDestroyResult( psResult );

     /*-----------------------------------------------------------------------------
     *  Warp to UTM coords
     *-----------------------------------------------------------------------------*/
    CPLDebug( "SRTM_CLIENT", "Warping to UTM..." );
    GDALDatasetH hDS = (GDALDatasetH) GDALOpen(CPLFormFilename(CPLGetPath(filename), "NINJA_SRTM", ".tif"), GA_ReadOnly);
    GDALDatasetH hUtmDS;
    bool rc = GDALWarpToUtm( filename, hDS, hUtmDS);
    if(rc != true)
    {
        return SURF_FETCH_E_IO_ERR;
    }

    GDALClose(hDS);
    GDALClose(hUtmDS);
    VSIUnlink(CPLFormFilename(CPLGetPath(filename), "NINJA_SRTM", ".tif"));

    return SURF_FETCH_E_NONE;
}

