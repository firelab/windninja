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
#include <stdbool.h>

/*TODO: 
 * Implement exhaustive tests for all wx_model_types for NinjaFetchForecast
 * Implement tests for NinjaFetchDemPoint (example code in apiTestPoint.c) and NinjaFetchStation
 */

int main()
{
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
    const char * demFileBBox = "/home/atw09001/src/wind/windninja/autotest/api/data/fetch/DEMBBox.tif"; // output file name
    char * fetch_type = "lcp"; // can be srtm, gmted, relief
    double resolution = 30.0; // 30 m resolution
    double boundsBox [] = {40.07, -104.0, 40.0, -104.07}; // Bounding box (north, east, south, west)
    err = NinjaFetchDEMBBox(ninjaArmy, boundsBox, demFileBBox, resolution, fetch_type, papszOptions);
    if (err != NINJA_SUCCESS){
        printf("NinjaFetchDEMBBox: err = %d\n", err);
    }
    
    /*
     * Testing fetching for a Forecast file 
     */
    const char*wx_model_type = "NOMADS-HRRR-CONUS-3-KM"; //  follow the cli argument output UCAR-NAM-CONUS-12-KM, 
    /* All Possible inputs for wx_model_type
     * UCAR-NAM-ALASKA-11-KM, 
     * UCAR-NDFD-CONUS-2.5-KM, 
     * UCAR-RAP-CONUS-13-KM, 
     * UCAR-GFS-GLOBAL-0.5-DEG, 
     * NOMADS-GFS-GLOBAL-0.25-DEG, 
     * NOMADS-HIRES-ARW-ALASKA-5-KM, 
     * NOMADS-HIRES-FV3-ALASKA-5-KM, 
     * NOMADS-HIRES-ARW-CONUS-5-KM, 
     * NOMADS-HIRES-FV3-CONUS-5-KM, 
     * NOMADS-HIRES-GUAM-5-KM, 
     * NOMADS-HIRES-HAWAII-5-KM, 
     * NOMADS-HIRES-PUERTO-RICO-5-KM, 
     * NOMADS-NAM-ALASKA-11.25-KM, 
     * NOMADS-NAM-CONUS-12-KM, 
     * NOMADS-NAM-NORTH-AMERICA-32-KM, 
     * NOMADS-NAM-NEST-ALASKA-3-KM, 
     * NOMADS-NAM-NEST-CONUS-3-KM, 
     * NOMADS-NAM-NEST-HAWAII-3-KM, 
     * NOMADS-NAM-NEST-PUERTO-RICO-3-KM, 
     * NOMADS-HRRR-ALASKA-3-KM, 
     * NOMADS-HRRR-CONUS-3-KM, 
     * NOMADS-HRRR-CONUS-SUBHOURLY-3-KM, 
     * NOMADS-HRRR-ALASKA-SUBHOURLY-3-KM, 
     * NOMADS-RAP-CONUS-13-KM, 
     * NOMADS-RAP-NORTH-AMERICA-32-KM
    */
    const char * demFileForecast = demFileBBox; // input DEM file
    int numNinjas = 1;
    const char* forecastFilename = NinjaFetchForecast(ninjaArmy, wx_model_type, numNinjas, demFileForecast, papszOptions);
    if (strcmp(forecastFilename, "exception") == 0){
        printf("NinjaFetchForecast: err = %s\n", forecastFilename);
    }
    else{
        printf("NinjaFetchForecast: forecastFilename = %s\n", forecastFilename);
    }

    /*
     * Testing fetching station data from a geotiff file.  
     */
    int year[2] = {2024, 2024};
    int month[2] = {2, 2};
    int day[2] = {2, 2};
    int hour[2] = {2, 2};
    int minute[2] = {2, 2};
    int size = 2;
    const char* output_path = "/home/atw09001/src/wind/windninja/autotest/api/data/fetch/";
    const char* elevation_file = "/home/atw09001/src/wind/windninja/autotest/api/data/missoula_valley.tif";
    const char* osTimeZone = "UTC";
    bool fetchLatestFlag = 1;
    double buffer = 10;
    const char* units = "mi";

    err = NinjaFetchStation(year, month, day, hour, minute, size, elevation_file, buffer, units, osTimeZone, fetchLatestFlag, output_path, papszOptions);
    if (err != NINJA_SUCCESS) {
        printf("NinjaFetchStation: err = %d\n", err);
    } else {
        printf("NinjaFetchStation: success\n");
    }

    /*
     * Testing fetching from a DEM point
     */
    double adfPoint[] = {-104.0, 40.07}; // Point coordinates (longitude, latitude)
    double adfBuff[] = {1.5, 1.5}; // Buffer to store the elevation value
    double dfCellSize = 30.0; // Cell size in meters
    char* pszDstFile = "/home/atw09001/src/wind/windninja/autotest/api/data/fetch/DEMpoint.tif";
    char* fetchType = "lcp";
    err = NinjaFetchDEMPoint(ninjaArmy, adfPoint, adfBuff, units, dfCellSize, pszDstFile, fetchType, papszOptions);
    if (err != NINJA_SUCCESS) {
        printf("NinjaFetchDemPoint: err = %d\n", err);
    } else {
        printf("NinjaFetchDemPoint: elevation = %f\n", adfBuff[0]);
    }

    return NINJA_SUCCESS;
}
