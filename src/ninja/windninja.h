/******************************************************************************
 *
 * $Id: ninja_conv.cpp 2258 2013-03-18 23:50:59Z kyle.shannon $
 *
 * Project:  WindNinja
 * Purpose:  C API
 * Author:   Levi Malott <lmnn3@mst.edu>
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
#include "ninja_errors.h"
/*-----------------------------------------------------------------------------
 *  Macros for Compilation Compatibility with gcc and g++
 *-----------------------------------------------------------------------------*/
#ifdef __cplusplus
    #define WN_C_START       extern "C" {
    #define WN_C_END         }
#else
    #define WN_C_START
    #define WN_C_END
#endif //__cplusplus

/*-----------------------------------------------------------------------------
 *  Handle Types for Internal WindNinja Classes
 *-----------------------------------------------------------------------------*/
//Ninja WINDNINJADLL_EXPORT NinjaCreateArmy(int nRuns);

#ifndef WINDNINJADLL_EXPORT
    #if defined(WIN32) && defined(WindNinjadll_EXPORTS)
        #define WINDNINJADLL_EXPORT _declspec(dllexport)
    #else
        #define WINDNINJADLL_EXPORT
    #endif //WIN32 && WINDNINJA_EXPORTS
#endif //WINDNINJADLL_EXPORT

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

WN_C_START

//Use structs instead of void * for type checking by C compilier
struct NinjaArmyH;
typedef struct NinjaArmyH NinjaArmyH;

struct NinjaToolsH;
typedef struct NinjaToolsH NinjaToolsH;

typedef int  NinjaErr;

