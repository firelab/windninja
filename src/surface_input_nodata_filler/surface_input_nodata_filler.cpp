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

void importBandData(GDALDataset* poDS, int bandNum, AsciiGrid<double>* ascii_grid, Elevation& dem)
{
    int nC, nR;
    double cS;
    double xL, yL;
    std::string prjStr;
    double nDV;
    int hasNdv;

    // get global header info
    // all these have the same value between poDS, dem, and all the other bands
    nC = poDS->GetRasterXSize();
    nR = poDS->GetRasterYSize();
    cS = dem.get_cellSize();
    xL = dem.get_xllCorner();
    yL = dem.get_yllCorner();
    prjStr = dem.prjString;

    GDALRasterBand* poBand;

    // get band specific header info (no data)
    poBand = poDS->GetRasterBand(bandNum);
    hasNdv = FALSE;
    nDV = poBand->GetNoDataValue(&hasNdv);
    if( hasNdv == FALSE )
    {
        nDV = -9999.0;
    }

    // set ascii grid info and sizes
    ascii_grid->set_headerData(nC, nR, xL, yL, cS, nDV, nDV, prjStr);

    // read in the data for the specific band into the ascii grid, one scanline at a time
    //double* panScanline = new double[nC];
    int* panScanline = new int[nC];

    for( int i = nR - 1; i >= 0; i-- )
    {
        //poBand->RasterIO(GF_Read, 0, i, nC, 1, panScanline, nC, 1, GDT_Float64, 0, 0);
        poBand->RasterIO(GF_Read, 0, i, nC, 1, panScanline, nC, 1, GDT_Int32, 0, 0);
        for( int j = 0; j < nC; j++ )
        {
            ascii_grid->set_cellValue(nR - 1 - i, j, panScanline[j]);
        }
    }
    delete[] panScanline;
}

void importBandData(GDALDataset* poDS, int bandNum, AsciiGrid<int>* ascii_grid, Elevation& dem)
{
    int nC, nR;
    double cS;
    double xL, yL;
    std::string prjStr;
    double nDV;
    int hasNdv;

    // get global header info
    // all these have the same value between poDS, dem, and all the other bands
    nC = poDS->GetRasterXSize();
    nR = poDS->GetRasterYSize();
    cS = dem.get_cellSize();
    xL = dem.get_xllCorner();
    yL = dem.get_yllCorner();
    prjStr = dem.prjString;

    GDALRasterBand* poBand;

    // get band specific header info (no data)
    poBand = poDS->GetRasterBand(bandNum);
    hasNdv = FALSE;
    nDV = poBand->GetNoDataValue(&hasNdv);
    if( hasNdv == FALSE )
    {
        nDV = -9999.0;
    }

    // set ascii grid info and sizes
    ascii_grid->set_headerData(nC, nR, xL, yL, cS, nDV, nDV, prjStr);

    // read in the data for the specific band into the ascii grid, one scanline at a time
    int* panScanline = new int[nC];

    for( int i = nR - 1; i >= 0; i-- )
    {
        poBand->RasterIO(GF_Read, 0, i, nC, 1, panScanline, nC, 1, GDT_Int32, 0, 0);
        for( int j = 0; j < nC; j++ )
        {
            ascii_grid->set_cellValue(nR - 1 - i, j, panScanline[j]);
        }
    }
    delete[] panScanline;
}

void writeBandData(GDALDataset* poDS, int bandNum, AsciiGrid<double>* ascii_grid)
{
    int nXSize = poDS->GetRasterXSize();
    int nYSize = poDS->GetRasterYSize();

    GDALRasterBand *poBand = poDS->GetRasterBand(bandNum);

    double *padfScanline;
    padfScanline = new double[nXSize];

    for(int i = nYSize-1; i >= 0; i--)
    {
        for(int j = 0; j < nXSize; j++)
        {
            padfScanline[j] = ascii_grid->get_cellValue(nYSize-1-i, j);
        }
        poBand->RasterIO(GF_Write, 0, i, nXSize, 1, padfScanline, nXSize, 1, GDT_Float64, 0, 0);
    }
    poBand->SetNoDataValue(ascii_grid->get_NoDataValue());
    delete [] padfScanline;
}

void writeBandDataIntStyle(GDALDataset* poDS, int bandNum, AsciiGrid<double>* ascii_grid)
{
    int nXSize = poDS->GetRasterXSize();
    int nYSize = poDS->GetRasterYSize();

    GDALRasterBand *poBand = poDS->GetRasterBand(bandNum);

    int *padfScanline;
    padfScanline = new int[nXSize];

    for(int i = nYSize-1; i >= 0; i--)
    {
        for(int j = 0; j < nXSize; j++)
        {
            padfScanline[j] = ascii_grid->get_cellValue(nYSize-1-i, j);
        }
        poBand->RasterIO(GF_Write, 0, i, nXSize, 1, padfScanline, nXSize, 1, GDT_Int32, 0, 0);
    }
    poBand->SetNoDataValue(ascii_grid->get_NoDataValue());
    delete [] padfScanline;
}

