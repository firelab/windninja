#include "provider.h"
#include "modeldata.h"
#include "../../ninja/windninja.h"

#include <iostream>
#include <list>
#include <QDebug>
#include <QFile>
#include <QStringList>
#include <QTextStream>
#include <vector>

using namespace std;


NinjaArmyH* ninjaArmy = NULL;
const char * comType = "cli";
const int nCPUs = 1;
char ** papszOptions = NULL;
NinjaErr err = 0;

Provider::Provider() {

}

int Provider::domain_average_exec(DomainAverageWind& input) {
  std::vector<double> speedVector = input.getSpeedList();
  const double* speedList = speedVector.data();
  std::vector<double> directionVector = input.getDirectionList();
  const double* directionList = directionVector.data();
  const char * demFile = input.getDemFile().c_str();
  double outputResolution = input.getOutputResolution();
  const char* initializationMethod = input.getInitializationMethod().c_str();
  const char* meshChoice = input.getMeshChoice().c_str();
  const char* vegetation = input.getVegetation().c_str();
  const int nLayers = input.getNLayers();
  const int diurnalFlag = input.getDiurnalFlag();
  const double height = input.getHeight();
  const char* heightUnits = input.getHeightUnits().c_str();
  const char* speedUnits = input.getSpeedUnits().c_str();
  bool momentumFlag = input.getMomentumFlag();
  unsigned int numNinjas = input.getNumNinjas();
  const char* outputPath = input.getOutputPath().c_str();


  /*
   * Setting up the simulation
   */
  err = NinjaInit(papszOptions); //initialize global singletons and environments (GDAL_DATA, etc.)
  ninjaArmy = NinjaMakeDomainAverageArmy(numNinjas, momentumFlag, speedList, speedUnits, directionList, papszOptions);

  if(err != NINJA_SUCCESS)
  {
    printf("NinjaInit: err = %d\n", err);
  }

  /*
   * Create the army
   */
  if( NULL == ninjaArmy )
  {
    printf("NinjaCreateArmy: ninjaArmy = NULL\n");
  }

  err = NinjaInit(papszOptions); //must be called for any simulation
  if(err != NINJA_SUCCESS)
  {
    printf("NinjaInit: err = %d\n", err);
  }

  /*
   * Prepare the army
   */
  for(unsigned int i=0; i<input.getNumNinjas(); i++)
  {
    err = NinjaSetCommunication(ninjaArmy, i, comType, papszOptions);
    if(err != NINJA_SUCCESS)
    {
      printf("NinjaSetCommunication: err = %d\n", err);
    }

    err = NinjaSetOutputPath(ninjaArmy, i, outputPath, papszOptions);
    if(err != NINJA_SUCCESS)
    {
      printf("NinjaSetNumberCPUs: err = %d\n", err);
    }

    err = NinjaSetNumberCPUs(ninjaArmy, i, nCPUs, papszOptions);
    if(err != NINJA_SUCCESS)
    {
      printf("NinjaSetNumberCPUs: err = %d\n", err);
    }

    err = NinjaSetInitializationMethod(ninjaArmy, i, initializationMethod, papszOptions);
    if(err != NINJA_SUCCESS)
    {
      printf("NinjaSetInitializationMethod: err = %d\n", err);
    }

    err = NinjaSetDem(ninjaArmy, i, demFile, papszOptions);
    if(err != NINJA_SUCCESS)
    {
      printf("NinjaSetDem: err = %d\n", err);
    }

    err = NinjaSetPosition(ninjaArmy, i, papszOptions);
    if(err != NINJA_SUCCESS)
    {
      printf("NinjaSetPosition: err = %d\n", err);
    }


    //heighUnits set static

    err = NinjaSetInputWindHeight(ninjaArmy, i, height, heightUnits, papszOptions);
    if(err != NINJA_SUCCESS)
    {
      printf("NinjaSetInputWindHeight: err = %d\n", err);
    }

    err = NinjaSetOutputWindHeight(ninjaArmy, i, height, heightUnits, papszOptions);
    if(err != NINJA_SUCCESS)
    {
      printf("NinjaSetOutputWindHeight: err = %d\n", err);
    }

    err = NinjaSetOutputSpeedUnits(ninjaArmy, i, speedUnits, papszOptions);
    if(err != NINJA_SUCCESS)
    {
      printf("NinjaSetOutputSpeedUnits: err = %d\n", err);
    }

    err = NinjaSetDiurnalWinds(ninjaArmy, i, diurnalFlag, papszOptions);
    if(err != NINJA_SUCCESS)
    {
      printf("NinjaSetDiurnalWinds: err = %d\n", err);
    }

    err = NinjaSetUniVegetation(ninjaArmy, i, vegetation, papszOptions);
    if(err != NINJA_SUCCESS)
    {
      printf("NinjaSetUniVegetation: err = %d\n", err);
    }

    err = NinjaSetMeshResolutionChoice(ninjaArmy, i, meshChoice, papszOptions);
    if(err != NINJA_SUCCESS)
    {
      printf("NinjaSetMeshResolutionChoice: err = %d\n", err);
    }

    err = NinjaSetNumVertLayers(ninjaArmy, i, nLayers, papszOptions);
    if(err != NINJA_SUCCESS)
    {
      printf("NinjaSetNumVertLayers: err = %d\n", err);
    }


    err = NinjaSetGoogOutFlag(ninjaArmy, i, true, papszOptions);
    if(err != NINJA_SUCCESS)
    {
      printf("NinjaSetGoogOutFlag err = %d\n", err);
    }
  }

  /*
   * Start the simulations
   */
  err = NinjaStartRuns(ninjaArmy, nCPUs, papszOptions);
  if(err != 1) //NinjaStartRuns returns 1 on success
  {
    printf("NinjaStartRuns: err = %d\n", err);
  }

  /*
   * Get the outputs
   */
  const double* outputSpeedGrid = NULL;
  const double* outputDirectionGrid = NULL;
  const char* outputGridProjection = NULL;
  const int nIndex = 0;
  const char* units = "m";

  //Google Maps Output
  outputSpeedGrid = NinjaGetOutputSpeedGrid(ninjaArmy, nIndex, papszOptions);
  if( NULL == outputSpeedGrid )
  {
    printf("Error in NinjaGetOutputSpeedGrid");
  }

  outputDirectionGrid = NinjaGetOutputDirectionGrid(ninjaArmy, nIndex, papszOptions);
  if( NULL == outputDirectionGrid )
  {
    printf("Error in NinjaGetOutputDirectionGrid");
  }

  outputGridProjection = NinjaGetOutputGridProjection(ninjaArmy, nIndex, papszOptions);
  if( NULL == outputGridProjection )
  {
    printf("Error in NinjaGetOutputGridProjection");
  }

  /*
   * Clean up
   */
  err = NinjaDestroyArmy(ninjaArmy, papszOptions);
  if(err != NINJA_SUCCESS)
  {
    printf("NinjaDestroyRuns: err = %d\n", err);
  }

  return NINJA_SUCCESS;
}

