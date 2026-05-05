/******************************************************************************
 *
 * Project:  WindNinja
 * Purpose:  C API testing
 * Author:   Nicholas Kim <kim.n.j@wustl.edu>
 *
 * gcc -g -Wall -o test_capi_output test_capi_output.c -lninja
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
#include <stdio.h> //for printf, FILE, fopen, fclose
#include <stdbool.h>

#define MAX_PATH_LEN 512

void printArray(const double* array, int size, const char* label) {
    if (array == NULL) {
        printf("Error: %s array is NULL\n", label);
        return;
    }

    printf("%s:\n", label);
    for (int i = 0; i < size; i++) {
        printf("%6.2f ", array[i]);
    }
    printf("\n");
}

int main()
{
    /*
     * Setting up the simulation
     */
    NinjaArmyH* ninjaArmy = NULL;
    const int nCPUs = 1;
    const char * runType = "C-API autotest";
    char ** papszOptions = NULL;
    NinjaErr err = 0;
    err = NinjaInit(runType, papszOptions); //initialize global singletons and environments (GDAL_DATA, etc.)
    if(err != NINJA_SUCCESS)
    {
        printf("NinjaInit: err = %d\n", err);
    }

    // manually set your wnDataPath (makes it easier for setting paths and testing)
    // must replace the "~/" part with your exact path
    const char* wnDataPath = "~/src/wind/windninja/data";

    /*
     * Setting up a log file, for ninjaCom, if desired
     */
    char multiStreamFilename[MAX_PATH_LEN];
    snprintf(multiStreamFilename, sizeof(multiStreamFilename), "%s%s", wnDataPath, "/../autotest/api/data/ninja.log");
    FILE* multiStream = fopen(multiStreamFilename, "w+");
    if(multiStream == NULL)
    {
        printf("error opening log file\n");
    }

    /*
     * Set up domain average run
     */
    /* inputs that do not vary among ninjas in an army */
    char demFile[MAX_PATH_LEN];
    snprintf(demFile, sizeof(demFile), "%s%s", wnDataPath, "/../autotest/api/data/missoula_valley.tif");
    const char * initializationMethod = "domain_average";
    const char * meshChoice = "coarse";
    const char * vegetation = "brush";
    const int nLayers = 20; //layers in the mesh
    const int diurnalFlag = 0; //set to 1 to use the diurnal slope wind parameterization inputs
    const int stabilityFlag = 0; //set to 1 to use the stability wind parameterization inputs. NOT to be used with a momentum solver run.
    const double height = 10.0;
    const char * heightUnits = "m";
    bool momentumFlag = 0; //we're using the conservation of mass solver
    //bool momentumFlag = 1; //we're using the conservation of momentum solver
    unsigned int numNinjas = 2; //two ninjas in the ninjaArmy - must be equal to array sizes

    //const int nIters = -1.0;  //set to value > 0.0 to override the default value of 1000. Used only by the conservation of momentum solver.
    const int nIters = 300;  // the cli and the GUI use a value of 300 instead of the default value of 1000.

    const double meshResolution = -1.0;  //set to value > 0.0 to override meshChoice with meshResolution value. Used only by the conservation of momentum solver.
    //const double meshResolution = 300.0;
    const char * meshResolutionUnits = "m";

    bool matchedPoints = true;  // for point initialization, but currently required as an input to SetInitializationMethod(). Should the match points pointInitialization algorythm be run, or should it just run as a domainAvgRun on the input wind field. ALWAYS set to true unless you know what you are doing.

    /* inputs that can vary among ninjas in an army */
    const double speedList[] = {5.5, 5.5};
    const char * speedUnits = "mps";
    const double directionList[] = {220, 300};

    // thermal parameterization inputs (diurnal and stability)
    const int year[2] = {2023, 2023};
    const int month[2] = {10, 11};
    const int day[2] = {10, 11};
    const int hour[2] = {12, 13};
    const int minute[2] = {30, 31};
    const char * timezone = "UTC";
    const double air[2] = {50.0, 50.0};
    const char * airUnits = "F";
    const double cloud[2] = {10.0, 10.0};
    const char * cloudUnits = "percent";

    /* inputs specific to output
     * Note: Outputs have default values if inputs are not specified (For example, resolution will default to the mesh resolution)
     */
    const double outputResolution = 100.0;
    const char * units = "m";
    const double clipAmount = 0.0;  // use values between 0.0 and 50.0
    //const double clipAmount = 20.0;
    const double lineWidth = 1.0;
    //// some google earth specific variables
    const char * scaling = "equal_interval";  // equivalent to "Uniform Range" in the gui
    //const char * scaling = "equal_color";  // equivalent to "Equal Count" in the gui, the default
    const char * colorScheme = "default";  // default (ROYGB), oranges, blues, greens, pinks, magic_beans, pink_to_green, ROPGW
    const bool scaleVectors = false;  // enable vector scaling based on wind speed
    const bool useConsistentColorScale = false;  // google earth, use the same color scale across multi-runs
    //// some pdf specific variables
    const int baseMap = 0; // 0 is "topofire", 1 is "hillshade". the connection goes down a lot for topofire, though it looks a lot better
    const double pdfHeight = 8.5;  // inches
    const double pdfWidth = 11.0;  // inches
    const double pdfDpi = 150;
    char outputPath[MAX_PATH_LEN];
    snprintf(outputPath, sizeof(outputPath), "%s%s", wnDataPath, "/../autotest/api/data/output");
    const bool outputFlag = 1;

    /*
     * Initialize the army
     */
    ninjaArmy = NinjaInitializeArmy();
    if( NULL == ninjaArmy )
    {
        printf("NinjaInitializeArmy: ninjaArmy = NULL\n");
    }

    /*
     * Customize the ninja communication
     */
    err = NinjaSetArmyMultiComStream(ninjaArmy, multiStream, papszOptions);
    if(err != NINJA_SUCCESS)
    {
        printf("NinjaSetArmyMultiComStream: err = %d\n", err);
    }

    /*
     * Make the army
     */
    if(diurnalFlag == 1 || stabilityFlag == 1)
    {
        err = NinjaMakeDomainAverageArmyThermalParameterization(ninjaArmy, numNinjas, momentumFlag, speedList, speedUnits, directionList, year, month, day, hour, minute, timezone, air, airUnits, cloud, cloudUnits, papszOptions);
    }
    else
    {
        err = NinjaMakeDomainAverageArmy(ninjaArmy, numNinjas, momentumFlag, speedList, speedUnits, directionList, papszOptions);
    }
    if(err != NINJA_SUCCESS)
    {
        printf("NinjaMakeDomainAverageArmy: err = %d\n", err);
    }

    /*
     * Prepare the army
     */
    // farsite atm file only works if very specific units. outputs in mph at 20 feet, or outputs in kph at 10 meters.
    //err = NinjaSetAsciiAtmFile(ninjaArmy, outputFlag, papszOptions);
    //if(err != NINJA_SUCCESS)
    //{
    //    printf("NinjaSetAsciiAtmFile: err = %d\n", err);
    //}
    for(unsigned int i=0; i<numNinjas; i++)
    {
      /*
       * Sets Simulation Variables
       */
      err = NinjaSetNumberCPUs(ninjaArmy, i, nCPUs, papszOptions);
      if(err != NINJA_SUCCESS)
      {
        printf("NinjaSetNumberCPUs: err = %d\n", err);
      }
    
      err = NinjaSetInitializationMethod(ninjaArmy, i, initializationMethod, matchedPoints, papszOptions);
      if(err != NINJA_SUCCESS)
      {
        printf("NinjaSetInitializationMethod: err = %d\n", err);
      }
    
      err = NinjaSetDem(ninjaArmy, i, demFile, papszOptions);
      if(err != NINJA_SUCCESS)
      {
        printf("NinjaSetDem: err = %d\n", err);
      }
    
      err = NinjaSetPosition(ninjaArmy, i, papszOptions);
      if(err != NINJA_SUCCESS)
      {
        printf("NinjaSetPosition: err = %d\n", err);
      }

      err = NinjaSetInputWindHeight(ninjaArmy, i, height, heightUnits, papszOptions);
      if(err != NINJA_SUCCESS)
      {
        printf("NinjaSetInputWindHeight: err = %d\n", err);
      }

      err = NinjaSetDiurnalWinds(ninjaArmy, i, diurnalFlag, papszOptions);
      if(err != NINJA_SUCCESS)
      {
        printf("NinjaSetDiurnalWinds: err = %d\n", err);
      }

      err = NinjaSetStabilityFlag(ninjaArmy, i, stabilityFlag, papszOptions);
      if(err != NINJA_SUCCESS)
      {
        printf("NinjaSetStabilityFlag: err = %d\n", err);
      }

      err = NinjaSetUniVegetation(ninjaArmy, i, vegetation, papszOptions);
      if(err != NINJA_SUCCESS)
      {
        printf("NinjaSetUniVegetation: err = %d\n", err);
      }

      if(nIters > 0.0)
      {
        err = NinjaSetNumberOfIterations(ninjaArmy, i, nIters, papszOptions);
        if(err != NINJA_SUCCESS)
        {
          printf("NinjaSetNumberOfIterations: err = %d\n", err);
        }
      }

      if(meshResolution > 0.0)
      {
        err = NinjaSetMeshResolution(ninjaArmy, i, meshResolution, meshResolutionUnits, papszOptions);
        if(err != NINJA_SUCCESS)
        {
          printf("NinjaSetMeshResolution: err = %d\n", err);
        }
      }
      else  // meshResolution not set, use meshChoice
      {
        err = NinjaSetMeshResolutionChoice(ninjaArmy, i, meshChoice, papszOptions);
        if(err != NINJA_SUCCESS)
        {
          printf("NinjaSetMeshResolutionChoice: err = %d\n", err);
        }
      }

      err = NinjaSetNumVertLayers(ninjaArmy, i, nLayers, papszOptions);
      if(err != NINJA_SUCCESS)
      {
          printf("NinjaSetNumVertLayers: err = %d\n", err);
      }

      /*
       * Sets Output Variables
       */
      err = NinjaSetOutputPath(ninjaArmy, i, outputPath, papszOptions);
      if(err != NINJA_SUCCESS)
      {
          printf("NinjaSetOutputPath: err = %d\n", err);
      }

      err = NinjaSetOutputWindHeight(ninjaArmy, i, height, heightUnits, papszOptions);
      if(err != NINJA_SUCCESS)
      {
        printf("NinjaSetOutputWindHeight: err = %d\n", err);
      }

      err = NinjaSetOutputSpeedUnits(ninjaArmy, i, speedUnits, papszOptions);
      if(err != NINJA_SUCCESS)
      {
        printf("NinjaSetOutputSpeedUnits: err = %d\n", err);
      }

      err = NinjaSetOutputBufferClipping(ninjaArmy, i, clipAmount, papszOptions);
      if(err != NINJA_SUCCESS)
      {
        printf("NinjaSetOutputBufferClipping: err = %d\n", err);
      }

      /* Google output (.kmz) */
      err = NinjaSetGoogOutFlag(ninjaArmy, i, outputFlag, papszOptions);
      if(err != NINJA_SUCCESS)
      {
          printf("NinjaSetGoogOutFlag: err = %d\n", err);
      }

      err = NinjaSetGoogResolution(ninjaArmy, i, outputResolution, units, papszOptions);
      if(err != NINJA_SUCCESS)
      {
          printf("NinjaSetGoogResolution: err = %d\n", err);
      }

      err = NinjaSetGoogSpeedScaling(ninjaArmy, i, scaling, papszOptions);
      if(err != NINJA_SUCCESS)
      {
          printf("NinjaSetGoogSpeedScaling: err = %d\n", err);
      }

      err = NinjaSetGoogLineWidth(ninjaArmy, i, lineWidth, papszOptions);
      if(err != NINJA_SUCCESS)
      {
          printf("NinjaSetGoogLineWidth: err = %d\n", err);
      }

      err = NinjaSetGoogColor(ninjaArmy, i, colorScheme, scaleVectors, papszOptions);
      if(err != NINJA_SUCCESS)
      {
          printf("NinjaSetGoogColor: err = %d\n", err);
      }

      err = NinjaSetGoogConsistentColorScale(ninjaArmy, i, useConsistentColorScale, numNinjas, papszOptions);
      if(err != NINJA_SUCCESS)
      {
          printf("NinjaSetGoogConsistentColorScale: err = %d\n", err);
      }

      /* Shapefile output (.shp) */
      err = NinjaSetShpOutFlag(ninjaArmy, i, outputFlag, papszOptions);
      if(err != NINJA_SUCCESS)
      {
          printf("NinjaSetShpOutFlag: err = %d\n", err);
      }

      err = NinjaSetShpResolution(ninjaArmy, i, outputResolution, units, papszOptions);
      if(err != NINJA_SUCCESS)
      {
          printf("NinjaSetShpResolution: err = %d\n", err);
      }

      /* Fire Behavior Output (.asc) */
      err = NinjaSetAsciiOutFlag(ninjaArmy, i, outputFlag, papszOptions);
      if(err != NINJA_SUCCESS)
      {
          printf("NinjaSetAsciiOutFlag: err = %d\n", err);
      }

      err = NinjaSetAsciiAaigridOutFlag(ninjaArmy, i, outputFlag, papszOptions);
      if(err != NINJA_SUCCESS)
      {
          printf("NinjaSetAsciiAaigridOutFlag: err = %d\n", err);
      }

      err = NinjaSetAsciiProjOutFlag(ninjaArmy, i, outputFlag, papszOptions);
      if(err != NINJA_SUCCESS)
      {
          printf("NinjaSetAsciiProjOutFlag: err = %d\n", err);
      }

      err = NinjaSetAsciiResolution(ninjaArmy, i, outputResolution, units, papszOptions);
      if(err != NINJA_SUCCESS)
      {
          printf("NinjaSetAsciiResolution: err = %d\n", err);
      }

      /* pdf output (.pdf) */
      err = NinjaSetPDFOutFlag(ninjaArmy, i, outputFlag, papszOptions);
      if(err != NINJA_SUCCESS)
      {
          printf("NinjaSetPDFOutFlag: err = %d\n", err);
      }

      err = NinjaSetPDFLineWidth(ninjaArmy, i, lineWidth, papszOptions);
      if(err != NINJA_SUCCESS)
      {
          printf("NinjaSetPDFLineWidth: err = %d\n", err);
      }

      err = NinjaSetPDFBaseMap(ninjaArmy, i, baseMap, papszOptions);
      if(err != NINJA_SUCCESS)
      {
          printf("NinjaSetPDFBaseMap: err = %d\n", err);
      }

      err = NinjaSetPDFDEM(ninjaArmy, i, demFile, papszOptions);
      if(err != NINJA_SUCCESS)
      {
          printf("NinjaSetPDFDEM: err = %d\n", err);
      }

      err = NinjaSetPDFSize(ninjaArmy, i, pdfHeight, pdfWidth, pdfDpi, papszOptions);
      if(err != NINJA_SUCCESS)
      {
          printf("NinjaSetPDFSize: err = %d\n", err);
      }

      err = NinjaSetPDFResolution(ninjaArmy, i, outputResolution, units, papszOptions);
      if(err != NINJA_SUCCESS)
      {
          printf("NinjaSetPDFResolution: err = %d\n", err);
      }

      /* VKT output (.vkt) */
      err = NinjaSetVtkOutFlag(ninjaArmy, i, outputFlag, papszOptions);
      if(err != NINJA_SUCCESS)
      {
          printf("NinjaSetVtkOutFlag: err = %d\n", err);
      }
    }

    /*
     * Start the simulations
     */
    err = NinjaStartRuns(ninjaArmy, nCPUs, papszOptions);
    if(err != 1) //NinjaStartRuns returns 1 on success
    {
        printf("NinjaStartRuns: err = %d\n", err);
    }

    /*
     * Get the outputs
     */
    const double* outputSpeedGrid = NULL;
    const double* outputDirectionGrid = NULL;
    const char* outputGridProjection = NULL;
    int outputGridnCols;
    int outputGridnRows;
    double outputGridCellSize;
    double outputGridxllCorner;
    double outputGridyllCorner;
    const int nIndex = 0;
    int gridSize = 10; // arbitrary value 

    outputSpeedGrid = NinjaGetOutputSpeedGrid(ninjaArmy, nIndex, papszOptions);
    if( NULL == outputSpeedGrid )
    {
        printf("Error in NinjaGetOutputSpeedGrid");
    }
    
    outputDirectionGrid = NinjaGetOutputDirectionGrid(ninjaArmy, nIndex, papszOptions);
    if( NULL == outputDirectionGrid )
    {
        printf("Error in NinjaGetOutputDirectionGrid");
    }
    
    outputGridProjection = NinjaGetOutputGridProjection(ninjaArmy, nIndex, papszOptions);
    if( NULL == outputGridProjection )
    {
        printf("Error in NinjaGetOutputGridProjection");
    }

    outputGridCellSize = NinjaGetOutputGridCellSize(ninjaArmy, nIndex, papszOptions);
    outputGridxllCorner = NinjaGetOutputGridxllCorner(ninjaArmy, nIndex, papszOptions);
    outputGridyllCorner = NinjaGetOutputGridyllCorner(ninjaArmy, nIndex, papszOptions);
    outputGridnCols = NinjaGetOutputGridnCols(ninjaArmy, nIndex, papszOptions);
    outputGridnRows = NinjaGetOutputGridnRows(ninjaArmy, nIndex, papszOptions);

    printArray(outputSpeedGrid, gridSize, "Output Speed Grid");
    printArray(outputDirectionGrid, gridSize, "Output Direction Grid");
    printf("Grid Cell Size: %f\n", outputGridCellSize);
    printf("Grid xllCorner: %f\n", outputGridxllCorner);
    printf("Grid yllCorner: %f\n", outputGridyllCorner);
    printf("Grid Columns: %d\n", outputGridnCols);
    printf("Grid Rows: %d\n", outputGridnRows);
    printf("Grid Projection: %s\n", outputGridProjection);

    /*
     * Clean up
     */
    err = NinjaDestroyArmy(ninjaArmy, papszOptions);
    if(err != NINJA_SUCCESS)
    {
        printf("NinjaDestroyRuns: err = %d\n", err);
    }

    if(multiStream != NULL)
    {
        if(fclose(multiStream) != 0)
        {
            printf("error closing log file\n");
        }
    }

    // must be called to cleanup ninjaInit();
    err = NinjaFinalize(papszOptions);
    if(err != NINJA_SUCCESS)
    {
        printf("NinjaFinalize: err = %d\n", err);
    }

    return NINJA_SUCCESS;
}
