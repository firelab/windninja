#include "controller.h"

Controller::Controller(MainWindow* view, QObject* parent)
    : QObject(parent), view(view)
{
    connect(view, &MainWindow::solveRequest, this, &Controller::onSolveRequest);
    connect(view, &MainWindow::timeZoneDataRequest, this, &Controller::onTimeZoneDataRequest);
    connect(view, &MainWindow::timeZoneDetailsRequest, this, &Controller::onTimeZoneDetailsRequest);
}

// Listens for solve request; facilitates model creation and provider passing
void Controller::onSolveRequest() {
  // Alias app state, used to determine which type of solution to run
  AppState& state = AppState::instance();

  // Determine which run to perform
  if (state.domainAverageWindOk) {
    DomainAverageWind domainAvgWind = setDomainAverageWind();
    provider.domain_average_exec(domainAvgWind);
  }
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
