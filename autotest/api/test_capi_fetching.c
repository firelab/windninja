/******************************************************************************
 *
 * Project:  WindNinja
 * Purpose:  C API testing
 * Author:   Nicholas Kim <kim.n.j@wustl.edu>
 *
 * gcc -g -Wall -o test_capi_fetching test_capi_fetching.c -lninja
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
#include <stdio.h> //for printf and strcmp
#include <string.h>

int main()
{
    NinjaH* ninjaArmy = NULL; 
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
    err = NinjaFetchDEMBBox(ninjaArmy, boundsBox, demFile, resolution, fetch_type);
    if (err != NINJA_SUCCESS){
        printf("NinjaFetchDEMBBox: err = %d\n", err);
    }
    const char*wx_model_type = "NOMADS-HRRR-CONUS-3-KM";
    int numNinjas = 2;
    const char* forecastFilename = NinjaFetchForecast(ninjaArmy, wx_model_type, numNinjas, demFile);
    if (strcmp(forecastFilename, "exception") == 0){
        printf("NinjaFetchForecast: err = %s\n", forecastFilename);
    }
    else{
        printf("NinjaFetchForecast: forecastFilename = %s\n", forecastFilename);
    }

    /*TODO: 
    
    - Implement exhaustive tests for all wx_model_types for NinjaFetchForecast

    - Implement tests for NinjaFetchDemPoint (example code in apiTestPoint.c) and NinjaFetchStation
    */
   
    int year[] = {2023};
    int month[] = {10};
    int day[] = {10};
    int hour[] = {12};
    int timeListSize = 1;
    const char* output_path = "./station";
    const char* elevation_file = "output.tif";
    const char* osTimeZone = "UTC";
    int fetchLatestFlag = 1;

    err = NinjaFetchStation(year, month, day, hour, timeListSize, output_path, elevation_file, osTimeZone, fetchLatestFlag);
    if (err != NINJA_SUCCESS) {
        printf("NinjaFetchStation: err = %d\n", err);
    } else {
        printf("NinjaFetchStation: success\n");
    }

    /*
        CPL DEBUG OUTPUT for NinjaFetchStation:

        STATION_FETCH: Found 1 Stations...
        STATION_FETCH: Path is Good...
        STATION_FETCH: Data fetched successfully, but hFeature returned NULL for station: 0. Skipping this station
        GDAL: GDALClose(https://api.synopticdata.com/v2/stations/timeseries?&bbox=-104.070943,40.000544,-103.998944,40.069408&network=1,2&vars=wind_speed,wind_direction,air_temp,solar_radiation,cloud_layer_1_code,cloud_layer_2_code,cloud_layer_3_code&status=active&recent=60&output=geojson&token=33e3c8ee12dc499c86de1f2076a9e9d4, this=0x55ec3fcf4c40)
        STATION_FETCH: DATA CHECK FAILED ON ALL STATIONS...
        NinjaFetchStation: err = 2
    */

    double adfPoint[] = {104.0, 40.07}; // Point coordinates (longitude, latitude)
    double adfBuff[] = {30, 30}; // Buffer to store the elevation value
    const char* units = "mi";
    double dfCellSize = 30.0; // Cell size in meters
    char* pszDstFile = "dem_point_output.tif";
    char* fetchType = "gmted";

    err = NinjaFetchDEMPoint(ninjaArmy, adfPoint, adfBuff, units, dfCellSize, pszDstFile, papszOptions, fetchType);
    if (err != NINJA_SUCCESS) {
        printf("NinjaFetchDemPoint: err = %d\n", err);
    } else {
        printf("NinjaFetchDemPoint: elevation = %f\n", adfBuff[0]);
    }
    return NINJA_SUCCESS;
    /*
        CPL DEBUG OUTPUT for NinjaFetchDemPoint:

        GDAL: GDALOpen(/vsizip//home/nickadmin/Firelab/windninja/autotest/api/../../../windninja/data/surface_data.zip/gmted.vrt, this=0x56138a22a270) succeeds as VRT.
        GDAL: GDALClose(/vsizip//home/nickadmin/Firelab/windninja/autotest/api/../../../windninja/data/surface_data.zip/gmted.vrt, this=0x56138a22a270)
        GDAL: GDALOpen(/vsizip//home/nickadmin/Firelab/windninja/autotest/api/../../../windninja/data/surface_data.zip/gmted.vrt, this=0x56138a22a270) succeeds as VRT.
        ERROR 1: PROJ: proj_create_from_database: crs not found
        GDAL: Could not compute area of interest
        ERROR 1: PROJ: proj_create_from_database: crs not found
        ERROR 1: PROJ: proj_create: unrecognized format / unknown name
        ERROR 6: Cannot find coordinate operations from `EPSG:4326' to `'
        NinjaFetchDemPoint: err = 2
    */
}
