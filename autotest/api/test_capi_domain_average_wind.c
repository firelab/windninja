/******************************************************************************
 *
 * Project:  WindNinja
 * Purpose:  C API testing
 * Author:   Nicholas Kim <kim.n.j@wustl.edu>
 *
 * gcc -g -Wall -o test_capi_domain_average_wind test_capi_domain_average_wind.c -lninja
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
    const int nCPUs = 4;
    char ** papszOptions = NULL;
    NinjaErr err = 0;
    err = NinjaInit(papszOptions); //initialize global singletons and environments (GDAL_DATA, etc.)
    if(err != NINJA_SUCCESS)
    {
        printf("NinjaInit: err = %d\n", err);
    }

    /*
     * Setting up a log file, for ninjaCom, if desired
     */
    FILE* multiStream = NULL;
    multiStream = fopen("/home/atw09001/src/wind/windninja/autotest/api/data/output/ninja.log", "w+");
    if(multiStream == NULL)
    {
        printf("error opening log file\n");
    }

    /* 
     * Set up domain average run 
     */
    /* inputs that do not vary among ninjas in an army */
    const char * demFile = "/home/atw09001/src/wind/windninja/autotest/api/data/missoula_valley.tif";
    const char * initializationMethod = "domain_average";
    const char * meshChoice = "coarse";
    const char * vegetation = "grass";
    const int nLayers = 20; //layers in the mesh
    const int diurnalFlag = 1; //diurnal slope wind parameterization not used
    const double height = 10.0;
    const char * heightUnits = "m";
    //bool momentumFlag = 0; //we're using the conservation of mass solver
    bool momentumFlag = 1; //we're using the conservation of momentum solver
    unsigned int numNinjas = 2; //two ninjas in the ninjaArmy - must be equal to array sizes

    /* inputs specific to output 
     * Note: Outputs have default values if inputs are not specified (like resolution)
     */
    const double outputResolution = 100.0;
    const char * units = "m";
    const double width = 1.0;
    const char * scaling = "equal_color";
    const char * outputPath  = "/home/atw09001/src/wind/windninja/autotest/api/data/output";
    const bool outputFlag = 1;

    /* inputs that can vary among ninjas in an army */
    const double speedList[] = {5.5, 5.5}; // matches the size of numNinjas
    const char * speedUnits = "mps";
    const double directionList[2] = {220.0, 300.0};
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

    const double meshResolution = -1.0;  //set to value > 0.0 to override meshChoice with meshResolution value
    //const double meshResolution = 300.0;
    const char * meshResolutionUnits = "m";

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
    err = NinjaSetMultiComStream(ninjaArmy, multiStream, papszOptions);
    if(err != NINJA_SUCCESS)
    {
        printf("NinjaSetMultiComStream: err = %d\n", err);
    }

    /*
     * Create the army
     */
    err = NinjaMakeDomainAverageArmy(ninjaArmy, numNinjas, momentumFlag, speedList, speedUnits, directionList, year, month, day, hour, minute, timezone, air, airUnits, cloud, cloudUnits, papszOptions);
    if( err != NINJA_SUCCESS)
    {
        printf("NinjaMakeDomainAverageArmy: err = %d\n", err);
    }

    /* 
     * Prepare the army
     */
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
    
      err = NinjaSetUniVegetation(ninjaArmy, i, vegetation, papszOptions);
      if(err != NINJA_SUCCESS)
      {
        printf("NinjaSetUniVegetation: err = %d\n", err);
      }

      if( meshResolution > 0.0 )
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

    return NINJA_SUCCESS;
}
