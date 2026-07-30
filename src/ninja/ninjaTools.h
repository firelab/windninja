/******************************************************************************
*
* $Id$
*
* Project:  WindNinja
* Purpose:  A class to preform WindNinja related functions for the C API
* Author:   Mason Willman <mason.willman@usda.gov>
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

#ifndef NINJATOOLS_H
#define NINJATOOLS_H

#include "fetch_factory.h"
#include "nomads_wx_init.h"
#include "wxModelInitializationFactory.h"
#include "wxStation.h"
#include "pointInitialization.h"

#include "ninja_errors.h"

#include "callbackFunctions.h"

#include "ninjaCom.h"

class ninjaTools
{
public:

    ninjaTools();
    ~ninjaTools();

//    ninjaTools(const ninjaTools& A);
//    ninjaTools& operator=(ninjaTools const& A);

    ninjaComClass *Com;  // pointer to the ninjaTools level com handler

    int fetchDEMBBox(double *boundsBox, const char *fileName, double resolution, const char* fetchType, char ** papszOptions=NULL );
    int fetchDEMPoint(double * adfPoint, double *adfBuff, const char* units, double dfCellSize, const char * pszDstFile, const char* fetchType, char ** papszOptions=NULL );

    int fetchWeatherModelData(const char* modelName, const char* demFile, const char* timeZone, int hours);
    int fetchArchiveWeatherModelData(const char* modelName, const char* demFile, const char* timeZone, int startYear, int startMonth, int startDay, int startHour, int endYear, int endMonth, int endDay, int endHour);
    std::vector<std::string> getForecastIdentifiers();
    std::vector<std::string> getTimeList(const char* modelName, std::string timeZone);
    int getStartHour(const char*modelIdentifier);
    int getEndHour(const char* modelIdentifer);

    int fetchStationFromBBox( const int* yearList, const int * monthList, const int * dayList, const int * hourList, const int * minuteList, const int size, const char* elevationFile, double buffer, const char* units, const char* timeZone, bool fetchLatestFlag, const char* outputPath, bool locationFileFlag, char ** papszOptions=NULL );
    int fetchStationByName( const int* yearList, const int * monthList, const int * dayList, const int * hourList, const int * minuteList, const int size, const char* elevationFile, const char* stationList, const char* timeZone, bool fetchLatestFlag, const char* outputPath, bool locationFileFlag, char ** papszOptions=NULL );
    int getTimeList( const int * inputYearList, const int * inputMonthList, const int * inputDayList, const int * inputHourList, const int * inputMinuteList, int * outputYearList, int* outputMonthList, int * outputDayList, int * outputHourList, int* outputMinuteList, int nTimeSteps, const char* timeZone );
    int generateSingleTimeObject( int inputYear, int inputMonth, int inputDay, int inputHour, int inputMinute, const char * timeZone, int * outYear, int * outMonth, int* outDay, int * outHour, int * outMinute );
    int checkTimeDuration( int* yearList, int* monthList, int * dayList, int * minuteList, int *hourList, int listSize, char ** papszOptions=NULL );

private:
    int nomadsCount;
    NomadsWxModel** nomadsModels;
    std::vector<std::string> modelIdentifiers;

public:
    /*-----------------------------------------------------------------------------
     *  Ninja Communication Methods
     *-----------------------------------------------------------------------------*/

    /**
    * \brief Set a ninjaComMessageHandler callback function to the ninjaTools level ninjaCom
    *
    * \param pMsgHandler A pointer to a ninjaComMessageHandler callback function.
    * \param pUser A pointer to the object or context associated with the callback function.
    * \return errval Returns NINJA_SUCCESS upon success
    */
    int setNinjaComMessageHandler( ninjaComMessageHandler pMsgHandler, void *pUser,
                                   char ** papszOptions = NULL);

    /**
    * \brief Set a ninjaCom multi-stream FILE handle to the ninjaTools level ninjaCom
    *
    * \param stream A pointer to a multi-stream FILE handle/stream.
    * \return errval Returns NINJA_SUCCESS upon success
    */
    int setNinjaMultiComStream( FILE* stream,
                                char ** papszOptions = NULL);
};

#endif // NINJATOOLS_H