int Provider::point_exec(PointInitialization& input) {
  /*
   * Setting up the simulation
   */

  //initialize global singletons and environments (GDAL_DATA, etc.)
  if(err != NINJA_SUCCESS)
  {
    printf("NinjaInit: err = %d\n", err);
  }

  /*
   * Create the army
   */
  vector<int> yearVector = input.getYear();
   int* year = yearVector.data();
  vector<int> monthVector = input.getMonth();
   int* month = monthVector.data();
  vector<int> dayVector = input.getDay();
   int* day = dayVector.data();
  vector<int> hourVector = input.getHour();
   int* hour = hourVector.data();
  vector<int> minuteVector = input.getMinute();
   int* minute = minuteVector.data();
   char* station_path = input.getStationPath();
   char* osTimeZone = input.getOSTimeZone();
  bool matchPointFlag = input.getMatchPointFlag();

   char * demFile = const_cast<char*>(input.getDemFile().c_str());
  double outputResolution = input.getOutputResolution();
  const char* initializationMethod = input.getInitializationMethod().c_str();
  const char* meshChoice = input.getMeshChoice().c_str();
  const char* vegetation = input.getVegetation().c_str();
  const int nLayers = input.getNLayers();
  const int diurnalFlag = input.getDiurnalFlag();
  const double height = input.getHeight();
  const char* heightUnits = input.getHeightUnits().c_str();
  bool momentumFlag = input.getMomentumFlag();
  unsigned int numNinjas = input.getNumNinjas();
  const char * outputPath =  input.getOutputPath().c_str();


  ninjaArmy = NinjaMakePointArmy(year, month, day, hour, minute, numNinjas, osTimeZone, station_path, demFile, matchPointFlag, momentumFlag, papszOptions);

  if( NULL == ninjaArmy )
  {
    printf("NinjaCreateArmy: ninjaArmy = NULL\n");
  }

  err = NinjaInit(papszOptions); //must be called for any simulation
  if(err != NINJA_SUCCESS)
  {
    printf("NinjaInit: err = %d\n", err);
  }

  /*
   * Prepare the army
   */
  for(unsigned int i=0; i<numNinjas; i++)
  {

    err = NinjaSetOutputPath(ninjaArmy, i, outputPath, papszOptions);
    if(err != NINJA_SUCCESS)
    {
      printf("NinjaSetOutputPath: err = %d\n", err);
    }

    err = NinjaSetGoogOutFlag(ninjaArmy, i, true, papszOptions);
    if(err != NINJA_SUCCESS)
    {
      printf("NinjaSetGoogleOutPut: err = %d\n", err);
    }

    err = NinjaSetCommunication(ninjaArmy, i, comType, papszOptions);
    if(err != NINJA_SUCCESS)
    {
      printf("NinjaSetCommunication: err = %d\n", err);
    }

    err = NinjaSetCommunication(ninjaArmy, i, comType, papszOptions);
    if(err != NINJA_SUCCESS)
    {
      printf("NinjaSetCommunication: err = %d\n", err);
    }

    err = NinjaSetNumberCPUs(ninjaArmy, i, nCPUs, papszOptions);
    if(err != NINJA_SUCCESS)
    {
      printf("NinjaSetNumberCPUs: err = %d\n", err);
    }

    err = NinjaSetInitializationMethod(ninjaArmy, i, initializationMethod, papszOptions);
    if(err != NINJA_SUCCESS)
    {
      printf("NinjaSetInitializationMethod: err = %d\n", err);
    }

    err = NinjaSetDem(ninjaArmy, i, demFile, papszOptions);
    if(err != NINJA_SUCCESS)
    {
      printf("NinjaSetDem: err = %d\n", err);
    }

    err = NinjaSetPosition(ninjaArmy, i, papszOptions);
    if(err != NINJA_SUCCESS)
    {
      printf("NinjaSetPosition: err = %d\n", err);
    }

    err = NinjaSetInputWindHeight(ninjaArmy, i, height, heightUnits, papszOptions);
    if(err != NINJA_SUCCESS)
    {
      printf("NinjaSetInputWindHeight: err = %d\n", err);
    }

    err = NinjaSetOutputWindHeight(ninjaArmy, i, height, heightUnits, papszOptions);
    if(err != NINJA_SUCCESS)
    {
      printf("NinjaSetOutputWindHeight: err = %d\n", err);
    }

           //err = NinjaSetOutputSpeedUnits(ninjaArmy, i, speedUnits, papszOptions);
           //if(err != NINJA_SUCCESS)
           //{
           //  printf("NinjaSetOutputSpeedUnits: err = %d\n", err);
           //}

    err = NinjaSetDiurnalWinds(ninjaArmy, i, diurnalFlag, papszOptions);
    if(err != NINJA_SUCCESS)
    {
      printf("NinjaSetDiurnalWinds: err = %d\n", err);
    }

    err = NinjaSetUniVegetation(ninjaArmy, i, vegetation, papszOptions);
    if(err != NINJA_SUCCESS)
    {
      printf("NinjaSetUniVegetation: err = %d\n", err);
    }

    err = NinjaSetMeshResolutionChoice(ninjaArmy, i, meshChoice, papszOptions);
    if(err != NINJA_SUCCESS)
    {
      printf("NinjaSetMeshResolutionChoice: err = %d\n", err);
    }

    err = NinjaSetNumVertLayers(ninjaArmy, i, nLayers, papszOptions);
    if(err != NINJA_SUCCESS)
    {
      printf("NinjaSetNumVertLayers: err = %d\n", err);
    }
  }

  /*
   * Start the simulations
   */
  err = NinjaStartRuns(ninjaArmy, nCPUs, papszOptions);
  if(err != 1) //NinjaStartRuns returns 1 on success
  {
    printf("NinjaStartRuns: err = %d\n", err);
  }

  return NINJA_SUCCESS;
}

