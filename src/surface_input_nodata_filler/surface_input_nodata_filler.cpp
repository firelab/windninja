/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja
 * Purpose:  Application for filling input surface no data values
 * Author:   Loren Atwood <loren.atwood@usda.gov>
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

#include "Elevation.h"
#include "ascii_grid.h"
#include "ninjaMathUtility.h"
#include "ninja_conv.h"
#include "ninja_init.h"

#ifdef _OPENMP
#include <omp.h>
#endif


void importElevationData(GDALDataset* poDS, Elevation* dem)
{
    int nC, nR;
    double cS, nDV;
    double xL, yL;

    double adfGeoTransform[6];

    // band 1 is always assumed to be elevation
    // assumed in meters
    dem->elevationUnits = Elevation::meters;

    // get global header info
    nC = poDS->GetRasterXSize();
    nR = poDS->GetRasterYSize();

    if( poDS->GetGeoTransform(adfGeoTransform) == CE_None )
    {
        // find corners...
        xL = adfGeoTransform[0];
        yL = adfGeoTransform[3] + (adfGeoTransform[5] * nR);

        // get cell size
        if( areEqual(abs(adfGeoTransform[1]), abs(adfGeoTransform[5]), 1000000000) )
        {
            cS = abs(adfGeoTransform[1]);
        }
        else
        {
            throw std::runtime_error("Rectangular cells were detected in your DEM. " \
                "WindNinja requires square cells (dx=dy) in the DEM.");
        }
    }
    else
    {
        throw std::runtime_error("no GeoTransform in data set");
    }

    // get band specific header info (no data)
    GDALRasterBand* poBand;
    poBand = poDS->GetRasterBand(1);

    int hasNdv = FALSE;

    nDV = poBand->GetNoDataValue(&hasNdv);
    if (hasNdv == FALSE)
        nDV = -9999.0;

    // assign values in Elevation dem from dataset
    dem->set_headerData(nC, nR, xL, yL, cS, nDV, nDV, dem->prjString);

    // read in value at i, j and set dem value
    double* padfScanline;
    padfScanline = new double[nC];

    for( int i = nR - 1; i >= 0; i-- )
    {
        poBand->RasterIO(GF_Read, 0, i, nC, 1, padfScanline, nC, 1, GDT_Float64, 0, 0);
        for( int j = 0; j < nC; j++ )
        {
            dem->set_cellValue(nR - 1 - i, j, padfScanline[j]);
        }
    }
    delete[] padfScanline;
}

void Usage()
{
    printf("surface_input_nodata_filler input_dem_file\n");
    exit(1);
}

int main(int argc, char *argv[])
{

    std::string input_dem_file = "";

    // parse input arguments
    int i = 1;
    while( i < argc )
    {
        if(EQUAL(argv[i], "--help") || EQUAL(argv[i], "--h"))
        {
            Usage();
        }
        else if( input_dem_file == "" )
        {
            input_dem_file = argv[i];
        }
        else
        {
            printf("Invalid argument: \"%s\"\n", argv[i]);
            Usage();
        }
        i++;
    }

    int isValidFile = CPLCheckForFile((char*)input_dem_file.c_str(),NULL);
    if( isValidFile != 1 )
    {
        printf("input_dem_file \"%s\" file does not exist!!\n", input_dem_file.c_str());
        Usage();
    }

    std::cout << "input_dem_file = \"" << input_dem_file.c_str() << "\"" << std::endl;


    NinjaInitialize();  // needed for GDALAllRegister()


    Elevation dem;


    GDALDataset *poDS;

    std::string GDALDriverName, GDALDriverLongName;
    char *pszPrj;

    // open GDALDataset, check
    poDS = (GDALDataset*)GDALOpen(input_dem_file.c_str(), GA_ReadOnly);
    if(poDS == NULL)
        throw std::runtime_error("Cannot open input_dem_file \"" + input_dem_file + "\"");

    // get the GDAL driver type
    GDALDriverName = poDS->GetDriver()->GetDescription();
    GDALDriverLongName = poDS->GetDriver()->GetMetadataItem(GDAL_DMD_LONGNAME);

    // check for the prj info
    if(poDS->GetProjectionRef() != NULL)
    {
        pszPrj = (char*)poDS->GetProjectionRef();
        
        // ESRI and OGC WKT are not always identical. Convert the projection string to ESRI WKT to ensure proper opening of 
        // our output products in ESRI systems. ESRI WKT should be handled by most other GIS systems as well.
        OGRSpatialReference spatial_ref;
        char* pszPrjEsri;
        spatial_ref.importFromWkt(&pszPrj);
        spatial_ref.morphToESRI();
        spatial_ref.exportToWkt(&pszPrjEsri);

        dem.set_prjString(pszPrjEsri);
    }

    importElevationData(poDS, &dem);
    //if (GDALDriverName == "LCP")
    //    importLCP(poDS);
    //else if (GDALDriverName == "GTiff")
    //    importGeoTIFF(poDS);
    //else
    //    importSingleBand(poDS);

    if(poDS)
        GDALClose((GDALDatasetH)poDS);

    // fill nodata here
    if( !dem.checkForNoDataValues() )
    {
        std::cout << "no NO_DATA values found to fill in elevation file" << std::endl;
    }
    else
    {
        std::cout << "filling NO_DATA values" << std::endl;
        #ifdef _OPENMP
        double startTime = omp_get_wtime();
        #endif
        if( !dem.fillNoDataValues(1, 99.0, dem.get_nCols()*dem.get_nRows()) )
            throw std::runtime_error("Could not fill NO_DATA values in AsciiGrid::fillNoDataValues()");
        #ifdef _OPENMP
        double endTime = omp_get_wtime();
        std::cout << "NO_DATA value filling time was " << endTime-startTime << " seconds" << std::endl;
        #endif

        // double check that the filling was successful
        if( dem.checkForNoDataValues() )
        {
            throw std::runtime_error("NO_DATA values still found in elevation file.");
        }
        if(GDALDriverName == "LCP")
        {
            dem.set_noDataValue(-9999.0);
            if( dem.checkForNoDataValues() )
            {
                throw std::runtime_error("NO_DATA values still found in elevation file.");
            }
        }

        // now edit/write/overwrite the gdal file with the ascii data
        std::cout << "overwriting input elevation file with NO_DATA filled data" << std::endl;
        GDALDataset *poDS;
        poDS = (GDALDataset*)GDALOpen(input_dem_file.c_str(), GA_Update);
        if(poDS == NULL)
        {
            throw std::runtime_error("Could not open DEM for writing");
        }

        int nXSize = poDS->GetRasterXSize();
        int nYSize = poDS->GetRasterYSize();

        GDALRasterBand *poBand = poDS->GetRasterBand(1);

        double *padfScanline;
        padfScanline = new double[nXSize];

        for(int i = nYSize-1; i >= 0; i--)
        {
            for(int j = 0; j < nXSize; j++)
            {
                padfScanline[j] = dem.get_cellValue(nYSize-1-i, j);
            }
            poBand->RasterIO(GF_Write, 0, i, nXSize, 1, padfScanline, nXSize, 1, GDT_Float64, 0, 0);
        }

        poBand->SetNoDataValue(dem.get_NoDataValue());

        delete [] padfScanline;
        GDALClose((GDALDatasetH)poDS);
    }


    return 0;
}
