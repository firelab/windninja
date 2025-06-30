#ifndef PROVIDER_H
#define PROVIDER_H

#include "appstate.h"
#include "modeldata.h"
#include <cmath>
#include <QDir>
#include <QFileInfo>
#include <QString>
#include <QTableWidget>
#include <vector>
#include <string>

using namespace std;

class Provider {
  private:
    class NinjaArmyH* ninjaArmy = nullptr;
    const char* comType = "cli";
    char** papszOptions = nullptr;
    int err = 0;

  public:
    Provider();
    int domain_average_exec(class DomainAverageWind& input);
    QVector<QVector<QString>> getTimeZoneData(bool showAllZones);
    QVector<QVector<QString>> parseDomainAvgTable(QTableWidget* table);
    vector<string> getOutputFileNames(QString demFile, QTableWidget* table, QString meshType, int numFiles, QString outputPath);
    QString getKmzFilePaths();
    QString getTimeZoneDetails(const QString& currentTimeZone);
    void setMapLayers();
    int point_exec(class PointInitialization& input);
    int wxmodel_exec(class WeatherModel& input);

    int fetchDEMBoundingBox(const string demFileOutPut, const string fetch_type, int resolution, double* boundsBox);
    int fetchForecast(const string wx_model_type, int numNinjas, const string demFile);
    int fetchStationData(int year, int month, int day, int hour, int minute, int numNinjas, const string output_path, int bufferZone, const string bufferUnits, const string demFile, const string osTimeZone, int fetchLatestFlag);
    int fetchFromDEMPoint (double adfPoints[2], double adfBuff[2], const string units, double dfCellSize, const string pszDstFile, const string fetchtype);

};

#endif //PROVIDER_H
