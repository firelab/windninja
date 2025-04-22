#ifndef PROVIDER_H
#define PROVIDER_H

#include "appstate.h"
#include "modeldata.h"
#include <QString>
#include <QTableWidget>
#include <vector>

class Provider {
  private:
    class NinjaArmyH* ninjaArmy = nullptr;
    const char* comType = "cli";
    const int nCPUs = 1;
    char** papszOptions = nullptr;
    int err = 0;

    int point_exec(class PointInitialization& input);
    int wxmodel_exec(class WeatherModel& input);

  public:
    Provider();
    int domain_average_exec(class DomainAverageWind& input);
    QVector<QVector<QString>> getTimeZoneData(bool showAllZones);
    QVector<QVector<QString>> parseDomainAvgTable(QTableWidget* table);
    QString getKmzFilePaths();
    QString getTimeZoneDetails(const QString& currentTimeZone);
    void setMapLayers();
};

#endif //PROVIDER_H
