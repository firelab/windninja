#ifndef NINJATOOLS_H
#define NINJATOOLS_H

#include "nomads_wx_init.h"
#include "wxModelInitializationFactory.h"

class ninjaTools
{
public:
    ninjaTools();
    void fetchWeatherModelData(const char* modelName, const char* demFile, int hours);
    std::vector<std::string> getForecastIdentifiers();
    std::vector<std::string> getTimeList(const char* modelName, std::string timeZone);
    int getStartHour(const char*modelIdentifier);
    int getEndHour(const char* modelIdentifer);

private:
    int nomadsCount;
    NomadsWxModel** nomadsModels;
    std::vector<std::string> modelIdentifiers;
};

#endif // NINJATOOLS_H
