
/******************************************************************************
 *
 * Project:  WindNinja
 * Purpose:  C API testing
 * Author:   Natalie Wagenbrenner <nwagenbrenner@gmail.com>
 *
 * gcc -g -Wall -o test_dem apiTestInMemoryDem.c -lninja -lgdal
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

#include "windninja.h"
 
int main()
{
    NinjaH* ninjaArmy = NULL; 
    const char * comType = "cli"; //communication type is always set to "cli"
    const int nCPUs = 1;
    char ** papszOptions = NULL;
    NinjaErr err = 0; 
    err = NinjaInit(); //must be called for any simulation
    if(err != NINJA_SUCCESS)
    {
      printf("NinjaInit: err = %d\n", err);
    }
    const char * demFile = "output.tif"; // output file name
    char * fetch_type = "gmted";
    int resolution = 30; // 30 m resolution
    double boundsBox [] = {40.07, -104.0, 40.0, -104.07}; // Bounding box (north, east, south, west)
    int err = NinjaFetchDEMBBox(boundsBox, demFile, resolution, fetch_type);
    if (err != NINJA_SUCCESS)
    {
        printf("Error in NinjaFetchDEMBBox");
    }
    /* inputs that apply to the whole army (must be the same for all ninjas) */
    const char * initializationMethod = "wxmodel";
    const char * meshChoice = "coarse";
    const char * vegetation = "grass";
    const int nLayers = 20; //layers in the mesh
    const int diurnalFlag = 0; //diurnal slope wind parameterization not used
    const char * speedUnits = "mps";
    const double height = 5.0;
    const char * heightUnits = "m";
    const char * timeZone = "America/Denver";
    int momentumFlag = 0; //we're using the conservation of mass solver
    int numNinjas = 2; //two ninjas in the ninjaArmy
    const char * wxmodel_type = "NOMADS-HRRR-CONUS-3-KM"; // specifying which weather model to fetch forecast from
    const char * forecastFileName = NinjaFetchForecast(wxmodel_type, numNinjas, demFile);
    if(forecastFileName.empty())
    {
    printf("Error in NinjaFetchForecast");
    }
    printf("Forecast file name: %s\n", forecastFileName.c_str());
    err = NinjaMakeArmy(&ninjaArmy, forecastFileName.c_str(), timeZone, momentumFlag);
    if(err != NINJA_SUCCESS)
    {
    printf("NinjaMakeArmy: err = %d\n", err);
    }
    for (unsigned int i = 0; i < numNinjas; i++)
        {
        err = NinjaSetCommunication(ninjaArmy, i, comType);
        if(err != NINJA_SUCCESS)
        {
            printf("NinjaSetCommunication: err = %d\n", err);
        }
        err = NinjaSetInitializationMethod(ninjaArmy, i, initializationMethod);
        if(err != NINJA_SUCCESS)
        {
            printf("NinjaSetInitializationMethod: err = %d\n", err);
        }
        err = NinjaSetGoogOutFlag(ninjaArmy, i, 1);
        if(err != NINJA_SUCCESS)
        {
            printf("NinjaSetGoogOutFlag: err = %d\n", err);
        }
        err = NinjaSetDem(ninjaArmy, i, demFile);
        if(err != NINJA_SUCCESS)
        {
            printf("NinjaSetDem: err = %d\n", err);
        }
        err = NinjaSetNumberCPUs(ninjaArmy, i, nCPUs);
        if(err != NINJA_SUCCESS)
        {
            printf("NinjaSetNumberCPUs: err = %d\n", err);
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
        err = NinjaSetNumVertLayers(ninjaArmy, i, nLayers);
        if(err != NINJA_SUCCESS)
        {
            printf("NinjaSetNumVertLayers: err = %d\n", err);
        }
        err = NinjaSetOutputSpeedUnits(ninjaArmy, i, speedUnits);
        if(err != NINJA_SUCCESS)
        {
            printf("NinjaSetOutputSpeedUnits: err = %d\n", err);
        }
        err = NinjaSetOutputWindHeight(ninjaArmy, i, height, heightUnits);
        if(err != NINJA_SUCCESS)
        {
            printf("NinjaSetOutputWindHeight: err = %d\n", err);
        }
        err = NinjaSetOutputPath(ninjaArmy, i, "/.");
        if(err != NINJA_SUCCESS)
        {
            printf("NinjaSetOutputPath: err = %d\n", err);
        }
        }
    /* start the runs */
    err = NinjaStartRuns(ninjaArmy, nCPUs);
    if(err != 1) //NinjaStartRuns returns 1 on success
    {
        printf("NinjaStartRuns: err = %d\n", err);
    }
    /* get the output wind speed and direction data */
    const double* outputSpeedGrid = NULL;
    const double* outputDirectionGrid = NULL;
    const char* outputGridProjection = NULL;
    const int nCols = 0;
    const int nRows = 0;
    const int nIndex = 0;
    
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
    
    const char* prj = NinjaGetOutputGridProjection(ninjaArmy, nIndex);
    const double cellSize = NinjaGetOutputGridCellSize(ninjaArmy, nIndex);
    const double xllCorner = NinjaGetOutputGridxllCorner(ninjaArmy, nIndex);
    const double yllCorner = NinjaGetOutputGridyllCorner(ninjaArmy, nIndex);
    const int nCols = NinjaGetOutputGridnCols(ninjaArmy, nIndex);
    const int nRows = NinjaGetOutputGridnRows(ninjaArmy, nIndex);
        /* clean up */
    err = NinjaDestroyArmy(ninjaArmy);
    if(err != NINJA_SUCCESS)
    {
        printf("NinjaDestroyRuns: err = %d\n", err);
    }
 
    return NINJA_SUCCESS;
}