/******************************************************************************
 * relief_fetch.cpp
 *
 * Project:  WindNinja
 * Purpose:  Downloads relief maps given a bounding box 
 * Author:   Levi Malott <levi.malott@mst.edu>
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



#include "relief_fetch.h"
#include <iostream>
                                          

ReliefFetch::ReliefFetch()
{
    path = "";
    Initialize();
}

ReliefFetch::ReliefFetch( std::string filepath )
{
    path = filepath;
    Initialize();
} 

ReliefFetch::~ReliefFetch()
{
    return;
}

SURF_FETCH_E ReliefFetch::Initialize()
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

SURF_FETCH_E ReliefFetch::FetchBoundingBox( double *bbox, double resolution,
                                            const char *filename, char ** options )
{
    /* parse options */
    GDALResampleAlg eAlg = GRA_NearestNeighbour;

    CPLSetConfigOption("GTIFF_DIRECT_IO", "YES");
    CPLSetConfigOption("CPL_VSIL_CURL_ALLOWED_EXTENSIONS", ".zip,.hgt,.tif,.vrt, .xml");

    GDALDriverH hDriver;
    GDALDatasetH hDstDS;
    GDALDatasetH hSrcDS;
    GDALDataType eDT;

    hSrcDS = GDALOpen(GetPath().c_str(), GA_ReadOnly);
    if(hSrcDS == NULL)
    {
        CPLError(CE_Failure, CPLE_AppDefined, "Could not open path '%s' for reading, download failed.", GetPath().c_str());
        return SURF_FETCH_E_IO_ERR;
    }

    eDT = GDALGetRasterDataType( GDALGetRasterBand( hSrcDS, 1 ) );
    if( GDT_Byte != eDT )
    {
        //add error info detailing incompatible data types
        CPLError(CE_Failure, CPLE_AppDefined, "Raster band contains incompatible byte data types, download failed.");
        return SURF_FETCH_E_IO_ERR;
    }

    const char *pszSrcWKT=NULL, *pszDstWKT = NULL;
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
        CPLError(CE_Failure, CPLE_AppDefined, "Could not warp image, download failed.");
        return SURF_FETCH_E_WARPER_ERR;
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
        CPLError(CE_Failure, CPLE_AppDefined, "Invalid nPixels and/or nLines for warp. Could not warp image, download failed.");
        return SURF_FETCH_E_WARPER_ERR;
    }

    adfDstGeoTransform[0] = dfMinX;
    adfDstGeoTransform[3] = dfMaxY;
    adfDstGeoTransform[1] = dfXRes;
    adfDstGeoTransform[5] = -dfYRes;


    hDriver = GDALGetDriverByName("GTiff");
    hDstDS = GDALCreate(hDriver, filename, nPixels, nLines, 
                        GDALGetRasterCount(hSrcDS), eDT, NULL);

    if(hDstDS == NULL)
    {
        CPLError(CE_Failure, CPLE_AppDefined, "Failed to create final output file for writing, download failed.");
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
        CPLError(CE_Failure, CPLE_AppDefined, "Could not warp image, download failed.");
        CPLFree((void*)pszDstWKT);
        GDALClose(hDstDS);
        GDALClose(hSrcDS);
        return SURF_FETCH_E_WARPER_ERR;
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

    if(nNoDataCount > 0)
    {
        CPLError(CE_Failure, CPLE_AppDefined, "Final downloaded elevation file contains '%d' noDataValues", nNoDataCount);
    }
    return nNoDataCount;
}