int Provider::wxmodel_exec(WeatherModel& input) {
  /*
   * Setting up the simulation
   */

  err = NinjaInit(papszOptions); //initialize global singletons and environments (GDAL_DATA, etc.)
  if(err != NINJA_SUCCESS)
  {
    printf("NinjaInit: err = %d\n", err);
  }

  /*
   * Set up Weather Model Initialization run
   */

  const char * demFile = input.getDemFile().c_str();
  double outputResolution = input.getOutputResolution();
  const char* initializationMethod = input.getInitializationMethod().c_str();
  const char* meshChoice = input.getMeshChoice().c_str();
  const char* vegetation = input.getVegetation().c_str();
  const int nLayers = input.getNLayers();
  const int diurnalFlag = input.getDiurnalFlag();
  const double height = input.getHeight();
  const char* heightUnits = input.getHeightUnits().c_str();
  bool momentumFlag = input.getMomentumFlag();
  unsigned int numNinjas = input.getNumNinjas();
  const char* outputPath = input.getOutputPath().c_str();


  /* inputs that can vary among ninjas in an army */
  //const char * speedUnits = input.get;

  /*
   * Create the army
   */
  const char * forecast = input.getForecast();
  const char * osTimeZone = input.getOSTimeZone();

  ninjaArmy = NinjaMakeWeatherModelArmy(forecast, osTimeZone, momentumFlag, papszOptions);
  if( NULL == ninjaArmy )
  {
    printf("NinjaCreateArmy: ninjaArmy = NULL\n");
  }

  err = NinjaInit(papszOptions); //must be called for any simulation
  if(err != NINJA_SUCCESS)
  {
    printf("NinjaInit: err = %d\n", err);
  }

  /*
   * Prepare the army
   */
  for(unsigned int i=0; i<numNinjas; i++)
  {
    err = NinjaSetOutputPath(ninjaArmy, i, outputPath, papszOptions);
    if(err != NINJA_SUCCESS)
    {
      printf("NinjaSetOutputPath: err = %d\n", err);
    }

    err = NinjaSetGoogOutFlag(ninjaArmy, i, true, papszOptions);
    if(err != NINJA_SUCCESS)
    {
      printf("NinjaSetGoogleOutPut: err = %d\n", err);
    }


    err = NinjaSetCommunication(ninjaArmy, i, comType, papszOptions);
    if(err != NINJA_SUCCESS)
    {
      printf("NinjaSetCommunication: err = %d\n", err);
    }

    err = NinjaSetNumberCPUs(ninjaArmy, i, nCPUs, papszOptions);
    if(err != NINJA_SUCCESS)
    {
      printf("NinjaSetNumberCPUs: err = %d\n", err);
    }

    err = NinjaSetInitializationMethod(ninjaArmy, i, initializationMethod, papszOptions);
    if(err != NINJA_SUCCESS)
    {
      printf("NinjaSetInitializationMethod: err = %d\n", err);
    }

    err = NinjaSetDem(ninjaArmy, i, demFile, papszOptions);
    if(err != NINJA_SUCCESS)
    {
      printf("NinjaSetDem: err = %d\n", err);
    }

    err = NinjaSetPosition(ninjaArmy, i, papszOptions);
    if(err != NINJA_SUCCESS)
    {
      printf("NinjaSetPosition: err = %d\n", err);
    }

    err = NinjaSetInputWindHeight(ninjaArmy, i, height, heightUnits, papszOptions);
    if(err != NINJA_SUCCESS)
    {
      printf("NinjaSetInputWindHeight: err = %d\n", err);
    }

    err = NinjaSetOutputWindHeight(ninjaArmy, i, height, heightUnits, papszOptions);
    if(err != NINJA_SUCCESS)
    {
      printf("NinjaSetOutputWindHeight: err = %d\n", err);
    }

           //err = NinjaSetOutputSpeedUnits(ninjaArmy, i, speedUnits, papszOptions);
           //if(err != NINJA_SUCCESS)
           //{
           //  printf("NinjaSetOutputSpeedUnits: err = %d\n", err);
           //}

    err = NinjaSetDiurnalWinds(ninjaArmy, i, diurnalFlag, papszOptions);
    if(err != NINJA_SUCCESS)
    {
      printf("NinjaSetDiurnalWinds: err = %d\n", err);
    }

    err = NinjaSetUniVegetation(ninjaArmy, i, vegetation, papszOptions);
    if(err != NINJA_SUCCESS)
    {
      printf("NinjaSetUniVegetation: err = %d\n", err);
    }

    err = NinjaSetMeshResolutionChoice(ninjaArmy, i, meshChoice, papszOptions);
    if(err != NINJA_SUCCESS)
    {
      printf("NinjaSetMeshResolutionChoice: err = %d\n", err);
    }

    err = NinjaSetNumVertLayers(ninjaArmy, i, nLayers, papszOptions);
    if(err != NINJA_SUCCESS)
    {
      printf("NinjaSetNumVertLayers: err = %d\n", err);
    }
  }

  /*
   * Start the simulations
   */
  err = NinjaStartRuns(ninjaArmy, nCPUs, papszOptions);
  if(err != 1) //NinjaStartRuns returns 1 on success
  {
    printf("NinjaStartRuns: err = %d\n", err);
  }

  /*
   * Get the outputs
   */
  const double* outputSpeedGrid = NULL;
  const double* outputDirectionGrid = NULL;
  const char* outputGridProjection = NULL;
  const int nIndex = 0;
  const char* units = "m";
  outputSpeedGrid = NinjaGetOutputSpeedGrid(ninjaArmy, nIndex, papszOptions);
  if( NULL == outputSpeedGrid )
  {
    printf("Error in NinjaGetOutputSpeedGrid");
  }

  outputDirectionGrid = NinjaGetOutputDirectionGrid(ninjaArmy, nIndex, papszOptions);
  if( NULL == outputDirectionGrid )
  {
    printf("Error in NinjaGetOutputDirectionGrid");
  }

  outputGridProjection = NinjaGetOutputGridProjection(ninjaArmy, nIndex, papszOptions);
  if( NULL == outputGridProjection )
  {
    printf("Error in NinjaGetOutputGridProjection");
  }

  /*
   * Clean up
   */
  err = NinjaDestroyArmy(ninjaArmy, papszOptions);
  if(err != NINJA_SUCCESS)
  {
    printf("NinjaDestroyRuns: err = %d\n", err);
  }

  return NINJA_SUCCESS;
}

