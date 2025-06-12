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

void prepAspectForNoDataFill(AsciiGrid<double>* aspect_grid)
{
    int nRows = aspect_grid->get_nRows();
    int nCols = aspect_grid->get_nCols();

    double rawValue;
    for(int i = 0; i < nRows; i++)
    {
        for(int j = 0; j < nCols; j++)
        {
            rawValue = aspect_grid->get_cellValue(i, j);
            if( rawValue == -1 )
            {
                rawValue = 0;
                aspect_grid->set_cellValue(i, j, rawValue);
            }
        }
    }
}

void cleanAspectAfterNoDataFill(AsciiGrid<double>* aspect_grid)
{
    int nRows = aspect_grid->get_nRows();
    int nCols = aspect_grid->get_nCols();

    double rawValue;
    for(int i = 0; i < nRows; i++)
    {
        for(int j = 0; j < nCols; j++)
        {
            rawValue = aspect_grid->get_cellValue(i, j);
            if( rawValue == 360.0 )
            {
                rawValue = 0.0;
                aspect_grid->set_cellValue(i, j, rawValue);
            }
        }
    }
}

void fillAsciiNoData(AsciiGrid<double>* ascii_grid, std::string band_name, std::string fill_type, bool& isNoDataFound)
{
    if( !ascii_grid->checkForNoDataValues() )
    {
        std::cout << "no NO_DATA values found to fill in " << band_name << " band" << std::endl;
        return;
    }
    isNoDataFound = true;
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

void fillAsciiNoData(AsciiGrid<int>* ascii_grid, std::string band_name, std::string fill_type, bool& isNoDataFound)
{
    if( !ascii_grid->checkForNoDataValues() )
    {
        std::cout << "no NO_DATA values found to fill in " << band_name << " band" << std::endl;
        return;
    }
    isNoDataFound = true;
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
    printf("\n"
           "surface_input_nodata_filler [--ow/overwrite_file]\n"
           "                            [--o/output_dem_file file]\n"
           "                            [--fvb/fill_vegetation_bands bool]\n"
           "                            input_dem_file\n"
           "\n"
           "Defaults:\n"
           "    --overwrite_file \"false\" (if output_dem_file already exists, or is specified as input_dem_file (the default when no output_dem_file is specified), allows overwriting)\n"
           "    --output_dem_file \"input_dem_file\" (requires --overwrite_file=true to allow overwriting an existing file)\n"
           "    --fill_vegetation_bands \"false\"\n"
           "\n"
           "Description:\n"
           "  the script expects a WindNinja dem as input (input_dem_file), which can either be a single band of elevation data, or a GTIFF/LCP file with the following 8 bands in this specific order:\n"
           "    band 1: elevation (ELEV) land height above mean sea level, in meters\n"
           "    band 2: slope (SLPD) percent change of elevation over a specific area, in degrees, 0 to 90\n"
           "    band 3: aspect (ASP) azimuth of the sloped surfaces across a landscape, in degrees, should be 0 to 360, but is actually -1 to 359, where the NO_DATA filling replaces values of -1 and 360 with a value of 0\n"
           "    band 4: fuel model (FBFM40) Scott & Burgan Fire Behavior Fuel Models (40), in categories\n"
           "    band 5: canopy cover (CC) proportion of the forest floor covered by the vertical projection of the tree crowns, in percent, 0 to 100\n"
           "    band 6: canopy height (CH) average height of the top of the vegetated canopy, in meters * 10, 0 to > 510\n"
           "    band 7: canopy bulk density (CBD) density of available canopy fuel in a stand, in kg m-3 * 100, 0 to > 45\n"
           "    band 8: canopy base height (CBH) average height from the ground to a forest stand's canopy bottom at which there is enough forest canopy fuel to propagate fire vertically into the canopy, in meters * 10, 0 to > 100\n"
           "\n"
           "  the script writes the input_dem_file to output_dem_file as a full copy, overwriting the selected bands with NO_DATA filled values. If the output_dem_file already exists, overwrite_file must be set to \"true\" or the script will not continue. If no output_dem_file is specified, or the output_dem_file is specified to be the input_dem_file in some way, overwrite_file must be set to \"true\" or the script will not continue, because the input_dem_file would then be overwritten\n"
           "\n"
           "  the script always overwrites the 1st elevation band, but filling the other vegetation bands requires setting fill_vegetation_bands to \"true\". This is made optional because filling NO_DATA values for any single band can be quite time consuming for even moderately sized dems, let alone for 8 or more bands\n"
           "\n"
           "  NO_DATA filling for the vegetation bands can be skipped because, upon detecting NO_DATA values in an input dem, WindNinja normally fills the NO_DATA values of the elevation band the same way as this script, with significant slowdown. But WindNinja just replaces NO_DATA values with set generic values for the vegetation bands, which has no slowdown, and the slight discontinuities this introduces at the edges of the NO_DATA filled regions have much less effect on the stability of the flow solution for the vegetation bands than they do for the elevation band\n"
           "\n"
           );
    exit(1);
}

int main(int argc, char *argv[])
{

    std::string input_dem_file = "";
    std::string output_dem_file = "";
    bool overwrite_file = false;
    bool fill_vegetation_bands = false;

    // parse input arguments
    int i = 1;
    while( i < argc )
    {
        if( EQUAL(argv[i], "--help") || EQUAL(argv[i], "--h") || EQUAL(argv[i], "-help") || EQUAL(argv[i], "-h") )
        {
            Usage();
        }
        else if( EQUAL(argv[i], "--output_dem_file") || EQUAL(argv[i], "--o") || EQUAL(argv[i], "-output_dem_file") || EQUAL(argv[i], "-o") )
        {
            output_dem_file = std::string( argv[++i] );
        }
        else if( EQUAL(argv[i], "--overwrite_file") || EQUAL(argv[i], "--ow") || EQUAL(argv[i], "-overwrite_file") || EQUAL(argv[i], "-ow") )
        {
            std::string input_str = std::string( argv[++i] );
            if( EQUAL( input_str.c_str(), "true" ) || EQUAL( input_str.c_str(), "t" ) || EQUAL( input_str.c_str(), "1" ) )
            {
                overwrite_file = true;
            }
            else if ( EQUAL( input_str.c_str(), "false" ) || EQUAL( input_str.c_str(), "f" ) || EQUAL( input_str.c_str(), "0" ) )
            {
                overwrite_file = false;
            }
            else
            {
                printf("\nInvalid argument for \"--overwrite_file\": \"%s\"\n", argv[i]);
                Usage();
            }
        }
        else if( EQUAL(argv[i], "--fill_vegetation_bands") || EQUAL(argv[i], "--fvb") || EQUAL(argv[i], "-fill_vegetation_bands") || EQUAL(argv[i], "-fvb") )
        {
            std::string input_str = std::string( argv[++i] );
            if( EQUAL( input_str.c_str(), "true" ) || EQUAL( input_str.c_str(), "t" ) || EQUAL( input_str.c_str(), "1" ) )
            {
                fill_vegetation_bands = true;
            }
            else if ( EQUAL( input_str.c_str(), "false" ) || EQUAL( input_str.c_str(), "f" ) || EQUAL( input_str.c_str(), "0" ) )
            {
                fill_vegetation_bands = false;
            }
            else
            {
                printf("\nInvalid argument for \"--fill_vegetation_bands\": \"%s\"\n", argv[i]);
                Usage();
            }
        }
        else if( input_dem_file == "" )
        {
            input_dem_file = argv[i];
        }
        else
        {
            printf("\nInvalid argument: \"%s\"\n", argv[i]);
            Usage();
        }
        i++;
    }

    // check parsed inputs
    int isValidFile = CPLCheckForFile((char*)input_dem_file.c_str(),NULL);
    if( isValidFile != 1 )
    {
        printf("\ninput_dem_file \"%s\" file does not exist!!\n", input_dem_file.c_str());
        Usage();
    }
    if( output_dem_file != "" )
    {
        std::string output_dem_path = CPLGetPath(output_dem_file.c_str());
        isValidFile = CPLCheckForFile((char*)output_dem_path.c_str(),NULL);
        if( isValidFile != 1 )
        {
            printf("\noutput_dem_file \"%s\" path does not exist!!\n", output_dem_file.c_str());
            Usage();
        }
        if( EQUAL( output_dem_path.c_str(), "/" ) )
        {
            printf("\noutput_dem_file \"%s\" path \"/\" is ROOT, did you mean to specify \"./\" ??\n", output_dem_file.c_str());
            Usage();
        }
        if( EQUAL( output_dem_file.c_str(), "./" ) )
        {
            printf("\noutput_dem_file \"%s\" is NOT a valid filename!!\n", output_dem_file.c_str());
            Usage();
        }
    }

    // print parsed inputs
    //std::cout << std::endl;
    //std::cout << "input_dem_file  = \"" <<  input_dem_file.c_str() << "\"" << std::endl;
    //std::cout << "output_dem_file = \"" << output_dem_file.c_str() << "\"" << std::endl;
    //std::cout << "overwrite_file = " << overwrite_file << std::endl;
    //std::cout << "fill_vegetation_bands = " << fill_vegetation_bands << std::endl;
    //std::cout << std::endl;

    // do additional settings/checks/warnings/overrides on parsed inputs
    if( output_dem_file != "" && overwrite_file == false )
    {
        isValidFile = CPLCheckForFile((char*)output_dem_file.c_str(),NULL);
        if( isValidFile == 1 )
        {
            printf("\n!! output_dem_file \"%s\" file already exists !!\n   if you still want to run the script with these inputs, set --overwrite_file to \"true\"\n\n", output_dem_file.c_str());
            Usage();
        }
    }
    if( output_dem_file == "" )
    {
        if( overwrite_file == false )
        {
            printf("\n!! output_dem_file not specified, so it would be set to input_dem_file, input_dem_file \"%s\" file would be overwritten !!\n   if you still want to run the script with these inputs, set --overwrite_file to \"true\"\n\n", input_dem_file.c_str());
            Usage();
        }
        output_dem_file = input_dem_file;
    }

    std::cout << std::endl;  // just cleaner with an extra line break in the command line output right here

    // start the script stuff
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
    {
        throw std::runtime_error("\nCannot open input_dem_file \""+input_dem_file+"\"");
    }

    // get the GDAL driver type
    GDALDriverName = poDS->GetDriver()->GetDescription();
    GDALDriverLongName = poDS->GetDriver()->GetMetadataItem(GDAL_DMD_LONGNAME);

    // check for the prj info
    if(poDS->GetProjectionRef() == NULL)
    {
        throw std::runtime_error("\nNo projection available in input_dem_file \""+input_dem_file+"\"");
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

    bool isLcp = false;
    //if( GDALDriverName == "LCP" || GDALDriverName == "GTiff" )  // this idea doesn't work because LCP files are now GTIFFs
    if( nBands > 1 )
    {
        // assume if greater than 1 band then it is a landscape GeoTIFF!
        isLcp = true;
        // but if not enough bands, then reject the dataset
        // but we also want to reject filling if there are more than 8 bands, need the exact specific order and count if there is vegetation in the dataset
        if( nBands < 8)
        {
            throw std::runtime_error("\nToo few bands in dataset for vegetation!\nexpected 8 bands, but dataset has " + std::to_string(nBands) + " bands");
        }
        if( nBands > 8)
        {
            throw std::runtime_error("\nToo many bands in dataset for vegetation!\nexpected 8 bands, but dataset has " + std::to_string(nBands) + " bands");
        }
    }

    importElevationData(poDS, &dem);

    if( isLcp == true && fill_vegetation_bands == true )
    {
        importBandData(poDS, 2, &slope, dem);
        importBandData(poDS, 3, &aspect, dem);
        importBandData(poDS, 4, &fuelModel, dem);
        importBandData(poDS, 5, &canopyCover, dem);
        importBandData(poDS, 6, &canopyHeight, dem);
        importBandData(poDS, 7, &canopyBulkDensity, dem);
        importBandData(poDS, 8, &canopyBaseHeight, dem);
    }

    if(poDS)
    {
        GDALClose((GDALDatasetH)poDS);
    }

    // fill nodata here
    // the ascii_grid.checkForNoDataValues() check is done during the fillAsciiNoData() call, rather than doing the check manually for each band here

    bool isNoDataFound = false;

    fillAsciiNoData(&dem, "elevation", "double", isNoDataFound);
    // double check that the filling was successful
    if(GDALDriverName == "LCP")
    {
        dem.set_noDataValue(-9999.0);
        if( dem.checkForNoDataValues() )
        {
            throw std::runtime_error("NO_DATA values still found in elevation band");
        }
    }

    if( isLcp == true && fill_vegetation_bands == true )
    {
        fillAsciiNoData(&slope, "slope", "double", isNoDataFound);

        //fillAsciiNoData(&aspect, "aspect", "double", isNoDataFound);
        bool found_aspect_noData = aspect.checkForNoDataValues();
        if( !found_aspect_noData )
        {
            prepAspectForNoDataFill(&aspect);
        }
        fillAsciiNoData(&aspect, "aspect", "angle", isNoDataFound);
        if( !found_aspect_noData )
        {
            cleanAspectAfterNoDataFill(&aspect);
        }

        fillAsciiNoData(&fuelModel, "fuel model", "categorical", isNoDataFound);
        fillAsciiNoData(&canopyCover, "canopy cover", "double", isNoDataFound);
        fillAsciiNoData(&canopyHeight, "canopy height", "double", isNoDataFound);
        fillAsciiNoData(&canopyBulkDensity, "canopy bulk density", "double", isNoDataFound);
        fillAsciiNoData(&canopyBaseHeight, "canopy base height", "double", isNoDataFound);
    }

    // now edit/write/overwrite the gdal file with the ascii data
    if( isNoDataFound == true )
    {
        #ifdef _OPENMP
        double startTime = omp_get_wtime();
        #endif

        // if the output file already exists, delete it, and replace it with a copy of the input file for editing
        int nRet;
        isValidFile = CPLCheckForFile((char*)output_dem_file.c_str(),NULL);
        if( isValidFile == 1 )
        {
            // CPLCopyFile() is NOT safe for copying to the same file location,
            // but currently we CANNOT detect whether two paths/filenames are the same if they are specified with different styles (.. and . stuff vs absolute path),
            // so actually need to make the copy from an intermediate copy of the input file, before deleting the pre-existing output file,
            // just in case the output file is the same as the input file
            std::string output_dem_path = CPLGetPath(output_dem_file.c_str());
            std::string tmp_dem_file = CPLFormFilename(output_dem_path.c_str(), "tmp", "tif");
            std::cout << "\noverwriting output_dem_file \"" << output_dem_file << "\" with NO_DATA filled data" << std::endl;
            nRet = CPLCopyFile(tmp_dem_file.c_str(), input_dem_file.c_str());
            VSIUnlink( output_dem_file.c_str() );
            nRet = CPLCopyFile(output_dem_file.c_str(), tmp_dem_file.c_str());
            VSIUnlink( tmp_dem_file.c_str() );
        } else
        {
            std::cout << "\nwriting output_dem_file \"" << output_dem_file << "\" with NO_DATA filled data" << std::endl;
            nRet = CPLCopyFile(output_dem_file.c_str(), input_dem_file.c_str());
        }

        GDALDataset *poDS_out;
        poDS_out = (GDALDataset*)GDALOpen(output_dem_file.c_str(), GA_Update);
        if(poDS_out == NULL)
        {
            throw std::runtime_error("Could not open output_dem_file \""+output_dem_file+"\" for writing");
        }

        writeBandData(poDS_out, 1, &dem);

        if( isLcp == true && fill_vegetation_bands == true )
        {
            writeBandDataIntStyle(poDS_out, 2, &slope);
            writeBandDataIntStyle(poDS_out, 3, &aspect);
            writeBandData(poDS_out, 4, &fuelModel);
            writeBandDataIntStyle(poDS_out, 5, &canopyCover);
            writeBandDataIntStyle(poDS_out, 6, &canopyHeight);
            writeBandDataIntStyle(poDS_out, 7, &canopyBulkDensity);
            writeBandDataIntStyle(poDS_out, 8, &canopyBaseHeight);
        }

        GDALClose((GDALDatasetH)poDS_out);
        #ifdef _OPENMP
        double endTime = omp_get_wtime();
        std::cout << "writing output_dem_file time was " << endTime-startTime << " seconds" << std::endl;
        #endif
    }

    return 0;
}