SURF_FETCH_E ReliefFetch::makeReliefOf( std::string infile, std::string outfile, int nXSize, int nYSize )
{
    //TODO: check to see if relief of infile already exists
    //
    CPLSetConfigOption("GTIFF_DIRECT_IO", "YES");

    GDALResampleAlg eAlg = GRA_NearestNeighbour;
    GDALDriverH hDriver;
    GDALDatasetH hDstDS;
    GDALDatasetH hSrcDS;
    GDALDatasetH inDS;
    GDALDataType eDT;
    CPLErr eErr;

    double src_gt[6], dst_gt[6];
    const char *pszSrcWKT = NULL, *pszDstWKT = NULL;
    unsigned int nbands = 0;
    unsigned int ncols = 0, nrows = 0;
    unsigned int xsize = 0, ysize = 0;

    /*Gather the input file geotransform and projection reference string*/
    inDS = GDALOpen( infile.c_str(), GA_ReadOnly );
    if( NULL == inDS )
    {
        CPLError(CE_Failure, CPLE_AppDefined, "Could not open infile '%s' for reading, makeReliefOf() failed.", infile.c_str());
        return SURF_FETCH_E_IO_ERR;
    }
    
    GDALGetGeoTransform( inDS, dst_gt );
    xsize = GDALGetRasterXSize( inDS );
    ysize = GDALGetRasterYSize( inDS );

    /* GTiff Create() driver requires OGC WKT projection reference */
    OGRSpatialReferenceH hSrcSRS = OSRNewSpatialReference( GDALGetProjectionRef( inDS ) );
/*
** OSRFixup was removed after the 3.0.0 release.  See:
** https://trac.osgeo.org/gdal/wiki/rfc73_proj6_wkt2_srsbarn
*/
#ifdef GDAL_COMPUTE_VERSION
#if GDAL_VERSION_NUM < GDAL_COMPUTE_VERSION(3,0,0)
    OSRFixup( hSrcSRS );
#endif /* GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(1,10,0) */
#endif /* GDAL_COMPUTE_VERSION */
    OSRExportToWkt( hSrcSRS, (char**)&pszDstWKT );
    OSRDestroySpatialReference( hSrcSRS );
    GDALClose( inDS );
    if( NULL ==  pszDstWKT )
    {
        CPLError(CE_Failure, CPLE_AppDefined, "Could not get dstWKT projection information, makeReliefOf() failed.");
        return SURF_FETCH_E_IO_ERR;
    }
    /*finished with the input file */

    /*Get the final information from the source DS */
    hSrcDS = GDALOpen( path.c_str(), GA_ReadOnly );
    if(hSrcDS == NULL)
    {
        CPLError(CE_Failure, CPLE_AppDefined, "Could not open path '%s' for reading, makeReliefOf() failed.", path.c_str());
        CPLFree((void*)pszDstWKT);
        return SURF_FETCH_E_IO_ERR;
    }
    nbands = GDALGetRasterCount( hSrcDS );
    if( nbands == 0 )
    {
        CPLError(CE_Failure, CPLE_AppDefined, "Read dataset contains no bands information, makeReliefOf() failed.");
        GDALClose( hSrcDS );
        CPLFree((void*)pszDstWKT);
        return SURF_FETCH_E_IO_ERR;
    }
    eDT = GDALGetRasterDataType( GDALGetRasterBand( hSrcDS, 1 ) );
    if( GDT_Byte != eDT )
    {
        CPLError(CE_Failure, CPLE_AppDefined, "Raster band contains incompatible byte data types, makeReliefOf() failed.");
        GDALClose( hSrcDS );
        CPLFree((void*)pszDstWKT);
        return SURF_FETCH_E_IO_ERR;
    }
    pszSrcWKT = GDALGetProjectionRef( hSrcDS );
    GDALGetGeoTransform( hSrcDS, src_gt );

    /*we want to keep the pixel resolution from hSrcDS*/
    if( nXSize <= 0 || nYSize <= 0 )
    {
        ncols = int( xsize * dst_gt[1] / src_gt[1] + 0.5 ); /* ( xsize * old_pixel_width ) / new_pixel_width */
        nrows = int( ysize * fabs(dst_gt[5]) / fabs(src_gt[5]) + 0.5 );/* ( ysize * old_pixel_height ) / new_pixel_height */
        dst_gt[1] = src_gt[1]; /* pixel width */
        dst_gt[5] = src_gt[5]; /* pixel height */
    }
    else
    {
        ncols = nXSize;
        nrows = nYSize;
        dst_gt[1] = (double)xsize * (double)dst_gt[1] / (double)nXSize;
        dst_gt[5] = -dst_gt[1];
    }

    /* configure the output ds before warping */ 
    hDriver = GDALGetDriverByName("GTiff");
    hDstDS = GDALCreate(hDriver, outfile.c_str(), 
                        ncols, nrows, 
                        nbands, eDT, NULL);

    if(hDstDS == NULL)
    {
        CPLError(CE_Failure, CPLE_AppDefined, "Failed to create final output file for writing, makeReliefOf() failed.");
        GDALClose( hSrcDS );
        CPLFree((void*)pszDstWKT);
        return SURF_FETCH_E_IO_ERR;
    }

    GDALSetProjection(hDstDS, pszDstWKT);
    GDALSetGeoTransform(hDstDS, dst_gt);

    /* warp the src ds to the output ds */
    GDALWarpOptions *psWarpOptions = GDALCreateWarpOptions();

    psWarpOptions->hSrcDS = hSrcDS;
    psWarpOptions->hDstDS = hDstDS;

    psWarpOptions->nBandCount = 0;
    psWarpOptions->pTransformerArg = 
        GDALCreateGenImgProjTransformer( hSrcDS, 
                                         pszSrcWKT, 
                                         hDstDS,
                                         pszDstWKT, 
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
        CPLError(CE_Failure, CPLE_AppDefined, "Could not warp image, makeReliefOf() failed.");
        GDALClose(hDstDS);
        GDALClose(hSrcDS);
        CPLFree((void*)pszDstWKT);
        return SURF_FETCH_E_WARPER_ERR;
    }

    GDALRasterBandH hSrcBand;
    GDALRasterBandH hDstBand;

    hSrcBand = GDALGetRasterBand(hSrcDS, 1);
    hDstBand = GDALGetRasterBand(hDstDS, 1);

    double dfNoData = GDALGetRasterNoDataValue(hSrcBand, NULL);

    GDALSetRasterNoDataValue(hDstBand, dfNoData);

    double *padfScanline;
    int nPixels = GDALGetRasterXSize(hDstDS);
    int nLines = GDALGetRasterYSize(hDstDS);
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

    GDALClose(hDstDS);
    GDALClose(hSrcDS);
    CPLFree((void*)pszDstWKT);
    CPLSetConfigOption("GTIFF_DIRECT_IO", "NO");

    if(nNoDataCount > 0)
    {
        CPLError(CE_Failure, CPLE_AppDefined, "Final makeReliefOf() elevation file contains '%d' noDataValues", nNoDataCount);
    }
    return nNoDataCount;
}

