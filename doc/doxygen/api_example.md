WindNinja C API Tutorial
---

### Setting up the simulation

The first step is to create a handle for a ninjaArmy.
A ninjaArmy is an array of one or more ninjas, where each ninja represents a single WindNinja 
simulation. A few other generic options must also be set.

```C
#include "windninja.h"

int main()
{
    NinjaArmyH* ninjaArmy = NULL; 
    const char * comType = "cli"; //communication type is always set to "cli"
    const int nCPUs = 1;
    char ** papszOptions = NULL;
    NinjaErr err = 0; 

    err = NinjaInit(); //must be called for any simulation
    if(err != NINJA_SUCCESS)
    {
    printf("NinjaInit: err = %d\n", err);
    }


```

### Setting up a domain average run with the conservation of mass solver

Specify a number of options to use for the simulations.

```C

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
```

### Creating the army

This step allocates the army and performs some generic initialization steps.

```C    
  /* create the army */
  ninjaArmy = NinjaMakeDomainAverageArmy(numNinjas, momentumFlag, speedList, speedUnits, directionList, papszOptions);
  
  if( NULL == ninjaArmy )
  {
    printf("NinjaCreateArmy: ninjaArmy = NULL\n");
  }

```
### Preparing the army

A number of functions must be called to prepare the army.

```C
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
```

### Starting the simulations

```C

  /* start the runs */
  err = NinjaStartRuns(ninjaArmy, nCPUs, papszOptions);
  if(err != 1) //NinjaStartRuns returns 1 on success
  {
      printf("NinjaStartRuns: err = %d\n", err);
  }

```

### Getting the outputs

```C

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

```
### Cleaning up

```C
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

```
