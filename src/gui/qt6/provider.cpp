#include "provider.h"
#include "modeldata.h"
#include "../../ninja/windninja.h"

#include <iostream>
#include <vector>
#include <list>

using namespace std;


NinjaArmyH* ninjaArmy = NULL;
const char * comType = "cli";
const int nCPUs = 1;
char ** papszOptions = NULL;
NinjaErr err = 0;

int NinjaSim::domain_average_exec(DomainAverageWind& input) {
  std::vector<double> speedVector = input.getSpeedList();
  const double* speedList = speedVector.data();

  std::vector<double> directionVector = input.getDirectionList();
  const double* directionList = speedVector.data();

  const char * demFile = input.getDemFile().c_str();
  double outputResolution = input.getOutputResolution();
  const char* initializationMethod = input.getInitializationMethod().c_str();
  const char* meshChoice = input.getMeshChoice().c_str();
  const char* vegetation = input.getVegetation().c_str();
  const int nLayers = input.getNLayers();
  const int diurnalFlag = input.getDiurnalFlag();
  const double height = input.getHeight();
  const char* heightUnits = input.getHeightUnits().c_str();
  const char* speedUnits = input.getSpeedUnits().c_str();
  bool momentumFlag = input.getMomentumFlag();
  unsigned int numNinjas = input.getNumNinjas();


  /*
   * Setting up the simulation
   */
  err = NinjaInit(papszOptions); //initialize global singletons and environments (GDAL_DATA, etc.)
  ninjaArmy = NinjaMakeDomainAverageArmy(numNinjas, momentumFlag, speedList, speedUnits, directionList, papszOptions);

  if(err != NINJA_SUCCESS)
  {
    printf("NinjaInit: err = %d\n", err);
  }

  /*
   * Create the army
   */
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
  for(unsigned int i=0; i<input.getNumNinjas(); i++)
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


           //heighUnits set static

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

  /*
   * Get the outputs
   */
  const double* outputSpeedGrid = NULL;
  const double* outputDirectionGrid = NULL;
  const char* outputGridProjection = NULL;
  const int nIndex = 0;
  const char* units = "m";
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

int NinjaSim::point_exec(PointInitialization& input) {
  /*
   * Setting up the simulation
   */

  //initialize global singletons and environments (GDAL_DATA, etc.)
  if(err != NINJA_SUCCESS)
  {
    printf("NinjaInit: err = %d\n", err);
  }

  /*
   * Create the army
   */
  int year = input.getYear();
  int month = input.getMonth();
  int day = input.getDay();
  int hour = input.getHour();
  int minute = input.getMinute();
  char* station_path = input.getStationPath();
  char* elevation_file = input.getElevationFile();
  char* osTimeZone = input.getOSTimeZone();
  bool matchPointFlag = input.getMatchPointFlag();
  bool momemtumFlag = input.getMomentumFlag();

  const char * demFile = input.getDemFile().c_str();
  double outputResolution = input.getOutputResolution();
  const char* initializationMethod = input.getInitializationMethod().c_str();
  const char* meshChoice = input.getMeshChoice().c_str();
  const char* vegetation = input.getVegetation().c_str();
  const int nLayers = input.getNLayers();
  const int diurnalFlag = input.getDiurnalFlag();
  const double height = input.getHeight();
  const char* heightUnits = input.getHeightUnits().c_str();
  bool momentumFlag = input.getMomentumFlag();
  unsigned int numNinjas = input.getNumNinjas();

  ninjaArmy = NinjaMakePointArmy(&year, &month, &day, &hour, &minute, osTimeZone, station_path, elevation_file, matchPointFlag, momemtumFlag, papszOptions);

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

           //err = NinjaSetOutputSpeedUnits(ninjaArmy, i, speedUnits, papszOptions);
           //if(err != NINJA_SUCCESS)
           //{
           //  printf("NinjaSetOutputSpeedUnits: err = %d\n", err);
           //}

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

int NinjaSim::wxmodel_exec(WeatherModel& input) {
  /*
   * Setting up the simulation
   */

  err = NinjaInit(papszOptions); //initialize global singletons and environments (GDAL_DATA, etc.)
  if(err != NINJA_SUCCESS)
  {
    printf("NinjaInit: err = %d\n", err);
  }

  /*
   * Set up Weather Model Initialization run
   */

  const char * demFile = input.getDemFile().c_str();
  double outputResolution = input.getOutputResolution();
  const char* initializationMethod = input.getInitializationMethod().c_str();
  const char* meshChoice = input.getMeshChoice().c_str();
  const char* vegetation = input.getVegetation().c_str();
  const int nLayers = input.getNLayers();
  const int diurnalFlag = input.getDiurnalFlag();
  const double height = input.getHeight();
  const char* heightUnits = input.getHeightUnits().c_str();
  bool momentumFlag = input.getMomentumFlag();
  unsigned int numNinjas = input.getNumNinjas();

  /* inputs that can vary among ninjas in an army */
  //const char * speedUnits = input.get;

  /*
   * Create the army
   */
  const char * forecast = input.getForecast();
  const char * osTimeZone = input.getOSTimeZone();

  ninjaArmy = NinjaMakeWeatherModelArmy(forecast, osTimeZone, momentumFlag, papszOptions);
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

           //err = NinjaSetOutputSpeedUnits(ninjaArmy, i, speedUnits, papszOptions);
           //if(err != NINJA_SUCCESS)
           //{
           //  printf("NinjaSetOutputSpeedUnits: err = %d\n", err);
           //}

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

  /*
   * Get the outputs
   */
  const double* outputSpeedGrid = NULL;
  const double* outputDirectionGrid = NULL;
  const char* outputGridProjection = NULL;
  const int nIndex = 0;
  const char* units = "m";
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

NinjaSim::NinjaSim(BaseInput& input) {
  if (input.getInitializationMethod() == "domain_average") {
    if (auto* domainWind = dynamic_cast<DomainAverageWind*>(&input)) {
      domain_average_exec(*domainWind);
    }
  }else if (input.getInitializationMethod() == "point") {
    if (auto* pointWind = dynamic_cast<PointInitialization*>(&input)) {
      point_exec(*pointWind);
    }
  }else if (input.getInitializationMethod() == "wxmodel") {
    if (auto* weatherWind = dynamic_cast<WeatherModel*>(&input)) {
      wxmodel_exec(*weatherWind);
    }
  }else{
    //error state
  }
}





