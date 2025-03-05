/******************************************************************************
 *
 * Project:  WindNinja
 * Purpose:  C API testing
 * Author:   Nicholas Kim <kim.n.j@wustl.edu>
 *
 * g++ -g -Wall -o api_capi_point test_capi_point_initialization_wind.c -lninja
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
    const char * comType = "cli"; //communication type is always set to "cli"
    const int nCPUs = 1;
    char ** papszOptions = NULL;
    NinjaErr err = 0; 
    err = NinjaInit(papszOptions); //initialize global singletons and environments (GDAL_DATA, etc.)
    if(err != NINJA_SUCCESS)
    {
      printf("NinjaInit: err = %d\n", err);
    }

    /* 
     * Set up domain average run 
     */

    const char * demFile = "/home/mason/Documents/Git/WindNinja/windninja/data/big_butte_small.tif"; 
    double outputResolution = 100; 
    const char * initializationMethod = "point";
    const char * meshChoice = "coarse";
    const char * vegetation = "grass";
    const int nLayers = 20; //layers in the mesh
    const int diurnalFlag = 0; //diurnal slope wind parameterization not used
    const double height = 10.0;
    const char * heightUnits = "m";
    bool momentumFlag = 0; //we're using the conservation of mass solver
    unsigned int numNinjas = 2; //two ninjas in the ninjaArmy
    
    /* inputs that can vary among ninjas in an army */
    const double speedList[2] = {5.5, 5.5};
    const char * speedUnits = "mps";
    const double directionList[2] = {220, 300};

    /* 
     * Create the army
     */
    int year[1] = {2023};
    int month[1] = {10};
    int day[1] = {10};
    int hour[1] = {12};
    int minute[1] = {60};
    char* station_path = "./station";
    char* elevation_file = "output.tif";
    char* osTimeZone = "UTC";
    bool matchPointFlag = 1;
    bool momemtumFlag = 0;
    ninjaArmy = NinjaMakePointArmy(year, month, day, hour, minute, osTimeZone, station_path, elevation_file, matchPointFlag, momemtumFlag, papszOptions);
    
    if( NULL == ninjaArmy )
    {
        printf("NinjaCreateArmy: ninjaArmy = NULL\n");
    }

    err = NinjaInit(papszOptions); //must be called for any simulation
    if(err != NINJA_SUCCESS)
    {
      printf("NinjaInit: err = %d\n", err);
    }
    
    /* 
     * Prepare the army
     */
    for(unsigned int i=0; i<numNinjas; i++)
    {
        err = NinjaSetCommunication(ninjaArmy, i, comType, papszOptions);
        if(err != NINJA_SUCCESS)
        {
          printf("NinjaSetCommunication: err = %d\n", err);
        }
     
        err = NinjaSetNumberCPUs(ninjaArmy, i, nCPUs, papszOptions);
        if(err != NINJA_SUCCESS)
        {
          printf("NinjaSetNumberCPUs: err = %d\n", err);
        }
     
        err = NinjaSetInitializationMethod(ninjaArmy, i, initializationMethod, papszOptions);
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
     
        err = NinjaSetMeshResolutionChoice(ninjaArmy, i, meshChoice, papszOptions);
        if(err != NINJA_SUCCESS)
        {
            printf("NinjaSetMeshResolutionChoice: err = %d\n", err);
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

    return NINJA_SUCCESS;
}
