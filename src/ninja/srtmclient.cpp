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

    // calculate the final bbox to clip to, data will be downloaded using a buffered bbox then later clipped to this bbox
    double warped_bbox[4];
    for(int i = 0; i < 4; i++)
        warped_bbox[i] = bbox[i];
    WarpBoundingBox(warped_bbox);

    // compute a buffered bbox using the methods defined by wxModelInitialization::ComputeWxModelBuffer()
    double maxX = bbox[1];  // east
    double maxY = bbox[0];  // north
    double minX = bbox[3];  // west
    double minY = bbox[2];  // south
    double Lx = maxX - minX;
    double Ly = maxY - minY;
    double buffer_x = Lx * 0.2;
    double buffer_y = Ly * 0.2;

    // the buffered bbox is used to download the raw data before warping and final clipping
    double buffered_bbox[4];
    buffered_bbox[0] = bbox[0] + buffer_y;  // north
    buffered_bbox[1] = bbox[1] + buffer_x;  // east
    buffered_bbox[2] = bbox[2] - buffer_y;  // south
    buffered_bbox[3] = bbox[3] - buffer_x;  // west

    //check that we are within the SRTM bounds
    std::string osDataPath = FindDataPath( "srtm_region.geojson" );
    const char *pszGeom;
    pszGeom = CPLSPrintf( "POLYGON((%lf %lf,%lf %lf,%lf %lf,%lf %lf,%lf %lf))",
                           buffered_bbox[1], buffered_bbox[0], buffered_bbox[3], buffered_bbox[0],
                           buffered_bbox[3], buffered_bbox[2], buffered_bbox[1], buffered_bbox[2],
                           buffered_bbox[1], buffered_bbox[0] );
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
    pszUrl = CPLSPrintf( SRTM_REQUEST_TEMPLATE, buffered_bbox[2], buffered_bbox[0], buffered_bbox[3], buffered_bbox[1], APIKey );
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
     *  Warp and clip to UTM coords
     *-----------------------------------------------------------------------------*/
    CPLDebug( "SRTM_CLIENT", "Warping to UTM..." );

    GDALResampleAlg eAlg = GRA_NearestNeighbour;

    CPLSetConfigOption("GTIFF_DIRECT_IO", "YES");
    CPLSetConfigOption("CPL_VSIL_CURL_ALLOWED_EXTENSIONS", ".zip,.hgt,.tif,.vrt");

    GDALDriverH hDriver;
    GDALDatasetH hDstDS;
    GDALDatasetH hSrcDS;

    hSrcDS = GDALOpen(CPLFormFilename(CPLGetPath(filename), "NINJA_SRTM", ".tif"), GA_ReadOnly);
    if(hSrcDS == NULL)
    {
        CPLError( CE_Failure, CPLE_AppDefined, "Failed to open downloaded image for warp, download failed." );
        return SURF_FETCH_E_IO_ERR;
    }
    hDriver = GDALGetDriverByName("GTiff");

    const char *pszSrcWKT, *pszDstWKT = NULL;
    pszSrcWKT = GDALGetProjectionRef(hSrcDS);

    OGRSpatialReference oSrcSRS, oDstSRS;

    oSrcSRS.importFromEPSG(4326);

    int nUtmZone = BoundingBoxUtm(bbox);

    oDstSRS.importFromEPSG(nUtmZone);
    oDstSRS.exportToWkt((char**)&pszDstWKT);

    void *hTransformArg;

    hTransformArg =
        GDALCreateGenImgProjTransformer(hSrcDS, pszSrcWKT, NULL, pszDstWKT,
                                        FALSE, 0, 1);

    double adfDstGeoTransform[6];
    int nPixels=0, nLines=0;
    CPLErr eErr;

    // Silence warnings with regard to exceeding limits
    CPLPushErrorHandler(CPLQuietErrorHandler);
    eErr = GDALSuggestedWarpOutput(hSrcDS, 
                                   GDALGenImgProjTransform, hTransformArg, 
                                   adfDstGeoTransform, &nPixels, &nLines);
    CPLPopErrorHandler();
    if(eErr != CE_None)
    {
        CPLError( CE_Failure, CPLE_AppDefined, "Could not warp image, download failed." );
        return SURF_FETCH_E_IO_ERR;
    }
    GDALDestroyGenImgProjTransformer(hTransformArg);

    double dfMaxX, dfMaxY, dfMinX, dfMinY;
    double dfXRes, dfYRes;

    dfXRes = dfYRes = resolution;  // hrm, might need to adjust this value using the warped bbox somehow, but I don't see such a thing done in gmted gdal_fetch, so maybe not
    dfMaxX = warped_bbox[1];  // east
    dfMaxY = warped_bbox[0];  // north
    dfMinX = warped_bbox[3];  // west
    dfMinY = warped_bbox[2];  // south

    nPixels = (int) ((dfMaxX - dfMinX + (dfXRes / 2.0)) / dfXRes);
    nLines = (int) ((dfMaxY - dfMinY + (dfYRes / 2.0)) / dfYRes);

    if(nPixels <= 0 || nLines <= 0)
    {
        CPLError( CE_Failure, CPLE_AppDefined, "Invalid nPixels and/or nLines for warp. Could not warp image, download failed." );
        return SURF_FETCH_E_WARPER_ERR;  // assumption
    }

    adfDstGeoTransform[0] = dfMinX;
    adfDstGeoTransform[3] = dfMaxY;
    adfDstGeoTransform[1] = dfXRes;
    adfDstGeoTransform[5] = -dfYRes;

    hDstDS = GDALCreate(hDriver, filename, nPixels, nLines, 
                        GDALGetRasterCount(hSrcDS), GDT_Float32, NULL);

    if(hDstDS == NULL)
    {
        CPLError( CE_Failure, CPLE_AppDefined, "Failed to open final srtm file for writing, download failed." );
        return SURF_FETCH_E_IO_ERR;
    }

    GDALSetProjection(hDstDS, pszDstWKT);
    GDALSetGeoTransform(hDstDS, adfDstGeoTransform);

    GDALWarpOptions *psWarpOptions = GDALCreateWarpOptions();

    psWarpOptions->hSrcDS = hSrcDS;
    psWarpOptions->hDstDS = hDstDS;

    psWarpOptions->nBandCount = 1;
    psWarpOptions->panSrcBands = 
        (int *) CPLMalloc(sizeof(int) * psWarpOptions->nBandCount );
    psWarpOptions->panSrcBands[0] = 1;
    psWarpOptions->panDstBands = 
        (int *) CPLMalloc(sizeof(int) * psWarpOptions->nBandCount );
    psWarpOptions->panDstBands[0] = 1;

    //psWarpOptions->pfnProgress = GDALTermProgress;

    psWarpOptions->pTransformerArg = 
        GDALCreateGenImgProjTransformer( hSrcDS, 
                                         GDALGetProjectionRef(hSrcDS), 
                                         hDstDS,
                                         GDALGetProjectionRef(hDstDS), 
                                         FALSE, 0.0, 1 );

    psWarpOptions->pfnTransformer = GDALGenImgProjTransform;

    psWarpOptions->eResampleAlg = eAlg;

    GDALWarpOperation oOperation;

    oOperation.Initialize( psWarpOptions );
    eErr = oOperation.ChunkAndWarpImage( 0, 0, 
                                         GDALGetRasterXSize( hDstDS ), 
                                         GDALGetRasterYSize( hDstDS ) );

    GDALDestroyGenImgProjTransformer( psWarpOptions->pTransformerArg );
    GDALDestroyWarpOptions( psWarpOptions );

    if( eErr != CE_None )
    {
        CPLError( CE_Failure, CPLE_AppDefined, "Could not warp image, download failed." );
        CPLFree((void*)pszDstWKT);
        GDALClose(hDstDS);
        GDALClose(hSrcDS);
        return SURF_FETCH_E_IO_ERR;
    }


    GDALRasterBandH hSrcBand;
    GDALRasterBandH hDstBand;

    hSrcBand = GDALGetRasterBand(hSrcDS, 1);
    hDstBand = GDALGetRasterBand(hDstDS, 1);

    double dfNoData = GDALGetRasterNoDataValue(hSrcBand, NULL);

    GDALSetRasterNoDataValue(hDstBand, dfNoData);

    double *padfScanline;
    padfScanline = (double *) CPLMalloc(sizeof(double)*nPixels);
    int nNoDataCount = 0;
    for(int i = 0;i < nLines;i++)
    {
        GDALRasterIO(hDstBand, GF_Read, 0, i, nPixels, 1, 
                     padfScanline, nPixels, 1, GDT_Float64, 0, 0);
        for(int j = 0; j < nPixels;j++)
        {
            if(CPLIsEqual(padfScanline[j], dfNoData))
                nNoDataCount++;
        }
    }

    if(nNoDataCount > 0)
    {
        std::string oErrorString = "SRTMClient::fetchBoundingBox() after downloading, warping, and clipping, found noDataValues!!!";
        CPLError( CE_Failure, CPLE_AppDefined, oErrorString.c_str() );
        return false;
    }

    CPLFree((void*)padfScanline);
    CPLFree((void*)pszDstWKT);

    GDALClose(hDstDS);
    GDALClose(hSrcDS);

    VSIUnlink(CPLFormFilename(CPLGetPath(filename), "NINJA_SRTM", ".tif"));

    CPLSetConfigOption("GTIFF_DIRECT_IO", "NO");
    CPLSetConfigOption("CPL_VSIL_CURL_ALLOWED_EXTENSIONS", NULL);

    return SURF_FETCH_E_NONE;
}

