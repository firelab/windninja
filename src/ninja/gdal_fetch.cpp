/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja
 * Purpose:  Download srtm data for input dem
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

#include "gdal_fetch.h"

GDALFetch::GDALFetch()
{
    Initialize();
}

GDALFetch::GDALFetch(std::string path)
{
    this->path = path;
    Initialize();
}

GDALFetch::~GDALFetch()
{
}

int GDALFetch::Initialize()
{
    GDALDataset *poDS = (GDALDataset*)GDALOpen(path.c_str(), GA_ReadOnly);
    if(poDS == NULL)
    {
        xRes = 0.0;
        yRes = 0.0;
        northeast_x = 0.0;
        northeast_y = 0.0;
        southeast_x = 0.0;
        southeast_y = 0.0;
        southwest_x = 0.0;
        southwest_y = 0.0;
        northwest_x = 0.0;
        northwest_y = 0.0;
        path = "";
    }
    else
    {
        double corners[8];
        double adfGeoTransform[6];
        GDALGetCorners(poDS, corners);
        int err = poDS->GetGeoTransform(adfGeoTransform);
        xRes = adfGeoTransform[1];
        yRes = adfGeoTransform[5];
        northeast_x = corners[0];
        northeast_y = corners[1];
        southeast_x = corners[2];
        southeast_y = corners[3];
        southwest_x = corners[4];
        southwest_y = corners[5];
        northwest_x = corners[6];
        northwest_y = corners[7];
    }
    GDALClose((GDALDatasetH)poDS);
    return 0;
}

/**
 * \brief Fetch surface data for a bounding box.
 *
 * \param bbox north, east, south west bounding box
 * \param resolution output resolution in meters
 * \param filename file to write data into
 * \param options key/value options, unused
 */
SURF_FETCH_E GDALFetch::FetchBoundingBox(double *bbox, double resolution,
                                         const char *filename,
                                         char **options)
{
    /* parse options */
    GDALResampleAlg eAlg = GRA_NearestNeighbour;

    CPLSetConfigOption("GTIFF_DIRECT_IO", "YES");
    CPLSetConfigOption("CPL_VSIL_CURL_ALLOWED_EXTENSIONS", ".zip,.hgt,.tif,.vrt");

    GDALDriverH hDriver;
    GDALDatasetH hDstDS;
    GDALDatasetH hSrcDS;

    hSrcDS = GDALOpen(GetPath().c_str(), GA_ReadOnly);
    if(hSrcDS == NULL)
    {
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

    /* Silence warnings with regard to exceeding limits */
    CPLPushErrorHandler(CPLQuietErrorHandler);
    eErr = GDALSuggestedWarpOutput(hSrcDS, 
                                   GDALGenImgProjTransform, hTransformArg, 
                                   adfDstGeoTransform, &nPixels, &nLines);
    CPLPopErrorHandler();
    if(eErr != CE_None)
    {
        return SURF_FETCH_E_IO_ERR;
    }
    GDALDestroyGenImgProjTransformer(hTransformArg);

    double adfWarpBbox[4];
    for(int i = 0; i < 4; i++)
        adfWarpBbox[i] = bbox[i];
    WarpBoundingBox(adfWarpBbox);

    double dfMaxX, dfMaxY, dfMinX, dfMinY;
    double dfXRes, dfYRes;
    dfXRes = dfYRes = resolution;
    dfMaxX = adfWarpBbox[1];
    dfMaxY = adfWarpBbox[0];
    dfMinX = adfWarpBbox[3];
    dfMinY = adfWarpBbox[2];

    nPixels = (int) ((dfMaxX - dfMinX + (dfXRes / 2.0)) / dfXRes);
    nLines = (int) ((dfMaxY - dfMinY + (dfYRes / 2.0)) / dfYRes);

    if(nPixels <= 0 || nLines <= 0)
    {
        return SURF_FETCH_E_WARPER_ERR; /*assumption*/
    }

    adfDstGeoTransform[0] = dfMinX;
    adfDstGeoTransform[3] = dfMaxY;
    adfDstGeoTransform[1] = dfXRes;
    adfDstGeoTransform[5] = -dfYRes;

    hDstDS = GDALCreate(hDriver, filename, nPixels, nLines, 
                        GDALGetRasterCount(hSrcDS), GDT_Float32, NULL);

    if(hDstDS == NULL)
    {
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
        CPLError( CE_Failure, CPLE_AppDefined, "Could not warp image, " \
                                               "download failed." );
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

    CPLFree((void*)padfScanline);
    CPLFree((void*)pszDstWKT);

    GDALClose(hDstDS);
    GDALClose(hSrcDS);

    CPLSetConfigOption("GTIFF_DIRECT_IO", "NO");
    CPLSetConfigOption("CPL_VSIL_CURL_ALLOWED_EXTENSIONS", NULL);

    return nNoDataCount;
}