#include "callbackFunctions.h"

    /*-----------------------------------------------------------------------------
     *  Contructor/Destructors
     *-----------------------------------------------------------------------------*/
    WINDNINJADLL_EXPORT NinjaArmyH* NinjaInitializeArmy();

    WINDNINJADLL_EXPORT NinjaErr NinjaMakeDomainAverageArmy
        ( NinjaArmyH * ninjaArmy, int numNinjas, bool momentumFlag, const double * speedList, const char * speedUnits, const double * directionList,
          const int * yearList, const int * monthList, const int * dayList, const int * hourList, const int * minuteList, const char * timeZone, const double * airTempList, const char* airTempUnits, const double * cloudCoverList, const char * cloudCoverUnits, char ** options);

    //TODO: add helper function to generate arrays of years, months, days, hours, and minutes from a station file
    WINDNINJADLL_EXPORT NinjaErr NinjaMakePointArmy
        ( NinjaArmyH * ninjaArmy, int * yearList, int * monthList, int * dayList, int * hourList, int * minuteList, int timeListSize, char * timeZone, const char ** stationFileNames, int numStationFiles, char * elevationFile, bool matchPointsFlag, bool momentumFlag, char ** options);

    //TODO: add helper function to get first and last timesteps in a forecast file
    //TODO: add helper function to get list of times in a forecast file
    //TODO: include parameters for start/stop times and a list of timesteps as options->for cases where you don't want to simulate every time step in the forecast file
    WINDNINJADLL_EXPORT NinjaErr NinjaMakeWeatherModelArmy
        ( NinjaArmyH * ninjaArmy, const char * forecastFilename, const char * timeZone, const char** inputTimeList, int size, bool momentumFlag, char ** options );

    WINDNINJADLL_EXPORT NinjaToolsH * NinjaMakeTools();

    WINDNINJADLL_EXPORT NinjaErr NinjaFetchWeatherData
        (NinjaToolsH* tools, const char* modelName, const char* demFile, int hours);

    WINDNINJADLL_EXPORT NinjaErr NinjaFetchArchiveWeatherData
        (NinjaToolsH* tools, const char* modelName, const char* demFile, const char * timeZone, int startYear, int startMonth, int startDay, int startHour, int endYear, int endMonth, int endDay, int endHour);

    WINDNINJADLL_EXPORT const char** NinjaGetAllWeatherModelIdentifiers(NinjaToolsH* tools, int* count);

    WINDNINJADLL_EXPORT NinjaErr NinjaGetWeatherModelHours
        (NinjaToolsH* tools, const char* modelIdentifier, int* starHours, int* endHour);

    WINDNINJADLL_EXPORT NinjaErr NinjaFreeAllWeatherModelIdentifiers
        (const char** identifiers, int count);

    WINDNINJADLL_EXPORT const char** NinjaGetWeatherModelTimeList
        (NinjaToolsH * tools, int * count, const char * fileName, const char * timeZone);

    WINDNINJADLL_EXPORT NinjaErr NinjaFreeWeatherModelTimeList
        (const char** timeList, int timeListSize);

    WINDNINJADLL_EXPORT NinjaErr NinjaFetchStationFromBBox
        (NinjaToolsH* tools, const int * yearList, const int * monthList, const int * dayList, const int * hourList, const int * minuteList, const int size, const char * elevationFile, double buffer, const char* units, const char * timeZone, bool fetchLatestFlag, const char * outputPath, bool locationFileFlag, char ** options );

    WINDNINJADLL_EXPORT NinjaErr NinjaFetchStationByName
        (NinjaToolsH* tools, const int * yearList, const int * monthList, const int * dayList, const int * hourList, const int * minuteList, const int size, const char * elevationFile, const char* stationList, const char * timeZone, bool fetchLatestFlag, const char * outputPath, bool locationFileFlag, char ** options );

    WINDNINJADLL_EXPORT NinjaErr NinjaFetchDEMPoint
        (NinjaArmyH * ninjaArmy, double * point, double * buff, const char * units, double cellSize, char * dstFile, char * fetchType, char ** options );

    WINDNINJADLL_EXPORT NinjaErr NinjaFetchDEMBBox
        (NinjaArmyH * ninjaArmy, double * boundsBox, const char * fileName, double resolution, char * fetchType, char ** options );

    WINDNINJADLL_EXPORT NinjaErr NinjaDestroyArmy
        ( NinjaArmyH * ninjaArmy, char ** options );

    /*-----------------------------------------------------------------------------
     *  Ninja Simulation Executors
     *-----------------------------------------------------------------------------*/
    WINDNINJADLL_EXPORT NinjaErr NinjaStartRuns
        ( NinjaArmyH * ninjaArmy, const unsigned int nprocessors, char ** options );

    WINDNINJADLL_EXPORT NinjaErr NinjaInit
        ( const char * runType, char ** options );

    /*-----------------------------------------------------------------------------
     *  Various Simulation Parameters
     *-----------------------------------------------------------------------------*/
    WINDNINJADLL_EXPORT NinjaErr NinjaSetDem
        ( NinjaArmyH * ninjaArmy, const int nIndex, const char * fileName, char ** options );

    WINDNINJADLL_EXPORT NinjaErr NinjaSetInMemoryDem
        ( NinjaArmyH * ninjaArmy, const int nIndex, const double * demValues,
          const int nXSize, const int nYSize, const double * geoRef, const char * prj, char ** options );

    WINDNINJADLL_EXPORT NinjaErr NinjaSetPosition
        ( NinjaArmyH * ninjaArmy, const int nIndex, char ** options );

    WINDNINJADLL_EXPORT NinjaErr NinjaSetInitializationMethod
        ( NinjaArmyH * ninjaArmy, const int nIndex, const char * initializationMethod, bool matchedPoints, char ** options );

    WINDNINJADLL_EXPORT NinjaErr NinjaSetNumberCPUs
        ( NinjaArmyH * ninjaArmy, const int nIndex, const int nCPUs, char ** options );

    /*  Communication  */
    WINDNINJADLL_EXPORT NinjaErr NinjaSetArmyComMessageHandler
        ( NinjaArmyH * ninjaArmy, ninjaComMessageHandler pMsgHandler, void *pUser, char ** options );

    WINDNINJADLL_EXPORT NinjaErr NinjaSetArmyMultiComStream
        ( NinjaArmyH * ninjaArmy, FILE* stream, char ** options );

    WINDNINJADLL_EXPORT NinjaErr NinjaSetToolsComMessageHandler
        ( NinjaToolsH * tools, ninjaComMessageHandler pMsgHandler, void *pUser, char ** options );

    WINDNINJADLL_EXPORT NinjaErr NinjaSetToolsMultiComStream
        ( NinjaToolsH * tools, FILE* stream, char ** options );

    /*  Input Parameters  */
    WINDNINJADLL_EXPORT NinjaErr NinjaSetInputSpeed
        ( NinjaArmyH * ninjaArmy, const int nIndex, const double speed, const char * units, char ** options );

    WINDNINJADLL_EXPORT NinjaErr NinjaSetInputDirection
        ( NinjaArmyH * ninjaArmy, const int nIndex, const double direction, char ** options );

    WINDNINJADLL_EXPORT NinjaErr NinjaSetInputWindHeight
        ( NinjaArmyH * ninjaArmy, const int nIndex, const double height, const char * units, char ** options );

    /*  Output Parameters  */
    WINDNINJADLL_EXPORT NinjaErr NinjaSetOutputWindHeight
        ( NinjaArmyH * ninjaArmy, const int nIndex, const double height, const char * units, char ** options );

    WINDNINJADLL_EXPORT NinjaErr NinjaSetOutputSpeedUnits
        ( NinjaArmyH * ninjaArmy, const int nIndex, const char * units, char ** options );

    WINDNINJADLL_EXPORT NinjaErr NinjaSetDiurnalWinds
        ( NinjaArmyH * ninjaArmy, const int nIndex, const bool flag, char ** options );

    WINDNINJADLL_EXPORT NinjaErr NinjaSetUniAirTemp
        ( NinjaArmyH * ninjaArmy, const int nIndex, const double temp, const char * units, char ** options );

    WINDNINJADLL_EXPORT NinjaErr NinjaSetUniCloudCover
        ( NinjaArmyH * ninjaArmy, const int nIndex, const double cloud_cover, const char * units, char ** options );

    WINDNINJADLL_EXPORT NinjaErr NinjaSetDateTime
        ( NinjaArmyH * ninjaArmy, const int nIndex, const int yr, const int mo,
          const int day, const int hr, const int min, const int sec,
          const char * timeZoneString, char ** options );

    WINDNINJADLL_EXPORT NinjaErr NinjaSetWxStationFilename
        ( NinjaArmyH * ninjaArmy, const int nIndex, const char * station_filename, char ** options );

    WINDNINJADLL_EXPORT NinjaErr NinjaSetUniVegetation
        ( NinjaArmyH * ninjaArmy, const int nIndex, const char * vegetation, char ** options );

    WINDNINJADLL_EXPORT NinjaErr NinjaSetNumVertLayers
        ( NinjaArmyH * ninjaArmy, const int nIndex, const int nLayers, char ** options );

    WINDNINJADLL_EXPORT char ** NinjaGetWxStations
        ( NinjaArmyH * ninjaArmy, const int nIndex, char ** options );

    WINDNINJADLL_EXPORT int NinjaGetDiurnalWindFlag
        ( NinjaArmyH * ninjaArmy, const int nIndex, char ** options );

    WINDNINJADLL_EXPORT const char * NinjaGetInitializationMethod
        ( NinjaArmyH * ninjaArmy, const int nIndex, char ** options );



    /*-----------------------------------------------------------------------------
     *  Dust Methods
     *-----------------------------------------------------------------------------*/
