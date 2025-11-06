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
}

void ninjaTools::fetchWeatherModelData(const char* modelName, const char* demFile, int hours)
{
    wxModelInitialization *model = NULL;
    model = wxModelInitializationFactory::makeWxInitializationFromId(std::string(modelName));

    if (!model) {
        throw std::runtime_error(std::string("Weather model not found: ") + modelName);
    }

    model->fetchForecast(demFile, hours);
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