// Time zone data provider
QVector<QVector<QString>> Provider::getTimeZoneData(bool showAllZones) {
  QVector<QVector<QString>> fullData;
  QVector<QVector<QString>> americaData;

  QFile file(":/date_time_zonespec.csv");

  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    qDebug() << "Failed to open CSV file.";
    return fullData;
  }

  QTextStream in(&file);
  bool firstLine = true;

  while (!in.atEnd()) {
    QString line = in.readLine();

    if (firstLine) {
      firstLine = false;
      continue;
    }

    QStringList tokens = line.split(",", Qt::KeepEmptyParts);
    QVector<QString> row;
    for (const QString& token : tokens)
      row.append(token.trimmed().remove('"'));

    if (!row.isEmpty())
      fullData.append(row);

    if (!row.isEmpty()) {
      QStringList parts = row[0].split("/", Qt::KeepEmptyParts);
      if (!parts.isEmpty() && parts[0] == "America" || row[0] == "Pacific/Honolulu") {
        americaData.append(row);
      }
    }
  }

  file.close();

  if (showAllZones) {
    return fullData;
  } else {
    return americaData;
  }

}

// Provider for getting time zone details
QString Provider::getTimeZoneDetails(const QString& currentTimeZone) {
  QVector<QString> matchedRow;
  QFile file(":/date_time_zonespec.csv");

  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    qWarning() << "Failed to open date_time_zonespec.csv";
    return "No data found";
  }

  QTextStream in(&file);
  bool firstLine = true;

  while (!in.atEnd()) {
    QString line = in.readLine();

    if (firstLine) {
      firstLine = false;
      continue;  // skip header
    }

    QStringList tokens = line.split(",", Qt::KeepEmptyParts);
    QVector<QString> row;

    for (const QString& token : tokens)
      row.append(token.trimmed().remove("\""));

    QString fullZone = row.mid(0, 1).join("/");

    if (fullZone == currentTimeZone) {
      matchedRow = row;
      break;
    }
  }

  file.close();

  if (matchedRow.isEmpty()) {
    return "No matching time zone found.";
  }

  QString standardName = matchedRow.value(2);
  QString daylightName = matchedRow.value(4);
  QString stdOffsetStr = matchedRow.value(5);  // Already in HH:MM:SS
  QString dstAdjustStr = matchedRow.value(6);  // Already in HH:MM:SS

  // Function to convert signed HH:MM:SS to total seconds
  auto timeToSeconds = [](const QString& t) -> int {
    QString s = t.trimmed();
    bool negative = s.startsWith('-');
    s = s.remove(QChar('+')).remove(QChar('-'));

    QStringList parts = s.split(':');
    if (parts.size() != 3) return 0;

    int h = parts[0].toInt();
    int m = parts[1].toInt();
    int sec = parts[2].toInt();

    int total = h * 3600 + m * 60 + sec;
    return negative ? -total : total;
  };

  // Convert total seconds back to HH:MM:SS with sign
  auto secondsToTime = [](int totalSec) -> QString {
    QChar sign = totalSec < 0 ? '-' : '+';
    totalSec = std::abs(totalSec);

    int h = totalSec / 3600;
    int m = (totalSec % 3600) / 60;
    int s = totalSec % 60;

    return QString("%1%2:%3:%4")
      .arg(sign)
      .arg(h, 2, 10, QChar('0'))
      .arg(m, 2, 10, QChar('0'))
      .arg(s, 2, 10, QChar('0'));
  };

  int stdSecs = timeToSeconds(stdOffsetStr);
  int dstSecs = timeToSeconds(dstAdjustStr);
  QString combinedOffsetStr = secondsToTime(stdSecs + dstSecs);

  return QString("Standard Name:\t\t%1\n"
                 "Daylight Name:\t\t%2\n"
                 "Standard Offset from UTC:\t%3\n"
                 "Daylight Offset from UTC:\t%4")
      .arg(standardName)
      .arg(daylightName)
      .arg(stdOffsetStr)
      .arg(combinedOffsetStr);
}

