
/******************************************************************************
 *
 * Project:  WindNinja
 * Purpose:  C API testing
 * Author:   Natalie Wagenbrenner <nwagenbrenner@gmail.com>
 *
 * g++ -g -Wall -o test_dem apiTestInMemoryDem.c -lninja -lgdal
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

#include "windninja.h"
#include <stdio.h>

#include "gdal.h"
#include "cpl_conv.h"

int main()
{
    NinjaH* ninjaArmy = NULL; 
    int numNinjas = 2;
    const char * comType = "cli";
    char ** papszOptions = NULL;
    NinjaErr err = 0; 

    /* inputs that apply to the whole army (must be the same for all ninjas) */
    const int nCPUs = 1;
    int momentumFlag = 0;
    const char * demFile = "/home/natalie/src/windninja/api_testing/big_butte_small.tif";
    const char * outputPath = "/home/natalie/src/windninja/api_testing/";
    const char * initializationMethod = "domain_average";
    const int diurnalFlag = 0;
    const char * meshChoice = "coarse";
    const char * vegetation = "grass";
    const int nLayers = 20; //layers in the mesh
    const int meshCount = 100000; //number of cells in the mesh (cfd runs only)

    const char* speedUnits = "mps";
    const double height = 5.0;
    const char* heightUnits = "m";

    const int googOutFlag = 1;

    const double* outputSpeedGrid = NULL;
    const double* outputDirectionGrid = NULL;
    const char* outputGridProjection = NULL;
    const int nIndex = 0;

    /* inputs that can vary among ninjas in an army */
    const double speed[2] = {5.5, 5.5};
    const double direction[2] = {220, 300};

    /* output grid data */
    double cellSize;
    double xllCorner;
    double yllCorner;
    int nCols;
    int nRows;

    /* for setting the in-memory DEM */
    GDALDatasetH  hDataset;
    int nXSize;
    int nYSize;
    double adfGeoTransform[6];
    const char* prj = NULL;
    double adfCorner[2];
    double* padfScanline;

    double* demValues = NULL;

    /*-------------set an in-memory DEM-------------*/
    GDALAllRegister();
    hDataset = GDALOpen( demFile, GA_ReadOnly );
    nXSize = GDALGetRasterXSize( hDataset );
    nYSize = GDALGetRasterYSize( hDataset );
    GDALGetGeoTransform( hDataset, adfGeoTransform );
    prj = GDALGetProjectionRef( hDataset );

    //find llcorner
    adfCorner[0] = adfGeoTransform[0];
    adfCorner[1] = adfGeoTransform[3] + ( nYSize * adfGeoTransform[5] );

    printf("llcorner = %f, %f\n", adfCorner[0], adfCorner[1]);
    printf("prj = %s\n", prj);

    GDALRasterBandH hBand;
    hBand = GDALGetRasterBand( hDataset, 1 );
    
    padfScanline = (double *) CPLMalloc(sizeof(double)*nXSize);
    demValues = new double[nXSize * nYSize];

    for(int i = nYSize - 1; i >= 0; i--)
    {
        GDALRasterIO( hBand, GF_Read, 0, i, nXSize, 1,
                      padfScanline, nXSize, 1, GDT_Float64,
                      0, 0 );

	for(int j = 0; j < nXSize; j++)
        {
            demValues[ j + nXSize*(nYSize-i-1) ] = padfScanline[j];
	}
    }

    printf("demValues = %f, %f, %f\n", demValues[0], demValues[1], demValues[300]);
    printf("nXSize, nYSize = %d, %d\n", nXSize, nYSize);

    /*-------------create the army-------------*/
    ninjaArmy = NinjaCreateArmy(numNinjas, momentumFlag, papszOptions);
    if( NULL == ninjaArmy )
    {
      printf("NinjaCreateArmy: ninjaArmy = NULL\n");
    }

    err = NinjaInit();
    if(err != NINJA_SUCCESS)
    {
      printf("NinjaInit: err = %d\n", err);
    }

    /*-------------set up the runs-------------*/
    for(int i=0; i<numNinjas; i++)
    {
        err = NinjaSetCommunication(ninjaArmy, i, comType);
        if(err != NINJA_SUCCESS)
        {
          printf("NinjaSetCommunication: err = %d\n", err);
        }

        err = NinjaSetNumberCPUs(ninjaArmy, i, nCPUs);
        if(err != NINJA_SUCCESS)
        {
          printf("NinjaSetNumberCPUs: err = %d\n", err);
        }

        err = NinjaSetInitializationMethod(ninjaArmy, i, initializationMethod);
        if(err != NINJA_SUCCESS)
        {
          printf("NinjaSetInitializationMethod: err = %d\n", err);
        }

        //outputPath must be set if writing output to disk and using an in-memory DEM
        err = NinjaSetOutputPath(ninjaArmy, i, outputPath);
        if(err != NINJA_SUCCESS)
        {
          printf("NinjaSetOutputPath: err = %d\n", err);
        }

        err = NinjaSetInMemoryDem(ninjaArmy, i, demValues, nXSize, nYSize, adfGeoTransform, prj);
        if(err != NINJA_SUCCESS)
        {
          printf("NinjaSetInMemoryDem: err = %d\n", err);
        }
//        err = NinjaSetDem(ninjaArmy, i, demFile);
//        if(err != NINJA_SUCCESS)
//        {
//          printf("NinjaSetDem: err = %d\n", err);
//        }

//        err = NinjaSetPosition(ninjaArmy, i);
//        if(err != NINJA_SUCCESS)
//        {
//          printf("NinjaSetPosition: err = %d\n", err);
//        }

        err = NinjaSetInputSpeed(ninjaArmy, i, speed[i], speedUnits);
        if(err != NINJA_SUCCESS)
        {
          printf("NinjaSetInputSpeed: err = %d\n", err);
        }

        err = NinjaSetInputDirection(ninjaArmy, i, direction[i]);
        if(err != NINJA_SUCCESS)
        {
          printf("NinjaSetInputDirection: err = %d\n", err);
        }

        err = NinjaSetInputWindHeight(ninjaArmy, i, height, heightUnits);
        if(err != NINJA_SUCCESS)
        {
          printf("NinjaSetInputWindHeight: err = %d\n", err);
        }

        err = NinjaSetOutputWindHeight(ninjaArmy, i, height, heightUnits);
        if(err != NINJA_SUCCESS)
        {
          printf("NinjaSetOutputWindHeight: err = %d\n", err);
        }

        err = NinjaSetOutputSpeedUnits(ninjaArmy, i, speedUnits);
        if(err != NINJA_SUCCESS)
        {
          printf("NinjaSetOutputSpeedUnits: err = %d\n", err);
        }

        err = NinjaSetDiurnalWinds(ninjaArmy, i, diurnalFlag);
        if(err != NINJA_SUCCESS)
        {
          printf("NinjaSetDiurnalWinds: err = %d\n", err);
        }

        err = NinjaSetUniVegetation(ninjaArmy, i, vegetation);
        if(err != NINJA_SUCCESS)
        {
          printf("NinjaSetUniVegetation: err = %d\n", err);
        }

        err = NinjaSetMeshResolutionChoice(ninjaArmy, i, meshChoice);
        if(err != NINJA_SUCCESS)
        {
            printf("NinjaSetMeshResolutionChoice: err = %d\n", err);
        }

        err = NinjaSetGoogOutFlag(ninjaArmy, i, googOutFlag);
        if(err != NINJA_SUCCESS)
        {
            printf("NinjaSetGoogOutFlag: err = %d\n", err);
        }

        err = NinjaSetNumVertLayers(ninjaArmy, i, nLayers);
        if(err != NINJA_SUCCESS)
        {
            printf("NinjaSetNumVertLayers: err = %d\n", err);
        }
    }

    /*-------------start the runs-------------*/
    err = NinjaStartRuns(ninjaArmy, nCPUs);
    if(err != 1) //NinjaStartRuns returns 1 on success
    {
        printf("NinjaStartRuns: err = %d\n", err);
    }

    /*-------------get the output wind speed and direction data-------------*/
    outputSpeedGrid = NinjaGetOutputSpeedGrid(ninjaArmy, nIndex);
    if( NULL == outputSpeedGrid )
    {
        printf("Error in NinjaGetOutputSpeedGrid");
    }

    outputDirectionGrid = NinjaGetOutputDirectionGrid(ninjaArmy, nIndex);
    if( NULL == outputDirectionGrid )
    {
        printf("Error in NinjaGetOutputDirectionGrid");
    }

    outputGridProjection = NinjaGetOutputGridProjection(ninjaArmy, nIndex);
    if( NULL == outputGridProjection )
    {
        printf("Error in NinjaGetOutputGridProjection");
    }

    prj = NinjaGetOutputGridProjection(ninjaArmy, nIndex);
    cellSize = NinjaGetOutputGridCellSize(ninjaArmy, nIndex);
    xllCorner = NinjaGetOutputGridxllCorner(ninjaArmy, nIndex);
    yllCorner = NinjaGetOutputGridyllCorner(ninjaArmy, nIndex);
    nCols = NinjaGetOutputGridnCols(ninjaArmy, nIndex);
    nRows = NinjaGetOutputGridnRows(ninjaArmy, nIndex);

    printf("outputspeed[0] = %f\n", outputSpeedGrid[0]);
    printf("cellSize = %f\n", cellSize);
    printf("xllCorner = %f\n", xllCorner);
    printf("yllCorner = %f\n", yllCorner);
    printf("nCols = %d\n", nCols);
    printf("nRows = %d\n", nRows);

    /*-------------clean up-------------*/
    err = NinjaDestroyArmy(ninjaArmy);
    if(err != NINJA_SUCCESS)
    {
        printf("NinjaDestroyRuns: err = %d\n", err);
    }

    delete [] demValues;
    demValues = NULL;
    CPLFree( padfScanline );
    GDALClose( hDataset );

    return NINJA_SUCCESS;
}
