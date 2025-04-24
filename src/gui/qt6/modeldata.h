#ifndef MODELDATA_H
#define MODELDATA_H

#include <string>
#include <vector>

using namespace std;

class BaseInput {
  private:
    string demFile;
    double outputResolution;
    string initializationMethod;
    string meshChoice;
    string vegetation;
    int nLayers;
    int diurnalFlag;
    double height;
    string heightUnits;
    bool momentumFlag;
    int numNinjas;
    string outputPath;

  public:
    BaseInput() {}

    // Constructor
    BaseInput(string demFile, double outputResolution, string initializationMethod, string meshChoice, string vegetation,
              int nLayers, int diurnalFlag, double height, string heightUnits, bool momentumFlag, int numNinjas, string outputPath)
      : demFile(demFile), outputResolution(outputResolution), initializationMethod(initializationMethod), meshChoice(meshChoice),
          vegetation(vegetation), nLayers(nLayers), diurnalFlag(diurnalFlag), height(height), heightUnits(heightUnits), momentumFlag(momentumFlag), numNinjas(numNinjas), outputPath(outputPath) {}

    // Getter methods
    const string& getDemFile() const { return demFile; }
    double getOutputResolution() { return outputResolution; }
    const string& getInitializationMethod() const { return initializationMethod; }
    const string& getMeshChoice() const { return meshChoice; }
    const string& getVegetation() const { return vegetation; }
    int getNLayers() { return nLayers; }
    int getDiurnalFlag() { return diurnalFlag; }
    double getHeight() { return height; }
    const string& getHeightUnits() const { return heightUnits; }
    bool getMomentumFlag() { return momentumFlag; }
    int getNumNinjas() { return numNinjas; }
    const string& getOutputPath() const { return outputPath; }
};

class DomainAverageWind : public BaseInput {
  private:
    vector<double> speedList;
    string speedUnits;
    vector<double> directionList;

  public:
    // Constructor
    DomainAverageWind(const BaseInput& baseInput,
                      vector<double> speedList, string speedUnits, vector<double> directionList)
      : BaseInput(baseInput), speedList(speedList), speedUnits(speedUnits), directionList(directionList) {}

    // Getter methods
    vector<double> getSpeedList() { return speedList; }
    const string& getSpeedUnits() const { return speedUnits; }
    vector<double> getDirectionList() { return directionList; }
};

class PointInitialization : public BaseInput {
  private:
    vector<int> year;
    vector<int> month;
    vector<int> day;
    vector<int> hour;
    vector<int> minute;
    char* station_path;
    char* osTimeZone;
    bool matchPointFlag;

  public:
    // Constructor
    PointInitialization(const BaseInput& baseInput,
                        vector<int> year, vector<int> month, vector<int> day, vector<int> hour, vector<int> minute, char* station_path, char* osTimeZone, bool matchPointFlag)
        : BaseInput(baseInput), year(year), month(month), day(day), hour(hour), minute(minute), station_path(station_path), osTimeZone(osTimeZone), matchPointFlag(matchPointFlag) {}

    // Getter methods
    vector<int> getYear() { return year; }
    vector<int> getMonth() { return month; }
    vector<int> getDay() { return day; }
    vector<int> getHour() { return hour; }
    vector<int> getMinute() { return minute; }
    char* getStationPath() { return station_path; }
    char* getOSTimeZone() { return osTimeZone; }
    bool getMatchPointFlag() {return matchPointFlag;}
};

class WeatherModel : public BaseInput {
  private:
    char* forecast;
    char* osTimeZone;

  public:
    // Constructor
    WeatherModel(const BaseInput& baseInput,
                 char* forecast, char* osTimeZone)
      : BaseInput(baseInput), forecast(forecast), osTimeZone(osTimeZone) {}

    // Getter methods
    char* getForecast() { return forecast; }
    char* getOSTimeZone() { return osTimeZone; }
};

class modeldata {
  public:
    modeldata() {}
};

#endif // MODELDATA_H