// Provider for parsing the domain average wind table
QVector<QVector<QString>> Provider::parseDomainAvgTable(QTableWidget* table) {
  QVector<QVector<QString>> result;

  int rowCount = table->rowCount();
  int colCount = table->columnCount();

  for (int row = 0; row < rowCount; ++row) {
    bool rowComplete = true;
    QVector<QString> rowData;

    for (int col = 0; col < colCount; ++col) {
      if (table->isColumnHidden(col)) {
        continue;  // skip this column entirely
      }

      QTableWidgetItem* item = table->item(row, col);

      if (!item || item->text().trimmed().isEmpty()) {
        rowComplete = false;
        break;
      }

      rowData.append(item->text().trimmed());
    }

    if (rowComplete) {
      result.append(rowData);
    }
  }
  return result;
}

int Provider::fetchDEMBoundingBox(const string demFileOutPut, const string fetch_type, int resolution, double* boundsBox)
{
  /*
   * Setting up NinjaArmy
   */

  NinjaArmyH* ninjaArmy = NULL;
  char ** papszOptions = NULL;
  NinjaErr err = 0;
  err = NinjaInit(papszOptions); // must be called for fetching and simulations
  if(err != NINJA_SUCCESS)
  {
    printf("NinjaInit: err = %d\n", err);
  }
  /*
   * Testing fetching from a DEM bounding box
   */
  // Bounding box (north, east, south, west)
  err = NinjaFetchDEMBBox(ninjaArmy, boundsBox, demFileOutPut.c_str(), resolution, strdup(fetch_type.c_str()), papszOptions);
  if (err != NINJA_SUCCESS){
    printf("NinjaFetchDEMBBox: err = %d\n", err);
  }

  return NINJA_SUCCESS;
}