#ifdef EMISSIONS
    WINDNINJADLL_EXPORT NinjaErr NinjaSetDustFilename
        ( NinjaArmyH * ninjaArmy, const int nIndex, const char* filename, char ** options );

    WINDNINJADLL_EXPORT NinjaErr NinjaSetDustFileOut
        ( NinjaArmyH * ninjaArmy, const int nIndex, const char* filename, char ** options );

    WINDNINJADLL_EXPORT NinjaErr NinjaSetDustFlag
        ( NinjaArmyH * ninjaArmy, const int nIndex, const int flag, char ** options );
#endif //EMISSIONS

    /*-----------------------------------------------------------------------------
     *  Stability Methods
     *-----------------------------------------------------------------------------*/
    WINDNINJADLL_EXPORT NinjaErr NinjaSetStabilityFlag
        ( NinjaArmyH * ninjaArmy, const int nIndex, const bool flag, char ** options );

    WINDNINJADLL_EXPORT NinjaErr NinjaSetAlphaStability
        ( NinjaArmyH * ninjaArmy, const int nIndex, const double stability_, char ** options );

    /*-----------------------------------------------------------------------------
     *  NinjaFoam Methods
     *-----------------------------------------------------------------------------*/
    WINDNINJADLL_EXPORT NinjaErr NinjaSetMeshCount
        ( NinjaArmyH * ninjaArmy, const int nIndex, const int meshCount, char ** options );

    /*-----------------------------------------------------------------------------
     *  Mesh Methods
     *-----------------------------------------------------------------------------*/
    WINDNINJADLL_EXPORT NinjaErr NinjaSetMeshResolutionChoice
        ( NinjaArmyH * ninjaArmy, const int nIndex, const char * choice, char ** options );

    WINDNINJADLL_EXPORT NinjaErr NinjaSetMeshResolution
        ( NinjaArmyH * ninjaArmy, const int nIndex, const double resolution,
         const char * units, char ** options );

    WINDNINJADLL_EXPORT NinjaErr NinjaSetNumVertLayers
        ( NinjaArmyH * ninjaArmy, const int nIndex, int vertLayers, char ** options );


    /*-----------------------------------------------------------------------------
     *  Output Methods
     *  TODO: Fully expose pdf functions 
     *-----------------------------------------------------------------------------*/
    WINDNINJADLL_EXPORT NinjaErr NinjaSetOutputPath
        ( NinjaArmyH * ninjaArmy, const int nIndex, const char * path, char ** options );

    WINDNINJADLL_EXPORT const double* NinjaGetOutputSpeedGrid
        ( NinjaArmyH * ninjaArmy, const int nIndex, char ** options );

    WINDNINJADLL_EXPORT const double* NinjaGetOutputDirectionGrid
        ( NinjaArmyH * ninjaArmy, const int nIndex, char ** options );

    WINDNINJADLL_EXPORT const char* NinjaGetOutputGridProjection
        ( NinjaArmyH * ninjaArmy, const int nIndex, char ** options );

    WINDNINJADLL_EXPORT const double NinjaGetOutputGridCellSize
        ( NinjaArmyH * ninjaArmy, const int nIndex, char ** options );

    WINDNINJADLL_EXPORT const double NinjaGetOutputGridxllCorner
        ( NinjaArmyH * ninjaArmy, const int nIndex, char ** options );

    WINDNINJADLL_EXPORT const double NinjaGetOutputGridyllCorner
        ( NinjaArmyH * ninjaArmy, const int nIndex, char ** options );

    WINDNINJADLL_EXPORT const int NinjaGetOutputGridnCols
        ( NinjaArmyH * ninjaArmy, const int nIndex, char ** options );

    WINDNINJADLL_EXPORT const int NinjaGetOutputGridnRows
        ( NinjaArmyH * ninjaArmy, const int nIndex, char ** options );

    WINDNINJADLL_EXPORT NinjaErr NinjaSetOutputBufferClipping
        ( NinjaArmyH * ninjaArmy, const int nIndex, const double percent, char ** options );

    WINDNINJADLL_EXPORT NinjaErr NinjaSetStationKML
        ( NinjaArmyH * ninjaArmy, const int nIndex, const char * demFileName, const char * outputDirectory, const char * outputSpeedUnits, char ** papszOptions);

    WINDNINJADLL_EXPORT NinjaErr NinjaSetWxModelGoogOutFlag
        ( NinjaArmyH * ninjaArmy, const int nIndex, const bool flag, char ** options );

    WINDNINJADLL_EXPORT NinjaErr NinjaSetWxModelShpOutFlag
        ( NinjaArmyH * ninjaArmy, const int nIndex, const bool flag, char ** options );

    WINDNINJADLL_EXPORT NinjaErr NinjaSetWxModelAsciiOutFlag
        ( NinjaArmyH * ninjaArmy, const int nIndex, const bool flag, char ** options );

    WINDNINJADLL_EXPORT NinjaErr NinjaSetGoogOutFlag
        ( NinjaArmyH * ninjaArmy, const int nIndex, const bool flag, char ** options );

    WINDNINJADLL_EXPORT NinjaErr NinjaSetGoogResolution
        ( NinjaArmyH * ninjaArmy, const int nIndex, const double resolution,
          const char * units, char ** options );

    WINDNINJADLL_EXPORT NinjaErr NinjaSetGoogSpeedScaling
        ( NinjaArmyH * ninjaArmy, const int nIndex, const char * scaling, char ** options );

    WINDNINJADLL_EXPORT NinjaErr NinjaSetGoogColor
        ( NinjaArmyH * army, const int nIndex, const char * colorScheme, bool scaling, char ** papszOptions );

    WINDNINJADLL_EXPORT NinjaErr NinjaSetGoogLineWidth
        ( NinjaArmyH * ninjaArmy, const int nIndex, const double width, char ** papszOptions );

    WINDNINJADLL_EXPORT NinjaErr NinjaSetGoogConsistentColorScale
        ( NinjaArmyH * army, const int nIndex, bool flag, int numRuns, char ** papszOptions);

    WINDNINJADLL_EXPORT NinjaErr NinjaSetShpOutFlag
        ( NinjaArmyH * ninjaArmy, const int nIndex, const bool flag, char ** papszOptions );

    WINDNINJADLL_EXPORT NinjaErr NinjaSetShpResolution
        ( NinjaArmyH * ninjaArmy, const int nIndex, const double resolution,
          const char * units, char ** options );

    WINDNINJADLL_EXPORT NinjaErr NinjaSetAsciiOutFlag
        ( NinjaArmyH * ninjaArmy, const int nIndex, const bool flag, char ** papszOptions );

    WINDNINJADLL_EXPORT NinjaErr NinjaSetAsciiResolution
        ( NinjaArmyH * ninjaArmy, const int nIndex, const double resolution,
          const char * units, char ** options );

    WINDNINJADLL_EXPORT NinjaErr NinjaSetAsciiAtmFile
        ( NinjaArmyH * army, bool flag, char ** papszOptions);

    WINDNINJADLL_EXPORT NinjaErr NinjaSetVtkOutFlag
        ( NinjaArmyH * ninjaArmy, const int nIndex, const bool flag, char ** papszOptions );

    WINDNINJADLL_EXPORT NinjaErr NinjaSetTxtOutFlag
        ( NinjaArmyH * ninjaArmy, const int nIndex, const bool flag, char ** papszOptions );

    WINDNINJADLL_EXPORT NinjaErr NinjaSetPDFOutFlag
        ( NinjaArmyH* ninjaArmy, const int nIndex, const bool flag, char ** papszOptions );

    WINDNINJADLL_EXPORT NinjaErr NinjaSetPDFResolution
        ( NinjaArmyH* ninjaArmy, const int nIndex, const double resolution, const char * units, char ** papszOptions );

    WINDNINJADLL_EXPORT NinjaErr NinjaSetPDFLineWidth
        ( NinjaArmyH* ninjaArmy, const int nIndex, const float lineWidth, char ** papszOptions );

    WINDNINJADLL_EXPORT NinjaErr NinjaSetPDFBaseMap
        ( NinjaArmyH* ninjaArmy, const int nIndex, const int eType, char ** papszOptions );

    WINDNINJADLL_EXPORT NinjaErr NinjaSetPDFDEM
        ( NinjaArmyH* ninjaArmy, const int nIndex, const char * demFileName, char ** papszOptions );

    WINDNINJADLL_EXPORT NinjaErr NinjaSetPDFSize
        ( NinjaArmyH* army, const int nIndex, const double height, const double width, const unsigned short dpi, char ** papszOptions);

    WINDNINJADLL_EXPORT const char * NinjaGetOutputPath
        ( NinjaArmyH * ninjaArmy, const int nIndex, char ** options );


    /*-----------------------------------------------------------------------------
     *  Termination Methods
     *-----------------------------------------------------------------------------*/

    WINDNINJADLL_EXPORT NinjaErr NinjaReset( NinjaArmyH * ninjaArmy, char ** options );
    WINDNINJADLL_EXPORT NinjaErr NinjaCancel( NinjaArmyH * ninjaArmy, char ** options );
    WINDNINJADLL_EXPORT NinjaErr NinjaCancelAndReset( NinjaArmyH * ninjaArmy, char ** options );

    /*-----------------------------------------------------------------------------
     *  Helper Methods
     *-----------------------------------------------------------------------------*/
    WINDNINJADLL_EXPORT int NinjaGetWxStationHeaderVersion(const char * filePath, char ** papszOptions);
    WINDNINJADLL_EXPORT NinjaErr NinjaGetTimeList(
        NinjaToolsH* tools,
        const int* inputYearList, const int* inputMonthList, const int* inputDayList,
        const int* inputHourList, const int* inputMinuteList,
        int* outputYearList, int* outputMonthList, int* outputDayList,
        int* outputHourList, int* outputMinuteList,
        int nTimeSteps, const char* timeZone);
    WINDNINJADLL_EXPORT NinjaErr NinjaGenerateSingleTimeObject(
        NinjaToolsH* tools,
        int inputYear, int inputMonth, int inputDay, int inputHour, int inputMinute, const char* timeZone,
        int* outYear, int* outMonth, int* outDay, int* outHour, int* outMinute);
    WINDNINJADLL_EXPORT NinjaErr NinjaCheckTimeDuration
        (NinjaToolsH* tools, int* yearList, int* monthList, int * dayList, int * minuteList, int *hourList, int listSize, char ** papszOptions);
    WINDNINJADLL_EXPORT NinjaErr NinjaWriteBlankWxStationFile( const char * outputStationFilename, char ** papszOptions );
    WINDNINJADLL_EXPORT NinjaErr NinjaGetRunKmzFilenames(NinjaArmyH * army, int *numRuns, char*** kmzFilenames, int *numStationKmls, char*** stationKmlFilenames, char*** weatherModelKmzFilenames, char ** papszOptions);
    WINDNINJADLL_EXPORT NinjaErr NinjaDestroyRunKmzFilenames(int numRuns, char** kmzFilenames, int numStationKmls, char** stationKmlFilenames, char** weatherModelKmzFilenames, char ** papszOptions);

WN_C_END
