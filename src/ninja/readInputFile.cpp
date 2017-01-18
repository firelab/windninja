/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja
 * Purpose:  Function for importing various GDALDatasets into WindNinja
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

#include "ninja.h"

/**
 * Read in the input file.  DEM files are read in and one band is imported.
 * LCP files use elevation and fuel model information or canopy height
 * information to set roughness parameters.
 *
 * @param file name of the input file
 */
void ninja::readInputFile(std::string file)
{
    //set dem file string
    input.dem.fileName = file;
    readInputFile();
}

/**
 * Read in the input file.  DEM files are read in and one band is imported.
 * LCP files use elevation and fuel model information or canopy height
 * information to set roughness parameters.
 *
 */
void ninja::readInputFile()
{
    GDALDataset *poDataset;

    std::string GDALDriverName, GDALDriverLongName, GDALProjRef;

    std::string ext;

    //open GDALDataset, check
    poDataset = (GDALDataset*)GDALOpen(input.dem.fileName.c_str(), GA_ReadOnly);
    if(poDataset == NULL)
        throw std::runtime_error("Cannot open input file for reading in ninja::readInputFile().");

    //get the GDAL driver type.
    GDALDriverName = poDataset->GetDriver()->GetDescription();
    GDALDriverLongName = poDataset->GetDriver()->
            GetMetadataItem(GDAL_DMD_LONGNAME);

    //check for the prj info...
    if(poDataset->GetProjectionRef() != NULL)
    {
        GDALProjRef = poDataset->GetProjectionRef();
        input.dem.set_prjString(GDALProjRef);
    }

    if(GDALDriverName == "LCP")
        importLCP(poDataset);
    else
        importSingleBand(poDataset);

    if(poDataset)
        GDALClose((GDALDatasetH)poDataset);

    if( input.dem.checkForNoDataValues() )
        throw std::runtime_error("NO_DATA values found in elevation file.");
    if(GDALDriverName == "LCP") {
        input.dem.set_noDataValue(-9999.0);
        if( input.dem.checkForNoDataValues() )
            throw std::runtime_error("NO_DATA values found in elevation file.");
    }
}

/**
 * Read in an lcp file and extract elevation and fuel information
 *
 * @param poDataset source dataset
 */
void ninja::importLCP(GDALDataset *poDataset)
{
    const char *szTemp;
    int nTemp;

    int nTotalBands;
    bool hasCrownFuels;
    bool hasGroundFuels;

    //units enums
    lengthUnits::eLengthUnits elevUnit;
    coverUnits::eCoverUnits cCoverUnits;
    lengthUnits::eLengthUnits cHeightUnits;

    //header data;
    int nR, nC;
    double cS, nDV;
    double xL = 0;
    double yL = 0;

    //get the size of the raster data
    nR = poDataset->GetRasterYSize();
    nC = poDataset->GetRasterXSize();

    double adfGeoTransform[6];
    if(poDataset->GetGeoTransform(adfGeoTransform) != CE_None)
	throw std::runtime_error("No geometric transformation could be " \
				 "determined for the .lcp file in " \
				 "ninja::importLCP().");
    else
    {
        //find corners...
        xL = adfGeoTransform[0];
        yL = adfGeoTransform[3] + (adfGeoTransform[5] * nR);
        cS = abs(adfGeoTransform[1]);
    }

    //check raster count to see what bands we do have
    nTotalBands = poDataset->GetRasterCount();
    if(nTotalBands == 10)
    {
        hasCrownFuels = true;
        hasGroundFuels = true;
    }
    else if(nTotalBands == 8) 
    {
        hasCrownFuels = true;
        hasGroundFuels = false;
    }
    else if(nTotalBands == 7)
    {
        hasCrownFuels = false;
        hasGroundFuels = false;
    }
    else if(nTotalBands == 5)
    {
        hasCrownFuels = false;
        hasGroundFuels = false;
    }

    //create band to access input data
    GDALRasterBand *poBand = NULL;

    //read the elevation band and write it to the Elevation class
    poBand = poDataset->GetRasterBand(1);

    int *hasNdv = NULL;
    nDV = poBand->GetNoDataValue(hasNdv);
    if(hasNdv == false)
        nDV = -9999.0;

    szTemp = poBand->GetMetadataItem("ELEVATION_UNIT");
    nTemp = atoi(szTemp);
    //set elevUnit to Jason's enum
    /*
      enum eDistanceUnits{
      feet,
      meters,
      miles,
      kilometers,
      feetTimesTen,
      metersTimesTen
      };
    */

    if(nTemp == 0)
        elevUnit = lengthUnits::meters;
    else if(nTemp == 1)
        elevUnit = lengthUnits::feet;

    //sets poData size too.
    input.dem.set_headerData(nC, nR, xL, yL, cS, nDV, nDV, input.dem.prjString);

    //read in value at i, j and set dem value.
    double *padfScanline;
    padfScanline = new double[nC];
    for(int i = nR - 1;i >= 0;i--) 
    {
        poBand->RasterIO(GF_Read, 0, i, nC, 1, padfScanline, nC, 1,
                 GDT_Float64, 0, 0);
        for(int j = 0;j < nC;j++)
        {
            input.dem.set_cellValue(nR - 1 - i, j, padfScanline[j]);
        }
    }

    //canopy cover, 0 = categories (0-4), 1 = percent
    poBand = poDataset->GetRasterBand(5);
    szTemp = poBand->GetMetadataItem("CANOPY_COV_UNIT");
    nTemp = atoi(szTemp);
    //convert value to Jason's enum for crown cover
    /*
      enum eCoverUnits{
      fraction,
      percent,
      canopyCategories
      };
    */
    if(nTemp == 0)
        cCoverUnits = coverUnits::canopyCategories;
    else if(nTemp == 1)
        cCoverUnits = coverUnits::percent;
    else
        cCoverUnits = coverUnits::percent;

    //canopy height units, 1 = meters, 2 = feet, 3 = meters x 10, 4 = feet x 10;
    if(hasCrownFuels) 
    {
        poBand = poDataset->GetRasterBand(6);
        szTemp = poBand->GetMetadataItem("CANOPY_HT_UNIT");
        nTemp = atoi(szTemp);
        switch(nTemp)
        {
        case 0:
            cHeightUnits = lengthUnits::meters;
            break;
        case 1:
            cHeightUnits = lengthUnits::feet;
            break;
        case 3:
            cHeightUnits = lengthUnits::metersTimesTen;
            break;
        case 4:
            cHeightUnits = lengthUnits::feetTimesTen;
            break;
        default:
            cHeightUnits = lengthUnits::meters;
        }
    }
    else
    {
        cHeightUnits = lengthUnits::meters;
    }

    setSurfaceGrids();

    //set fuel bed depth units
    lengthUnits::eLengthUnits fDepthUnits = lengthUnits::meters;

    //read in data for the other bands, on scanline at a time, and set rough
    int *panScanlineFuelM = new int [nC];
    int *panScanlineCanopyH = new int [nC];
    int *panScanlineCanopyC = new int [nC];

    for(int i = nR - 1;i >= 0;i--)
    {
        //get fuel model band and write scanline
        poBand = poDataset->GetRasterBand(4);
        poBand->RasterIO(GF_Read, 0, i, nC, 1, panScanlineFuelM, nC, 1,
                 GDT_Int32, 0, 0);
        //if canopy fuels are there, get the associated data
        if(hasCrownFuels)
        {
            //height
            poBand = poDataset->GetRasterBand(6);
            poBand->RasterIO(GF_Read, 0, i, nC, 1, panScanlineCanopyH, nC, 1,
                     GDT_Int32, 0, 0);
        }
        //cover
        poBand = poDataset->GetRasterBand(5);
        poBand->RasterIO(GF_Read, 0, i, nC, 1, panScanlineCanopyC, nC, 1,
                             GDT_Int32, 0, 0);

        int nHeight;
        for(int j = 0;j < nC;j++)
        {
            //set the roughness/diurnal stuff for diurnal
            if(hasCrownFuels)
            {
                nHeight = panScanlineCanopyH[j];
            }
            else
            {
                nHeight = 15;
            }
            computeSurfPropForCell(i, j, nHeight,
                                   cHeightUnits,
                                   (double) (panScanlineCanopyC[j]),
                                   cCoverUnits,
                                   panScanlineFuelM[j],
                                   getFuelBedDepth(panScanlineFuelM[j]),
                                   fDepthUnits);
        }
    }

    /*
     * Cleanup
     */
    delete[] padfScanline;
    delete[] panScanlineFuelM;
    delete[] panScanlineCanopyH;
    delete[] panScanlineCanopyC;
}

