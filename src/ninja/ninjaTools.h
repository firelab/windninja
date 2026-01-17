#ifndef NINJATOOLS_H
#define NINJATOOLS_H

#include "nomads_wx_init.h"
#include "wxModelInitializationFactory.h"

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

    void fetchWeatherModelData(const char* modelName, const char* demFile, int hours);
    std::vector<std::string> getForecastIdentifiers();
    std::vector<std::string> getTimeList(const char* modelName, std::string timeZone);
    int getStartHour(const char*modelIdentifier);
    int getEndHour(const char* modelIdentifer);

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
