#include "modeldata.h"

class BaseInput{
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
    char* getDemFile(){
      return demFile;
    }

    double getOutputResolution(){
      return outputResolution;
    }

    string getInitializationMethod(){
      return intializationMethod;
    }

    string getMeshChoice(){
      return meshChoice;
    }

    string getVegetation(){
      return vegetation;
    }

    int getNLayers(){
      return nLayers;
    }

    int getdiurnalFlag(){
      return diurnalFlag;
    }

    double getHeight(){
      return height;
    }

    bool getMomentumFlag(){
      return momentumFlag;
    }

    int getNumNinjas(){
      return numNinjas;
    }

};

class DomainAverageWind : public BaseInput{
  private:
    double[] speedList;
    char* speedUnits;
    double[] directionList;

  public:
    DomainAverageWind(char* demFile, double outputResolution, string initializationMethod, string meshChoice, string vegetation, int nLayers, int diurnalFlag, double height, bool momentumFlag, int numNinjas, double[] speedList, char* speedUnits, double[] directionList){
      //potential for data validation thinking controller handled.
      this->demFile = demFile;
      this->outputResolution = outputResolution;
      this->initializationMethod = initializationMethod;
      this->meshChoice = meshChoice;
      this->vegetation = vegetation;
      this->nLayers = nLayers;
      this->diurnalFlag = diurnalFlag;
      this->height = height;
      this->momentumFlag = momentumFlag;
      this->numNinjas = numNinjas;

      this->speedList = speedList;
      this->speedUnits = speedUnits;
      this->directionList = directionList;
    }

    double[] getSpeedList(){
      return speedList;
    }

    char* getSpeedUnits(){
      return speedUnits;
    }

    double[] getDirectionList(){
      return directionList;
    }
};

class PointInitialization : public BaseInput{
  private:
    int year;
    int month;
    int day;
    int hour;
    int minute;
    char* station_path;
    char* elevation_file;
    char* osTimeZone;
    bool momentumFlag;

  public:
    PointInitialization(char* demFile, double outputResolution, string initializationMethod, string meshChoice, string vegetation, int nLayers, int diurnalFlag, double height, bool momentumFlag, int numNinjas){
      //potential for data validation thinking controller handled.
      this->demFile = demFile;
      this->outputResolution = outputResolution;
      this->initializationMethod = initializationMethod;
      this->meshChoice = meshChoice;
      this->vegetation = vegetation;
      this->nLayers = nLayers;
      this->diurnalFlag = diurnalFlag;
      this->height = height;
      this->momentumFlag = momentumFlag;
      this->numNinjas = numNinjas;

      this->year = year;
      this->month = month;
      this->day = day;
      this->hour = hour;
      this->minute = minute;
      this->station_path = station_path;
      this->elevation_file = elevation_file;
      this->osTimeZone = osTimeZone;
      this->momentumFlag = momentumFlag;
    }

    int getYear(){
      return year;
    }

    int getMonth(){
      return month;
    }

    int getDay(){
      return day;
    }

    int getHour(){
      return hour;
    }

    int getMinute(){
      return minute;
    }

    char* getStationPath(){
      return station_path;
    }

    char* getElevationFile(){
      return elevation_file;
    }

    char* getOSTimeZone(){
      return osTimeZone;
    }

    bool getMomentumFlag(){
      return momentumFlag;
    }
};

class WeatherModel : public BaseInput{
  private:
    char* forecast;
    char* osTimeZone;

  public:
    WeatherModel(char* demFile, double outputResolution, string initializationMethod, string meshChoice, string vegetation, int nLayers, int diurnalFlag, double height, bool momentumFlag, int numNinjas){
      //potential for data validation thinking controller handled.
      this->demFile = demFile;
      this->outputResolution = outputResolution;
      this->initializationMethod = initializationMethod;
      this->meshChoice = meshChoice;
      this->vegetation = vegetation;
      this->nLayers = nLayers;
      this->diurnalFlag = diurnalFlag;
      this->height = height;
      this->momentumFlag = momentumFlag;
      this->numNinjas = numNinjas;
    }

    char* getForecast(){
      return forecast;
    }

    char* getOSTimeZone(){
      return osTimeZone;
    }

};

modeldata::modeldata() {}