void writeBandData(GDALDataset* poDS, int bandNum, AsciiGrid<int>* ascii_grid)
{
    int nXSize = poDS->GetRasterXSize();
    int nYSize = poDS->GetRasterYSize();

    GDALRasterBand *poBand = poDS->GetRasterBand(bandNum);

    int *padfScanline;
    padfScanline = new int[nXSize];

    for(int i = nYSize-1; i >= 0; i--)
    {
        for(int j = 0; j < nXSize; j++)
        {
            padfScanline[j] = ascii_grid->get_cellValue(nYSize-1-i, j);
        }
        poBand->RasterIO(GF_Write, 0, i, nXSize, 1, padfScanline, nXSize, 1, GDT_Int32, 0, 0);
    }
    poBand->SetNoDataValue(ascii_grid->get_NoDataValue());
    delete [] padfScanline;
}

void fillAsciiNoData(AsciiGrid<double>* ascii_grid, std::string band_name, std::string fill_type)
{
    std::cout << "filling NO_DATA values in " << band_name << " band..." << std::endl;
    #ifdef _OPENMP
    double startTime = omp_get_wtime();
    #endif
    if( fill_type == "double" )
    {
        if( !ascii_grid->fillNoDataValues(1, 99.0, ascii_grid->get_nCols()*ascii_grid->get_nRows()) )
            throw std::runtime_error("Could not fill NO_DATA values in AsciiGrid::fillNoDataValues()");
    } else if( fill_type == "angle" )
    {
        if( !ascii_grid->fillNoDataValuesAngle(1, 99.0, ascii_grid->get_nCols()*ascii_grid->get_nRows()) )
            throw std::runtime_error("Could not fill NO_DATA values in AsciiGrid::fillNoDataValues()");
    } else if( fill_type == "categorical" )
    {
        if( !ascii_grid->fillNoDataValuesCategorical(1, 99.0, ascii_grid->get_nCols()*ascii_grid->get_nRows()) )
            throw std::runtime_error("Could not fill NO_DATA values in AsciiGrid::fillNoDataValues()");
    } else
    {
        throw std::runtime_error("fillAsciiNoData() input fill_type \""+fill_type+"\" is not a valid fill_type!!\n  valid fill_types are \"double\", \"categorical\", and \"angle\"");
    }
    #ifdef _OPENMP
    double endTime = omp_get_wtime();
    std::cout << "NO_DATA value filling time was " << endTime-startTime << " seconds" << std::endl;
    #endif
    // double check that the filling was successful
    if( ascii_grid->checkForNoDataValues() )
    {
        throw std::runtime_error("NO_DATA values still found in "+band_name+" band.");
    }
}