int Provider::fetchForecast(const string wx_model_type, int numNinjas, const string demFile){
  /*
   * const char*wx_model_type = "NOMADS-HRRR-CONUS-3-KM";
   * int numNinjas = 2;
   */

  /*
   * Setting up NinjaArmy
   */
  NinjaArmyH* ninjaArmy = NULL;
  char ** papszOptions = NULL;
  NinjaErr err = 0;
  err = NinjaInit(papszOptions); // must be called for fetching and simulations
  if(err != NINJA_SUCCESS)
  {
    printf("NinjaInit: err = %d\n", err);
  }
  /*
   * Testing fetching for a Forecast file (wx_model_type are the names listed in the weather station download in WindNinja)
   */
  const char* forecastFilename = NinjaFetchForecast(ninjaArmy, wx_model_type.c_str(), numNinjas, demFile.c_str(), papszOptions);
  if (strcmp(forecastFilename, "exception") == 0){
    printf("NinjaFetchForecast: err = %s\n", forecastFilename);
  }
  else{
    printf("NinjaFetchForecast: forecastFilename = %s\n", forecastFilename);
  }

  return NINJA_SUCCESS;
}

int Provider::fetchStationData(int year, int month, int day, int hour, int minute, int numNinjas, const string output_path, int bufferZone, const string bufferUnits, const string demFile, const string osTimeZone, int fetchLatestFlag){
  /*
   * int year[1] = {2023};
   * int month[1] = {10};
   * int day[1] = {10};
   * int hour[1] = {12};
   * int minute[1] = {60};
   * const char* output_path = "";
   * const char* elevation_file = "data/missoula_valley.tif";
   * const char* osTimeZone = "UTC";
   * bool fetchLatestFlag = 1;
   * /

 /*
  * Setting up NinjaArmy
  */
  NinjaArmyH* ninjaArmy = NULL;
  char ** papszOptions = NULL;
  NinjaErr err = 0;
  err = NinjaInit(papszOptions); // must be called for fetching and simulations
  if(err != NINJA_SUCCESS)
  {
    printf("NinjaInit: err = %d\n", err);
  }

  /*
   * Testing fetching station data from a geotiff file.
   */

  err = NinjaFetchStation(&year, &month, &day, &hour, &minute, numNinjas, demFile.c_str(), bufferZone, bufferUnits.c_str(), osTimeZone.c_str(), fetchLatestFlag, output_path.c_str(), papszOptions);
  if (err != NINJA_SUCCESS) {
    printf("NinjaFetchStation: err = %d\n", err);
  } else {
    printf("NinjaFetchStation: success\n");
  }

  return NINJA_SUCCESS;
}

