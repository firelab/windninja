/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja
 * Purpose:  Executable for fetching dem data
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

#include <stdlib.h>
#include <stdio.h>
#include "fetch_factory.h"
#include "ninjaUnits.h"
#include "ninja.h"
#include "ninja_conv.h"
#include "ninja_init.h"

#ifdef GDAL_COMPUTE_VERSION
#if GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(1,10,0)
#include "ogr_api.h"
#include "ogr_geocoding.h"
#define FETCH_DEM_GEOCODE 1
#ifdef WIN32
#undef FETCH_DEM_GEOCODE
#endif /* WIN32 */
#endif /* GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(1,10,0) */
#endif /* GDAL_COMPUTE_VERSION */

#ifndef WGS_84_WKT
#define WGS_84_WKT "GEOGCS[\"WGS 84\"," \
                       "DATUM[\"WGS_1984\"," \
                           "SPHEROID[\"WGS 84\",6378137,298.257223563," \
                           "AUTHORITY[\"EPSG\",\"7030\"]]," \
                       "AUTHORITY[\"EPSG\",\"6326\"]]," \
                    "PRIMEM[\"Greenwich\",0," \
                        "AUTHORITY[\"EPSG\",\"8901\"]]," \
                    "UNIT[\"degree\",0.0174532925199433," \
                        "AUTHORITY[\"EPSG\",\"9122\"]]," \
                    "AUTHORITY[\"EPSG\",\"4326\"]]"
#endif

#ifdef _OPENMP
    omp_lock_t netCDF_lock;
#endif

void Usage()
{ 
    printf("fetch_dem [--bbox north east south west]\n"                  );
    printf("          [--point x y x_buf y_buf]\n"                       );
    printf("          [--ogr_ds filename] [--ogr_buf x_buf y_buf]\n"     );
    printf("          [--ogr_lyr lyr_name] [--ogr_where  where_clause]\n");

#ifdef FETCH_DEM_GEOCODE
    printf("          [--geocode placename x_buf y_buf]\n"               );
#endif
    printf("          [--buf_units miles/kilometers]\n"                  );
    printf("          [--out_res res]\n"                                 );
    printf("          [--src srtm"                                       );
    printf("/cop30"                                                      );
#ifdef HAVE_GMTED
    printf("/gmted"                                                      );
#endif
#ifdef WITH_LCP_CLIENT
    printf("/lcp"                                                        );
#endif
    printf("] [--fill_no_data]\n"           );
#ifdef WITH_LCP_CLIENT
    printf("          [--lcp_srs epsg_code]\n"                           );
#endif
    /*
    ** XXX: Add resampling algorithm support
    */
#ifdef TOTALLY_NOT_DEFINED
    printf("          [--resample_alg near/bilinear]\n"                  );
#endif
    printf("          dst_file\n"                                        );
    exit(1);
}

#ifdef FETCH_DEM_GEOCODE
static int GeocodePlaceName(const char *pszPlaceName, double *x, double *y)
{

    OGRGeocodingSessionH hGeoCoder = OGRGeocodeCreateSession(NULL);
    if(hGeoCoder == NULL)
    {
        *x = *y = 0;
        return 1;
    }
     OGRLayerH hLayer = OGRGeocode(hGeoCoder, pszPlaceName, NULL, NULL);
    if(hLayer == NULL)
    {
        *x = *y = 0;
        OGRGeocodeDestroySession(hGeoCoder);
        return 1;
    }
    OGRFeatureH hFeature;
    OGRGeometryH hGeometry;
    OGRGeometryH hCentroid;
    if((hFeature = OGR_L_GetNextFeature(hLayer)) == NULL)
    {
        *x = *y = 0;
        OGRGeocodeDestroySession(hGeoCoder);
        return 1;
    }

    hGeometry = OGR_F_GetGeometryRef(hFeature);
    hCentroid = OGR_G_CreateGeometry(wkbPoint);
    OGR_G_Centroid(hGeometry, hCentroid);
    *x = OGR_G_GetX(hCentroid, 0);
    *y = OGR_G_GetY(hCentroid, 0);

    OGR_G_DestroyGeometry(hGeometry);
    OGR_G_DestroyGeometry(hCentroid);
    OGRGeocodeDestroySession(hGeoCoder);
    return 0;
}
#endif /* FETCH_DEM_GEOCODE */

