#include "ninjaTools.h"

ninjaTools::ninjaTools()
{
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
    CPLDebug( "WINDNINJA", "Loaded %d NOMADS models", nomadsCount );
}

void ninjaTools::fetchWeatherModelData(const char* modelName, const char* demFile, int hours)
{
    wxModelInitialization *model = NULL;
    for(int i = 0; i < nomadsCount; i++)
    {
        if(nomadsModels[i]->getForecastIdentifier() == modelName)
        {
            model = nomadsModels[i];
            break;
        }
    }

    if (!model) {
        throw std::runtime_error(std::string("Weather model not found: ") + modelName);
    }

    model->fetchForecast(demFile, hours);
}

std::vector<std::string> ninjaTools::getForecastIdentifiers()
{
    for(int i = 0; i < nomadsCount; i++)
    {
        modelIdentifiers.push_back(nomadsModels[i]->getForecastIdentifier());
    }
    return modelIdentifiers;
}

std::vector<std::string> ninjaTools::getTimeList(const char* fileName, std::string timeZone)
{
    std::string tz = "America/Denver";
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
    for(int i = 0; i < nomadsCount; i++)
    {
        if(nomadsModels[i]->getForecastIdentifier() == modelIdentifier)
        {
            model = nomadsModels[i];
            break;
        }
    }
    return model->getStartHour();
}

int ninjaTools::getEndHour(const char* modelIdentifier)
{
    wxModelInitialization *model = NULL;
    for(int i = 0; i < nomadsCount; i++)
    {
        if(nomadsModels[i]->getForecastIdentifier() == modelIdentifier)
        {
            model = nomadsModels[i];
            break;
        }
    }
    return model->getEndHour();
}
