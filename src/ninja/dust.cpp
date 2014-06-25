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
    
    /* -------------------------------------------------------------------- */
    /*  Open ds and create copy as GTiff to support writing                 */
    /* -------------------------------------------------------------------- */
    
    grid.write_Grid("temp_burn_grid", 1);

    GDALDataset *poDS;
    GDALDataset *poSrcDS;
    
    poSrcDS = (GDALDataset*)GDALOpen("temp_burn_grid", GA_ReadOnly);
    
    GDALDriver *poDriver = GetGDALDriverManager()->GetDriverByName("GTiff");
    poDS = poDriver->CreateCopy("temp_burn_grid", poSrcDS, FALSE, NULL, NULL, NULL);

    OGRRegisterAll();
    OGRDataSource *poOGRDS;
    poOGRDS = OGRSFDriverRegistrar::Open(input.dustFilename.c_str(), FALSE);
    OGRLayer *poLayer;
    OGRFeature *poFeature;
    OGRGeometry *poGeo;
    
    poLayer = poOGRDS->GetLayer(0);
    poLayer->ResetReading();
    poFeature = poLayer->GetNextFeature();
    poGeo = poFeature->GetGeometryRef();
    OGRGeometryH hPolygon = (OGRGeometryH) poGeo;


    /* -------------------------------------------------------------------- */
    /*  Check for same CRS in fire perimeter and DEM files                  */
    /* -------------------------------------------------------------------- */

    char *pszDstWKT, *pszSrcWKT;
    OGRSpatialReference *poSrcSRS, oDstSRS;
    pszDstWKT = (char*)poDS->GetProjectionRef(); //DEM CRS
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
    
    eErr = GDALRasterizeGeometries(poDS, 1, &nTargetBand, 1, &hPolygon, pfnTransformer, NULL, &BurnValue, NULL, NULL, NULL);
    if(eErr != CE_None)
    {
        throw std::runtime_error("Error in GDALRasterizeGeometies in Dust:MakeGrid().");
    }
    
    GDAL2AsciiGrid(poDS, 1, grid);
    //grid.write_Grid("dustgridFileOut", 1);
    
    /* -------------------------------------------------------------------- */
    /*   clean up                                                           */
    /* -------------------------------------------------------------------- */
    
    GDALClose((GDALDatasetH) poDS);
    GDALClose((GDALDatasetH) poSrcDS);
    OGRDataSource::DestroyDataSource( poOGRDS );

    if("temp_burn_grid")
        VSIUnlink("temp_burn_grid");
    if ("temp_burn_grid.prj")
        VSIUnlink("temp_burn_grid.prj");

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
    const double ustar_threshold = 0.22;  //threshold friction velocity for burned soil
    const double K = 0.0007; //PM10 release factor for burned soil, units are 1/m (0.004)
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
