#ifndef MODELDATA_H
#define MODELDATA_H

#include <string.h>
#include <vector>

using namespace std;

class BaseInput {
private:
  char* demFile;
  double outputResolution;
  string initializationMethod;
  string meshChoice;
  string vegetation;
  int nLayers;
  int diurnalFlag;
  double height;
  bool momentumFlag;
  int numNinjas;

public:
  // Constructor
  BaseInput(char* demFile, double outputResolution, string initializationMethod, string meshChoice, string vegetation,
            int nLayers, int diurnalFlag, double height, bool momentumFlag, int numNinjas)
      : demFile(demFile), outputResolution(outputResolution), initializationMethod(initializationMethod), meshChoice(meshChoice),
        vegetation(vegetation), nLayers(nLayers), diurnalFlag(diurnalFlag), height(height), momentumFlag(momentumFlag), numNinjas(numNinjas) {}

         // Getter methods
  char* getDemFile() { return demFile; }
  double getOutputResolution() { return outputResolution; }
  string getInitializationMethod() { return initializationMethod; }
  string getMeshChoice() { return meshChoice; }
  string getVegetation() { return vegetation; }
  int getNLayers() { return nLayers; }
  int getDiurnalFlag() { return diurnalFlag; }
  double getHeight() { return height; }
  bool getMomentumFlag() { return momentumFlag; }
  int getNumNinjas() { return numNinjas; }
};

class DomainAverageWind : public BaseInput {
private:
  vector<double> speedList;
  char* speedUnits;
  vector<double> directionList;

public:
  // Constructor
  DomainAverageWind(char* demFile, double outputResolution, string initializationMethod, string meshChoice, string vegetation,
                    int nLayers, int diurnalFlag, double height, bool momentumFlag, int numNinjas,
                    vector<double> speedList, char* speedUnits, vector<double> directionList)
      : BaseInput(demFile, outputResolution, initializationMethod, meshChoice, vegetation, nLayers, diurnalFlag, height, momentumFlag, numNinjas),
        speedList(speedList), speedUnits(speedUnits), directionList(directionList) {}

         // Getter methods
  vector<double> getSpeedList() { return speedList; }
  char* getSpeedUnits() { return speedUnits; }
  vector<double> getDirectionList() { return directionList; }
};

class PointInitialization : public BaseInput {
private:
  int year;
  int month;
  int day;
  int hour;
  int minute;
  char* station_path;
  char* elevation_file;
  char* osTimeZone;

public:
  // Constructor
  PointInitialization(char* demFile, double outputResolution, string initializationMethod, string meshChoice, string vegetation,
                      int nLayers, int diurnalFlag, double height, bool momentumFlag, int numNinjas,
                      int year, int month, int day, int hour, int minute, char* station_path, char* elevation_file, char* osTimeZone)
      : BaseInput(demFile, outputResolution, initializationMethod, meshChoice, vegetation, nLayers, diurnalFlag, height, momentumFlag, numNinjas),
        year(year), month(month), day(day), hour(hour), minute(minute), station_path(station_path), elevation_file(elevation_file), osTimeZone(osTimeZone) {}

         // Getter methods
  int getYear() { return year; }
  int getMonth() { return month; }
  int getDay() { return day; }
  int getHour() { return hour; }
  int getMinute() { return minute; }
  char* getStationPath() { return station_path; }
  char* getElevationFile() { return elevation_file; }
  char* getOSTimeZone() { return osTimeZone; }
};

class WeatherModel : public BaseInput {
private:
  char* forecast;
  char* osTimeZone;

public:
  // Constructor
  WeatherModel(char* demFile, double outputResolution, string initializationMethod, string meshChoice, string vegetation,
               int nLayers, int diurnalFlag, double height, bool momentumFlag, int numNinjas,
               char* forecast, char* osTimeZone)
      : BaseInput(demFile, outputResolution, initializationMethod, meshChoice, vegetation, nLayers, diurnalFlag, height, momentumFlag, numNinjas),
        forecast(forecast), osTimeZone(osTimeZone) {}

         // Getter methods
  char* getForecast() { return forecast; }
  char* getOSTimeZone() { return osTimeZone; }
};

class modeldata {
public:
  modeldata() {}
};

#endif // MODELDATA_H
