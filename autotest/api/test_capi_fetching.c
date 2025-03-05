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

// FetchStation we will always have "yearList, monthList, dayList, hourList, minuteList must be the same length!" error message thrown
// Comment the FetchStation section and the program will runwithout error (still needs more testing)

int main()
{
    NinjaArmyH* ninjaArmy = NULL; 
    char ** papszOptions = NULL;
    NinjaErr err = 0; 
    err = NinjaInit(papszOptions); //must be called for any simulation
    if(err != NINJA_SUCCESS)
    {
      printf("NinjaInit: err = %d\n", err);
    }

    const char * demFile = "output.tif"; // output file name
    char * fetch_type = "gmted";
    double resolution = 30; // 30 m resolution
    double boundsBox [] = {40.07, -104.0, 40.0, -104.07}; // Bounding box (north, east, south, west)
    err = NinjaFetchDEMBBox(ninjaArmy, boundsBox, demFile, resolution, fetch_type, papszOptions);
    if (err != NINJA_SUCCESS){
        printf("NinjaFetchDEMBBox: err = %d\n", err);
    }
    
    const char*wx_model_type = "NOMADS-HRRR-CONUS-3-KM";
    int numNinjas = 2;
    const char* forecastFilename = NinjaFetchForecast(ninjaArmy, wx_model_type, numNinjas, demFile, papszOptions);
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
    int year[1] = {2023};
    int month[1] = {10};
    int day[1] = {10};
    int hour[1] = {12};
    int minute[1] = {60};
    const char* output_path = "./station";
    const char* elevation_file = "output.tif";
    const char* osTimeZone = "UTC";
    bool fetchLatestFlag = 1;
    err = NinjaFetchStation(year, month, day, hour, minute, elevation_file, osTimeZone, fetchLatestFlag, output_path, papszOptions);
    if (err != NINJA_SUCCESS) {
        printf("NinjaFetchStation: err = %d\n", err);
    } else {
        printf("NinjaFetchStation: success\n");
    }

    double adfPoint[] = {104.0, 40.07}; // Point coordinates (longitude, latitude)
    double adfBuff[] = {30, 30}; // Buffer to store the elevation value
    const char* units = "mi";
    double dfCellSize = 30.0; // Cell size in meters
    char* pszDstFile = "dem_point_output.tif";
    char* fetchType = "gmted";
    err = NinjaFetchDEMPoint(ninjaArmy, adfPoint, adfBuff, units, dfCellSize, pszDstFile, fetchType, papszOptions);
    if (err != NINJA_SUCCESS) {
        printf("NinjaFetchDemPoint: err = %d\n", err);
    } else {
        printf("NinjaFetchDemPoint: elevation = %f\n", adfBuff[0]);
    }

    return NINJA_SUCCESS;
}