int main(int argc, char *argv[])
{
    NinjaInitialize();

    double adfBbox[4];
    double adfPoint[2];
    double adfBuff[2];
    const char *pszUnits = "miles";
    double dfCellSize = 90.0;
    const char *pszResample = "near";
    const char *pszSource = NULL;
    const char *pszGeocodeName = NULL;
    const char *pszOgrDataSource = NULL;
    const char *pszOgrLayer = NULL;
    const char *pszOgrWhere = NULL;
    double adfOgrBuff[2] = {0, 0};
    int bFillNoData = FALSE;
    const char *pszDstFile = NULL;
    const char *pszEpsgCode = NULL;
    int bLcpDownload = FALSE;
    char **papszOptions = NULL;

    int bHasBbox = FALSE;
    int bHasPoint = FALSE;

    int nSrtmError;
    lengthUnits::eLengthUnits units;
    GDALResampleAlg rAlg;
    GDALDataset *poDS;
    SurfaceFetch *fetch;

    /*
    ** Remove when resampling support is added.
    */
    UNREFERENCED_PARAM(rAlg);

    int i;

    /* Parse the command line arguments */
    if(argc <= 1)
    {
        Usage();
    }

    i = 1;
    while(i < argc)
    {
        if(EQUAL(argv[i], "--bbox"))
        {
            adfBbox[0] = CPLAtof(argv[++i]);
            adfBbox[1] = CPLAtof(argv[++i]);
            adfBbox[2] = CPLAtof(argv[++i]);
            adfBbox[3] = CPLAtof(argv[++i]);
            bHasBbox = TRUE;
        }
        else if(EQUAL(argv[i], "--point"))
        {
            adfPoint[0] = CPLAtof(argv[++i]);
            adfPoint[1] = CPLAtof(argv[++i]);
            adfBuff[0] = CPLAtof(argv[++i]);
            adfBuff[1] = CPLAtof(argv[++i]);
            bHasPoint = TRUE;
        }
        else if(EQUAL(argv[i], "--buf_units"))
        {
            pszUnits = argv[++i];
        }
        else if(EQUAL(argv[i], "--out_res"))
        {
            dfCellSize = CPLAtof(argv[++i]);
        }
        else if(EQUAL(argv[i], "--r"))
        {
            pszResample = argv[++i];
        }
        else if(EQUAL(argv[i], "--src"))
        {
            pszSource = argv[++i];
        }
        else if(EQUAL(argv[i], "--fill_no_data"))
        {
            bFillNoData = TRUE;
        }
#ifdef FETCH_DEM_GEOCODE
        else if(EQUAL(argv[i], "--geocode"))
        {
            pszGeocodeName = argv[++i];
            adfBuff[0] = CPLAtof(argv[++i]);
            adfBuff[1] = CPLAtof(argv[++i]);
        }
#endif
        else if(EQUAL(argv[i], "--ogr_ds"))
        {
            pszOgrDataSource = argv[++i];
        }
        else if(EQUAL(argv[i], "--ogr_buf"))
        {
            adfOgrBuff[0] = CPLAtof(argv[++i]);
            adfOgrBuff[1] = CPLAtof(argv[++i]);
        }
        else if(EQUAL(argv[i], "--ogr_lyr"))
        {
            pszOgrLayer = argv[++i];
        }
        else if(EQUAL(argv[i], "--ogr_where"))
        {
            pszOgrWhere = argv[++i];
        }
#ifdef WITH_LCP_CLIENT
        else if (EQUAL(argv[i], "--lcp_srs"))
        {
            pszEpsgCode = argv[++i];
            papszOptions = CSLAddNameValue(papszOptions, "OVERRIDE_BEST_UTM", pszEpsgCode );
        }
#endif
        else if(EQUAL(argv[i], "--h") || EQUAL(argv[i], "--help"))
        {
            Usage();
        }
        else if(pszDstFile == NULL)
        {
            pszDstFile = argv[i];
        }
        else
        {
            printf("Invalid argument: %s\n", argv[i]);
            Usage();
        }
        i++;
    }

    if(!bHasBbox && !bHasPoint && !pszGeocodeName && !pszOgrDataSource)
    {
        fprintf(stderr, "No location input specified.  "
                        "Must supply --bbox or --point or --geocode\n");
        Usage();
    }

    if(pszGeocodeName && bHasPoint)
    {
        fprintf(stderr, "Too many location inputs specified.  "
                        "Must supply -bbox or -point or -geocode\n");
        Usage();
    }
    if(pszGeocodeName)
    {

#ifdef FETCH_DEM_GEOCODE
        int nErr = GeocodePlaceName(pszGeocodeName, &adfPoint[0], &adfPoint[1]);
        if(nErr)
        {
            fprintf(stderr, "Could not geocode point for %s\n", pszGeocodeName);
            Usage();
        }
        bHasPoint = TRUE;
#else
        fprintf(stderr, "Geocoder not available in this GDAL Version.\n");
        Usage();
#endif /* FETCH_DEM_GEOCODE */
    }
    if(bHasBbox && bHasPoint)
    {
        fprintf(stderr, "Too many location inputs specified.  "
                        "Must supply --bbox or --point or --gecode\n");
        Usage();
    }

    if(EQUAL(pszUnits, "miles"))
    {
        units = lengthUnits::miles;
    }
    else if(EQUAL(pszUnits, "kilometers"))
    {
        units = lengthUnits::kilometers;
    }
    else
    {
        fprintf(stderr, "Units must be 'miles' or 'kilometers'\n");
        Usage();
    }

    if(dfCellSize <= 0.0)
    {
        fprintf(stderr, "Cell size must be positive\n");
        Usage();
    }

    if(EQUAL(pszResample, "near"))
    {
        rAlg = GRA_NearestNeighbour;
    }
    else if(EQUAL(pszResample, "bilinear"))
    {
        rAlg = GRA_Bilinear;
    }
    else
    {
        fprintf(stderr, "Resampling must be 'near' or 'bilinear'\n");
        Usage();
    }
    if(!pszSource)
    {
        fprintf(stderr, "Source must be one of 'us', 'world', ");
#ifdef HAVE_GMTED
        fprintf(stderr, "'gmted', ");
#endif
#ifdef WITH_LCP_CLIENT
        fprintf(stderr, "'lcp' ");
#endif
        fprintf(stderr, "\n");
        Usage();
    }
    if(EQUAL(pszSource, "srtm"))
    {
        fetch = FetchFactory::GetSurfaceFetch(FetchFactory::SRTM);
    }
    else if(EQUAL(pszSource, "cop30"))
    {
        fetch = FetchFactory::GetSurfaceFetch(FetchFactory::COP30);
    }
#ifdef HAVE_GMTED
    else if(EQUAL(pszSource, "gmted"))
    {
        fetch = FetchFactory::GetSurfaceFetch(FetchFactory::WORLD_GMTED);
    }
#endif
#ifdef WITH_LCP_CLIENT
    else if(EQUAL(pszSource, "lcp"))
    {
        bLcpDownload = TRUE;
        fetch = FetchFactory::GetSurfaceFetch(FetchFactory::LCP);
    }
#endif /* WITH_LCP_CLIENT */
    else
    {
    fprintf(stderr, "Source must be one of 'srtm', 'cop30'");
#ifdef HAVE_GMTED
        fprintf(stderr, ", 'gmted'");
#endif
#ifdef WITH_LCP_CLIENT
        fprintf(stderr, ", 'lcp' ");
#endif
        fprintf(stderr, "\n");
        Usage();
    }

    if(pszDstFile == NULL)
    {
        fprintf(stderr, "Must specify destination file\n");
        Usage();
    }

    if(pszOgrDataSource != NULL)
    {
        OGRDataSourceH hDS = OGROpen(pszOgrDataSource, FALSE, NULL);
        if(hDS == NULL)
        {
            fprintf(stderr, "Invalid OGR datasource\n");
            Usage();
        }
        OGRLayerH hLayer;
        if(pszOgrLayer != NULL)
        {
            hLayer = OGR_DS_GetLayerByName(hDS, pszOgrLayer);
        }
        else
        {
            hLayer = OGR_DS_GetLayer(hDS, 0);
            pszOgrLayer = OGR_L_GetName(hLayer);
        }
        if(pszOgrWhere != NULL)
        {
            OGR_L_SetAttributeFilter(hLayer, pszOgrWhere);
        }
        if(hLayer == NULL)
        {
            fprintf(stderr, "Invalid OGR layer\n");
            OGR_DS_Destroy(hDS);
            Usage();
        }
        OGREnvelope oEnvelope;
        OGRFeatureH hFeature;
        OGRGeometryH hGeometry;
        OGR_L_ResetReading(hLayer);
        hFeature = OGR_L_GetNextFeature(hLayer);
        if(hFeature == NULL)
        {
            fprintf(stderr, "Invalid OGR feature.\n");
            Usage();
        }
        hGeometry = OGR_F_GetGeometryRef(hFeature);
        if(hGeometry == NULL)
        {
            fprintf(stderr, "Invalid OGR geometry.\n");
            OGR_DS_Destroy(hDS);
            Usage();
        }
        OGRSpatialReferenceH hSRS;
        OGR_G_GetEnvelope(hGeometry, &oEnvelope);
        hSRS = OSRNewSpatialReference(WGS_84_WKT);
        OGR_G_TransformTo(hGeometry, hSRS);
        adfBbox[0] = oEnvelope.MaxY + adfOgrBuff[1];
        adfBbox[1] = oEnvelope.MaxX + adfOgrBuff[0];
        adfBbox[2] = oEnvelope.MinY - adfOgrBuff[1];
        adfBbox[3] = oEnvelope.MinX - adfOgrBuff[0];
        bHasBbox = TRUE;
        OGR_DS_Destroy(hDS);
    }
    if(!bLcpDownload && papszOptions != NULL)
    {
        fprintf(stderr, "Warning, ignoring lcp_srs flag, only valid in lcp\n");
    }
    if(bHasBbox)
    {
        if( adfBbox[0] < adfBbox[2] || adfBbox[1] < adfBbox[3] )
        {
            CPLDebug("FETCH_DEM", "BBox: %lf, %lf, %lf, %lf", adfBbox[0],
                                                              adfBbox[1],
                                                              adfBbox[2],
                                                              adfBbox[3] );
            fprintf(stderr, "Invalid bounding box, inverted values\n");
            Usage();
        }
        nSrtmError = fetch->FetchBoundingBox(adfBbox, dfCellSize, pszDstFile,
                                             papszOptions);
    }
    else
    {
        nSrtmError = fetch->FetchPoint(adfPoint, adfBuff, units, dfCellSize,
                                       pszDstFile, papszOptions);
    }

    if(nSrtmError < 0)
    {
        if(nSrtmError == SURF_FETCH_E_IO_ERR)
            fprintf(stderr, "Failed to open a dataset\n");
        else if(nSrtmError == SURF_FETCH_E_BOUNDS_ERR)
            fprintf(stderr, "Request fell out of bounds of srtm data\n");
        else if(nSrtmError == SURF_FETCH_E_WARPER_ERR)
            fprintf(stderr, "Failed to warp data\n");
        else
            fprintf(stderr, "Unknown error occurred\n");
        return nSrtmError;
    }

    if(nSrtmError > 0 && bFillNoData)
    {
        poDS = (GDALDataset*)GDALOpen(pszDstFile, GA_Update);
        if(poDS == NULL)
        {
            fprintf(stderr, "Failed to open new dataset\n");
            return 1;
        }
        nSrtmError = GDALFillBandNoData(poDS, 1, 100);
        GDALClose((GDALDatasetH)poDS);
        if(nSrtmError > 0)
        {
            fprintf(stderr, "Failed to fill no data\n");
            return 1;
        }
    }
    return 0;
}

