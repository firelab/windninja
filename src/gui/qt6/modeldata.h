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
    virtual ~BaseInput() {}

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
    DomainAverageWind(char* demFile, double outputResolution, string initializationMethod, string meshChoice, string vegetation,
                      int nLayers, int diurnalFlag, double height, string heightUnits, bool momentumFlag, int numNinjas, string outputPath,
                      vector<double> speedList, string speedUnits, vector<double> directionList)
      : BaseInput(demFile, outputResolution, initializationMethod, meshChoice, vegetation, nLayers, diurnalFlag, height, heightUnits, momentumFlag, numNinjas, outputPath),
          speedList(speedList), speedUnits(speedUnits), directionList(directionList) {}

    // Getter methods
    vector<double> getSpeedList() { return speedList; }
    const string& getSpeedUnits() const { return speedUnits; }
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
    bool matchPointFlag;

  public:
    // Constructor
    PointInitialization(char* demFile, double outputResolution, string initializationMethod, string meshChoice, string vegetation,
                        int nLayers, int diurnalFlag, double height, string heightUnits, bool momentumFlag, int numNinjas, string outputPath,
                        int year, int month, int day, int hour, int minute, char* station_path, char* elevation_file, char* osTimeZone, bool matchPointFlag)
      : BaseInput(demFile, outputResolution, initializationMethod, meshChoice, vegetation, nLayers, diurnalFlag, height, heightUnits, momentumFlag, numNinjas, outputPath),
        year(year), month(month), day(day), hour(hour), minute(minute), station_path(station_path), elevation_file(elevation_file), osTimeZone(osTimeZone), matchPointFlag(matchPointFlag) {}

    // Getter methods
    int getYear() { return year; }
    int getMonth() { return month; }
    int getDay() { return day; }
    int getHour() { return hour; }
    int getMinute() { return minute; }
    char* getStationPath() { return station_path; }
    char* getElevationFile() { return elevation_file; }
    char* getOSTimeZone() { return osTimeZone; }
    bool getMatchPointFlag() {return matchPointFlag;}
};

class WeatherModel : public BaseInput {
  private:
    char* forecast;
    char* osTimeZone;

  public:
    // Constructor
    WeatherModel(char* demFile, double outputResolution, string initializationMethod, string meshChoice, string vegetation,
                 int nLayers, int diurnalFlag, double height, string heightUnits, bool momentumFlag, int numNinjas, string outputPath,
                 char* forecast, char* osTimeZone)
      : BaseInput(demFile, outputResolution, initializationMethod, meshChoice, vegetation, nLayers, diurnalFlag, height, heightUnits, momentumFlag, numNinjas, outputPath),
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