/**
 * Import elevation data from a single band input file
 *
 * @param poDataset source dataset
 */
void ninja::importSingleBand(GDALDataset *poDataset)
{
    int nC, nR;
    double cS, nDV;
    double xL, yL;

    double adfGeoTransform[6];

    //assumed in meters
    input.dem.elevationUnits = Elevation::meters;

    //get global header info
    nC = poDataset->GetRasterXSize();
    nR = poDataset->GetRasterYSize();

    if(poDataset->GetGeoTransform(adfGeoTransform) == CE_None) {
	//find corners...
	xL = adfGeoTransform[0];
	yL = adfGeoTransform[3] + (adfGeoTransform[5] * nR);

    //get cell size
    if(areEqual(abs(adfGeoTransform[1]), abs(adfGeoTransform[5]), 100000))
	    cS = abs(adfGeoTransform[1]);
	else
            throw std::runtime_error("Rectangular cells were detected in your DEM. WindNinja requires " \
                                 "square cells (dx=dy) in the DEM.");
    }

    //get band specific header info (no data)
    GDALRasterBand *poBand;
    poBand = poDataset->GetRasterBand(1);

    int hasNdv = FALSE;

    nDV = poBand->GetNoDataValue(&hasNdv);
    if(hasNdv == FALSE)
        nDV = -9999.0;

    //assign values in Elevation dem from dataset
    input.dem.set_headerData(nC, nR, xL, yL, cS, nDV, nDV, input.dem.prjString);

    //read in value at i, j and set dem value.
    double *padfScanline;
    padfScanline = new double[nC];

    for(int i = nR - 1;i >= 0;i--) {
	poBand->RasterIO(GF_Read, 0, i, nC, 1, padfScanline, nC, 1,
			 GDT_Float64, 0, 0);
	for(int j = 0;j < nC;j++) {
	    input.dem.set_cellValue(nR - 1 - i, j, padfScanline[j]);
	}
    }
    delete[] padfScanline;
}

/**
 * Set surface information based on lcp data
 *
 */
void ninja::setSurfaceGrids()
{
    input.surface.RoughnessUnits = lengthUnits::meters;
    input.surface.Rough_hUnits = lengthUnits::meters;
    input.surface.Rough_dUnits = lengthUnits::meters;

    input.surface.Roughness.set_headerData(input.dem);
    input.surface.Rough_h.set_headerData(input.dem);
    input.surface.Rough_d.set_headerData(input.dem);
    input.surface.Albedo.set_headerData(input.dem);
    input.surface.Bowen.set_headerData(input.dem);
    input.surface.Cg.set_headerData(input.dem);
    input.surface.Anthropogenic.set_headerData(input.dem);
}

