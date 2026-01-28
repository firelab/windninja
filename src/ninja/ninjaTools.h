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

    void fetchWeatherModelData(const char* modelName, const char* demFile, int hours);
    void fetchArchiveWeatherModelData(const char* modelName, const char* demFile, const char* timeZone, int startYear, int startMonth, int startDay, int startHour, int endYear, int endMonth, int endDay, int endHour);
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
