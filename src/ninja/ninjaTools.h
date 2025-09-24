#ifndef NINJATOOLS_H
#define NINJATOOLS_H

#include "nomads_wx_init.h"

class ninjaTools
{
public:
    ninjaTools();
    void fetchWeatherModelData(const char* modelName, const char* demFile, int hours);
    std::vector<std::string> getForecastIdentifiers();
    int getStartHour(const char*modelIdentifier);
    int getEndHour(const char* modelIdentifer);

private:
    int nomadsCount;
    NomadsWxModel** nomadsModels;
    std::vector<std::string> modelIdentifiers;
};

#endif // NINJATOOLS_H
