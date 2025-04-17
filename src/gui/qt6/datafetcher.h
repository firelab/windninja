#ifndef DATAFETCHER_H
#define DATAFETCHER_H

#include "../../ninja/windninja.h"
#include <string>

using namespace std;

/*TODO:
 * Implement exhaustive tests for all wx_model_types for NinjaFetchForecast
 * Implement tests for NinjaFetchDemPoint (example code in apiTestPoint.c) and NinjaFetchStation
 */

int fetchDEMBoundingBox(const string demFileOutPut, const string fetch_type, int resolution, double* boundsBox)
{
  /*
   * const char * demFile = "data/output.tif"; // output file name
   * char * fetch_type = "gmted";
   * double resolution = 30; // 30 m resolution
   * double boundsBox [] = {40.07, -104.0, 40.0, -104.07};
   */

  /*
   * Setting up NinjaArmy
   */
  NinjaArmyH* ninjaArmy = NULL;
  char ** papszOptions = NULL;
  NinjaErr err = 0;
  err = NinjaInit(papszOptions); // must be called for fetching and simulations
  if(err != NINJA_SUCCESS)
  {
    printf("NinjaInit: err = %d\n", err);
  }
  /*
   * Testing fetching from a DEM bounding box
   */
  // Bounding box (north, east, south, west)
  err = NinjaFetchDEMBBox(ninjaArmy, boundsBox, demFileOutPut.c_str(), resolution, strdup(fetch_type.c_str()), papszOptions);
  if (err != NINJA_SUCCESS){
    printf("NinjaFetchDEMBBox: err = %d\n", err);
  }

  return NINJA_SUCCESS;
}

int fetchForecast(const string wx_model_type, int numNinjas, const string demFile){
  /*
   * const char*wx_model_type = "NOMADS-HRRR-CONUS-3-KM";
   * int numNinjas = 2;
   */

  /*
   * Setting up NinjaArmy
   */
  NinjaArmyH* ninjaArmy = NULL;
  char ** papszOptions = NULL;
  NinjaErr err = 0;
  err = NinjaInit(papszOptions); // must be called for fetching and simulations
  if(err != NINJA_SUCCESS)
  {
    printf("NinjaInit: err = %d\n", err);
  }
  /*
   * Testing fetching for a Forecast file (wx_model_type are the names listed in the weather station download in WindNinja)
   */
  const char* forecastFilename = NinjaFetchForecast(ninjaArmy, wx_model_type.c_str(), numNinjas, demFile.c_str(), papszOptions);
  if (strcmp(forecastFilename, "exception") == 0){
    printf("NinjaFetchForecast: err = %s\n", forecastFilename);
  }
  else{
    printf("NinjaFetchForecast: forecastFilename = %s\n", forecastFilename);
  }

  return NINJA_SUCCESS;
}

int fetchStationData(int year, int month, int day, int hour, int minute, int numNinjas, const string output_path, int bufferZone, const string bufferUnits, const string demFile, const string osTimeZone, int fetchLatestFlag){
  /*
   * int year[1] = {2023};
   * int month[1] = {10};
   * int day[1] = {10};
   * int hour[1] = {12};
   * int minute[1] = {60};
   * const char* output_path = "";
   * const char* elevation_file = "data/missoula_valley.tif";
   * const char* osTimeZone = "UTC";
   * bool fetchLatestFlag = 1;
   * /

  /*
   * Setting up NinjaArmy
   */
  NinjaArmyH* ninjaArmy = NULL;
  char ** papszOptions = NULL;
  NinjaErr err = 0;
  err = NinjaInit(papszOptions); // must be called for fetching and simulations
  if(err != NINJA_SUCCESS)
  {
    printf("NinjaInit: err = %d\n", err);
  }

  /*
   * Testing fetching station data from a geotiff file.
   */

  err = NinjaFetchStation(&year, &month, &day, &hour, &minute, numNinjas, demFile.c_str(), bufferZone, bufferUnits.c_str(), osTimeZone.c_str(), fetchLatestFlag, output_path.c_str(), papszOptions);
  if (err != NINJA_SUCCESS) {
    printf("NinjaFetchStation: err = %d\n", err);
  } else {
    printf("NinjaFetchStation: success\n");
  }

  return NINJA_SUCCESS;
}

int fetchFromDEMPoint (double adfPoints[2], double adfBuff[2], const string units, double dfCellSize, const string pszDstFile, const string fetchtype){
  /*
   * double adfPoint[] = {104.0, 40.07}; // Point coordinates (longitude, latitude)
   * double adfBuff[] = {30, 30}; // Buffer to store the elevation value
   * const char* units = "mi";
   * double dfCellSize = 30.0; // Cell size in meters
   * char* pszDstFile = "data/dem_point_output.tif";
   * char* fetchType = "gmted";
  */

  /*
   * Setting up NinjaArmy
   */
  NinjaArmyH* ninjaArmy = NULL;
  char ** papszOptions = NULL;
  NinjaErr err = 0;
  err = NinjaInit(papszOptions); // must be called for fetching and simulations
  if(err != NINJA_SUCCESS)
  {
    printf("NinjaInit: err = %d\n", err);
  }


  /*
   * Testing fetching from a DEM point
   */
  err = NinjaFetchDEMPoint(ninjaArmy, adfPoints, adfBuff, units.c_str(), dfCellSize, strdup(pszDstFile.c_str()), strdup(fetchtype.c_str()), papszOptions);
  if (err != NINJA_SUCCESS) {
    printf("NinjaFetchDemPoint: err = %d\n", err);
  } else {
    printf("NinjaFetchDemPoint: elevation = %f\n", adfBuff[0]);
  }

  return NINJA_SUCCESS;
}



#endif // DATAFETCHER_H
