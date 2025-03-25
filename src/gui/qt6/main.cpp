#include "mainwindow.h"
#include "../../ninja/windninja.h"

#include <QApplication>

int main(int argc, char *argv[]) {
  QApplication a(argc, argv);
  MainWindow w;

  // From tutorial: step 1
  NinjaArmyH* ninjaArmy = NULL;
  const char * comType = "cli"; //communication type is always set to "cli"
  const int nCPUs = 1;
  char ** papszOptions = NULL;
  NinjaErr err = 0;

  err = NinjaInit(papszOptions); //must be called for any simulation
  if(err != NINJA_SUCCESS)
  {
    printf("NinjaInit: err = %d\n", err);
  }

  // From tutorial: step 2
  /* inputs that apply to the whole army (must be the same for all ninjas) */
  const char * demFile = "big_butte_small.tif";
  const char * initializationMethod = "domain_average";
  const char * meshChoice = "coarse";
  const char * vegetation = "grass";
  const int nLayers = 20; //layers in the mesh
  const int diurnalFlag = 0; //diurnal slope wind parameterization not used
  const double height = 5.0;
  const char * heightUnits = "m";
  bool momentumFlag = 0; //we're using the conservation of mass solver
  unsigned int numNinjas = 2; //two ninjas in the ninjaArmy

  /* inputs that can vary among ninjas in an army */
  const double speed[2] = {5.5, 5.5};
  const char * speedUnits = "mps";
  const double direction[2] = {220, 300};

  // From tutorial: step 3 /* create the army */
  ninjaArmy = NinjaMakeDomainAverageArmy(numNinjas, momentumFlag, speed, speedUnits, direction, papszOptions);

  if( NULL == ninjaArmy )
  {
    printf("NinjaCreateArmy: ninjaArmy = NULL\n");

  // From tutorial: step 4
  /* set up the runs */
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

    err = NinjaSetInitializationMethod(ninjaArmy, i, initializationMethod,    papszOptions);
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
  }

  // Tutorial 5: Start the simulation
  /* start the runs */
  err = NinjaStartRuns(ninjaArmy, nCPUs, papszOptions);
  if(err != 1) //NinjaStartRuns returns 1 on success
  {
      printf("NinjaStartRuns: err = %d\n", err);
  }

  // Tutorial 6: Get outputs
  /* get the output wind speed and direction data */
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

  // Tutorial 7
  /*
   * Clean up
   */
  err = NinjaDestroyArmy(ninjaArmy, papszOptions);
  if(err != NINJA_SUCCESS)
  {
    printf("NinjaDestroyRuns: err = %d\n", err);
  }

  return NINJA_SUCCESS;

  w.show();
  return a.exec();
}