void fillAsciiNoData(AsciiGrid<int>* ascii_grid, std::string band_name, std::string fill_type)
{
    std::cout << "filling NO_DATA values in " << band_name << " band..." << std::endl;
    #ifdef _OPENMP
    double startTime = omp_get_wtime();
    #endif
    if( fill_type == "double" )
    {
        if( !ascii_grid->fillNoDataValues(1, 99.0, ascii_grid->get_nCols()*ascii_grid->get_nRows()) )
            throw std::runtime_error("Could not fill NO_DATA values in AsciiGrid::fillNoDataValues()");
    } else if( fill_type == "angle" )
    {
        if( !ascii_grid->fillNoDataValuesAngle(1, 99.0, ascii_grid->get_nCols()*ascii_grid->get_nRows()) )
            throw std::runtime_error("Could not fill NO_DATA values in AsciiGrid::fillNoDataValues()");
    } else if( fill_type == "categorical" )
    {
        if( !ascii_grid->fillNoDataValuesCategorical(1, 99.0, ascii_grid->get_nCols()*ascii_grid->get_nRows()) )
            throw std::runtime_error("Could not fill NO_DATA values in AsciiGrid::fillNoDataValues()");
    } else
    {
        throw std::runtime_error("fillAsciiNoData() input fill_type \""+fill_type+"\" is not a valid fill_type!!\n  valid fill_types are \"double\", \"categorical\", and \"angle\"");
    }
    #ifdef _OPENMP
    double endTime = omp_get_wtime();
    std::cout << "NO_DATA value filling time was " << endTime-startTime << " seconds" << std::endl;
    #endif
    // double check that the filling was successful
    if( ascii_grid->checkForNoDataValues() )
    {
        throw std::runtime_error("NO_DATA values still found in "+band_name+" band.");
    }
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


    Elevation dem;  // ELEV, land height above mean sea level, in meters
    AsciiGrid<double> slope;  // SLPD, percent change of elevation over a specific area, in degrees, 0 to 90
    AsciiGrid<double> aspect;  // ASP, Azimuth of the sloped surfaces across a landscape, in degrees, 0 to 360 but I see -1 to 359 in there
    AsciiGrid<int> fuelModel;  // FBFM40, Scott & Burgan Fire Behavior Fuel Models (40), categories
    AsciiGrid<double> canopyCover;  // CC, Proportion of the forest floor covered by the vertical projection of the tree crowns, in percent but kind of categorical?, 0 to 100
    AsciiGrid<double> canopyHeight;  // CH, Average height of the top of the vegetated canopy, in meters * 10, 0 to > 510
    AsciiGrid<double> canopyBulkDensity;  // CBD, Density of available canopy fuel in a stand, in kg m-3 * 100, 0 to > 45
    AsciiGrid<double> canopyBaseHeight;  // CBH, Average height from the ground to a forest stand's canopy bottom at which there is enough forest canopy fuel to propagate fire vertically into the canopy, in meters * 10, 0 to > 100

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
    if(poDS->GetProjectionRef() == NULL)
    {
        throw std::runtime_error("No projection available in input_dem_file \"" + input_dem_file + "\"");
    } else
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

    int nBands = poDS->GetRasterCount();

    importElevationData(poDS, &dem);
    //if (GDALDriverName == "LCP")
    //    importLCP(poDS);
    //else if (GDALDriverName == "GTiff")
    //    importGeoTIFF(poDS);
    //else
    //    importSingleBand(poDS);
    if( GDALDriverName == "LCP" || GDALDriverName == "GTiff" )
    {
        // assume if 8 or greater bands then it is a landscape GeoTIFF!
        // but we want to reject filling if there are more than 8 bands
        if( nBands < 8)
        {
            throw std::runtime_error("Too few bands in dataset for vegetation!\nexpected 8 bands, but dataset has " + std::to_string(nBands) + " bands");
        }
        if( nBands > 8)
        {
            throw std::runtime_error("Too many bands in dataset for GTiff!\nexpected 8 bands, but dataset has " + std::to_string(nBands) + " bands");
        }
        importBandData(poDS, 2, &slope, dem);
        importBandData(poDS, 3, &aspect, dem);
        importBandData(poDS, 4, &fuelModel, dem);
        importBandData(poDS, 5, &canopyCover, dem);
        importBandData(poDS, 6, &canopyHeight, dem);
        importBandData(poDS, 7, &canopyBulkDensity, dem);
        importBandData(poDS, 8, &canopyBaseHeight, dem);
    }

    if(poDS)
        GDALClose((GDALDatasetH)poDS);

    // fill nodata here
    if( !dem.checkForNoDataValues() )
    {
        std::cout << "no NO_DATA values found to fill in dem file elevation band" << std::endl;
    }
    else
    {
        fillAsciiNoData(&dem, "elevation", "double");
        // double check that the filling was successful
        if(GDALDriverName == "LCP")
        {
            dem.set_noDataValue(-9999.0);
            if( dem.checkForNoDataValues() )
            {
                throw std::runtime_error("NO_DATA values still found in elevation band.");
            }
        }

        if( GDALDriverName == "LCP" || GDALDriverName == "GTiff" )
        {
            fillAsciiNoData(&slope, "slope", "double");
            //fillAsciiNoData(&aspect, "aspect", "double");
            fillAsciiNoData(&aspect, "aspect", "angle");
            fillAsciiNoData(&fuelModel, "fuelModel", "categorical");
            fillAsciiNoData(&canopyCover, "canopyCover", "double");
            fillAsciiNoData(&canopyHeight, "canopyHeight", "double");
            fillAsciiNoData(&canopyBulkDensity, "canopyBulkDensity", "double");
            fillAsciiNoData(&canopyBaseHeight, "canopyBaseHeight", "double");
        }

        // now edit/write/overwrite the gdal file with the ascii data
        std::cout << "overwriting input elevation file with NO_DATA filled data" << std::endl;
        #ifdef _OPENMP
        double startTime = omp_get_wtime();
        #endif

        GDALDataset *poDS;
        poDS = (GDALDataset*)GDALOpen(input_dem_file.c_str(), GA_Update);
        if(poDS == NULL)
        {
            throw std::runtime_error("Could not open DEM for writing");
        }

        writeBandData(poDS, 1, &dem);

        if( GDALDriverName == "LCP" || GDALDriverName == "GTiff" )
        {
            writeBandDataIntStyle(poDS, 2, &slope);
            writeBandDataIntStyle(poDS, 3, &aspect);
            writeBandData(poDS, 4, &fuelModel);
            writeBandDataIntStyle(poDS, 5, &canopyCover);
            writeBandDataIntStyle(poDS, 6, &canopyHeight);
            writeBandDataIntStyle(poDS, 7, &canopyBulkDensity);
            writeBandDataIntStyle(poDS, 8, &canopyBaseHeight);
        }

        GDALClose((GDALDatasetH)poDS);
        #ifdef _OPENMP
        double endTime = omp_get_wtime();
        std::cout << "overwriting input elevation file time was " << endTime-startTime << " seconds" << std::endl;
        #endif
    }


    return 0;
}
