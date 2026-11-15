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
#include <stdio.h> //for printf, FILE, fopen, fclose
#include <stdbool.h>

#define MAX_PATH_LEN 512

/*TODO: 
 * Implement exhaustive tests for all wx_model_types for NinjaFetchForecast
 * Implement tests for NinjaFetchDemPoint (example code in apiTestPoint.c) and NinjaFetchStation
 */

int main()
{
    /*
     * Setting up NinjaTools
     */
    NinjaToolsH* ninjaTools = NULL;
    const char * runType = "C-API autotest";
    char ** papszOptions = NULL;
    NinjaErr err = 0; 
    err = NinjaInit(runType, papszOptions); //initialize global singletons and environments (GDAL_DATA, etc.)
    if(err != NINJA_SUCCESS)
    {
        printf("NinjaInit: err = %d\n", err);
    }

    // manually set your wnDataPath (makes it easier for setting paths and testing)
    // must replace the "~/" part with your exact path
    const char* wnDataPath = "~/src/wind/windninja/data";

    /*
     * Setting up a log file, for ninjaCom, if desired
     */
    char multiStreamFilename[MAX_PATH_LEN];
    snprintf(multiStreamFilename, sizeof(multiStreamFilename), "%s%s", wnDataPath, "/../autotest/api/data/ninja.log");
    FILE* multiStream = fopen(multiStreamFilename, "w+");
    if(multiStream == NULL)
    {
        printf("error opening log file\n");
    }

    /*
     * Initialize ninjaTools
     */
    ninjaTools = NinjaMakeTools();

    /*
     * Customize the ninja communication
     */
    err = NinjaSetToolsMultiComStream(ninjaTools, multiStream, papszOptions);
    if(err != NINJA_SUCCESS)
    {
        printf("NinjaSetToolsMultiComStream: err = %d\n", err);
    }

    /*
     * Testing fetching from a DEM bounding box
     * TODO: implement nan filling of the result, if needed.
     */
    char demFileBBox[MAX_PATH_LEN];
    snprintf(demFileBBox, sizeof(demFileBBox), "%s%s", wnDataPath, "/../autotest/api/data/fetch/DEMBBox.tif"); // output file name
    char * fetch_type = "lcp"; // can be srtm, gmted, relief
    double resolution = 30.0; // 30 m resolution
    double boundsBox [] = {40.07, -104.0, 40.0, -104.07}; // Bounding box (north, east, south, west)
    err = NinjaFetchDEMBBox(ninjaTools, boundsBox, demFileBBox, resolution, fetch_type, papszOptions);
    if(err != NINJA_SUCCESS)
    {
        printf("NinjaFetchDEMBBox: err = %d\n", err);
    } else
    {
        printf("NinjaFetchDEMBBox: success\n");
    }
    
    /*
     * Testing fetching for a Forecast file
     */
    const char* wx_model_type = "NOMADS-HRRR-CONUS-3-KM";  // follow the cli argument output UCAR-NAM-CONUS-12-KM,
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
    const char* osTimeZone_forecast = "UTC";
    int numForecastHours = 1;
    err = NinjaFetchWeatherData(ninjaTools, wx_model_type, demFileForecast, osTimeZone_forecast, numForecastHours);//, papszOptions);
    if(err != NINJA_SUCCESS)
    {
        printf("NinjaFetchWeatherData: err = %d\n", err);
    } else
    {
        printf("NinjaFetchWeatherData: success\n");
    }

    /*
     * Testing fetching for a Pastcast file
     */
    const char* wx_model_type_pastcast = "PASTCAST-GCP-HRRR-CONUS-3-KM";
    const char* osTimeZone_pastcast = "UTC";
    int startYear = 2024;
    int startMonth = 2;
    int startDay = 2;  // UTC time
    int startHour = 2;  // UTC time
    int endYear = 2024;
    int endMonth = 2;
    int endDay = 2;  // UTC time
    int endHour = 2;  // UTC time
    err = NinjaFetchArchiveWeatherData(ninjaTools, wx_model_type_pastcast, demFileForecast, osTimeZone_pastcast, startYear, startMonth, startDay, startHour, endYear, endMonth, endDay, endHour);//, papszOptions);
    if(err != NINJA_SUCCESS)
    {
        printf("NinjaFetchArchiveWeatherData: err = %d\n", err);
    } else
    {
        printf("NinjaFetchArchiveWeatherData: success\n");
    }

    /*
     * Testing fetching station data from a geotiff file.
     */
    // always lists of size 2 for these methods
    // if doing a latestTime fetch, the fetch just ignores the later times in the list
    int size = 2;
    int year[2] = {2024, 2024};
    int month[2] = {2, 2};
    int day[2] = {2, 2};  // UTC time
    int hour[2] = {2, 2};  // UTC time
    int minute[2] = {2, 2};
    char output_path[MAX_PATH_LEN];
    char elevation_file[MAX_PATH_LEN];
    snprintf(output_path, sizeof(output_path), "%s%s", wnDataPath, "/../autotest/api/data/fetch/");
    snprintf(elevation_file, sizeof(elevation_file), "%s%s", wnDataPath, "/../autotest/api/data/missoula_valley.tif");
    const char* osTimeZone = "UTC";
    bool fetchLatestFlag = 0;  // set to 1 if you want the latestTime instead of the above time list
    double buffer = 10;  // distance around elevation file
    const char* units = "mi";
    bool locationFileFlag = 1; //set to 1 if you want the location file for the station data to be output

    err = NinjaFetchStationFromBBox(ninjaTools, year, month, day, hour, minute, size, elevation_file, buffer, units, osTimeZone, fetchLatestFlag, output_path, locationFileFlag, papszOptions);
    if(err != NINJA_SUCCESS)
    {
        printf("NinjaFetchStationFromBBox: err = %d\n", err);
    } else
    {
        printf("NinjaFetchStationFromBBox: success\n");
    }

    /*
     * Testing fetching station data from a list of station IDs.
     * use the helper functions to generate the desired list of times for this fetching method
     * so convert from local time to UTC time
     *
     * hrm, I like that it comes out with the same times as above
     * but I DON'T like that the filenames come out different, for UTC vs local time. a future TODO in sorting this out.
     */
    // always lists of size 2 for these methods
    // if doing a latestTime fetch, the fetch just ignores the later times in the list
    const char* stationIds = "KMSO,BLMM8,PNTM8,FINM8";
    ////const char* osTimeZone_fromId = "UTC";
    const char* osTimeZone_fromId = "America/Denver";
    //int inSize = 2;
    int inYear[2] = {2024, 2024};
    int inMonth[2] = {2, 2};
    int inDay[2] = {1, 1};  // local time
    int inHour[2] = {19, 19};  // local time, 7 hrs earlier
    //int inDay[2] = {2, 2};  // UTC time
    //int inHour[2] = {2, 2};  // UTC time
    int inMinute[2] = {2, 2};
    int outSize = 2;
    // make sure the outputs have the same size as outSize.
    int outYear[2], outMonth[2], outDay[2], outHour[2], outMinute[2];
    err = NinjaGetTimeList(ninjaTools, inYear, inMonth, inDay, inHour, inMinute, outYear, outMonth, outDay, outHour, outMinute, outSize, osTimeZone_fromId);
    if(err != NINJA_SUCCESS)
    {
        printf("NinjaGetTimeList: err = %d\n", err);
    }
    if(fetchLatestFlag == 0)
    {
        err = NinjaCheckTimeDuration(ninjaTools, outYear, outMonth, outDay, outHour, outMinute, outSize, papszOptions);
        if(err != NINJA_SUCCESS)
        {
            printf("NinjaCheckTimeDuration: err = %d\n", err);
        }
    }
    err = NinjaFetchStationByName(ninjaTools, outYear, outMonth, outDay, outHour, outMinute, outSize, elevation_file, stationIds, osTimeZone_fromId, fetchLatestFlag, output_path, locationFileFlag, papszOptions);
    if(err != NINJA_SUCCESS)
    {
        printf("NinjaFetchStationByName: err = %d\n", err);
    } else
    {
        printf("NinjaFetchStationByName: success\n");
    }

    /*
     * Testing fetching from a DEM point
     * TODO: implement nan filling of the result, if needed.
     */
    double adfPoint[] = {-104.0, 40.07}; // Point coordinates (longitude, latitude)
    double adfBuff[] = {1.5, 1.5}; // Buffer to store the elevation value
    double dfCellSize = 30.0; // Cell size in meters
    char pszDstFile[MAX_PATH_LEN];
    snprintf(pszDstFile, sizeof(pszDstFile), "%s%s", wnDataPath, "/../autotest/api/data/fetch/DEMpoint.tif");
    char* fetchType = "lcp";
    err = NinjaFetchDEMPoint(ninjaTools, adfPoint, adfBuff, units, dfCellSize, pszDstFile, fetchType, papszOptions);
    if(err != NINJA_SUCCESS)
    {
        printf("NinjaFetchDemPoint: err = %d\n", err);
    } else
    {
        printf("NinjaFetchDemPoint: elevation = %f\n", adfBuff[0]);
    }

    /*
     * Clean up
     */

    // not yet needed/implemented in the code, but it should be.
    //err = NinjaDestroyTools(ninjaTools, papszOptions);
    //if(err != NINJA_SUCCESS)
    //{
    //    printf("NinjaDestroyTools: err = %d\n", ninjaErr);
    //}

    if(multiStream != NULL)
    {
        if(fclose(multiStream) != 0)
        {
            printf("error closing log file\n");
        }
    }

    // must be called to cleanup ninjaInit();
    err = NinjaFinalize(papszOptions);
    if(err != NINJA_SUCCESS)
    {
        printf("NinjaFinalize: err = %d\n", err);
    }

    return NINJA_SUCCESS;
}
