/******************************************************************************
 *
 * Project:  WindNinja
 * Purpose:  C API testing
 * Author:   Mason Willman <mason.willman
 *
 * gcc -g -Wall -o test_capi_point_initialization_wind test_capi_point_initialization_wind.c -lninja
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
#include <stdio.h> //for printf
#include <stdbool.h>

int main()
{
    /*
     * Setting up the simulation
     */
    NinjaArmyH* ninjaArmy = NULL;
    const int nCPUs = 3;
    const char * runType = "C-API autotest";
    char ** papszOptions = NULL;
    NinjaErr err = 0;
    err = NinjaInit(runType, papszOptions); //initialize global singletons and environments (GDAL_DATA, etc.)
    if(err != NINJA_SUCCESS)
    {
        printf("NinjaInit: err = %d\n", err);
    }

    /*
     * Setting up a log file, for ninjaCom, if desired
     */
    FILE* multiStream = NULL;
    multiStream = fopen("/home/atw09001/src/wind/windninja/autotest/api/data/ninja.log", "w+");
    if(multiStream == NULL)
    {
        printf("error opening log file\n");
    }

    /*
     * Set up point initialization run
     */
    /* inputs that can vary among ninjas in an army */
    const char * initializationMethod = "point";
    const char * meshChoice = "coarse";
    const char * vegetation = "grass";
    const int nLayers = 20; //layers in the mesh
    const int diurnalFlag = 0; //set to 1 to use the diurnal slope wind parameterization inputs
    const int stabilityFlag = 0; //set to 1 to use the stability wind parameterization inputs. NOT to be used with a momentum solver run.
    const double height = 10.0;
    const char * heightUnits = "m";
    const char * speedUnits = "mps";
    bool momentumFlag = 0; //we're using the conservation of mass solver
    ////bool momentumFlag = 1; //we're using the conservation of momentum solver. NOTE: CANNOT DO POINT INITIALIZATION WITH A MOMENTUM SOLVER RUN.
    ////unsigned int numNinjas = 2; //two ninjas in the ninjaArmy - this will eventually be equal to the number of times in the timeList to be run on the set of station files

    //const int nIters = -1.0;  //set to value > 0.0 to override the default value of 1000. Used only by the conservation of momentum solver.
    const int nIters = 300;  // the cli and the GUI use a value of 300 instead of the default value of 1000.

    const double meshResolution = -1.0;  //set to value > 0.0 to override meshChoice with meshResolution value. Used only by the conservation of momentum solver.
    //const double meshResolution = 300.0;
    const char * meshResolutionUnits = "m";

    /* timeListSize is technically used instead of the number of ninjas */
    /* but timeListSize is technically used more like a numTimeSteps, it can be less than or equal to the size of the time arrays */
    //int timeListSize = numNinjas;
    int timeListSize = 2;
    //int timeListSize = 1;  // make sure to only use 1 if doing latestTime data, or it tries to converge on some kind of NO_DATA situation or something.
    //// the fetched data times, hrm the run seems to be way stricter than the fetch for building the times
    //int year[2] = {2024, 2024};
    //int month[2] = {2, 2};
    //int day[2] = {2, 2};
    //int hour[2] = {2, 2};  // local time
    ////int hour[2] = {8, 8};  // UTC time
    //int minute[2] = {0, 59};
    //// the /windninja/data/ times
    int year[2] = {2018, 2018};
    int month[2] = {6, 6};
    //int day[2] = {20, 21};  // local time
    //int hour[2] = {21, 21};  // local time
    int day[2] = {21, 22}; // UTC time
    int hour[2] = {3, 3};  // UTC time
    int minute[2] = {28, 28};
    ////// will need to run fetch test to get wxstation data, or grab the data from the windninja/data folder (that seems easier)
    //// hrm, doesn't seem to like the station location files, like the cli does
    ////int numStationFiles = 1;
    ////const char* station_paths[1] = {"/home/atw09001/src/wind/windninja/autotest/api/data/fetch/WXSTATIONS-2026-04-29-1331-missoula_valley/missoula_valley_stations_6.csv.csv"};  // latestTime
    ////const char* station_paths[1] = {"/home/atw09001/src/wind/windninja/autotest/api/data/fetch/WXSTATIONS-2024-02-02-0202-2024-02-02-0202-missoula_valley/missoula_valley_stations_6.csv.csv"};  // timeSeries
    ////const char* station_paths[1] = {"/home/atw09001/src/wind/windninja/data/WXSTATIONS-MDT-2018-06-20-2128-2018-06-21-2128-missoula_valley/missoula_valley_stations_4.csv"};  // timeSeries
    ////const char* station_paths[1] = {"/home/atw09001/src/wind/windninja/data/WXSTATIONS-2018-06-25-1237-missoula_valley/missoula_valley_stations_4.csv"};  // latestTime
    int numStationFiles = 4;
    const char* station_paths[4] = {"/home/atw09001/src/wind/windninja/data/WXSTATIONS-MDT-2018-06-20-2128-2018-06-21-2128-missoula_valley/KMSO-MDT-2018-06-20_2128-2018-06-21_2128-0.csv", "/home/atw09001/src/wind/windninja/data/WXSTATIONS-MDT-2018-06-20-2128-2018-06-21-2128-missoula_valley/TS934-MDT-2018-06-20_2128-2018-06-21_2128-1.csv", "/home/atw09001/src/wind/windninja/data/WXSTATIONS-MDT-2018-06-20-2128-2018-06-21-2128-missoula_valley/PNTM8-MDT-2018-06-20_2128-2018-06-21_2128-2.csv", "/home/atw09001/src/wind/windninja/data/WXSTATIONS-MDT-2018-06-20-2128-2018-06-21-2128-missoula_valley/TR266-MDT-2018-06-20_2128-2018-06-21_2128-3.csv"};  // timeSeries
    //const char* station_paths[4] = {"/home/atw09001/src/wind/windninja/data/WXSTATIONS-2018-06-25-1237-missoula_valley/KMSO-2018-06-25_1237-0.csv", "/home/atw09001/src/wind/windninja/data/WXSTATIONS-2018-06-25-1237-missoula_valley/TS934-2018-06-25_1237-1.csv", "/home/atw09001/src/wind/windninja/data/WXSTATIONS-2018-06-25-1237-missoula_valley/PNTM8-2018-06-25_1237-2.csv", "/home/atw09001/src/wind/windninja/data/WXSTATIONS-2018-06-25-1237-missoula_valley/TR266-2018-06-25_1237-3.csv"};  // latestTime
    // the fetched data
    //int numStationFiles = 6;
    //const char* station_paths[6] = {"/home/atw09001/src/wind/windninja/autotest/api/data/fetch/WXSTATIONS-2026-04-29-1331-missoula_valley/KMSO-2026-04-29_1331-0.csv", "/home/atw09001/src/wind/windninja/autotest/api/data/fetch/WXSTATIONS-2026-04-29-1331-missoula_valley/BLMM8-2026-04-29_1331-1.csv", "/home/atw09001/src/wind/windninja/autotest/api/data/fetch/WXSTATIONS-2026-04-29-1331-missoula_valley/PNTM8-2026-04-29_1331-2.csv", "/home/atw09001/src/wind/windninja/autotest/api/data/fetch/WXSTATIONS-2026-04-29-1331-missoula_valley/FINM8-2026-04-29_1331-3.csv", "/home/atw09001/src/wind/windninja/autotest/api/data/fetch/WXSTATIONS-2026-04-29-1331-missoula_valley/NINM8-2026-04-29_1331-4.csv", "/home/atw09001/src/wind/windninja/autotest/api/data/fetch/WXSTATIONS-2026-04-29-1331-missoula_valley/TT350-2026-04-29_1331-5.csv"};  // latestTime
    //const char* station_paths[6] = {"/home/atw09001/src/wind/windninja/autotest/api/data/fetch/WXSTATIONS-2024-02-02-0202-2024-02-02-0202-missoula_valley/KMSO-2024-02-02_0202-2024-02-02_0202-0.csv", "/home/atw09001/src/wind/windninja/autotest/api/data/fetch/WXSTATIONS-2024-02-02-0202-2024-02-02-0202-missoula_valley/BLMM8-2024-02-02_0202-2024-02-02_0202-1.csv", "/home/atw09001/src/wind/windninja/autotest/api/data/fetch/WXSTATIONS-2024-02-02-0202-2024-02-02-0202-missoula_valley/PNTM8-2024-02-02_0202-2024-02-02_0202-2.csv", "/home/atw09001/src/wind/windninja/autotest/api/data/fetch/WXSTATIONS-2024-02-02-0202-2024-02-02-0202-missoula_valley/FINM8-2024-02-02_0202-2024-02-02_0202-3.csv", "/home/atw09001/src/wind/windninja/autotest/api/data/fetch/WXSTATIONS-2024-02-02-0202-2024-02-02-0202-missoula_valley/NINM8-2024-02-02_0202-2024-02-02_0202-4.csv", "/home/atw09001/src/wind/windninja/autotest/api/data/fetch/WXSTATIONS-2024-02-02-0202-2024-02-02-0202-missoula_valley/MOMM8-2024-02-02_0202-2024-02-02_0202-5.csv"};  // timeSeries
    char* demFile = "/home/atw09001/src/wind/windninja/autotest/api/data/missoula_valley.tif";
    //char* osTimeZone = "UTC";
    char* osTimeZone = "America/Denver";
    bool matchPointsFlag = 1;  // needed to match the station data, or else you only get a domainAverageRun of the initial wind field constructed from the station data

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
    err = NinjaMakePointArmy(ninjaArmy, year, month, day, hour, minute, timeListSize, osTimeZone, station_paths, numStationFiles, demFile, matchPointsFlag, momentumFlag, papszOptions);
    if(err != NINJA_SUCCESS)
    {
        printf("NinjaMakePointArmy: err = %d\n", err);
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

        err = NinjaSetInitializationMethod(ninjaArmy, i, initializationMethod, matchPointsFlag, papszOptions);
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
