//
// Created by Codsl on 4/8/2025.
//

#ifndef PROVIDER_H
#define PROVIDER_H

#include "modeldata.h"
#include <vector>

class NinjaSim {
  private:
    class NinjaArmyH* ninjaArmy = nullptr;
    const char* comType = "cli";
    const int nCPUs = 1;
    char** papszOptions = nullptr;
    int err = 0;

    int domain_average_exec(class DomainAverageWind& input);
    int point_exec(class PointInitialization& input);
    int wxmodel_exec(class WeatherModel& input);

  public:
    explicit NinjaSim(class BaseInput& input);
};

#endif //PROVIDER_H
