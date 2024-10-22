#include "../ninja/windninja.h"
#include <cstdio>

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
    double resolution = 30; // 30 m resolution
    double boundsBox [] = {40.07, -104.0, 40.0, -104.07}; // Bounding box (north, east, south, west)
    err = NinjaFetchDEMBBox(boundsBox, demFile, resolution, fetch_type);
    /* inputs that apply to the whole army (must be the same for all ninjas) */
    const char * initializationMethod = "domain_average";
    const char * meshChoice = "coarse";
    const char * vegetation = "grass";
    const int nLayers = 20; //layers in the mesh
    const int diurnalFlag = 0; //diurnal slope wind parameterization not used
    const char * speedUnits = "mps";
    const double height = 5.0;
    const char * heightUnits = "m";
    int momentumFlag = 0; //we're using the conservation of mass solver
    int numNinjas = 2; //two ninjas in the ninjaArmy
    
    /* inputs that can vary among ninjas in an army */
    const double speed[2] = {5.5, 5.5};
    const double direction[2] = {220, 300};
    /* create the army */
    ninjaArmy = NinjaCreateArmy(numNinjas, papszOptions);
    if( NULL == ninjaArmy )
    {
        printf("NinjaCreateArmy: ninjaArmy = NULL\n");
    }
    /* set up the runs */
    for(unsigned int i=0; i<numNinjas; i++)
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
 
    err = NinjaSetDem(ninjaArmy, i, demFile);
    if(err != NINJA_SUCCESS)
    {
      printf("NinjaSetDem: err = %d\n", err);
    }
 
    err = NinjaSetPosition(ninjaArmy, i);
    if(err != NINJA_SUCCESS)
    {
      printf("NinjaSetPosition: err = %d\n", err);
    }
 
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
 
    err = NinjaSetNumVertLayers(ninjaArmy, i, nLayers);
    if(err != NINJA_SUCCESS)
    {
        printf("NinjaSetNumVertLayers: err = %d\n", err);
    }
    }
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
    lengthUnits::eLengthUnits units = lengthUnits::meters;
    
    outputSpeedGrid = NinjaGetOutputSpeedGrid(ninjaArmy, nIndex,resolution, units);
    if( NULL == outputSpeedGrid )
    {
        printf("Error in NinjaGetOutputSpeedGrid");
    }
    
    outputDirectionGrid = NinjaGetOutputDirectionGrid(ninjaArmy, nIndex, resolution,units);
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
    const int outputGridnCols = NinjaGetOutputGridnCols(ninjaArmy, nIndex);
    const int outputGridnRows = NinjaGetOutputGridnRows(ninjaArmy, nIndex);
    /* clean up */
    err = NinjaDestroyArmy(ninjaArmy);
    if(err != NINJA_SUCCESS)
    {
        printf("NinjaDestroyRuns: err = %d\n", err);
    }
 
    return NINJA_SUCCESS;
}