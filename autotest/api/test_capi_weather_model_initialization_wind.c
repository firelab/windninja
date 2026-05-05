/******************************************************************************
 *
 * Project:  WindNinja
 * Purpose:  C API testing
 * Author:   Nicholas Kim <kim.n.j@wustl.edu>
 *
 * gcc -g -Wall -o test_capi_weather_model_initialization_wind test_capi_weather_model_initialization_wind.c -lninja
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
     * Set up Weather Model Initialization run
     */
    char demFile[MAX_PATH_LEN];
    //snprintf(demFile, sizeof(demFile), "%s%s", wnDataPath, "/../autotest/api/data/missoula_valley.tif");
    snprintf(demFile, sizeof(demFile), "%s%s", wnDataPath, "/../autotest/api/data/fetch/DEMBBox.tif");
    const char * initializationMethod = "wxmodel";
    const char * meshChoice = "coarse";
    const char * vegetation = "grass";
    const int nLayers = 20; //layers in the mesh
    const int diurnalFlag = 0; //set to 1 to use the diurnal slope wind parameterization inputs
    const int stabilityFlag = 0; //set to 1 to use the stability wind parameterization inputs. NOT to be used with a momentum solver run.
    const double height = 10.0;
    const char * heightUnits = "m";
    bool momentumFlag = 0; //we're using the conservation of mass solver
    //bool momentumFlag = 1; //we're using the conservation of momentum solver
    ////unsigned int numNinjas = 2; //two ninjas in the ninjaArmy - this will eventually be equal to the number of times in the weather stations files

    //const int nIters = -1.0;  //set to value > 0.0 to override the default value of 1000. Used only by the conservation of momentum solver.
    const int nIters = 300;  // the cli and the GUI use a value of 300 instead of the default value of 1000.

    const double meshResolution = -1.0;  //set to value > 0.0 to override meshChoice with meshResolution value. Used only by the conservation of momentum solver.
    //const double meshResolution = 300.0;
    const char * meshResolutionUnits = "m";

    //const char * osTimeZone = "UTC";
    const char * osTimeZone = "America/Denver";

    /* inputs that can vary among ninjas in an army */
    const char * speedUnits = "mps";


    /* timeListSize is technically used instead of the number of ninjas */
    //// will need to run test_capi_fetching.c to get wxstation data, there is no predownloaded data in the windninja/data folder for this case
    /// so update the folder path accordingly for the fresh data.
    /// will have to update the time list accordingly as well.
    char forecast[MAX_PATH_LEN];
    snprintf(forecast, sizeof(forecast), "%s%s", wnDataPath, "/../autotest/api/data/fetch/NOMADS-HRRR-CONUS-3-KM-DEMBBox.tif/20260504T2300/20260504T2300.zip");
    //snprintf(forecast, sizeof(forecast), "%s%s", wnDataPath, "/../autotest/api/data/fetch/PASTCAST-GCP-HRRR-CONUS-3-KM-DEMBBox.tif/20240202T0200/20240202T0200.zip");

    /*//// manually set a time from the file (needs updated each time)
    int timeListSize = 1;
    ////const char* inputTimeList[1] = {"20260504T2300"};  // apparently wrong format
    const char* inputTimeList[1] = {"2026-May-04 17:00:00 MDT"};  // convert from UTC to MDT, 6 hrs back*/

    //// or get the full time list read from the file, and pick indices
    //// yeah, picking indices can be a pain without printing out the list, making a new list from specific times is also a pain
    //// so this is meant more to just use the whole time list, with a debug option to print out the full list
    //// this involves setting up a ninjaTools instance
    // Initialize ninjaTools
    NinjaToolsH* ninjaTools = NinjaMakeTools();
    // Customize the ninja communication
    err = NinjaSetToolsMultiComStream(ninjaTools, multiStream, papszOptions);
    if(err != NINJA_SUCCESS)
    {
        printf("NinjaSetToolsMultiComStream: err = %d\n", err);
    }
    // now get the list of times
    int timeListSize = 0;
    const char **inputTimeList = NinjaGetWeatherModelTimeList(ninjaTools, &timeListSize, forecast, osTimeZone);
    if(inputTimeList == NULL)
    {
        printf("NinjaGetWeatherModelTimeList: Failed to fill timeList\n");
    }
    if(timeListSize == 0)
    {
        printf("NinjaGetWeatherModelTimeList: returned empty timeList\n");
    }
    for(unsigned int i = 0; i < timeListSize; i++)
    {
        printf("timeList[%d] = '%s'\n", i, inputTimeList[i]);
    }
    // not yet needed/implemented in the code, but it should be.
    // if ninjaTools was used/generated, clean it up afterwards
    //err = NinjaDestroyTools(ninjaTools, papszOptions);
    //if(err != NINJA_SUCCESS)
    //{
    //    printf("NinjaDestroyTools: err = %d\n", ninjaErr);
    //}


    bool matchedPoints = true;  // for point initialization, but currently required as an input to SetInitializationMethod(). Should the match points pointInitialization algorythm be run, or should it just run as a domainAvgRun on the input wind field. ALWAYS set to true unless you know what you are doing.

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
    err = NinjaMakeWeatherModelArmy(ninjaArmy, forecast, osTimeZone, inputTimeList, timeListSize, momentumFlag, papszOptions);
    if(err != NINJA_SUCCESS)
    {
        printf("NinjaMakeWeatherModelArmy: err = %d\n", err);
    }

    /*
     * Prepare the army
     */
    for(unsigned int i=0; i<timeListSize; i++)
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
    const int nIndex = 0;

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
