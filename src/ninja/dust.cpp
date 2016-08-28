/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja
 * Purpose:  Dust emission model
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
#include "dust.h"


Dust::Dust()        //default constructor
{

}

Dust::~Dust()      //destructor
{

}

#ifdef EMISSIONS
void Dust::MakeGrid(WindNinjaInputs &input, AsciiGrid<double> &grid)
{
    /*------------------------------------------*/
    /* Open grid as a GDAL dataset              */
    /*------------------------------------------*/
    int nXSize = grid.get_nCols();
    int nYSize = grid.get_nRows();
    
    GDALDriverH hDriver = GDALGetDriverByName( "MEM" );
        
    GDALDatasetH hMemDS = GDALCreate(hDriver, "", nXSize, nYSize, 1, GDT_Float64, NULL);
    
    double *padfScanline;
    padfScanline = new double[nXSize];
    
    double adfGeoTransform[6];
    adfGeoTransform[0] = grid.get_xllCorner();
    adfGeoTransform[1] = grid.get_cellSize();
    adfGeoTransform[2] = 0;
    adfGeoTransform[3] = grid.get_yllCorner()+(grid.get_nRows()*grid.get_cellSize());
    adfGeoTransform[4] = 0;
    adfGeoTransform[5] = -grid.get_cellSize();
        
    char* pszDstWKT = (char*)grid.prjString.c_str();
    GDALSetProjection(hMemDS, pszDstWKT);
    GDALSetGeoTransform(hMemDS, adfGeoTransform);
        
    GDALRasterBandH hBand = GDALGetRasterBand( hMemDS, 1 );
        
    GDALSetRasterNoDataValue(hBand, -9999.0);        

    for(int i=nYSize-1; i>=0; i--)
    {
        for(int j=0; j<nXSize; j++)
        {  
            padfScanline[j] = grid.get_cellValue(nYSize-1-i, j);
        }
        GDALRasterIO(hBand, GF_Write, 0, i, nXSize, 1, padfScanline, nXSize,
                     1, GDT_Float64, 0, 0);
    }
    
    /*------------------------------------------*/
    /* Get the geometry info                    */
    /*------------------------------------------*/
    
    OGRDataSourceH hOGRDS = 0;
    hOGRDS = OGROpen(input.dustFilename.c_str(), FALSE, 0);
    if(hOGRDS == NULL)
    {
        throw std::runtime_error("Could not open the fire perimeter file '" +
              input.dustFilename + "' for reading.");
    }
    OGRLayer *poLayer;
    OGRFeature *poFeature;
    OGRGeometry *poGeo;
    
    poLayer = (OGRLayer*)OGR_DS_GetLayer(hOGRDS, 0);
    poLayer->ResetReading();
    poFeature = poLayer->GetNextFeature();
    poGeo = poFeature->GetGeometryRef();
    OGRGeometryH hPolygon = (OGRGeometryH) poGeo;


    /* -------------------------------------------------------------------- */
    /*  Check for same CRS in fire perimeter and DEM files                  */
    /* -------------------------------------------------------------------- */

    char *pszSrcWKT;
    OGRSpatialReference *poSrcSRS, oDstSRS;
    poSrcSRS = poLayer->GetSpatialRef(); //shapefile CRS
    poSrcSRS->exportToWkt( &pszSrcWKT );

    //printf("CRS of DEM is:\n %s\n", pszDstWKT);
    //printf("WKT CRS of .shp is:\n %s\n", pszSrcWKT);
    
    oDstSRS.importFromWkt( &pszDstWKT );
    
    char *pszDstProj4, *pszSrcProj4;
    oDstSRS.exportToProj4( &pszDstProj4 );
    poSrcSRS->exportToProj4( &pszSrcProj4 );
    
    //printf("proj4 of .shp is:\n %s\n", pszSrcProj4);
    //printf("proj4 of dem is:\n %s\n", pszDstProj4);
    
    /* -------------------------------------------------------------------- */
    /*  If the CRSs are not equal, convert shapefile CRS to DEM CRS         */
    /* -------------------------------------------------------------------- */

    GDALTransformerFunc pfnTransformer = NULL;
    if( !EQUAL( pszSrcProj4, pszDstProj4 ) ){ //tranform shp CRS to DEM CRS
        poGeo->transformTo(&oDstSRS);
    }
     
    /* -------------------------------------------------------------------- */
    /*  Rasterize the shapefile                                             */
    /* -------------------------------------------------------------------- */
    
    int nTargetBand = 1;
    double BurnValue = 1.0;
    CPLErr eErr;
    
    eErr = GDALRasterizeGeometries(hMemDS, 1, &nTargetBand, 1, &hPolygon, pfnTransformer, NULL, &BurnValue, NULL, NULL, NULL);
    if(eErr != CE_None)
    {
        throw std::runtime_error("Error in GDALRasterizeGeometies in Dust:MakeGrid().");
    }
    
    GDAL2AsciiGrid((GDALDataset*)hMemDS, 1, grid);
    
    /* -------------------------------------------------------------------- */
    /*   clean up                                                           */
    /* -------------------------------------------------------------------- */
    
    if( hMemDS != NULL ){
        GDALClose( hMemDS );
        hMemDS = NULL;
    }
    
    OGR_DS_Destroy(hOGRDS);
}
#endif

/**
 * @brief Calculate PM10 vertical flux at each ground node
 * This calculation is done based on:
 * Fv = K*rho/g*ustar*(ustar^2 - ustar_threshold^2)
 * where K is a PM10 release factor for a given soil.
 * Units for Fv are mg/m2/s.
 * The calcluation is only done if ustar > ustar_threshold
 * at a given node.
 * @param Reference to the grid to be popoulated
 * @param Reference to grid with ustar values
 */
#ifdef EMISSIONS
void Dust::ComputePM10(AsciiGrid<double> &grid_ustar, AsciiGrid<double> &grid_dust)
{
    int i, j;
    const double ustar_threshold = 0.22;  //0.22 paper; //0.20 fall value //0.55 spring value //threshold friction velocity for burned soil
    const double K = 0.0007; //0.0066 fall value; //0.0003 spring value //PM10 release factor for burned soil, units are 1/m (0.004)
    const double rho_ = 1.164*1000000; //convert units from kg/m^3 to mg/m^3
    const double g = 9.81; //acceleration of gravity (m/s2)
    
    for(i=0;i<grid_dust.get_nRows();i++)
    {
        for(j=0;j<grid_dust.get_nCols();j++)
        {
            if(grid_dust(i,j) != grid_dust.get_noDataValue() && grid_ustar(i,j) > ustar_threshold)
            {
                grid_dust(i,j) = K*rho_/g*grid_ustar(i,j)*(grid_ustar(i,j)*grid_ustar(i,j) - ustar_threshold*ustar_threshold);
            }
            else if(grid_dust(i,j) != grid_dust.get_noDataValue() && grid_ustar(i,j) < ustar_threshold)
            {
                grid_dust(i,j) = 0.0;
            }
            else if(grid_dust(i,j) == grid_dust.get_noDataValue())
            {
                continue;
            }
            else
            {
                throw std::out_of_range("Range error in Dust::ComputePM10()");
            }
        }
    }
    //grid_dust.write_Grid("dustFileOut_", 2);
}
#endif