int Provider::fetchFromDEMPoint (double adfPoints[2], double adfBuff[2], const string units, double dfCellSize, const string pszDstFile, const string fetchtype){
  /*
   * double adfPoint[] = {104.0, 40.07}; // Point coordinates (longitude, latitude)
   * double adfBuff[] = {30, 30}; // Buffer to store the elevation value
   * const char* units = "mi";
   * double dfCellSize = 30.0; // Cell size in meters
   * char* pszDstFile = "data/dem_point_output.tif";
   * char* fetchType = "gmted";
   */

  /*
   * Setting up NinjaArmy
   */
  NinjaArmyH* ninjaArmy = NULL;
  char ** papszOptions = NULL;
  NinjaErr err = 0;
  err = NinjaInit(papszOptions); // must be called for fetching and simulations
  if(err != NINJA_SUCCESS)
  {
    printf("NinjaInit: err = %d\n", err);
  }


  /*
   * Testing fetching from a DEM point
   */
  err = NinjaFetchDEMPoint(ninjaArmy, adfPoints, adfBuff, units.c_str(), dfCellSize, strdup(pszDstFile.c_str()), strdup(fetchtype.c_str()), papszOptions);
  if (err != NINJA_SUCCESS) {
    printf("NinjaFetchDemPoint: err = %d\n", err);
  } else {
    printf("NinjaFetchDemPoint: elevation = %f\n", adfBuff[0]);
  }

  return NINJA_SUCCESS;
}

vector<string> Provider::getOutputFileNames(QString demFile, QTableWidget *table, QString meshValue, int numFiles, QString outputPath) {
  vector<string> outputFiles;
  QDir outDir(outputPath);
  QString demName = QFileInfo(demFile).completeBaseName();
  int meshInt = static_cast<int>(std::round(meshValue.toDouble()));
  QString meshSize = QString::number(meshInt) + "m";

  for (int i = 0; i < numFiles; i++) {
    QString direction = table->item(i, 1)->text().trimmed();
    QString speed = table->item(i, 0)->text().trimmed();
    QString filePath = outDir.filePath(QString("%1_%2_%3_%4.kmz")
                           .arg(demName)
                           .arg(direction)
                           .arg(speed)
                           .arg(meshSize));
    outputFiles.push_back(filePath.toStdString());
  }

  return outputFiles;
}
