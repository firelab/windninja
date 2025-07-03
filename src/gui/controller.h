#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <QObject>
#include <vector>
#include "appstate.h"
#include "mainwindow.h"
#include "modeldata.h"
#include "provider.h"

class Controller : public QObject {
    Q_OBJECT
public:
    Controller(MainWindow* view, QObject* parent = nullptr);

private:
  MainWindow* view;
  Provider provider;
  BaseInput setBaseInput();
  DomainAverageWind setDomainAverageWind();
  PointInitialization setPointInitialization();
  WeatherModel setWeatherModel();

  void onSolveRequest();
  void onTimeZoneDataRequest();
  void ontimeZoneDetailsRequest();
  void onGetDEMrequest(std::array<double, 4> boundsBox, QString outputFile);
};

#endif // CONTROLLER_H
