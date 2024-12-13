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
    err = NinjaFetchDEMBBox(ninjaArmy, boundsBox, demFile, resolution, fetch_type);
    if (err != NINJA_SUCCESS){
        printf("NinjaFetchDEMBBox: err = %d\n", err);
    }
    fetch_type = "srtm";
    err = NinjaFetchDEMBBox(ninjaArmy, boundsBox, demFile, resolution, fetch_type);
    if (err != NINJA_SUCCESS){
        printf("NinjaFetchDEMBBox: err = %d\n", err);
    }
    fetch_type = "relief";
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

    - Implement tests for NinjaFetchDemPoint and NinjaFetchStation
    */


    return NINJA_SUCCESS;
}