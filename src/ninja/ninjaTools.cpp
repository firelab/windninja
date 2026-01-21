#include "ninjaTools.h"

/**
* @brief Default constructor.
*
*/
ninjaTools::ninjaTools()
{
    Com = new ninjaComClass();
    Com->runNumber = 9999;
    Com->printRunNumber = false;

    nomadsCount = 0;
    while( apszNomadsKeys[nomadsCount][0] != NULL )
    {
        nomadsCount++;
    }
    nomadsModels = new NomadsWxModel*[nomadsCount];
    int i = 0;
    while( apszNomadsKeys[i][0] != NULL )
    {
        nomadsModels[i] = new NomadsWxModel( apszNomadsKeys[i][0] );
        i++;
    }
}

/**
* @brief Destructor.
*
*/
ninjaTools::~ninjaTools()
{
    if(nomadsModels)
    {
        for(int i = 0; i < nomadsCount; i++)
        {
            if(nomadsModels[i])
            {
                free(nomadsModels[i]);
            }
        }

        free(nomadsModels);
    }

    delete Com;
}

/**
* @brief Copy constructor.
*
* @param An Object to copy.
*/
/*ninjaTools::ninjaTools(const ninjaTools& A)
{
    nomadsCount = A.nomadsCount;
    nomadsModels = new NomadsWxModel*[nomadsCount];
    for(int i = 0; i < nomadsCount; i++)
    {
        nomadsModels[i] = new NomadsWxModel( apszNomadsKeys[i][0] ); // this should ACTUALLY be something like "new NomadsWxModel( A.nomadsModels[i] )" or something like that, but I don't think it has a proper copy constructor setup. I guess just replicate the constructor for now.
    }

    Com = new ninjaComClass(*A.Com);
}*/

/**
* @brief Equals operator.
*
* @param A Right-hand side.
* @return An Object equal to the one on the right-hand side;
*/
/*ninjaTools& ninjaTools::operator=(ninjaTools const& A)
{
    if(&A != this)
    {
        // I don't even want to know where to begin for the nomadsModels here

        delete Com;
        Com = new ninjaComClass();
        *Com = *A.Com;
    }
    return *this;
}*/

void ninjaTools::fetchWeatherModelData(const char* modelName, const char* demFile, int hours)
{
    try
    {
        wxModelInitialization *model = NULL;

        model = wxModelInitializationFactory::makeWxInitializationFromId(std::string(modelName));

        if (!model) {
            throw std::runtime_error(std::string("Weather model not found: ") + modelName);
        }

        std::string forecastFileName = model->fetchForecast(demFile, hours);
        if(forecastFileName == "exception")
        {
            Com->ninjaCom(ninjaComClass::ninjaFailure, "Exception caught: ninjaTools::fetchWeatherModelData() returned an invalid forecastFileName.");
            return NINJA_E_INVALID;
        }
    }
    catch(armyException &e)
    {
        Com->ninjaCom(ninjaComClass::ninjaFailure, "Exception caught: %s", e.what());
        throw;
    }
    catch( exception& e )
    {
        Com->ninjaCom(ninjaComClass::ninjaFailure, "Exception caught: %s", e.what());
        throw;
    }
    catch( ... )
    {
        Com->ninjaCom(ninjaComClass::ninjaFailure, "Exception caught: Cannot determine exception type.");
        throw;
    }
}

std::vector<std::string> ninjaTools::getForecastIdentifiers()
{
    ncepGfsSurfInitialization gfs;
    ncepNamSurfInitialization nam;
    ncepNamAlaskaSurfInitialization namAla;
    ncepRapSurfInitialization rap;
    ncepNdfdInitialization ndfd;

    modelIdentifiers.push_back(ndfd.getForecastIdentifier());
    modelIdentifiers.push_back(nam.getForecastIdentifier());
    modelIdentifiers.push_back(rap.getForecastIdentifier());
    modelIdentifiers.push_back(namAla.getForecastIdentifier());
    modelIdentifiers.push_back(gfs.getForecastIdentifier());

    for(int i = 0; i < nomadsCount; i++)
    {
        modelIdentifiers.push_back(nomadsModels[i]->getForecastIdentifier());
    }

    GCPWxModel archive;
    modelIdentifiers.push_back(archive.getForecastIdentifier());
    return modelIdentifiers;
}

std::vector<std::string> ninjaTools::getTimeList(const char* fileName, std::string timeZone)
{
    wxModelInitialization *model = NULL;
    model = wxModelInitializationFactory::makeWxInitialization(fileName);
    std::vector<blt::local_date_time> temp = model->getTimeList(timeZone);
    std::vector<std::string> timeList;

    for(int i = 0; i < temp.size(); i++)
    {
        timeList.push_back(temp[i].to_string());
    }

    return timeList;
}

int ninjaTools::getStartHour(const char* modelIdentifier)
{
    wxModelInitialization *model = NULL;
    model = wxModelInitializationFactory::makeWxInitializationFromId(modelIdentifier);
    return model->getStartHour();
}

int ninjaTools::getEndHour(const char* modelIdentifier)
{
    wxModelInitialization *model = NULL;
    model = wxModelInitializationFactory::makeWxInitializationFromId(modelIdentifier);
    return model->getEndHour();
}

/*-----------------------------------------------------------------------------
 *  Ninja Communication Methods
 *-----------------------------------------------------------------------------*/

int ninjaTools::setNinjaComMessageHandler( ninjaComMessageHandler pMsgHandler, void *pUser,
                                           char ** papszOptions )
{
    try
    {
        Com->set_messageHandler(pMsgHandler, pUser);
    }
    catch( ... )
    {
        std::cerr << "CRITICAL: ninjaTools level ninjaComMessageHandler not set. Messages will NOT be delivered." << std::endl;
        return NINJA_E_INVALID;
    }
    return NINJA_SUCCESS;
}

int ninjaTools::setNinjaMultiComStream( FILE* stream,
                                        char ** papszOptions )
{
    try
    {
        Com->multiStream = stream;
    }
    catch( ... )
    {
        std::cerr << "ERROR: ninjaTools level ninjaCom multiStream FILE pointer not set." << std::endl;
        return NINJA_E_INVALID;
    }
    return NINJA_SUCCESS;
}
