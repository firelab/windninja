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
    /* inputs that do not vary among ninjas in an army */
    const char * demFile = "/home/mason/Documents/Git/WindNinja/windninja/autotest/api/data/missoula_valley.tif"; 
    const char * initializationMethod = "domain_average";
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

    /* inputs specific to output 
     * Note: Outputs have default values if inputs are not specified (like resolution)
     */
    const double outputResolution = 10.0;
    const char * units = "m";
    const double width = 1.0;
    const char * scaling = "equal_color";
    const char * outputPath  = "/home/mason/Documents/Git/WindNinja/windninja/autotest/api/data/output";
    const bool outputFlag = 1;

    /* 
     * Create the army
     */
    ninjaArmy = NinjaMakeDomainAverageArmy(numNinjas, momentumFlag, speedList, speedUnits, directionList, papszOptions);
    if( NULL == ninjaArmy )
    {
        printf("NinjaCreateArmy: ninjaArmy = NULL\n");
    }

    /* 
     * Prepare the army
     */
    for(unsigned int i=0; i<numNinjas; i++)
    {
      /* 
       * Sets Simulation Variables
       */
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

      /* 
       * Sets Output Variables
       */
      err = NinjaSetOutputPath(ninjaArmy, i, outputPath, papszOptions);
      if(err != NINJA_SUCCESS)
      {
          printf("NinjaSetOutputPath: err = %d\n", err);
      }

      err = NinjaSetGoogOutFlag(ninjaArmy, i, outputFlag, papszOptions);
      if(err != NINJA_SUCCESS)
      {
          printf("NinjaSetGoogOutFlag: err = %d\n", err);
      }

      err = NinjaSetGoogResolution (ninjaArmy, i, outputResolution, units, papszOptions);
      if(err != NINJA_SUCCESS)
      {
          printf("NinjaSetGoogResolution: err = %d\n", err);
      }

      err = NinjaSetGoogSpeedScaling (ninjaArmy, i, scaling, papszOptions);
      if(err != NINJA_SUCCESS)
      {
          printf("NinjaSetGoogSpeedScaling: err = %d\n", err);
      }

      err = NinjaSetGoogLineWidth (ninjaArmy, i, width, papszOptions);
      if(err != NINJA_SUCCESS)
      {
          printf("NinjaSetGoogLineWidth: err = %d\n", err);
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
 
    return NINJA_SUCCESS;
}
