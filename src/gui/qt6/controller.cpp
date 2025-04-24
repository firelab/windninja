#include "controller.h"
#include <string>
#include <vector>

using  namespace std;

Controller::Controller(MainWindow* view, QObject* parent)
    : QObject(parent), view(view)
{
    connect(view, &MainWindow::solveRequest, this, &Controller::onSolveRequest);
    connect(view, &MainWindow::timeZoneDataRequest, this, &Controller::onTimeZoneDataRequest);
    connect(view, &MainWindow::timeZoneDetailsRequest, this, &Controller::onTimeZoneDetailsRequest);
    connect(view, &MainWindow::getDEMrequest, this, &Controller::onGetDEMrequest);
}

// Listens for solve request; facilitates model creation and provider passing
void Controller::onSolveRequest() {
  // Alias app state, used to determine which type of solution to run
  AppState& state = AppState::instance();

  // Determine which run to perform
  if (state.domainAverageWindOk) {
    DomainAverageWind domainAvgWind = setDomainAverageWind();
    provider.domain_average_exec(domainAvgWind);
  }else if(state.pointInitializationOk){
    PointInitialization pointInit = setPointInitialization();
    provider.point_exec(pointInit);
  }else if(state.weatherModelOk){
    WeatherModel weatherModel = setWeatherModel();
    provider.wxmodel_exec(weatherModel);
  }


  vector<string> outputFileList = provider.getOutputFileNames(
    view->getUi()->elevFilePath->text(),
    view->getUi()->windTableData,
    view->getUi()->meshResValue->text(),
    provider.parseDomainAvgTable(view->getUi()->windTableData).size(),
    view->getUi()->outputDirectory->toPlainText());

  view->loadMapKMZ(outputFileList);
}

// Get time zone list from provider
void Controller::onTimeZoneDataRequest() {
  // Call provider to get 2D vector with timezone data
  bool showAllZones = view->getUi()->showAllTimeZones->isChecked();
  QVector<QVector<QString>> timeZoneData = provider.getTimeZoneData(showAllZones);

  // Clear timezone list
  view->getUi()->timeZoneSelector->clear();

  // Populate timezone list
  for (const QVector<QString>& zone : timeZoneData) {
    if (!zone.isEmpty()) {
      view->getUi()->timeZoneSelector->addItem(zone[0]);
    }
  }

  // Default to America/Denver
  view->getUi()->timeZoneSelector->setCurrentText("America/Denver");
}

// Get time zone details from provider
void Controller::onTimeZoneDetailsRequest() {
  QString currentTimeZone = view->getUi()->timeZoneSelector->currentText();
  QString timeZoneDetails = provider.getTimeZoneDetails(currentTimeZone);

  // Set value in ui
  view->getUi()->timeZoneDetails->setText(timeZoneDetails);
}

void Controller::onGetDEMrequest(std::array<double, 4> boundsBox, QString outputFile) {

  // Get correct fetch type
  // TODO: set correct string for landscape files in else condition
  int fetchIndex = view->getUi()->fetchType->currentIndex();
  string fetchType;
  if (fetchIndex == 0) {
    fetchType = "srtm";
  } else if (fetchIndex	== 1) {
    fetchType = "gmted";
  } else {
    fetchType = "land";
  }

  double resolution = view->getUi()->meshResValue->value();

  provider.fetchDEMBoundingBox(outputFile.toStdString(), fetchType, resolution, boundsBox.data());
  view->getUi()->elevFilePath->setText(outputFile);
}

/*
 * Helper functions that construct the API input models
 */

BaseInput Controller::setBaseInput() {
  QString demPath = view->getUi()->elevFilePath->text();
  double outputResolution = view->getUi()->meshResValue->value();
  QString initMethod;
  if (view->getUi()->useDomainAvgWind->isChecked()) {
    initMethod = "domain_average";
  } else if (view->getUi()->usePointInit->isChecked()) {
    initMethod = "point";
  } else {
    initMethod = "wxmodel";
  }
  QString meshType = view->getUi()->meshResType->currentText().toLower();
  QString vegetation = view->getUi()->vegetationType->currentText().toLower();
  int nLayers = 20;
  int diurnalFlag = view->getUi()->useDiurnalWind->isChecked() ? 1 : 0;

  double height = view->getUi()->windHeightValue->value();
  QString heightUnits;
  if (view->getUi()->windHeightFeet) {
    heightUnits = "ft";
  } else {
    heightUnits = "m";
  }

  bool useMomentum = view->getUi()->useCOMM->isChecked() ? 1 : 0;
  int numNinjas = 1;
  // Count the number of ninjas, depending on the wind method being used
  QVector<QVector<QString>> domainAvgTable = provider.parseDomainAvgTable(view->getUi()->windTableData);
  if (domainAvgTable.size() > 0) {
    numNinjas = domainAvgTable.size();
  }
  QString outputPath = view->getUi()->outputDirectory->toPlainText();

  return BaseInput (
    demPath.toStdString(),
    outputResolution,
    initMethod.toStdString(),
    meshType.toStdString(),
    vegetation.toStdString(),
    nLayers,
    diurnalFlag,
    height,
    heightUnits.toStdString(),
    useMomentum,
    numNinjas,
    outputPath.toStdString()
  );
}

DomainAverageWind Controller::setDomainAverageWind() {
  BaseInput baseInput = setBaseInput();

  // Get all wind data
  QVector<QVector<QString>> windData = provider.parseDomainAvgTable(view->getUi()->windTableData);

  // Get speed and direction lists
  vector<double> speedList;
  vector<double> directionList;
  for (int i = 0; i < windData.size(); i++) {
    speedList.push_back(windData[i][0].toDouble());
    directionList.push_back(windData[i][1].toDouble());
  }
  QString speedUnits = view->getUi()->speedUnits->currentText();

  return DomainAverageWind (
    baseInput,
    speedList,
    speedUnits.toStdString(),
    directionList
  );
}

PointInitialization Controller::setPointInitialization() {
  //Todo Implement Point Methods
  BaseInput baseInput = setBaseInput();

  int year = 0;
  int month = 0;
  int day = 0;
  int hour = 0;
  int minute = 0;
  char* station_path = "NULL";
  char* elevation_file = "NULL";
  char* osTimeZone= "NULL";
  bool matchPointFlag = false;

  return PointInitialization (
      baseInput,
      year,
      month,
      day,
      hour,
      minute,
      station_path,
      elevation_file,
      osTimeZone,
      matchPointFlag
      );
}


WeatherModel Controller::setWeatherModel() {
  //Todo Implement WeatherModel
  BaseInput baseInput = setBaseInput();

  char* forecast = "NULL";
  char* osTimeZone= "NULL";

  return WeatherModel (
      baseInput,
      forecast,
      osTimeZone
      );
}
