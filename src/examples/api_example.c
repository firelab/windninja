#include <stdio.h>
#include "../ninja/windninja.h"
#include <string>
int main()
{
    NinjaH* ninjaArmy = NULL; 
    const char * comType = "cli"; //communication type is always set to "cli"
    const int nCPUs = 1;
    char ** papszOptions = NULL;
    NinjaErr err = 0; 
    const char * demFile = "output.tif";
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
    const std::string demFileName = "/home/nicholas/Firelab/WindNew/windninja/src/examples/output.tif";
    const std::string timeZone = "America/Denver";
    const std::string wxmodel_type = "NOMADS-HRRR-CONUS-3-KM";
    const std::string forecastFilename = NinjaFetchForecast(wxmodel_type, timeZone, 1, demFileName);
    printf("Forecast filename: %s\n", forecastFilename.c_str());
    /* inputs that can vary among ninjas in an army */
    const double speed[2] = {5.5, 5.5};
    const double direction[2] = {220, 300};
    ninjaArmy = NinjaCreateArmy(numNinjas, papszOptions);
    if( NULL == ninjaArmy )
    {
    printf("NinjaCreateArmy: ninjaArmy = NULL\n");
    }
    
    err = NinjaInit(); //must be called for any simulation
    if(err != NINJA_SUCCESS)
    {
    printf("NinjaInit: err = %d\n", err);
    }
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
    err = NinjaSetOutputPath(ninjaArmy, i, "./");
    if(err != NINJA_SUCCESS)
    {
        printf("NinjaSetOutputPath: err = %d\n", err);
    }
    err = NinjaSetGoogOutFlag(ninjaArmy, i, 1);
    if(err != NINJA_SUCCESS)
    {
        printf("NinjaSetGoogOutFlag: err = %d\n", err);
    }
}
    err = NinjaStartRuns(ninjaArmy, nCPUs);
    if(err != 1) //NinjaStartRuns returns 1 on success
    {
        printf("NinjaStartRuns: err = %d\n", err);
    }
    const double* outputSpeedGrid = NULL;
    const double* outputDirectionGrid = NULL;
    const char* outputGridProjection = NULL;
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
    err = NinjaDestroyArmy(ninjaArmy);
    if(err != NINJA_SUCCESS)
    {
        printf("NinjaDestroyRuns: err = %d\n", err);
    }
 
    return NINJA_SUCCESS;
}